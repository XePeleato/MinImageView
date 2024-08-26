//
// Created by Eduardo on 31/07/2021.
//

#include <iostream>
#include <bitset>
#include "MinixFS.h"

namespace minixfs {

int pow(int base, int exp)
{
    int result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}

int min(int a, int b) {
	if (a < b) {
		return a;
	}
    return b;
}

    SetupState MinixFS::setup(const std::string &filename) {
        mStream = std::make_unique<Stream>();
        mStream->m_file.open(filename.c_str(),std::ifstream::binary | std::ifstream::in | std::ofstream::out);

        if (!mStream->m_file.is_open()) {
            return SetupState::ErrorFile;
        }
        mSuperBlock = new SuperBlock(*mStream);

        if (!mSuperBlock->validate()) {
            return SetupState::ErrorFormat;
        }

        {
            std::lock_guard<std::mutex> lock(mStream->m_fileMutex);
            mInodeBitmap = std::vector<unsigned short>(0x800);
            mStream->m_file.seekg(0x2000, std::ifstream::beg);
            mStream->m_file.read(reinterpret_cast<char*>(mInodeBitmap.data()), 0x1000);

            mBlockBitmap = std::vector<unsigned short>(0x800);
            mStream->m_file.seekg(0x3000, std::ifstream::beg);
            mStream->m_file.read(reinterpret_cast<char*>(mBlockBitmap.data()), 0x1000);
        }

        mInodes = new Inodes(*mStream, mSuperBlock->s_ninodes);

        loadEntries();

        return SetupState::Success;
    }

    Inode *MinixFS::find(const std::wstring& filename) {
        std::wcout << L"Finding " + filename + L'\n';
        auto node = mEntryMap.find(filename);
        if (node != mEntryMap.end()) {
            std::wcout << L"Found " + filename + L'\n';
            return &mInodes->operator[](node->second);
        }
        std::wcout << L"Not Found " + filename + L'\n';
        return nullptr;
    }

    size_t MinixFS::readInode(const Inode& inode, void *buffer, size_t size, size_t offset) const {
        size_t read = 0;
        size_t toRead = size;

        zone_t block = offset / 0x1000;
        while (toRead > 0) {
            if (block < V2_NR_DZONES) {
                // Direct block.
                size_t r = readBlock(inode.d2_zone[block], reinterpret_cast<char *>(buffer) + read, min(0x1000, toRead), offset % 0x1000);
                toRead -= r;
                read += r;
            } else {
                // Indirect block
                size_t r = readIndirectBlock(inode.d2_zone[V2_NR_DZONES], reinterpret_cast<char *>(buffer) + read, toRead, block - V2_NR_DZONES, offset % 0x1000);
                toRead -= r;
                read += r;
            }
            block++;
        }
        return read;
    }

    size_t MinixFS::readBlock(zone_t block, void *buffer, size_t size, size_t offset) const {
        std::lock_guard<std::mutex> lock(mStream->m_fileMutex);
        mStream->m_file.seekg(0x1000 * block + offset, std::ifstream::beg);

        mStream->m_file.read(static_cast<char *>(buffer), size);
        return size;
    }

    size_t MinixFS::readIndirectBlock(zone_t block, void *buffer, size_t size, zone_t blkOffset, size_t offset) const {
        size_t read = 0;
        size_t toRead = size;
        std::vector<zone_t> zones(1024);
        readBlock(block, zones.data(), 0x1000, 0);
        size_t zone = blkOffset;
        while (zones[zone] != 0 && toRead > 0) {
            auto direct = zones[zone];
           size_t r = readBlock(direct, static_cast<char *>(buffer) + read, min(0x1000, toRead), offset);
           read += r;
           toRead -= r;
           zone++;
        }
        return read;
    }

    void MinixFS::loadEntries() {
		std::filesystem::path root("/");
		mEntryMap.emplace(root, 1);
		loadEntry(1, root);
    }

    void MinixFS::createFile(std::wstring &path, bool isDir) {
        auto existing = find(path);

        if (existing) {
            std::wcout << L"FILE  " + path  + L"ALREADY CREATED...\n";
        }

        ino_t inode = 1;
        while (getInode(inode)) {
            inode++;
        }
        setInode(inode, true);
        Inode ino = mInodes->operator[](inode);
        ino.d2_size = 0;
        ino.d2_nlinks = isDir ? 2 : 1;
        ino.d2_mode = 7 + (isDir ? I_DIRECTORY : I_REGULAR);

        zone_t block = mSuperBlock->s_firstdatazone;
        if (isDir) {
            while (getBlock(block)) {
                block++;
            }
            setBlock(block, true);
            ino.d2_zone[0] = block;
        }
        mInodes->operator[](inode) = ino;

        // Write inode bitmap value
        std::lock_guard<std::mutex> lock(mStream->m_fileMutex);

        // Write inode
        mStream->m_file.seekg(0x4000 + 64 * (inode - 1), std::ifstream::beg);
        mStream->m_file.write(reinterpret_cast<const char *>(&ino), 64);
        // -----------------------------------------------------------------------

        mEntryMap.emplace(path, inode);

        auto parentPath = std::filesystem::path(path).parent_path();
        auto parent = find(parentPath);
        auto parentSize = parent->d2_size;
        auto blocks = parent->d2_zone;

        zone_t lastBlock = 0;
        while (blocks[lastBlock] != 0) {
            lastBlock++;
        }
        lastBlock = blocks[lastBlock - 1];


        auto filename = std::filesystem::path(path).filename().u8string();

        // Write directory entry
        V7Direct child;
        const char * filenamecstr = reinterpret_cast<const char *>(filename.c_str());
        strncpy(child.d_name, filenamecstr, filename.size());
        child.d_ino = inode;
        mStream->m_file.seekg(0x1000 * lastBlock + parentSize, std::ifstream::beg);
        mStream->m_file.write(reinterpret_cast<const char *>(&child), 64);

        // Extend parent
        parent->d2_size += 64;
        auto parentEntry = mEntryMap.find(parentPath.wstring());
        mStream->m_file.seekg(0x4000 + 64 * (parentEntry->second - 1), std::ifstream::beg);
        mStream->m_file.write(reinterpret_cast<const char *>(parent), 64);

    }

	bool MinixFS::writeFile(std::wstring &filename,
                                const void *buffer,
                                size_t NumberOfBytesToWrite,
                                unsigned long *NumberOfBytesWritten,
                                size_t Offset) {

        // First check if file is new (no data blocks)
        printf("Offset: %zu\n", Offset);
        auto ino = find(filename);

        int neededBlocks = (NumberOfBytesToWrite + 0xFFF) / 0x1000;
        int directBlocks = min(7, neededBlocks);

        if (neededBlocks > 7 + 1024 + pow(1024, 2)) {
            *NumberOfBytesWritten = 0;
			return false;
        }

        if (neededBlocks > 7) {
            // Count the indirect block
            neededBlocks++;
        }
        std::vector<zone_t> indirectBlocks;

        std::lock_guard<std::mutex> lock(mStream->m_fileMutex);
        // Block allocation
        for (int i = 0, reserved = 0; i < mSuperBlock->s_zones && reserved < neededBlocks; i++) {
            if (!getBlock(i)) {
                setBlock(i, true);
                if (reserved < 8) {
					ino->d2_zone[reserved] = i;
                } else {
                    indirectBlocks.push_back(i);
                }
                reserved++;
            }
        }

        //Write of indirect block
        if (neededBlocks > V2_NR_DZONES) {
            mStream->m_file.seekg(0x1000 * ino->d2_zone[V2_NR_DZONES], std::ifstream::beg);
            mStream->m_file.write(reinterpret_cast<char*>(indirectBlocks.data()), indirectBlocks.size() * sizeof(zone_t));
        }

        // Actual writing
        for (int i = 0; i < (NumberOfBytesToWrite + 0xFFF) / 0x1000; i++) {
            if (i < 7) {
                // Direct blocks
                mStream->m_file.seekg(0x1000 * ino->d2_zone[i], std::ifstream::beg);
                mStream->m_file.write(reinterpret_cast<const char *>(buffer) + i * 0x1000,
                                      min(NumberOfBytesToWrite, 0x1000));
            } else {
                mStream->m_file.seekg(0x1000 * indirectBlocks[i - 7], std::ifstream::beg);
                mStream->m_file.write(reinterpret_cast<const char *>(buffer) + i * 0x1000,
                                      min(NumberOfBytesToWrite, 0x1000));
            }
        }
        *NumberOfBytesWritten = NumberOfBytesToWrite;

        // Write inode
        ino->d2_size += NumberOfBytesToWrite;
        auto entry = mEntryMap.find(filename);
        mStream->m_file.seekg(0x4000 + 64 * (entry->second - 1), std::ifstream::beg);
        mStream->m_file.write(reinterpret_cast<const char *>(ino), 64);

        return true;

    }

	void MinixFS::loadEntry(ino_t inode, std::filesystem::path &path) {
        auto dir = mInodes->operator[](inode);
        auto dirSize = dir.d2_size;
        auto totalEntries = dirSize / 64;
        std::vector<V7Direct> entries(totalEntries);
        readInode(dir, entries.data(), dirSize, 0);

        for (int i = 0; i < totalEntries; i++) {
            auto entry = entries[i];
            std::cout << "New entry: ";
            std::cout << entry.d_name;
            std::cout << std::endl;

            if (entry.d_ino == 0) {
                continue; // Dirty (?)
            }

			auto thisPath = std::filesystem::path(path);
			//thisPath /= path;
            if (strcmp(entry.d_name, ".") != 0) {
                thisPath /= entry.d_name;
            }
            mEntryMap.emplace(thisPath, entry.d_ino);

            auto childIno = mInodes->operator[](entry.d_ino);

            if ((childIno.d2_mode & I_TYPE) == I_DIRECTORY) {
                if (strcmp(entry.d_name, ".") == 0) continue;
                if (strcmp(entry.d_name, "..") == 0) continue;
				loadEntry(entry.d_ino, thisPath);
			}
		}
    }

    std::vector<DirEntry*> MinixFS::listFolder(const std::filesystem::path &path) const {
        std::vector<DirEntry*> ret;
        auto node = mEntryMap.find(path);

        if (node != mEntryMap.end()) {
            auto dir = mInodes->operator[](node->second);
            auto dirSize = dir.d2_size;
            auto totalEntries = dirSize / 64;
            std::vector<V7Direct> entries(totalEntries);
            readInode(dir, entries.data(), dirSize, 0);

            for (int i = 0; i < totalEntries; i++) {
                ret.push_back(new DirEntry(mInodes->operator[](entries[i].d_ino), entries[i]));
            }
        }
        return ret;
    }

	size_t MinixFS::getFreeSpace() const {
        auto totalSpace = mSuperBlock->s_zones * 0x1000;
        auto used = 0;
        for (int i = 0; i < mSuperBlock->s_zones; i++) {
            if (getBlock(i)) {
                used += 0x1000;
            }
        }
        return totalSpace - used;
    }

	size_t MinixFS::getTotalSpace() const {
        return mSuperBlock->s_zones * 0x1000;
    }

	bool MinixFS::getBlock(zone_t zone) const {
        int base = mSuperBlock->s_firstdatazone;
        if (zone < base) {
            return true;
        }
        zone -= (base - 1);
        int w = zone / 16;
        int s = zone % 16;
        int mask = 1 << s;
        return (mBlockBitmap[w] & mask) != 0;
    }

	bool MinixFS::getInode(ino_t ino) const {
	    ino -= 1; // Disk starts at inode 1 (root directory)
		if (ino < 0) ino = 1;
        int w = ino / 16;
        int s = ino % 16;
        int mask = 1 << s;
        return (mInodeBitmap[w] & mask) != 0;
    }

	void MinixFS::setBlock(zone_t zone, bool set) {
        int base = mSuperBlock->s_firstdatazone;
        if (zone < base) {
            return;
        }
        zone -= (base - 1);
        int w = zone / 16;
        int s = zone % 16;

        if (set) {
            mBlockBitmap[w] |= 1UL << s;
        } else {
            mBlockBitmap[w] &= ~(1UL << s);
        }
        mStream->m_file.seekg(0x3000 + zone / 8, std::ifstream::beg);
        mStream->m_file.write(reinterpret_cast<char*>(mBlockBitmap.data() + w), 2);
    }

	void MinixFS::setInode(ino_t ino, bool set) {
        ino -= 1; // Disk starts at inode 1 (root directory)
        int w = ino / 16;
        int s = ino % 16;
        if (set) {
            mInodeBitmap[w] |= 1UL << s;
        } else {
            mInodeBitmap[w] &= ~(1UL << s);
        }
        mStream->m_file.seekg(0x2000 + ino / 8, std::ifstream::beg);
        mStream->m_file.write(reinterpret_cast<char*>(mInodeBitmap.data() + w), 2);
	}


		void MinixFS::deleteEntry(const std::wstring& filename) {
        std::wcout << L"\n\n\n\n***************Deleting " + filename + L'\n\n\n\n';
		auto node = mEntryMap.find(filename);
        if (node != mEntryMap.end()) {
			// Mark inode as free
            setInode(node->second, false);
			Inode ino = mInodes->operator[](node->second);
            // Mark blocks as free
			for (int i = 0; i < 7; i++) {
                zone_t usedBlock = ino.d2_zone[i];
				if (usedBlock > 0 && usedBlock < mSuperBlock->s_zones)
                    setBlock(usedBlock, false);
			}

			const char empty[64] = { 0 };
            // Write inode
            mStream->m_file.seekg(0x4000 + 64 * (node->second - 1), std::ifstream::beg);
            mStream->m_file.write(empty, 64);

            std::vector<zone_t> zones(1024);
            readBlock(ino.d2_zone[7], zones.data(), 0x1000, 0);
            for (zone_t zone : zones) {
                if (zone > 0 && zone < mSuperBlock->s_zones) {
                    setBlock(zone, false);
                }
            }

            // Delete entry in parent directory
            auto parentPath = std::filesystem::path(filename).parent_path();
            auto parent = find(parentPath);
            auto parentSize = parent->d2_size;
            auto totalEntries = parentSize / 64;
            std::vector<V7Direct> entries(totalEntries);
            readInode(*parent, entries.data(), parentSize, 0);

            size_t del = -1;
            for (size_t i = 0; i < entries.size(); i++) {
                V7Direct entry = entries[i];
                if (entry.d_ino == node->second) {
                    entries.erase(entries.begin() + i);
                    break;
                }
            }
            // Write directory entry
            unsigned long ret;
            std::wstring str = std::wstring(parentPath.c_str());
            parent->d2_size = 0; // so writeFile writes the right size
            writeFile(str, reinterpret_cast<const char*>(entries.data()), entries.size() * 64, &ret, 0);
        }
	}

        void MinixFS::createDirSelfLinks(std::wstring& path) {
        V7Direct self, parent;
        ino_t inode = mEntryMap.find(path)->second;
        self.d_ino = inode;
		strncpy(self.d_name, ".", 2);

        parent.d_ino = 1;
		strncpy(parent.d_name, "..", 2);
        Inode *ino = &mInodes->operator[](inode);
        ino->d2_size = 64 * 2; // . and ..
        mStream->m_file.seekg(0x1000 * ino->d2_zone[0], std::ifstream::beg);
        mStream->m_file.write(reinterpret_cast<const char*>(&self), 64);
        mStream->m_file.write(reinterpret_cast<const char*>(&parent), 64);

        // Update links count
        auto parentPath = std::filesystem::path(path).parent_path();
        ino_t parentIno = mEntryMap.find(std::wstring(parentPath.c_str()))->second;

        Inode parentInode = mInodes->operator[](parentIno);
        parentInode.d2_nlinks++;

        mStream->m_file.seekg(0x4000 + 64 * (parentIno - 1), std::ifstream::beg);
        mStream->m_file.write(reinterpret_cast<const char*>(&parentInode), 64);

    }
}

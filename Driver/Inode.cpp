//
// Created by Eduardo on 31/07/2021.
//

#include "Inode.h"
namespace minixfs {
    Inode::Inode(Stream& file) {

        file.m_file.read((char *) &d2_mode, sizeof(d2_mode));
        file.m_file.read((char *) &d2_nlinks, sizeof(d2_nlinks));
        file.m_file.read((char *) &d2_uid, sizeof(d2_uid));
        file.m_file.read((char *) &d2_gid, sizeof(d2_gid));
        file.m_file.read((char *) &d2_size, sizeof(d2_size));
        file.m_file.read((char *) &d2_atime, sizeof(d2_atime));
        file.m_file.read((char *) &d2_mtime, sizeof(d2_mtime));
        file.m_file.read((char *) &d2_ctime, sizeof(d2_ctime));

        for (int i = 0; i < V2_NR_TZONES; i++) {
            file.m_file.read((char *) &d2_zone[i], sizeof(d2_zone[i]));
        }

    }

    Inodes::Inodes(Stream &file, ino_t ninodes) {
        mInodes = std::vector<Inode *>(ninodes);

        std::lock_guard<std::mutex> lock(file.m_fileMutex);
        file.m_file.seekg(0x4000, std::ifstream::beg);

        for (int i = 0; i < ninodes; i++) {
            mInodes[i] = new Inode(file);
        }
    }
}


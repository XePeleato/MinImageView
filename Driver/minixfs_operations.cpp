//
// Created by Eduardo on 31/07/2021.
//

#include <sys/dir.h>
#include <iostream>
#include "MinixFS.h"

namespace utils {
    static minixfs::MinixFS *getContext(PDOKAN_FILE_INFO dokanfileinfo) {
        return reinterpret_cast<minixfs::MinixFS *>(dokanfileinfo->DokanOptions->GlobalContext);
    }
}

namespace minixfs {
    static NTSTATUS DOKAN_CALLBACK minixfs_createfile(
            LPCWSTR filename, PDOKAN_IO_SECURITY_CONTEXT, ACCESS_MASK desiredaccess,
            ULONG fileattributes, ULONG, ULONG createdisposition, ULONG createoptions,
            PDOKAN_FILE_INFO dokanfileinfo
            ) {
        auto ctx = utils::getContext(dokanfileinfo);

        ACCESS_MASK generic_desiredaccess;
        DWORD creation_disposition;
        DWORD file_attributes_and_flags;

        DokanMapKernelToUserCreateFileFlags(
                desiredaccess, fileattributes, createoptions, createdisposition,
                &generic_desiredaccess, &file_attributes_and_flags,
                &creation_disposition);


        auto filename_str = std::wstring(filename);

        std::wcout << L"filename: " + filename_str + L'\n';

        auto entry = ctx->find(filename);

        if (std::filesystem::path(filename).filename().wstring().length() > DIRSIZ) {
            return STATUS_OBJECT_NAME_INVALID;
        }

        // Windows will automatically try to create and access different system
        // directories.
        if (filename_str == L"\\System Volume Information" ||
        filename_str == L"\\$RECYCLE.BIN") {
            return STATUS_NO_SUCH_FILE;
        }

        if (entry && (entry->d2_mode & I_TYPE) == I_DIRECTORY) {
            if (createoptions & FILE_NON_DIRECTORY_FILE)
                return STATUS_FILE_IS_A_DIRECTORY;
            dokanfileinfo->IsDirectory = true;
        }

        if (dokanfileinfo->IsDirectory) {
            if (creation_disposition == CREATE_NEW ||
            creation_disposition== OPEN_ALWAYS) {
                if (entry) {
                    return STATUS_OBJECT_NAME_COLLISION;
                }

                // CREATE DIRECTORY IN MINIX IMAGE
                ctx->createFile(filename_str, true);
            }

            if (entry && (entry->d2_mode & I_TYPE) != I_DIRECTORY) {
                return STATUS_NOT_A_DIRECTORY;
            }

            if (!entry) {
                return STATUS_OBJECT_NAME_NOT_FOUND;
            }

        } else {
            // File then
            // Don't overwrite ., .. TODO

            switch (creation_disposition) {
                case CREATE_ALWAYS:
                    /*
                     * Creates a new file, always.
                     *
                     * We handle FILE_SUPERSEDE here as it is converted to TRUNCATE_EXISTING
                     * by DokanMapKernelToUserCreateFileFlags.
                     */

                    // CREATE FILE
                    ctx->createFile(filename_str, false);
                    break;
                case CREATE_NEW:
                    /*
                     * Creates a new file, only if it does not already exist.
                     */
                    if (entry) return STATUS_OBJECT_NAME_COLLISION;

                    ctx->createFile(filename_str, false);
                    break;
                case OPEN_ALWAYS:
                    /*
                     * Opens a file, always.
                     */
                    if (!entry) {
                        // CREATE FILE
                    } else {
                        if (desiredaccess & FILE_EXECUTE) {
                            struct tm newtime;
                            __time64_t long_time;
                            localtime_s(&newtime, &long_time);
                            //entry->d2_atime = long_time; TODO SET TIME
                        }
                    }
                    break;
                case OPEN_EXISTING:
                    /*
                     * Opens a file or device, only if it exists.
                     * If the specified file or device does not exist, the function fails
                     * and the last-error code is set to ERROR_FILE_NOT_FOUND
                     */
                    if (!entry) {
                        return STATUS_OBJECT_NAME_NOT_FOUND;
                    }

                    if (desiredaccess & FILE_EXECUTE) {
                        // SET TIME
                    }
                    break;
                case TRUNCATE_EXISTING:
                    /*
                     * Opens a file and truncates it so that its size is zero bytes, only if
                     * it exists. If the specified file does not exist, the function fails
                     * and the last-error code is set to ERROR_FILE_NOT_FOUND
                     */
                    if (!entry) {
                        return STATUS_OBJECT_NAME_NOT_FOUND;
                    }

                    // SET TIME AND SET SIZE TO 0
                    break;
                default:
                    break;
            }
        }

        /*
         * CREATE_NEW && OPEN_ALWAYS
         * If the specified file exists, the function fails and the last-error code is
         * set to ERROR_FILE_EXISTS
         */
        if (entry && (creation_disposition == CREATE_NEW ||
        creation_disposition == OPEN_ALWAYS))
            return STATUS_OBJECT_NAME_COLLISION;

        return STATUS_SUCCESS;
    }

    static void DOKAN_CALLBACK minixfs_cleanup(LPCWSTR filename,
                                             PDOKAN_FILE_INFO dokanfileinfo) {
        // nothing i guess
    }

    static void DOKAN_CALLBACK minixfs_closeFile(LPCWSTR filename,
                                               PDOKAN_FILE_INFO /*dokanfileinfo*/) {
        auto filename_str = std::wstring(filename);
        // nothing?
    }


    static NTSTATUS DOKAN_CALLBACK minixfs_readfile(LPCWSTR filename, LPVOID buffer,
                                                  DWORD bufferlength,
                                                  LPDWORD readlength,
                                                  LONGLONG offset,
                                                  PDOKAN_FILE_INFO dokanfileinfo) {
        auto filename_str = std::wstring(filename);
        auto ctx = utils::getContext(dokanfileinfo);

        auto f = ctx->find(filename_str);
        if (!f) return STATUS_OBJECT_NAME_NOT_FOUND;

        *readlength = ctx->readInode(*f, buffer, bufferlength, offset);

        return STATUS_SUCCESS;
    }

    static inline void LlongToDwLowHigh(const LONGLONG& v, DWORD& low,
                                        DWORD& hight) {
        hight = v >> 32;
        low = static_cast<DWORD>(v);
    }


    static inline void LlongToFileTime(LONGLONG v, FILETIME& filetime) {
        LlongToDwLowHigh(v, filetime.dwLowDateTime, filetime.dwHighDateTime);
    }

    static NTSTATUS DOKAN_CALLBACK minixfs_getfileinformation(LPCWSTR filename, LPBY_HANDLE_FILE_INFORMATION buffer,
                                                              PDOKAN_FILE_INFO dokanfileinfo) {
        auto filename_str = std::wstring(filename);
        auto ctx = utils::getContext(dokanfileinfo);

        auto f = ctx->find(filename_str);

        if (!f) return STATUS_OBJECT_NAME_NOT_FOUND;
        DWORD attribs = FILE_ATTRIBUTE_ARCHIVE;

        if ((f->d2_mode & I_TYPE) == I_DIRECTORY) {
            attribs |= FILE_ATTRIBUTE_DIRECTORY;
        }
        buffer->dwFileAttributes = attribs;
        LlongToFileTime(f->d2_ctime, buffer->ftCreationTime);
        LlongToFileTime(f->d2_atime, buffer->ftLastAccessTime);
        LlongToFileTime(f->d2_mtime, buffer->ftLastWriteTime);
        LlongToDwLowHigh(f->d2_size, buffer->nFileSizeLow, buffer->nFileSizeHigh);
        LlongToDwLowHigh(0, buffer->nFileIndexLow, buffer->nFileIndexHigh);
        buffer->nNumberOfLinks = f->d2_nlinks;
        buffer->dwVolumeSerialNumber = 0x19831116;

        return STATUS_SUCCESS;
    }

    static NTSTATUS DOKAN_CALLBACK minixfs_findfiles(LPCWSTR filename,
                                                     PFillFindData fill_finddata,
                                                     PDOKAN_FILE_INFO dokanfileinfo) {
        auto ctx = utils::getContext(dokanfileinfo);

        auto filename_str = std::wstring(filename);

        WIN32_FIND_DATAW findData;
        ZeroMemory(&findData, sizeof(WIN32_FIND_DATAW));

        auto files = ctx->listFolder(filename);

        for (const auto& f : files) {
            auto fileName = f->mEntry.d_name;
            std::copy(&fileName[0], &fileName[DIRSIZ - 1], std::begin(findData.cFileName));

            DWORD attribs = FILE_ATTRIBUTE_ARCHIVE;

            if ((f->mIno.d2_mode & I_TYPE) == I_DIRECTORY) {
                attribs |= FILE_ATTRIBUTE_DIRECTORY;
            }
            findData.dwFileAttributes = attribs;

            LlongToFileTime(f->mIno.d2_ctime, findData.ftCreationTime);
            LlongToFileTime(f->mIno.d2_atime, findData.ftLastAccessTime);
            LlongToFileTime(f->mIno.d2_mtime, findData.ftLastWriteTime);
            LlongToDwLowHigh(f->mIno.d2_size, findData.nFileSizeLow, findData.nFileSizeHigh);

            fill_finddata(&findData, dokanfileinfo);
        }
        return STATUS_SUCCESS;
    }

    static NTSTATUS DOKAN_CALLBACK minixfs_getvolumeinformation(
            LPWSTR volumename_buffer, DWORD volumename_size,
            LPDWORD volume_serialnumber, LPDWORD maximum_component_length,
            LPDWORD filesystem_flags, LPWSTR filesystem_name_buffer,
            DWORD filesystem_name_size, PDOKAN_FILE_INFO /*dokanfileinfo*/) {

        wcscpy_s(volumename_buffer, volumename_size, L"Dokan MinixFS");
        *volume_serialnumber = 0x19831116;
        *maximum_component_length = DIRSIZ;
        *filesystem_flags = FILE_CASE_SENSITIVE_SEARCH | FILE_CASE_PRESERVED_NAMES |
                 FILE_UNICODE_ON_DISK;

        wcscpy_s(filesystem_name_buffer, filesystem_name_size, L"Dokan XISO");

        return STATUS_SUCCESS;
    }

    static NTSTATUS DOKAN_CALLBACK minixfs_getdiskfreespace(
            PULONGLONG free_bytes_available, PULONGLONG total_number_of_bytes,
            PULONGLONG total_number_of_free_bytes, PDOKAN_FILE_INFO dokanfileinfo) {
        auto ctx = utils::getContext(dokanfileinfo);

        *free_bytes_available = ctx->getFreeSpace();
        *total_number_of_bytes = ctx->getTotalSpace();;
        *total_number_of_free_bytes = ctx->getFreeSpace();

        return STATUS_SUCCESS;
    }

    static NTSTATUS DOKAN_CALLBACK minixfs_writefile(LPCWSTR filename,
                                                     LPCVOID Buffer,
                                                     DWORD NumberOfBytesToWrite,
                                                     LPDWORD NumberOfBytesWritten,
                                                     LONGLONG Offset,
                                                     PDOKAN_FILE_INFO DokanFileInfo) {
        auto ctx = utils::getContext(DokanFileInfo);
        auto filename_str = std::wstring(filename);

        return ctx->writeFile(filename_str, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten, Offset);
    }

    void setup(DOKAN_OPERATIONS &dokanOperations) {
        dokanOperations.ZwCreateFile = minixfs_createfile;
        dokanOperations.ReadFile = minixfs_readfile;
        dokanOperations.GetFileInformation = minixfs_getfileinformation;
        dokanOperations.FindFiles = minixfs_findfiles;
        dokanOperations.GetDiskFreeSpace = minixfs_getdiskfreespace;
        dokanOperations.GetVolumeInformation = minixfs_getvolumeinformation;
        dokanOperations.WriteFile = minixfs_writefile;
    }
}
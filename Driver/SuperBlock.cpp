//
// Created by Eduardo on 31/07/2021.
//

#include "SuperBlock.h"
#include <vector>

namespace minixfs {
    SuperBlock::SuperBlock(Stream &stream) {
        {
            std::lock_guard<std::mutex> lock(stream.m_fileMutex);

            stream.m_file.seekg(SUPER_BLOCK_BYTES, std::ifstream::beg);
            stream.m_file.read((char *) &s_ninodes, sizeof(s_ninodes));
            stream.m_file.read((char *) &s_nzones, sizeof(s_nzones));
            stream.m_file.read((char *) &s_imap_blocks, sizeof(s_imap_blocks));
            stream.m_file.read((char *) &s_zmap_blocks, sizeof(s_zmap_blocks));
            stream.m_file.read((char *) &s_firstdatazone, sizeof(s_firstdatazone));
            stream.m_file.read((char *) &s_log_zone_size, sizeof(s_log_zone_size));
            stream.m_file.read((char *) &s_pad, sizeof(s_pad));
            stream.m_file.read((char *) &s_max_size, sizeof(s_max_size));
            stream.m_file.read((char *) &s_zones, sizeof(s_zones));
            stream.m_file.read((char *) &s_magic, sizeof(s_magic));
            stream.m_file.read((char *) &s_pad2, sizeof(s_pad2));
            stream.m_file.read((char *) &s_block_size, sizeof(s_block_size));
            stream.m_file.read((char *) &s_disk_version, sizeof(s_disk_version));
        }
    }

    bool SuperBlock::validate() const {
        return s_magic == SUPER_V3;
    }
}
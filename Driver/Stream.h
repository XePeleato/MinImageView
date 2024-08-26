//
// Created by Eduardo on 31/07/2021.
//

#ifndef MINIMAGEDRV_STREAM_H
#define MINIMAGEDRV_STREAM_H


#include <fstream>
#include <mutex>

class Stream {
public:
    Stream() = default;

    std::fstream m_file;
    std::streampos m_offset;

    std::mutex m_fileMutex;
};


#endif //MINIMAGEDRV_STREAM_H

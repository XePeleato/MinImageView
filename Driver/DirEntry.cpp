//
// Created by Eduardo on 01/08/2021.
//

#include "DirEntry.h"

namespace minixfs {

    DirEntry::DirEntry(Inode& inode, V7Direct& dirEnt) : mIno(inode), mEntry(dirEnt) {
    }
}
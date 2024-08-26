//
// Created by Eduardo on 01/08/2021.
//

#ifndef MINIMAGEDRV_DIRENTRY_H
#define MINIMAGEDRV_DIRENTRY_H

#include "Inode.h"
#include "V7Direct.h"

namespace minixfs {
    class DirEntry {
    public:
        DirEntry(Inode& inode, V7Direct& dirEnt);
        Inode mIno;
        V7Direct mEntry;
    };
}


#endif //MINIMAGEDRV_DIRENTRY_H

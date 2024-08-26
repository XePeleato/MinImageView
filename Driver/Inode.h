//
// Created by Eduardo on 31/07/2021.
//

#ifndef MINIMAGEDRV_INODE_H
#define MINIMAGEDRV_INODE_H


#include <vector>
#include "types.h"
#include "const.h"
#include "Stream.h"

namespace minixfs {
    class Inode {
    public:
        Inode(Stream& file);
        mode_t d2_mode;		/* file type, protection, etc. */
        u16_t d2_nlinks;		/* how many links to this file. HACK! */
        uid_t d2_uid;			/* user id of the file's owner. */
        u16_t d2_gid;			/* group number HACK! */
        off_t d2_size;		/* current file size in bytes */
        long d2_atime;		/* when was file data last accessed */
        long d2_mtime;		/* when was file data last changed */
        long d2_ctime;		/* when was inode data last changed */
        zone_t d2_zone[V2_NR_TZONES];	/* block nums for direct, ind, and dbl ind */
    };

    class Inodes {
    public:
        Inodes(Stream& file, ino_t ninodes);

        Inode& operator[] (size_t i) {
            return *mInodes[i - 1];
        }

        Inode& operator[] (size_t i) const {
            return *mInodes[i - 1];
        }


    //private:
        std::vector<Inode *> mInodes;
    };
}


#endif //MINIMAGEDRV_INODE_H

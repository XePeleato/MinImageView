//
// Created by Eduardo on 31/07/2021.
//

#ifndef MINIMAGEDRV_SUPERBLOCK_H
#define MINIMAGEDRV_SUPERBLOCK_H

#include "Stream.h"
#include "types.h"
#include "const.h"

namespace minixfs {
    class SuperBlock {
    public:
        SuperBlock(Stream &stream);
        bool validate() const;


        ino_t s_ninodes;		/* # usable inodes on the minor device */
        zone1_t  s_nzones;		/* total device size, including bit maps etc */
        short s_imap_blocks;		/* # of blocks used by inode bit map */
        short s_zmap_blocks;		/* # of blocks used by zone bit map */
        zone1_t s_firstdatazone;	/* number of first data zone */
        short s_log_zone_size;	/* log2 of blocks/zone */
        short s_pad;			/* try to avoid compiler-dependent padding */
        off_t s_max_size;		/* maximum file size on this device */
        zone_t s_zones;		/* number of zones (replaces s_nzones in V2) */
        short s_magic;		/* magic number to recognize super-blocks */

        /* The following items are valid on disk only for V3 and above */

        /* The block size in bytes. Minimum MIN_BLOCK SIZE. SECTOR_SIZE
         * multiple. If V1 or V2 filesystem, this should be
         * initialised to STATIC_BLOCK_SIZE. Maximum MAX_BLOCK_SIZE.
         */
        short s_pad2;			/* try to avoid compiler-dependent padding */
        unsigned short s_block_size;	/* block size in bytes. */
        char s_disk_version;		/* filesystem format sub-version */
    };
}


#endif //MINIMAGEDRV_SUPERBLOCK_H

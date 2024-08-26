//
// Created by Eduardo on 31/07/2021.
//

#ifndef MINIMAGEDRV_CONST_H
#define MINIMAGEDRV_CONST_H
/* Tables sizes */
#define V1_NR_DZONES       7	/* # direct zone numbers in a V1 inode */
#define V1_NR_TZONES       9	/* total # zone numbers in a V1 inode */
#define V2_NR_DZONES       7	/* # direct zone numbers in a V2 inode */
#define V2_NR_TZONES      10	/* total # zone numbers in a V2 inode */

#define NR_FILPS         256	/* # slots in filp table */
#define NR_INODES        256	/* # slots in "in core" inode table */
#define NR_SUPERS         12	/* # slots in super block table */
#define NR_LOCKS           8	/* # slots in the file locking table */

/* The type of sizeof may be (unsigned) long.  Use the following macro for
 * taking the sizes of small objects so that there are no surprises like
 * (small) long constants being passed to routines expecting an int.
 */
#define usizeof(t) ((unsigned) sizeof(t))

/* File system types. */
#define SUPER_MAGIC   0x137F	/* magic number contained in super-block */
#define SUPER_REV     0x7F13	/* magic # when 68000 disk read on PC or vv */
#define SUPER_V2      0x2468	/* magic # for V2 file systems */
#define SUPER_V2_REV  0x6824	/* V2 magic written on PC, read on 68K or vv */
#define SUPER_V3      0x4d5a	/* magic # for V3 file systems */

#define V1		   1	/* version number of V1 file systems */
#define V2		   2	/* version number of V2 file systems */
#define V3		   3	/* version number of V3 file systems */

/* Miscellaneous constants */
#define SU_UID 	 ((uid_t) 0)	/* super_user's uid_t */
#define SERVERS_UID ((uid_t) 11) /* who may do FSSIGNON */
#define SYS_UID  ((uid_t) 0)	/* uid_t for processes MM and INIT */
#define SYS_GID  ((gid_t) 0)	/* gid_t for processes MM and INIT */
#define NORMAL	           0	/* forces get_block to do disk read */
#define NO_READ            1	/* prevents get_block from doing disk read */
#define PREFETCH           2	/* tells get_block not to read or mark dev */

#define XPIPE   (-NR_TASKS-1)	/* used in fp_task when susp'd on pipe */
#define XLOCK   (-NR_TASKS-2)	/* used in fp_task when susp'd on lock */
#define XPOPEN  (-NR_TASKS-3)	/* used in fp_task when susp'd on pipe open */
#define XSELECT (-NR_TASKS-4)	/* used in fp_task when susp'd on select */

#define NO_BIT   ((bit_t) 0)	/* returned by alloc_bit() to signal failure */

#define DUP_MASK        0100	/* mask to distinguish dup2 from dup */

#define LOOK_UP            0 /* tells search_dir to lookup string */
#define ENTER              1 /* tells search_dir to make dir entry */
//#define DELETE             2 /* tells search_dir to delete entry */
#define IS_EMPTY           3 /* tells search_dir to ret. OK or ENOTEMPTY */

/* write_map() args */
#define WMAP_FREE	(1 << 0)

#define PATH_TRANSPARENT 000   /* parse_path stops at final object */
#define PATH_PENULTIMATE 001   /* parse_path stops at last but one name */
#define PATH_OPAQUE      002   /* parse_path stops at final name */
#define PATH_NONSYMBOLIC 004   /* parse_path scans final name if symbolic */
#define PATH_STRIPDOT    010   /* parse_path strips /. from path */
#define EAT_PATH         PATH_TRANSPARENT
#define EAT_PATH_OPAQUE  PATH_OPAQUE
#define LAST_DIR         PATH_PENULTIMATE
#define LAST_DIR_NOTDOT  PATH_PENULTIMATE | PATH_STRIPDOT
#define LAST_DIR_EATSYM  PATH_NONSYMBOLIC
#define SYMLOOP		16

#define CLEAN              0	/* disk and memory copies identical */
#define DIRTY              1	/* disk and memory copies differ */
#define ATIME            002	/* set if atime field needs updating */
#define CTIME            004	/* set if ctime field needs updating */
#define MTIME            010	/* set if mtime field needs updating */

#define BYTE_SWAP          0	/* tells conv2/conv4 to swap bytes */

#define END_OF_FILE   (-104)	/* eof detected */

#define ROOT_INODE         1		/* inode number for root directory */
#define BOOT_BLOCK  ((block_t) 0)	/* block number of boot block */
#define SUPER_BLOCK_BYTES (1024)	/* bytes offset */
#define START_BLOCK 	2		/* first block of FS (not counting SB) */

#define DIR_ENTRY_SIZE       usizeof (struct direct)  /* # bytes/dir entry   */
#define NR_DIR_ENTRIES(b)   ((b)/DIR_ENTRY_SIZE)  /* # dir entries/blk   */
#define SUPER_SIZE      usizeof (struct super_block)  /* super_block size    */
#define PIPE_SIZE(b)          (V1_NR_DZONES*(b))  /* pipe size in bytes  */

#define FS_BITMAP_CHUNKS(b) ((b)/usizeof (bitchunk_t))/* # map chunks/blk   */
#define FS_BITCHUNK_BITS		(usizeof(bitchunk_t) * CHAR_BIT)
#define FS_BITS_PER_BLOCK(b)	(FS_BITMAP_CHUNKS(b) * FS_BITCHUNK_BITS)

/* Derived sizes pertaining to the V1 file system. */
#define V1_ZONE_NUM_SIZE           usizeof (zone1_t)  /* # bytes in V1 zone  */
#define V1_INODE_SIZE             usizeof (d1_inode)  /* bytes in V1 dsk ino */

/* # zones/indir block */
#define V1_INDIRECTS (_STATIC_BLOCK_SIZE/V1_ZONE_NUM_SIZE)

/* # V1 dsk inodes/blk */
#define V1_INODES_PER_BLOCK (_STATIC_BLOCK_SIZE/V1_INODE_SIZE)

/* Derived sizes pertaining to the V2 file system. */
#define V2_ZONE_NUM_SIZE            usizeof (zone_t)  /* # bytes in V2 zone  */
#define V2_INODE_SIZE             usizeof (d2_inode)  /* bytes in V2 dsk ino */
#define V2_INDIRECTS(b)   ((b)/V2_ZONE_NUM_SIZE)  /* # zones/indir block */
#define V2_INODES_PER_BLOCK(b) ((b)/V2_INODE_SIZE)/* # V2 dsk inodes/blk */


#define CHIP INTEL

#define EXTERN        extern	/* used in *.h files */
#define PRIVATE       static	/* PRIVATE x limits the scope of x */
#define PUBLIC			/* PUBLIC is the opposite of PRIVATE */
#define FORWARD       static	/* some compilers require this to be 'static'*/

#define TRUE               1	/* used for turning integers into Booleans */
#define FALSE              0	/* used for turning integers into Booleans */

#define HZ	          60	/* clock freq (software settable on IBM-PC) */

#define SUPER_USER (uid_t) 0	/* uid_t of superuser */

//#define NULL     ((void *)0)	/* null pointer */
#define CPVEC_NR          16	/* max # of entries in a SYS_VCOPY request */
#define CPVVEC_NR         64	/* max # of entries in a SYS_VCOPY request */
#define NR_IOREQS	MIN(NR_BUFS, 64)
/* maximum number of entries in an iorequest */

/* Message passing constants. */
#define MESS_SIZE (sizeof(message))	/* might need usizeof from FS here */
#define NIL_MESS ((message *) 0)	/* null pointer */

/* Memory related constants. */
#define SEGMENT_TYPE  0xFF00	/* bit mask to get segment type */
#define SEGMENT_INDEX 0x00FF	/* bit mask to get segment index */

#define LOCAL_SEG     0x0000	/* flags indicating local memory segment */
#define NR_LOCAL_SEGS      3	/* # local segments per process (fixed) */
#define T                  0	/* proc[i].mem_map[T] is for text */
#define D                  1	/* proc[i].mem_map[D] is for data */
#define S                  2	/* proc[i].mem_map[S] is for stack */

#define REMOTE_SEG    0x0100	/* flags indicating remote memory segment */
#define NR_REMOTE_SEGS     3    /* # remote memory regions (variable) */

#define BIOS_SEG      0x0200	/* flags indicating BIOS memory segment */
#define NR_BIOS_SEGS       3    /* # BIOS memory regions (variable) */

#define PHYS_SEG      0x0400	/* flag indicating entire physical memory */

/* Labels used to disable code sections for different reasons. */
#define DEAD_CODE	   0	/* unused code in normal configuration */
#define FUTURE_CODE	   0	/* new code to be activated + tested later */
#define TEMP_CODE	   1	/* active code to be removed later */

/* Process name length in the PM process table, including '\0'. */
#define PROC_NAME_LEN	16

/* Miscellaneous */
#define BYTE            0377	/* mask for 8 bits */
#define READING            0	/* copy data to user */
#define WRITING            1	/* copy data from user */
#define NO_NUM        0x8000	/* used as numerical argument to panic() */
#define NIL_PTR   (char *) 0	/* generally useful expression */
#define HAVE_SCATTERED_IO  1	/* scattered I/O is now standard */

/* Macros. */
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))

/* Memory is allocated in clicks. */
#if (CHIP == INTEL)
#define CLICK_SIZE      4096	/* unit in which memory is allocated */
#define CLICK_SHIFT       12	/* log2 of CLICK_SIZE */
#endif

#if (CHIP == SPARC) || (CHIP == M68000)
#define CLICK_SIZE	4096	/* unit in which memory is allocated */
#define CLICK_SHIFT	  12	/* log2 of CLICK_SIZE */
#endif

/* Click to byte conversions (and vice versa). */
#define HCLICK_SHIFT       4	/* log2 of HCLICK_SIZE */
#define HCLICK_SIZE       16	/* hardware segment conversion magic */
#if CLICK_SIZE >= HCLICK_SIZE
#define click_to_hclick(n) ((n) << (CLICK_SHIFT - HCLICK_SHIFT))
#else
#define click_to_hclick(n) ((n) >> (HCLICK_SHIFT - CLICK_SHIFT))
#endif
#define hclick_to_physb(n) ((phys_bytes) (n) << HCLICK_SHIFT)
#define physb_to_hclick(n) ((n) >> HCLICK_SHIFT)

#define ABS             -999	/* this process means absolute memory */

/* Flag bits for i_mode in the inode. */
#define I_TYPE          0170000	/* this field gives inode type */
#define I_SYMBOLIC_LINK 0120000	/* file is a symbolic link */
#define I_REGULAR       0100000	/* regular file, not dir or special */
#define I_BLOCK_SPECIAL 0060000	/* block special file */
#define I_DIRECTORY     0040000	/* file is a directory */
#define I_CHAR_SPECIAL  0020000	/* character special file */
#define I_NAMED_PIPE	0010000 /* named pipe (FIFO) */
#define I_SET_UID_BIT   0004000	/* set effective uid_t on exec */
#define I_SET_GID_BIT   0002000	/* set effective gid_t on exec */
#define ALL_MODES       0006777	/* all bits for user, group and others */
#define RWX_MODES       0000777	/* mode bits for RWX only */
#define R_BIT           0000004	/* Rwx protection bit */
#define W_BIT           0000002	/* rWx protection bit */
#define X_BIT           0000001	/* rwX protection bit */
#define I_NOT_ALLOC     0000000	/* this inode is free */

/* Some limits. */
#define MAX_BLOCK_NR  ((block_t) 077777777)	/* largest block number */
#define HIGHEST_ZONE   ((zone_t) 077777777)	/* largest zone number */
#define MAX_INODE_NR ((ino_t) 037777777777)	/* largest inode number */
#define MAX_FILE_POS ((off_t) 037777777777)	/* largest legal file offset */

#define MAX_SYM_LOOPS	8	/* how many symbolic links are recursed */

#define NO_BLOCK              ((block_t) 0)	/* absence of a block number */
#define NO_ENTRY                ((ino_t) 0)	/* absence of a dir entry */
#define NO_ZONE                ((zone_t) 0)	/* absence of a zone number */
#define NO_DEV                  ((dev_t) 0)	/* absence of a device numb */
#endif //MINIMAGEDRV_CONST_H

#ifndef RW_RTGCNFS_H
#define RW_RTGCNFS_H

#include "rw/rtfsyst.h"
#include <dolphin/dvd.h>
#include "layout_check.h"

/* The RenderWare file system on the GameCube disc: files read through the DVD library, each with
 * a read cache for reads that are not 32-byte aligned. The type and member names are inferred from
 * the code that uses them. */

#ifdef __cplusplus
extern "C" {
#endif

/* A 64-bit file offset as an 8-byte aggregate. The read reports the file position to OSReport in
 * this form: MWCC copies such an argument to memory and passes its address, as the original does.
 * Whether the file's position member itself has this type is not established. */
typedef struct RtGCNFileOffset {
    RwInt64 value;
} RtGCNFileOffset;
CHECK_SIZEOF(RtGCNFileOffset, 0x8);

/* A file of the disc file system (0x90 bytes) */
typedef struct RtGCNFile {
    /* 0x00 */ RtFile file;          /* 0x40 bytes with its alignment */
    /* 0x40 */ DVDFileInfo fileInfo; /* its command block's user data holds the last read result */
    /* 0x7C */ RwUInt8 *cache;       /* 32-byte aligned */
    /* 0x80 */ RwBool cacheValid;
    /* 0x84 */ RwInt32 cacheOffset; /* of the next cached byte to copy */
    /* 0x88 */ RwInt32 cacheSize;
    /* 0x8C */ RwUInt8 field_0x8C[0x4];
} RtGCNFile;
CHECK_SIZEOF(RtGCNFile, 0x90);

/* The last device state the file system met, which its error callback is told (inferred) */
#define RTGCNFS_ERROR_NONE 1
#define RTGCNFS_ERROR_FATAL 2
#define RTGCNFS_ERROR_NODISK 3
#define RTGCNFS_ERROR_COVEROPEN 4
#define RTGCNFS_ERROR_WRONGDISK 5
#define RTGCNFS_ERROR_RETRY 6
#define RTGCNFS_ERROR_OTHER 7
#define RTGCNFS_ERROR_BUSY 8

extern volatile RwInt32 lbl_805EDE90; /* RTGCNFS_ERROR_*, initially none */

/* the disc file system (inferred) */
RtFile *fn_8047A7C8(RtFileSystem *fileSystem, RwInt32 index); /* a file */
const RwChar *fn_8047A7EC(RtFileSystem *fileSystem);          /* the path prefix */
void fn_8047A7F4(RtFileSystem *fileSystem);                   /* closes */
RwInt32 fn_8047A824(RtFileSystem *fileSystem, RtFile *file, const RwChar *name,
                    RwUInt32 mode);                  /* opens a file */
void fn_8047A938(RtFile *file);                      /* closes a file */
void fn_8047A970(s32 result, DVDFileInfo *fileInfo); /* a read's callback */
RwInt32 fn_8047A978(RtGCNFile *file, void *buffer, RwInt32 length, RwInt32 offset,
                    RwBool async);                                       /* reads the device */
RwUInt32 fn_8047AC10(RtFile *file, void *buffer, RwUInt32 length);       /* reads a file */
RwUInt32 fn_8047B050(RtFile *file, const void *buffer, RwUInt32 length); /* writes a file */
RwInt64 fn_8047B060(RtFile *file, RwInt32 offset, RwInt32 origin);       /* sets the position */
RwInt32 fn_8047B154(RtFile *file,
                    RwInt32 unused); /* polls a file; the second argument is not read */
void fn_8047B384(RtFile *file);      /* aborts a read */
RwInt32 fn_8047B3AC(RtFile *file);   /* a file's status */
RwBool fn_8047B3EC(RtFileSystem *fileSystem, const RwChar *name); /* whether a file exists */
RtFileSystem *fn_8047B40C(RwInt32 maxNbOpenFiles, RwUInt8 *cacheBuffers, RwInt32 cacheSize,
                          const RwChar *deviceName, const RwChar *name); /* creates */

#ifdef __cplusplus
}
#endif

#endif

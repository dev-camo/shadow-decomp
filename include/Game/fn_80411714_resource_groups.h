#ifndef FN_80411714_RESOURCE_GROUPS_H
#define FN_80411714_RESOURCE_GROUPS_H

#include "layout_check.h"

/* Groups of numbered resources (the unit at 0x80411714; every name here is inferred from the code).
 * A group is loaded from a file image or registered from a single block of data, covers a range
 * of consecutive 16-bit ids and is linked into a list of the loaded groups. An id is zero when it
 * names nothing. */

/* A record in the chain that an item holds; the chain ends with a record whose size is 0xFFFF. The
 * size is the distance to the next record. fn_80412D7C reverses the bytes of the chain of a group
 * written in the opposite byte order. */
typedef struct ResourceRecord {
    /* 0x00 */ unsigned short flags;
    /* 0x02 */ unsigned short size;
} ResourceRecord;
CHECK_SIZEOF(ResourceRecord, 0x4);

/* The header of a block of an item's data. Elements follow it (16-bit elements when flag 1 of the
 * data is set, otherwise 32-bit), and the size is the distance to the next block. */
typedef struct ResourceBlock {
    /* 0x00 */ int size;
    /* 0x04 */ unsigned short unknown04;
    /* 0x06 */ unsigned char unknown06;
    /* 0x07 */ signed char hasNext;
} ResourceBlock;
CHECK_SIZEOF(ResourceBlock, 0x8);

/* The data of an item: a header followed by a chain of blocks (version 3 and later). */
typedef struct ResourceData {
    /* 0x00 */ unsigned char version;
    /* 0x01 */ unsigned char byteOrderFlag;
    /* 0x04 */ unsigned int entryId;
    /* 0x08 */ unsigned int size; /* in bytes, header included */
    /* 0x0C */ unsigned char unknown0C;
    /* 0x0D */ unsigned char flags;
    /* 0x0E */ unsigned char unknown0E[2];
    /* 0x10 */ unsigned short unknown10;
    /* 0x12 */ unsigned short unknown12;
    /* 0x14 */ char name[32];
    /* 0x34 */ ResourceBlock firstBlock;
} ResourceData;
CHECK_SIZEOF(ResourceData, 0x3C);

/* The entry of an id: the name, the data and a chain of records. */
typedef struct ResourceItem {
    /* 0x00 */ char *name;
    /* 0x04 */ ResourceData *data;
    /* 0x08 */ ResourceRecord *records;
} ResourceItem;
CHECK_SIZEOF(ResourceItem, 0xC);

/* A copy in main memory of an item whose data lies in ARAM; the copy is shared by reference
 * count. */
typedef struct ResourceInstance {
    /* 0x00 */ unsigned short id;
    /* 0x02 */ short referenceCount;
    /* 0x04 */ ResourceItem item;
    /* 0x10 */ struct ResourceInstance *next;
    /* 0x14 */ unsigned char unknown14[0xC];
} ResourceInstance;
CHECK_SIZEOF(ResourceInstance, 0x20);

/* Where the three parts of an item lie in ARAM. */
typedef struct ResourceSource {
    /* 0x00 */ unsigned int nameAddress;
    /* 0x04 */ int nameSize;
    /* 0x08 */ unsigned int dataAddress;
    /* 0x0C */ int dataSize;
    /* 0x10 */ unsigned int recordsAddress;
    /* 0x14 */ int recordsSize;
} ResourceSource;
CHECK_SIZEOF(ResourceSource, 0x18);

/* The header of a file image, of the group that fn_80412BFC allocates for a single item, or of
 * the group with copies that fn_80413538 allocates (followed by its sources). */
typedef struct ResourceGroup {
    /* 0x00 */ unsigned short firstId;
    /* 0x02 */ unsigned short count;
    /* 0x04 */ unsigned char ownsGroup;
    /* 0x05 */ unsigned char byteOrderFlag;
    /* 0x06 */ unsigned char hasInstances;
    /* 0x07 */ unsigned char unknown07;
    /* 0x08 */ void (*release)(void *);
    /* 0x0C */ struct ResourceGroup *next;
    /* 0x10 */ ResourceItem *items;
    /* 0x14 */ unsigned int aramAddress;
    /* 0x18 */ ResourceInstance *instances;
    /* 0x1C: ResourceSource sources[count] for a group with instances */
} ResourceGroup;
CHECK_SIZEOF(ResourceGroup, 0x1C);

#ifdef __cplusplus
extern "C" {
#endif

void fn_80411714(void);
unsigned int fn_804123DC(unsigned short id);
ResourceBlock *fn_804124DC(unsigned short id);
int fn_804125F4(float *out, unsigned short id);
unsigned short fn_80412738(const char *name);
const char *fn_80412820(unsigned short id);
ResourceData *fn_80412920(unsigned short id);
void fn_804129D4(unsigned short id);
unsigned short fn_80412B74(void *image, void (*release)(void *), int assignId);
unsigned short fn_80412BFC(void *data, const char *path, void (*release)(void *), int assignId);
void fn_80412D7C(ResourceGroup *group);
unsigned short fn_80413058(ResourceGroup *group);
void fn_80413240(unsigned short id);
void fn_80413318(unsigned short id);
ResourceGroup *fn_80413538(ResourceGroup *group);
int fn_804139B4(ResourceData *data);

#ifdef __cplusplus
}
#endif

#endif

#ifndef GAME_FN_80410630_PACK_H
#define GAME_FN_80410630_PACK_H

#include "Game/fn_8042_name_registry.h"
#include "layout_check.h"

/* Packs of named nodes loaded from files: the layouts come from the loader in fn_80410630.c and
 * every name is inferred. The files are written in either byte order and the loader reverses the
 * values of the opposite one in place. */

struct PackNode;

/* A link between nodes. The file stores the offset of the target from the start of its entry; the
 * loader replaces it by the address. */
typedef union PackNodeLink {
    unsigned int offset;
    struct PackNode *node;
} PackNodeLink;
CHECK_SIZEOF(PackNodeLink, 0x4);

typedef struct PackNode {
    /* 0x00 */ unsigned int id;
    /* 0x04 */ unsigned short values[5];
    /* 0x10 */ float rows[3][4];
    /* 0x40 */ unsigned char unknown40[8];
    /* 0x48 */ PackNodeLink first;
    /* 0x4C */ PackNodeLink second;
    /* 0x50 */ char name[32];
} PackNode;
CHECK_SIZEOF(PackNode, 0x70);

/* The part of an entry that fn_804106FC returns for an id. Its callers read value04 as a 16-bit
 * value, value08 as a float and find the entry's root node at 0x2C. */
typedef struct PackEntryInfo {
    /* 0x00 */ unsigned int id;
    /* 0x04 */ unsigned short value04;
    /* 0x06 */ unsigned short nodeCount;
    /* 0x08 */ float value08;
    /* 0x0C */ char name[32];
} PackEntryInfo;
CHECK_SIZEOF(PackEntryInfo, 0x2C);

/* An entry of a pack, followed by its nodes; the first node at 0x30 is the root of a tree. */
typedef struct PackEntry {
    /* 0x00 */ unsigned char version;
    /* 0x01 */ unsigned char byteOrderFlag;
    /* 0x04 */ PackEntryInfo info;
} PackEntry;
CHECK_SIZEOF(PackEntry, 0x30);

/* The entries of a pack: the file stores their offset from the start of the pack and the loader
 * replaces it by the address. */
typedef union PackEntriesLink {
    unsigned int offset;
    PackEntry *entries;
} PackEntriesLink;
CHECK_SIZEOF(PackEntriesLink, 0x4);

/* The header of a pack: the loader links it into a list of the loaded packs. For a single entry
 * without a pack header, fn_804109C4 allocates the header; a pack that fn_804108F0 loads from a
 * file has its header at the start of the file. */
typedef struct PackFile {
    /* 0x00 */ int entryCount;
    /* 0x04 */ unsigned int id;
    /* 0x08 */ unsigned char ownsHeader;
    /* 0x09 */ unsigned char byteOrderFlag;
    /* 0x0C */ void (*release)(void *);
    /* 0x10 */ PackEntriesLink entries;
    /* 0x14 */ struct PackFile *next;
    /* 0x18 */ unsigned char unknown18[8];
} PackFile;
CHECK_SIZEOF(PackFile, 0x20);

#ifdef __cplusplus
extern "C" {
#endif

unsigned int fn_80410630(const char *path);
unsigned int fn_80410684(const char *name);
unsigned int fn_804106C0(const char *name);
PackEntryInfo *fn_804106FC(unsigned int id);
void fn_80410774(void);
void fn_804107C0(int arg0, int arg1, int arg2, int arg3);
void fn_804107EC(unsigned int id);
unsigned int fn_804108F0(void *file, void (*release)(void *));
unsigned int fn_804109C4(PackEntry *entry, void (*release)(void *));
int fn_80410A4C(PackFile *pack);
int fn_80410BB4(char *name, const char *path);
void fn_80410C7C(PackEntry *entry, PackNode *node);
void fn_80410D40(PackEntry *entry, PackNode *node);

#ifdef __cplusplus
}
#endif

#endif

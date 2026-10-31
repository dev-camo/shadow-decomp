/* Loader of packs of named nodes (the original file name is unknown; every name here is inferred
 * from the code). A pack holds entries, each with a name and a tree of nodes. Loading fixes the
 * byte order of a pack written in the opposite order, relocates the tree links and registers the
 * entry and node names in two name registries; the loaded packs form a list. Definitions are in
 * address order, with automatic inlining one level deep (see configure.py). */
#include <string.h>

#include "Game/fn_804110EC_allocator_hooks.h"
#include "Game/fn_80410630_pack.h"
#include "Game/fn_8042_byte_order.h"

/* The uninitialised globals are laid out last-defined first, so they are listed from the highest
 * address down: the registry of node names, the registry of entry names and the list of loaded
 * packs. */
NameRegistry *lbl_805F1390;
NameRegistry *lbl_805F138C;
PackFile *lbl_805F1388;

/* Returns the id of the entry that the last component of the path names, or 0. */
unsigned int fn_80410630(const char *path) {
    char name[32];

    if (fn_80410BB4(name, path)) {
        if (lbl_805F138C != 0) {
            return fn_80420C74(lbl_805F138C, name);
        }
        return 0;
    }
    return 0;
}

/* Look up the id registered for a node name and for an entry name. */
unsigned int fn_80410684(const char *name) {
    if (lbl_805F1390 != 0) {
        return fn_80420C74(lbl_805F1390, name);
    }
    return 0;
}

unsigned int fn_804106C0(const char *name) {
    if (lbl_805F138C != 0) {
        return fn_80420C74(lbl_805F138C, name);
    }
    return 0;
}

/* Matching workaround: a macro for the same reason as NEEDS_BYTE_SWAP. Measured with an inline
 * function: this object has 697 instructions instead of the original's 687. Revisit with
 * NEEDS_BYTE_SWAP.
 *
 * The entry that follows an entry and its nodes. */
#define NEXT_ENTRY(entry)                                                                          \
    ((PackEntry *)((char *)((entry) + 1) + (entry)->info.nodeCount * sizeof(PackNode)))

/* Returns the entry with the given id, or 0 (the helper name is inferred). */
static inline PackEntry *FindEntry(unsigned int id) {
    PackFile *pack = lbl_805F1388;

    /* The original returns early on an empty list before the loop, which tests the head again
     * (fn_804106FC+0x04: cmplwi, bne, li r4,0 and a branch to the end). */
    if (pack == 0) {
        return 0;
    }
    for (; pack != 0; pack = pack->next) {
        PackEntry *entry = pack->entries.entries;
        int index;

        for (index = 0; index < pack->entryCount; index++) {
            if (id == entry->info.id) {
                return entry;
            }
            entry = NEXT_ENTRY(entry);
        }
    }
    return 0;
}

/* Returns the information of the entry with the given id, or 0. */
PackEntryInfo *fn_804106FC(unsigned int id) {
    PackEntry *found = FindEntry(id);

    if (found != 0) {
        return &found->info;
    }
    return 0;
}

/* Destroys the two name registries. */
void fn_80410774(void) {
    if (lbl_805F138C != 0) {
        fn_80421050(lbl_805F138C);
        lbl_805F138C = 0;
    }
    if (lbl_805F1390 != 0) {
        fn_80421050(lbl_805F1390);
        lbl_805F1390 = 0;
    }
}

/* Creates the two name registries. The game's only caller passes four values that this function
 * does not use. */
void fn_804107C0(int arg0, int arg1, int arg2, int arg3) {
    lbl_805F138C = fn_804210AC();
    lbl_805F1390 = fn_804210AC();
}

/* Unloads the pack with the given id: unlists it, drops the registrations of its entries and nodes
 * and hands the pack to its release function (the data when the header was allocated by
 * fn_804109C4, otherwise the pack itself). */
void fn_804107EC(unsigned int id) {
    int index;
    PackFile *pack;
    PackEntry *entry;
    PackFile *previous;
    void (*release)(void *);

    if (lbl_805F1388 == 0) {
        return;
    }

    previous = 0;
    pack = lbl_805F1388;
    while (pack != 0) {
        if (pack->id == id) {
            if (previous != 0) {
                previous->next = pack->next;
            } else {
                lbl_805F1388 = pack->next;
            }

            entry = pack->entries.entries;
            for (index = 0; index < pack->entryCount; index++) {
                fn_80410C7C(entry, (PackNode *)(entry + 1));
                fn_80420CE8(lbl_805F138C, entry->info.name, entry->info.id);
                entry = NEXT_ENTRY(entry);
            }

            release = pack->release;
            if (pack->ownsHeader) {
                if (release != 0) {
                    release(pack->entries.entries);
                }
                fn_804110FC(pack);
            } else if (release != 0) {
                release(pack);
            }
            return;
        }
        previous = pack;
        pack = pack->next;
    }
}

/* Loads a pack file, whose header is at its start; returns the id of the pack, or 0. */
unsigned int fn_804108F0(void *file, void (*release)(void *)) {
    PackFile *pack = file;

    if (pack == 0) {
        return 0;
    }

    if (NEEDS_BYTE_SWAP(pack->byteOrderFlag)) {
        fn_804210E4(&pack->entryCount, &pack->entryCount, 1);
        fn_804210E4(&pack->id, &pack->id, 1);
        fn_804210E4(&pack->entries, &pack->entries, 1);
        fn_804210E4(&pack->next, &pack->next, 1);
    }
    pack->entries.entries = (PackEntry *)(pack->entries.offset + (unsigned int)pack);
    pack->release = release;

    if (fn_80410A4C(pack)) {
        return pack->id;
    }
    return 0;
}

/* Loads one entry that has no pack header, allocating a header for it; returns the id of the pack,
 * or 0. */
unsigned int fn_804109C4(PackEntry *entry, void (*release)(void *)) {
    PackFile *pack = fn_80411134(1, sizeof(PackFile));

    pack->entryCount = 1;
    pack->ownsHeader = 1;
    if (fn_804211CC()) {
        pack->byteOrderFlag = 1;
    }
    pack->release = release;
    pack->entries.entries = entry;

    if (!fn_80410A4C(pack)) {
        fn_804110FC(pack);
        return 0;
    }
    return pack->id;
}

/* Fixes the byte order of an entry header; returns 0 for a version below 4. */
static inline int FixEntryHeader(PackEntry *entry) {
    if (entry->version >= 4) {
        if (NEEDS_BYTE_SWAP(entry->byteOrderFlag)) {
            fn_804210E4(&entry->info.id, &entry->info.id, 1);
            fn_8042118C(&entry->info.value04, &entry->info.value04, 1);
            fn_8042118C(&entry->info.nodeCount, &entry->info.nodeCount, 1);
            fn_804210E4(&entry->info.value08, &entry->info.value08, 1);
        }
        return 1;
    }
    return 0;
}

/* Registers the entries of a pack and their nodes, then appends the pack to the list of loaded
 * packs. Returns 0 when an entry is invalid or none could be registered. */
int fn_80410A4C(PackFile *pack) {
    int index;
    PackEntry *entry = pack->entries.entries;

    pack->id = 0;
    for (index = 0; index < pack->entryCount; index++) {
        if (!FixEntryHeader(entry)) {
            return 0;
        }
        entry->info.id = fn_80420EE0(lbl_805F138C, entry->info.name);
        if (entry->info.id != 0) {
            pack->id = entry->info.id;
        }
        fn_80410D40(entry, (PackNode *)(entry + 1));
        entry = NEXT_ENTRY(entry);
    }
    if (pack->id == 0) {
        return 0;
    }

    if (lbl_805F1388 != 0) {
        PackFile *last = lbl_805F1388;

        while (last->next != 0) {
            last = last->next;
        }
        last->next = pack;
    } else {
        lbl_805F1388 = pack;
    }
    return 1;
}

/* Copies the last component of the path, up to its first '.' and its first '_', into name when
 * that is shorter than the 32 bytes of name, and returns whether it did. */
int fn_80410BB4(char *name, const char *path) {
    char buffer[256];
    const char *separator = "/";
    char *p;

    /* Matching workaround: the scan runs on rest, and path is reassigned from it after each match
     * instead of serving as the scan pointer itself. With path alone, the register copies of the
     * two parameters swap and the loop branch moves (b 0x5B8 becomes b 0x5B0). Revisit when the
     * original's loop form is found. */
    const char *rest = path;

    while ((p = strstr(rest, separator)) != 0) {
        rest = p + strlen(separator);
        path = rest;
    }
    strcpy(buffer, path);

    p = strstr(buffer, ".");
    if (p != 0) {
        *p = 0;
    }
    p = strstr(buffer, "_");
    if (p != 0) {
        *p = 0;
    }

    /* Matching workaround: the scratch pointer p, which held the strstr results, is pointed at
     * buffer for the last two calls. The target keeps both the strstr result of the loop
     * (mr. r31,r3) and buffer's address (addi r31,r1,8) in the same callee-saved register. The
     * natural forms all differ: naming buffer in the calls recomputes the address at each call
     * and makes fn_80410BB4 one word short; a separate pointer to buffer differs too (declared up
     * front it changes the code from object offset 0x5A0 on, assigned here it takes another
     * register); an inline helper taking name and buffer recomputes the address (addi r3,r1,8
     * and addi r4,r1,8) at each use. Revisit when the original's local layout is found. */
    p = buffer;
    if (strlen(p) < 32) {
        strcpy(name, p);
        return 1;
    }
    return 0;
}

/* Drops the registrations of a node and of its descendants. The first parameter is not used. */
void fn_80410C7C(PackEntry *entry, PackNode *node) {
    fn_80420CE8(lbl_805F1390, node->name, node->id);
    if (node->first.node != 0) {
        fn_80410C7C(entry, node->first.node);
    }
    if (node->second.node != 0) {
        fn_80410C7C(entry, node->second.node);
    }
}

/* Fixes the byte order of a node, registers its name and does the same for its descendants, whose
 * links it relocates on the way. */
void fn_80410D40(PackEntry *entry, PackNode *node) {
    if (NEEDS_BYTE_SWAP(entry->byteOrderFlag)) {
        fn_804210E4(&node->id, &node->id, 1);
        fn_8042118C(&node->values[0], &node->values[0], 1);
        fn_8042118C(&node->values[1], &node->values[1], 1);
        fn_8042118C(&node->values[2], &node->values[2], 1);
        fn_8042118C(&node->values[3], &node->values[3], 1);
        fn_8042118C(&node->values[4], &node->values[4], 1);
        fn_804210E4(node->rows[0], node->rows[0], 4);
        fn_804210E4(node->rows[1], node->rows[1], 4);
        fn_804210E4(node->rows[2], node->rows[2], 4);
        fn_804210E4(&node->first, &node->first, 1);
        fn_804210E4(&node->second, &node->second, 1);
    }
    fn_80420DA4(lbl_805F1390, node->name, node->id);

    if (node->first.offset != 0) {
        node->first.node = (PackNode *)(node->first.offset + (unsigned int)entry);
        fn_80410D40(entry, node->first.node);
    }
    if (node->second.offset != 0) {
        node->second.node = (PackNode *)(node->second.offset + (unsigned int)entry);
        fn_80410D40(entry, node->second.node);
    }
}

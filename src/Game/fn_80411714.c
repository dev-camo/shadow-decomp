/* Groups of numbered resources (the original file name is unknown; every name here is inferred from
 * the code). A group is a file image or a single block of data that holds items for a range of
 * 16-bit ids. The loaded groups form a list; an item can be read through its group or, for a
 * group whose data lies in ARAM, through a counted copy in main memory. Definitions are in address
 * order, with automatic inlining one level deep (see configure.py).
 *
 * Not complete: fn_80411818 (0x5D4 bytes) and fn_80411DEC (0x5F0 bytes) are not written yet, and
 * fn_804124DC, fn_80413058, fn_80413318 and fn_80413538 do not match yet. */
#include <stddef.h>
#include <string.h>

#include "Game/fn_804110EC_allocator_hooks.h"
#include "Game/fn_80410630_pack.h"
#include "Game/fn_80411714_resource_groups.h"
#include "Game/fn_8042_byte_order.h"
#include "Game/fn_8042_name_registry.h"

/* The uninitialised globals are laid out last-defined first, so they are listed from the highest
 * address down. */
static char nameBuffer[256];
static char blockIteratorInitialised;
static ResourceBlock *currentBlock;
unsigned short lbl_805F13BE;
unsigned short lbl_805F13BC;
unsigned short lbl_805F13BA;
unsigned short lbl_805F13B8;
ResourceGroup *lbl_805F13B0;

/* Returns the item of an id, or 0.
 *
 * Like FindEntry (fn_80410630.c), the original returns early on an empty list before the loop
 * that tests the head again (fn_80412820+0x20: cmplwi, bne, li r3,0 and a branch to the end).
 *
 * Matching workaround: the id is copied into an int (idValue) for the range test. Comparing the
 * unsigned short id directly gives cmplw where the original has cmpw, and adds a branch to
 * fn_804124DC; an int parameter instead of the copy changes the register of the masked id and
 * the branch offsets. Revisit when the original's types for the id are known. */
static inline ResourceItem *FindItem(unsigned short id) {
    ResourceGroup *group = lbl_805F13B0;
    int idValue;

    if (group == 0) {
        return 0;
    }
    idValue = id;
    for (; group != 0; group = group->next) {
        if (group->firstId <= idValue && group->firstId + group->count > idValue) {
            int offset = id - group->firstId;

            if (group->hasInstances) {
                ResourceInstance *instance;

                for (instance = group->instances; instance != 0; instance = instance->next) {
                    if (instance->id == id) {
                        return &instance->item;
                    }
                }
                return 0;
            }
            return &group->items[offset];
        }
    }
    return 0;
}

/* Drops one reference to the copy of an id's item and frees the copy with its last reference. */
static inline void ReleaseInstance(unsigned short id) {
    ResourceGroup *group = lbl_805F13B0;

    if (id == 0) {
        return;
    }
    for (; group != 0; group = group->next) {
        if (group->firstId <= id && id < group->firstId + group->count) {
            ResourceInstance *previous;
            ResourceInstance *instance;

            if (!group->hasInstances) {
                return;
            }
            previous = 0;
            for (instance = group->instances; instance != 0; instance = instance->next) {
                if (instance->id == id) {
                    if (--instance->referenceCount == 0) {
                        if (previous != 0) {
                            previous->next = instance->next;
                        } else {
                            group->instances = instance->next;
                        }
                        fn_804110FC(instance);
                    }
                    return;
                }
                previous = instance;
            }
            return;
        }
    }
}

/* Reverses the bytes of the header and blocks of data written in the opposite byte order; returns
 * 0, without changing anything, for a version below 3. */
static inline int FixDataHeader(ResourceData *data) {
    ResourceBlock *block;

    if (data->version >= 3) {
        if (NEEDS_BYTE_SWAP(data->byteOrderFlag)) {
            fn_804210E4(&data->entryId, &data->entryId, 1);
            fn_804210E4(&data->size, &data->size, 1);
            fn_8042118C(&data->unknown10, &data->unknown10, 1);
            fn_8042118C(&data->unknown12, &data->unknown12, 1);

            block = &data->firstBlock;
            while (block != 0) {
                int elementBytes;

                fn_804210E4(&block->size, &block->size, 1);
                fn_8042118C(&block->count, &block->count, 1);
                /* An int, so that the division is signed. */
                elementBytes = block->size - sizeof(ResourceBlock);
                if (data->flags & 1) {
                    fn_8042118C(block + 1, block + 1, elementBytes / 2);
                } else {
                    fn_804210E4(block + 1, block + 1, elementBytes / 4);
                }
                if (block->hasNext) {
                    block = (ResourceBlock *)((char *)block + block->size);
                } else {
                    block = 0;
                }
            }
        }
        return 1;
    }
    return 0;
}

/* Rounds a size up to a multiple of 32 bytes, the unit of the ARAM transfers. */
static inline int RoundUpTo32(int size) {
    if (size % 32 != 0) {
        size += 32 - size % 32;
    }
    return size;
}

/* Rounds an address down to a multiple of 32 bytes, the alignment of the ARAM transfers. */
static inline char *RoundDownTo32(char *address) {
    unsigned int excess = (unsigned int)address % 32;

    if (excess != 0) {
        address -= excess;
    }
    return address;
}

/* Frees the copies of every group and forgets the current ids that lie in a group with copies. */
void fn_80411714(void) {
    ResourceGroup *group;

    for (group = lbl_805F13B0; group != 0; group = group->next) {
        ResourceInstance *instance;

        if (!group->hasInstances) {
            continue;
        }
        for (instance = group->instances; instance != 0;) {
            ResourceInstance *next = instance->next;

            fn_804110FC(instance);
            instance = next;
        }
        group->instances = 0;

        if (lbl_805F13B8 >= group->firstId && lbl_805F13B8 < group->firstId + group->count) {
            lbl_805F13B8 = 0;
        }
        if (lbl_805F13BA >= group->firstId && lbl_805F13BA < group->firstId + group->count) {
            lbl_805F13BA = 0;
        }
        if (lbl_805F13BC >= group->firstId && lbl_805F13BC < group->firstId + group->count) {
            lbl_805F13BC = 0;
        }
        if (lbl_805F13BE >= group->firstId && lbl_805F13BE < group->firstId + group->count) {
            lbl_805F13BE = 0;
        }
    }
}

/* Returns the entry id of the data of an id's item (0 for an id without an item). */
unsigned int fn_804123DC(unsigned short id) {
    ResourceData *data;
    unsigned int entryId = 0;

    fn_80413318(id);
    data = fn_80412920(id);
    if (data != 0) {
        entryId = data->entryId;
    }
    ReleaseInstance(id);
    return entryId;
}

/* Steps through the blocks of an item's data: with an id it returns the first block, with 0 the one
 * that follows the block returned last. Returns 0 at the end. */
ResourceBlock *fn_804124DC(unsigned short id) {
    if (!blockIteratorInitialised) {
        currentBlock = 0;
        blockIteratorInitialised = 1;
    }

    if (id != 0) {
        ResourceItem *item = FindItem(id);

        if (item != 0) {
            currentBlock = &item->data->firstBlock;
        } else {
            currentBlock = 0;
        }
    } else if (currentBlock != 0) {
        if (currentBlock->hasNext) {
            currentBlock = (ResourceBlock *)((char *)currentBlock + currentBlock->size);
        } else {
            currentBlock = 0;
        }
    }
    return currentBlock;
}

/* Stores the values at 0x10 (in out[0] and out[3]) and 0x12 (in out[1]) of an id's data as floats
 * and returns whether the id has data. */
int fn_804125F4(float *out, unsigned short id) {
    ResourceData *data;
    int found = 0;

    fn_80413318(id);
    data = fn_80412920(id);
    if (data != 0) {
        found = 1;
        out[0] = out[3] = data->unknown10;
        out[1] = data->unknown12;
    }
    ReleaseInstance(id);
    return found;
}

/* Returns the id of the item with the given name, or 0. */
unsigned short fn_80412738(const char *name) {
    int index;
    ResourceGroup *group;

    if (name == 0) {
        return 0;
    }
    if (lbl_805F13B0 == 0) {
        return 0;
    }
    for (group = lbl_805F13B0; group != 0; group = group->next) {
        for (index = 0; index < group->count; index++) {
            if (group->hasInstances) {
                if (strcmp(name, fn_80412820(group->firstId + index)) == 0) {
                    return index + group->firstId;
                }
            } else {
                if (strcmp(name, group->items[index].name) == 0) {
                    return index + group->firstId;
                }
            }
        }
    }
    return 0;
}

/* Returns a copy of the name of an id's item in a buffer that the next call overwrites. It makes,
 * or adds a reference to, the copy of an item in ARAM and does not drop it. */
const char *fn_80412820(unsigned short id) {
    const char *name = 0;
    ResourceItem *item;

    fn_80413318(id);
    item = FindItem(id);
    if (item != 0) {
        name = item->name;
    }
    if (name != 0) {
        strcpy(nameBuffer, name);
        name = nameBuffer;
    }
    return name;
}

/* Returns the data of an id's item, or 0. */
ResourceData *fn_80412920(unsigned short id) {
    ResourceItem *item = FindItem(id);

    if (item != 0) {
        return item->data;
    }
    return 0;
}

/* Unloads the group whose first id is the given one: unlists it, releases the ARAM block and frees
 * the copies of a group with copies (forgetting the current ids in its range), and hands the group
 * to its release function (the data of its item when fn_80412BFC allocated the group, otherwise
 * the group itself). */
void fn_804129D4(unsigned short id) {
    ResourceGroup *group;
    ResourceGroup *previous;

    if (lbl_805F13B0 == 0) {
        return;
    }
    for (group = lbl_805F13B0, previous = 0; group != 0; previous = group, group = group->next) {
        void (*release)(void *);

        if (group->firstId != id) {
            continue;
        }

        if (previous != 0) {
            previous->next = group->next;
        } else {
            lbl_805F13B0 = group->next;
        }
        release = group->release;

        if (group->hasInstances) {
            ResourceInstance *instance = group->instances;

            fn_80421300(group->aramAddress);
            while (instance != 0) {
                ResourceInstance *next = instance->next;

                fn_804110FC(instance);
                instance = next;
            }
            if (lbl_805F13B8 >= group->firstId && lbl_805F13B8 < group->firstId + group->count) {
                lbl_805F13B8 = 0;
            }
            if (lbl_805F13BA >= group->firstId && lbl_805F13BA < group->firstId + group->count) {
                lbl_805F13BA = 0;
            }
            if (lbl_805F13BC >= group->firstId && lbl_805F13BC < group->firstId + group->count) {
                lbl_805F13BC = 0;
            }
            if (lbl_805F13BE >= group->firstId && lbl_805F13BE < group->firstId + group->count) {
                lbl_805F13BE = 0;
            }
        }

        if (group->ownsGroup) {
            if (release != 0) {
                release(group->items->data);
            }
            fn_804110FC(group);
        } else if (release != 0) {
            release(group);
        }
        return;
    }
}

/* Loads a file image whose header is a group; returns the first id of the group, or 0. A true
 * assignId makes the group take the first free ids instead of the ones in the file. The image is
 * handed to its release function when the group cannot be listed. */
unsigned short fn_80412B74(void *image, void (*release)(void *), int assignId) {
    ResourceGroup *group = image;
    unsigned short id;

    if (group == 0) {
        return 0;
    }

    fn_80412D7C(group);
    group->release = release;
    if (assignId) {
        group->firstId = 0;
    }

    id = fn_80413058(group);
    if (id == 0 && release != 0) {
        release(image);
    }
    return id;
}

/* Loads one block of data as a group of a single item, named like the last component of the path
 * up to its first '.'. Returns the id of the item, or 0 when the data is not valid. */
unsigned short fn_80412BFC(void *data, const char *path, void (*release)(void *), int assignId) {
    const char *separator = "/";
    const char *position;
    ResourceGroup *group;
    int size;
    const char *name;
    unsigned short id;

    if (!fn_804139B4(data)) {
        if (release != 0) {
            release(data);
        }
        return 0;
    }

    name = path;
    position = strstr(name, separator);
    while (position != 0) {
        name = position + strlen(separator);
        position = strstr(name, separator);
    }

    position = strstr(name, ".");
    if (position != 0) {
        size = position - name + 1;
    } else {
        size = strlen(name) + 1;
    }

    /* The size is rounded up to a multiple of 4 with 4 - size % 4, which adds 4 bytes when it is
     * one already; that is the original's allocation size. */
    group = fn_80411134(1, size + (4 - size % 4) + offsetof(ResourceGroup, aramAddress) +
                               sizeof(ResourceItem));
    group->count = 1;
    group->ownsGroup = 1;
    group->release = release;
    /* The item lies at 0x14, over the two fields that only a group with copies uses and the word
     * after the header; its name follows at 0x20. */
    group->items = (ResourceItem *)&group->aramAddress;
    group->items->name = (char *)(group->items + 1);
    group->items->data = data;
    strncpy(group->items->name, name, size - 1);
    if (assignId) {
        group->firstId = 0;
    }

    id = fn_80413058(group);
    if (id == 0) {
        if (release != 0) {
            release(data);
        }
        fn_804110FC(group);
    }
    return id;
}

/* Fixes the byte order of a group read from a file image and relocates the offsets in it: the
 * items, their names, data and records. */
void fn_80412D7C(ResourceGroup *group) {
    int index;
    int swap = 0;

    if (NEEDS_BYTE_SWAP(group->byteOrderFlag)) {
        swap = 1;
        fn_8042118C(&group->firstId, &group->firstId, 1);
        fn_8042118C(&group->count, &group->count, 1);
        fn_804210E4(&group->items, &group->items, 1);
    }
    group->items = (ResourceItem *)((char *)group->items + (unsigned int)group);

    for (index = 0; index < group->count; index++) {
        if (swap) {
            fn_804210E4(&group->items[index].name, &group->items[index].name, 1);
            fn_804210E4(&group->items[index].data, &group->items[index].data, 1);
            fn_804210E4(&group->items[index].records, &group->items[index].records, 1);
        }
        group->items[index].name = (char *)group + (unsigned int)group->items[index].name;
        group->items[index].data =
            (ResourceData *)((char *)group + (unsigned int)group->items[index].data);

        FixDataHeader(group->items[index].data);

        if (group->items[index].records != 0) {
            ResourceRecord *record;

            group->items[index].records =
                (ResourceRecord *)((char *)group + (unsigned int)group->items[index].records);
            if (swap) {
                record = group->items[index].records;
                fn_8042118C(record, record, 2);
                while ((short)record->size != -1) {
                    if (record->flags & 0x8000) {
                        fn_8042118C(record + 1, record + 1, 2);
                    } else if (record->flags & 0x4000) {
                        fn_8042118C(record + 1, record + 1, 1);
                    }
                    record = (ResourceRecord *)((char *)record + record->size);
                    fn_8042118C(record, record, 2);
                }
            }
        }
    }
}

/* Appends a group to the list of loaded groups and gives it the ids that its range needs: the
 * first free ones when its first id is 0. Looks up the pack entry id of every item's data by name
 * and stores it in the data. Returns the first id of the group that fn_80413538 returns, or 0
 * when the range overlaps a loaded group, the group is already listed, or no free range exists. */
unsigned short fn_80413058(ResourceGroup *group) {
    ResourceGroup *other;
    ResourceGroup *last;
    int index;

    if (lbl_805F13B0 != 0) {
        for (other = lbl_805F13B0; other != 0; other = other->next) {
            if (other == group) {
                return 0;
            }
        }
    }

    if (group->firstId != 0) {
        if (lbl_805F13B0 != 0) {
            for (other = lbl_805F13B0; other != 0; other = other->next) {
                last = other;
                if (other->firstId > group->firstId &&
                    other->firstId - group->count < group->firstId) {
                    return 0;
                }
                if (other->firstId < group->firstId &&
                    other->firstId + other->count > group->firstId) {
                    return 0;
                }
            }
            last->next = group;
        } else {
            lbl_805F13B0 = group;
        }
    } else if (lbl_805F13B0 != 0) {
        int candidate = 1;
        unsigned short end = 0;

        for (;;) {
            for (other = lbl_805F13B0; other != 0; other = other->next) {
                if (other->firstId <= candidate && other->firstId + other->count > candidate) {
                    end = other->firstId + other->count - 1;
                    candidate = end;
                    break;
                }
            }
            if (other == 0 && candidate - end >= group->count) {
                group->firstId = end + 1;
                break;
            }
            if (candidate == 0xFFFF) {
                return 0;
            }
            candidate++;
        }
        for (other = lbl_805F13B0; other->next != 0; other = other->next) {
        }
        other->next = group;
    } else {
        group->firstId = 1;
        lbl_805F13B0 = group;
    }

    for (index = 0; index < group->count; index++) {
        group->items[index].data->entryId = fn_804106C0(group->items[index].data->name);
    }
    return fn_80413538(group)->firstId;
}

/* Drops one reference to the copy of an id's item. */
void fn_80413240(unsigned short id) {
    ReleaseInstance(id);
}

/* Makes a copy in main memory of an id's item whose data lies in ARAM, or adds a reference to the
 * copy that exists. The three parts of the item are read from ARAM into one block, each from a
 * 32-byte boundary. */
void fn_80413318(unsigned short id) {
    ResourceGroup *group;

    if (id == 0) {
        return;
    }
    for (group = lbl_805F13B0; group != 0; group = group->next) {
        if (group->firstId <= id && id < group->firstId + group->count) {
            ResourceInstance *previous = 0;
            ResourceInstance *instance;
            ResourceSource *sources;
            ResourceSource *source;
            int nameOffset;
            int dataOffset;
            int recordsOffset;
            int nameSize;
            int dataSize;
            int recordsSize;
            unsigned char *copy;
            unsigned char *part;

            if (!group->hasInstances) {
                return;
            }
            instance = group->instances;
            sources = (ResourceSource *)(group + 1);
            for (; instance != 0; instance = instance->next) {
                if (instance->id == id) {
                    instance->referenceCount++;
                    return;
                }
                previous = instance;
            }

            source = sources + (id - group->firstId);
            nameOffset = source->nameAddress % 32;
            dataOffset = source->dataAddress % 32;
            nameSize = nameOffset + source->nameSize;
            dataSize = dataOffset + source->dataSize;
            if (source->recordsAddress != 0) {
                recordsOffset = source->recordsAddress % 32;
            } else {
                recordsOffset = 0;
            }
            recordsSize = source->recordsSize != 0 ? recordsOffset + source->recordsSize : 0;
            nameSize = RoundUpTo32(nameSize);
            dataSize = RoundUpTo32(dataSize);
            recordsSize = RoundUpTo32(recordsSize);

            /* The record, up to 32 bytes to align the first part, then the parts. */
            instance =
                fn_80411134(1, nameSize + dataSize + recordsSize + sizeof(ResourceInstance) + 32);
            if (instance == 0) {
                return;
            }
            instance->referenceCount = 1;
            instance->id = id;

            copy = (unsigned char *)(instance + 1);
            if ((unsigned int)copy % 32 != 0) {
                copy += 32 - (unsigned int)copy % 32;
            }
            fn_804211E4(copy, source->nameAddress - nameOffset, nameSize);
            instance->item.name = (char *)copy + nameOffset;

            part = copy + nameSize;
            fn_804211E4(part, source->dataAddress - dataOffset, dataSize);
            instance->item.data = (ResourceData *)(part + dataOffset);

            if (source->recordsAddress != 0) {
                part += dataSize;
                fn_804211E4(part, source->recordsAddress - recordsOffset, recordsSize);
                instance->item.records = (ResourceRecord *)(part + recordsOffset);
            }

            if (previous != 0) {
                previous->next = instance;
            } else {
                group->instances = instance;
            }
            return;
        }
    }
}

/* Converts a loaded group into a group with copies: the parts of its items are written to one ARAM
 * block (the image from the 32-byte boundary before the first item's name, or, for a group that
 * owns itself, its name and then its data, each from its own 32-byte boundary), a new group
 * records where each part lies (a ResourceSource per item), and the new group takes the place of
 * the old one in the list. A group that fn_80412BFC allocated (it owns itself) hands its data to
 * the release function and is freed; any other group is handed to its release function. Returns
 * the new group, or the old group when there is no memory, the ARAM allocation fails or a group
 * that owns itself has other than one item. */
ResourceGroup *fn_80413538(ResourceGroup *group) {
    ResourceGroup *converted;
    ResourceSource *sources;
    ResourceGroup *other;
    void (*release)(void *);
    int index;

    converted = fn_80411134(1, group->count * sizeof(ResourceSource) + sizeof(ResourceGroup));
    if (converted == 0) {
        return group;
    }
    sources = (ResourceSource *)(converted + 1);

    if (group->ownsGroup) {
        /* A group that owns itself: the name and data of its single item, written one after the
         * other. */
        char *nameBase;
        char *dataBase;
        int nameBytes;
        int dataBytes;

        /* The original returns here without freeing the new group. */
        if (group->count != 1) {
            return group;
        }
        nameBase = RoundDownTo32(group->items->name);
        sources->nameSize = strlen(group->items->name) + 1;
        sources->nameAddress = group->items->name - nameBase;
        nameBytes = RoundUpTo32(group->items->name + sources->nameSize - nameBase);

        dataBase = RoundDownTo32((char *)group->items->data);
        sources->dataSize = group->items->data->size;
        sources->dataAddress = nameBytes + ((char *)group->items->data - dataBase);
        dataBytes = RoundUpTo32((char *)group->items->data + sources->dataSize - dataBase);

        converted->aramAddress = fn_80421368(nameBytes + dataBytes);
        if (converted->aramAddress == 0) {
            fn_804110FC(converted);
            return group;
        }
        fn_80421270(converted->aramAddress, nameBase, nameBytes);
        fn_80421270(converted->aramAddress + nameBytes, dataBase, dataBytes);
    } else {
        /* The parts of all items lie in the image: write it from the first name to the end of the
         * last part. */
        char *base = RoundDownTo32(group->items[0].name);
        char *end = 0;
        int bytes;

        for (index = 0; index < group->count; index++) {
            if (group->items[index].name != 0) {
                sources[index].nameSize = strlen(group->items[index].name) + 1;
                sources[index].nameAddress = group->items[index].name - base;
                if (end < group->items[index].name + sources[index].nameSize) {
                    end = group->items[index].name + sources[index].nameSize;
                }
            }
            if (group->items[index].data != 0) {
                sources[index].dataSize = group->items[index].data->size;
                sources[index].dataAddress = (char *)group->items[index].data - base;
                if (end < (char *)group->items[index].data + sources[index].dataSize) {
                    end = (char *)group->items[index].data + sources[index].dataSize;
                }
            }
            if (group->items[index].records != 0) {
                ResourceRecord *record = group->items[index].records;

                sources[index].recordsSize = sizeof(ResourceRecord);
                while ((short)record->size != -1) {
                    sources[index].recordsSize += record->size;
                    record = (ResourceRecord *)((char *)record + record->size);
                }
                sources[index].recordsAddress = (char *)group->items[index].records - base;
                if (end < (char *)group->items[index].records + sources[index].recordsSize) {
                    end = (char *)group->items[index].records + sources[index].recordsSize;
                }
            }
        }

        bytes = RoundUpTo32(end - base);
        converted->aramAddress = fn_80421368(bytes);
        if (converted->aramAddress == 0) {
            fn_804110FC(converted);
            return group;
        }
        fn_80421270(converted->aramAddress, base, bytes);
    }

    /* The new group copies offsets 0x00-0x13 of the old group (through items), then drops the
     * items, marks itself as a group with copies that does not own itself, and is released with
     * fn_804110FC. The original copies these fields first and then overrides some of them. */
    converted->firstId = group->firstId;
    converted->count = group->count;
    converted->ownsGroup = group->ownsGroup;
    converted->byteOrderFlag = group->byteOrderFlag;
    converted->hasInstances = group->hasInstances;
    converted->unknown07 = group->unknown07;
    converted->release = group->release;
    converted->next = group->next;
    converted->items = group->items;
    converted->items = 0;
    converted->hasInstances = 1;
    converted->ownsGroup = 0;
    converted->release = fn_804110FC;

    /* Turn the offsets in the sources into ARAM addresses (a records offset of 0 means no
     * records). */
    for (index = 0; index < converted->count; index++) {
        sources[index].nameAddress = converted->aramAddress + sources[index].nameAddress;
        sources[index].dataAddress = converted->aramAddress + sources[index].dataAddress;
        if (sources[index].recordsAddress != 0) {
            sources[index].recordsAddress = converted->aramAddress + sources[index].recordsAddress;
        }
    }

    for (other = lbl_805F13B0; other != 0; other = other->next) {
        if (other->next == group) {
            other->next = converted;
            break;
        }
    }
    if (lbl_805F13B0 == group) {
        lbl_805F13B0 = converted;
    }

    release = group->release;
    if (group->ownsGroup) {
        if (release != 0) {
            release(group->items->data);
        }
        fn_804110FC(group);
    } else if (release != 0) {
        release(group);
    }
    return converted;
}

/* Fixes the byte order of the data and returns whether it is valid: it has a supported version and
 * the three lowest flags set. */
int fn_804139B4(ResourceData *data) {
    int valid = 1;

    if (!FixDataHeader(data)) {
        return 0;
    }
    if (!(data->flags & 1)) {
        valid = 0;
    }
    if (!(data->flags & 2)) {
        valid = 0;
    }
    if (!(data->flags & 4)) {
        valid = 0;
    }
    return valid;
}

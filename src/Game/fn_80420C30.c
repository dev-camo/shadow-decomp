/* Library support code at 0x80420C30-0x804210E4 (the original file name is unknown; every name
 * here is inferred from the code): the start-up and shut-down wrappers and a name registry.
 * Definitions are in address order, with automatic inlining one level deep (see configure.py). */
#include <string.h>

#include "Game/fn_80413B30_motion_keys.h"
#include "Game/fn_804110EC_allocator_hooks.h"
#include "Game/fn_8042_name_registry.h"
#include "layout_check.h"

/* The registry is a singly linked list of entries. */
typedef struct NameRegistryEntry {
    /* 0x00 */ unsigned int id;
    /* 0x04 */ char *name;
    /* 0x08 */ unsigned int unknown08; /* set to 0 when an entry is created; never read */
    /* 0x0C */ int referenceCount;
    /* 0x10 */ struct NameRegistryEntry *next;
} NameRegistryEntry;
CHECK_SIZEOF(NameRegistryEntry, 0x14);

struct NameRegistry {
    /* 0x00 */ NameRegistryEntry *first;
};
CHECK_SIZEOF(struct NameRegistry, 0x4);

void fn_80420C30(void) {
    fn_80421500();
}

void fn_80420C50(void) {
    fn_80415784();
    fn_80421560();
}

unsigned int fn_80420C74(NameRegistry *registry, const char *name) {
    NameRegistryEntry *entry;

    if (registry == 0 || name == 0) {
        return 0;
    }

    for (entry = registry->first; entry != 0; entry = entry->next) {
        if (strcmp(entry->name, name) == 0) {
            return entry->id;
        }
    }
    return 0;
}

void fn_80420CE8(NameRegistry *registry, const char *name, unsigned int id) {
    NameRegistryEntry *entry;
    NameRegistryEntry *previous = 0;

    if (registry == 0 || name == 0) {
        return;
    }

    for (entry = registry->first; entry != 0; entry = entry->next) {
        if (strcmp(entry->name, name) == 0 && entry->id == id) {
            if (--entry->referenceCount == 0) {
                if (previous != 0) {
                    previous->next = entry->next;
                } else {
                    registry->first = entry->next;
                }
                fn_804110FC(entry->name);
                fn_804110FC(entry);
            }
            return;
        }
        previous = entry;
    }
}

unsigned int fn_80420DA4(NameRegistry *registry, const char *name, unsigned int id) {
    NameRegistryEntry *entry;
    NameRegistryEntry *last = 0;

    if (registry == 0 || name == 0) {
        return 0;
    }

    if (registry->first == 0) {
        entry = registry->first = fn_80411134(1, sizeof(NameRegistryEntry));
    } else {
        entry = registry->first;
        while (entry != 0) {
            if (strcmp(entry->name, name) == 0 && entry->id == id) {
                entry->referenceCount++;
                return entry->id;
            }
            if (entry->next == 0) {
                last = entry;
                entry = fn_80411134(1, sizeof(NameRegistryEntry));
                break;
            }
            entry = entry->next;
        }
    }

    if (entry != 0) {
        entry->unknown08 = 0;
        entry->name = fn_80411134(1, strlen(name) + 1);
        entry->next = 0;
        strcpy(entry->name, name);
        entry->id = id;
        entry->referenceCount = 1;
        if (last != 0) {
            last->next = entry;
        }
        return entry->id;
    }
    return 0;
}

unsigned int fn_80420EE0(NameRegistry *registry, const char *name) {
    NameRegistryEntry *entry;
    NameRegistryEntry *last = 0;
    unsigned int id;

    if (registry == 0 || name == 0) {
        return 0;
    }

    if (registry->first == 0) {
        entry = registry->first = fn_80411134(1, sizeof(NameRegistryEntry));
    } else {
        entry = registry->first;
        while (entry != 0) {
            if (strcmp(entry->name, name) == 0) {
                entry->referenceCount++;
                return entry->id;
            }
            if (entry->next == 0) {
                last = entry;
                entry = fn_80411134(1, sizeof(NameRegistryEntry));
                break;
            }
            entry = entry->next;
        }
    }

    if (entry != 0) {
        /* The new identifier is the first one, counting up from 1, that no entry uses. */
        for (id = 1; id <= 0xFFFFFFFF; id++) {
            NameRegistryEntry *user;

            for (user = registry->first; user != 0; user = user->next) {
                if (id == user->id) {
                    break;
                }
            }
            if (user == 0) {
                break;
            }
        }

        entry->unknown08 = 0;
        entry->name = fn_80411134(1, strlen(name) + 1);
        entry->next = 0;
        entry->referenceCount = 1;
        strcpy(entry->name, name);
        entry->id = id;
        if (last != 0) {
            last->next = entry;
        }
        return entry->id;
    }
    return 0;
}

void fn_80421050(NameRegistry *registry) {
    NameRegistryEntry *entry;
    NameRegistryEntry *next;

    if (registry == 0) {
        return;
    }

    for (entry = registry->first; entry != 0; entry = next) {
        next = entry->next;
        fn_804110FC(entry->name);
        fn_804110FC(entry);
    }
    fn_804110FC(registry);
}

NameRegistry *fn_804210AC(void) {
    NameRegistry *registry = fn_80411134(1, sizeof(NameRegistry));

    if (registry != 0) {
        registry->first = 0;
    }
    return registry;
}

#ifndef GAME_GADGETTYPEINFO_H
#define GAME_GADGETTYPEINFO_H

#include <dolphin/types.h>

class UnkGadgetOwner_800C9CD0;

/* Per-gadget-type descriptor (name inferred). Each gadget unit defines one, and a central table
 * lists them all. The layout comes from the descriptors in the data; field meanings beyond the
 * name and the factory are unknown, as are the two hooks' parameters. */
struct GadgetTypeInfo {
    /* 0x00 */ const char *name; /* upper-case type name, e.g. "ELEC_EXIT" */
    /* 0x04 */ void (*hookA)(void);
    /* 0x08 */ void (*hookB)(void);
    /* 0x0C */ void (*create)(UnkGadgetOwner_800C9CD0 *owner);
    /* 0x10 */ u32 field_0x10;
    /* 0x14 */ u32 field_0x14;
    /* 0x18 */ u16 id; /* unique per type (inferred) */
    /* 0x1A */ u8 field_0x1A;
    /* 0x1B */ u8 field_0x1B;
    /* 0x1C */ u8 field_0x1C;
    /* 0x20 */ void *field_0x20;
};

#endif

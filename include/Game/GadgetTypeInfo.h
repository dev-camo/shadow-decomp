#ifndef GAME_GADGETTYPEINFO_H
#define GAME_GADGETTYPEINFO_H

#include <dolphin/types.h>
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;
class UnkParamCursor_800CBC20;

/* One entry of a gadget type's parameter table (name inferred), ended by an entry whose type is 0.
 * The names are strings such as "range" or "0:SOFT 1:HARD". Across all tables, type 4 entries keep
 * float values at 0x14, 0x18 and 0x1C (they look like default, minimum and maximum) and types 1-3
 * keep an integer limit at 0x10; these meanings are inferred. */
struct GadgetParamInfo {
    /* 0x00 */ u32 type;
    /* 0x04 */ const char *name;
    /* 0x08 */ u32 field_0x8;
    /* 0x0C */ u32 field_0xC;
    /* 0x10 */ s32 field_0x10;
    /* 0x14 */ float field_0x14;
    /* 0x18 */ float field_0x18;
    /* 0x1C */ float field_0x1C;
    /* 0x20 */ const char *(*getLabel)(UnkParamCursor_800CBC20 *cursor); /* name inferred */
    /* 0x24 */ u32 field_0x24;
};
CHECK_SIZEOF(GadgetParamInfo, 0x28);

/* The usual label callback: returns the name of the entry the cursor points at (behaviour
 * inferred from its body). */
extern "C" const char *fn_800CBC20(UnkParamCursor_800CBC20 *cursor);

/* Per-gadget-type descriptor (name inferred). Each gadget unit defines one, and a central table
 * lists them all. The layout comes from the descriptors in the data; field meanings beyond the
 * name and the factory are unknown, as are the two hooks' parameters. */
struct GadgetTypeInfo {
    /* 0x00 */ const char *name; /* registered type name, e.g. "ELEC_EXIT" */
    /* 0x04 */ void (*hookA)(void);
    /* 0x08 */ void (*hookB)(void);
    /* 0x0C */ void (*create)(UnkGadgetOwner_800C9CD0 *owner);
    /* 0x10 */ u32 field_0x10;
    /* 0x14 */ u32 field_0x14;
    /* 0x18 */ u16 id; /* meaning unknown */
    /* 0x1A */ u8 field_0x1A;
    /* 0x1B */ u8 field_0x1B;
    /* 0x1C */ u8 field_0x1C;
    /* 0x20 */ const GadgetParamInfo *params; /* null or empty if no parameters */
};
CHECK_SIZEOF(GadgetTypeInfo, 0x24);

#endif

#ifndef GAME_UNKPARAMCURSOR_800CBC20_H
#define GAME_UNKPARAMCURSOR_800CBC20_H

#include "Game/GadgetTypeInfo.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* What the label callback of a parameter table entry receives (placeholder name from the shared
 * callback fn_800CBC20, which returns the name of the table's entry at m_index). A callback that
 * names values, such as CircusBall's for "type", returns a label for the value m_value of the
 * entry m_index (behaviour inferred). */
class UnkParamCursor_800CBC20 {
  public:
    /* 0x0 */ const GadgetParamInfo *m_params;
    /* 0x4 */ s32 m_index;
    /* 0x8 */ s32 m_value;
    /* The label callbacks of the E_P_COLI and BK_TANK_COLI gadgets read this float when m_index
     * is 4, their fifth entry (USA (GUPE8P) 0x8025E860 and 0x80321320), and return a "no timer"
     * label unless it is above 0: the value of a float entry (name and meaning inferred from those
     * two reads; the BK_TANK_COLI gadget's fifth entry has the type 4). */
    /* 0xC */ float m_floatValue;
};
/* The size covers the members that the reconstructed callbacks read. Nothing reconstructed creates
 * a cursor, so the full extent is unknown. */
CHECK_SIZEOF(UnkParamCursor_800CBC20, 0x10);

#endif

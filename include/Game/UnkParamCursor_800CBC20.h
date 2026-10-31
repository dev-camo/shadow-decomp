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
};
CHECK_SIZEOF(UnkParamCursor_800CBC20, 0xC);

#endif

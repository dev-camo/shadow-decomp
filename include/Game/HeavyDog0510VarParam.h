#ifndef GAME_HEAVYDOG0510VARPARAM_H
#define GAME_HEAVYDOG0510VARPARAM_H

#include "Game/HeavyDogVarParam.h"
#include <dolphin/types.h>
#include "layout_check.h"

class HeavyDog0510;

/* HeavyDog0510VarParam (class name from RTTI; bases HeavyDogVarParam and BossVarParam): the
 * variables of the HEAVY_DOG_0510 boss. The constructor sets the members below apart from
 * field_0x1F4 and field_0x1F8, which it leaves uninitialised; HeavyDog0510's Init sets
 * field_0x1F4 and field_0x200. The meanings of the values are unknown. The constructor and the
 * destructor are inline; the copies the linker keeps are those of the HEAVY_DOG_0510 unit
 * (0x80254198, 0x802539B8), which call HeavyDogVarParam's in the HEAVY_DOG_0410 unit. */
class HeavyDog0510VarParam : public HeavyDogVarParam {
  public:
    HeavyDog0510VarParam() {
        field_0x1D8 = 100.0f;
        field_0x1DC = 10.0f;
        field_0x1E0 = 5.0f;
        field_0x1E4 = 3;
        field_0x1E8 = 400.0f;
        field_0x1EC = 400.0f;
        field_0x1F0 = 20.0f;
        field_0x1FC = 0.0f;
        field_0x200 = 0;
    }
    virtual ~HeavyDog0510VarParam() {}

    /* 0x1D8 */ float field_0x1D8;
    /* 0x1DC */ float field_0x1DC;
    /* 0x1E0 */ float field_0x1E0;
    /* 0x1E4 */ s32 field_0x1E4;
    /* 0x1E8 */ float field_0x1E8;
    /* 0x1EC */ float field_0x1EC;
    /* 0x1F0 */ float field_0x1F0;
    /* Init stores the length of field_0xEC with its y set to 0 (fn_8040CF50). */
    /* 0x1F4 */ float field_0x1F4;
    /* A float the function at 0x8026BF54 (USA) stores at 0x8026C024 and loads for arithmetic at
     * 0x8026C030. */
    /* 0x1F8 */ float field_0x1F8;
    /* 0x1FC */ float field_0x1FC;
    /* 0x200 */ HeavyDog0510 *field_0x200; /* the boss task, stored by its Init */
};
CHECK_SIZEOF(HeavyDog0510VarParam, 0x204);

#endif

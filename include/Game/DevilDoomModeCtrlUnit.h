#ifndef GAME_DEVILDOOMMODECTRLUNIT_H
#define GAME_DEVILDOOMMODECTRLUNIT_H

#include "Game/BossCtrlUnit.h"
#include "Game/DevilDoomCtrlUnit.h"
#include "Game/DevilDoomVarParam.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The mode control unit of the DEVIL_DOOM boss (class name and bases from RTTI; see
 * DevilDoomCtrlUnit.h and BossCtrlUnit.h). Its own file at 0x8030CC08-0x80310034 (USA) is not
 * reconstructed yet; the boss task creates it. */

/* The constructor is inline: the boss task's copy (0x8030C27C) is the one the linker keeps. Its
 * vtable (0x8055B84C-0x8055B884, USA) points to the destructor, Update and CreateUnit declared
 * below; its other function entries are adjustment thunks. The words after the vtable are
 * CreateUnit's switch table. */
class DevilDoomModeCtrlUnit : public DevilDoomCtrlUnit, public BossModeCtrlUnit {
  public:
    DevilDoomModeCtrlUnit(DevilDoomVarParam *param)
        : BossCtrlUnit(param), DevilDoomCtrlUnit(param), BossModeCtrlUnit(param) {}
    virtual ~DevilDoomModeCtrlUnit();
    virtual bool Update(float arg);
    virtual BossCtrlUnit *CreateUnit(int mode);
};
CHECK_SIZEOF(DevilDoomModeCtrlUnit, 0x28);

#endif

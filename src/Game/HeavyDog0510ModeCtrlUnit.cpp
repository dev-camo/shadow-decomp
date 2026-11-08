/* HeavyDog0510ModeCtrlUnit (class name from RTTI): the control unit of the HEAVY_DOG_0510 boss
 * that runs its current mode through the mode units of HeavyDog0510ModeCtrlUnit.h.
 * BossModeCtrlUnit::Update passes CreateUnit BossVarParam::field_0xC, which it fills from the
 * mode requested in field_0x14 (see BossVarParam). File name inferred from the class. The linker
 * keeps this file's copies of the inline methods and vtables of the mode units and of
 * HeavyDog0510CtrlUnit. The methods below are in reverse address order (deferred inlining emits
 * functions last-defined first). */
#include "Game/BossCtrlUnit.h"
#include "Game/HeavyDog0510CtrlUnit.h"
#include "Game/HeavyDog0510ModeCtrlUnit.h"

BossCtrlUnit *HeavyDog0510ModeCtrlUnit::CreateUnit(int mode) {
    switch (mode) {
    /* There is no case for 2, the mode most Update bodies request when they finish: like 0 and
     * any mode outside 0-12 it creates WAIT through default. */
    case 0:
    default:
        return new HeavyDog0510ModeCtrlUnit_WAIT(m_heavyDog0510Param);
    case 1:
        return new HeavyDog0510ModeCtrlUnit_EDIT(m_heavyDog0510Param);
    case 3:
        return new HeavyDog0510ModeCtrlUnit_CHASE(m_heavyDog0510Param);
    case 4:
        return new HeavyDog0510ModeCtrlUnit_ESCAPE(m_heavyDog0510Param);
    case 5:
        return new HeavyDog0510ModeCtrlUnit_GET_ATK(m_heavyDog0510Param);
    case 6:
        return new HeavyDog0510ModeCtrlUnit_START(m_heavyDog0510Param);
    case 7:
        return new HeavyDog0510ModeCtrlUnit_BELOW_HALF(m_heavyDog0510Param);
    case 8:
        return new HeavyDog0510ModeCtrlUnit_DESTROY(m_heavyDog0510Param);
    case 9:
        return new HeavyDog0510ModeCtrlUnit_ATK_ROCKET(m_heavyDog0510Param);
    case 10:
        return new HeavyDog0510ModeCtrlUnit_ATK_BARRIER(m_heavyDog0510Param);
    case 11:
        return new HeavyDog0510ModeCtrlUnit_ATK_MINE(m_heavyDog0510Param);
    case 12:
        return new HeavyDog0510ModeCtrlUnit_MOVETEST(m_heavyDog0510Param);
    }
}

HeavyDog0510ModeCtrlUnit::~HeavyDog0510ModeCtrlUnit() {}

bool HeavyDog0510ModeCtrlUnit::Update(float arg) {
    return BossModeCtrlUnit::Update(arg);
}

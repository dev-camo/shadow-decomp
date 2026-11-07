#ifndef GAME_HEAVYDOG0510_H
#define GAME_HEAVYDOG0510_H

#include "Game/BossRootTask.h"
#include "Game/HeavyDog0510VarParam.h"
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

/* HeavyDog0510 (class name from RTTI; bases BossRootTask and Task): the boss task of the gadget
 * type "HEAVY_DOG_0510". It owns its variables, a HeavyDog0510VarParam. */
class HeavyDog0510 : public BossRootTask {
  public:
    HeavyDog0510(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag);
    virtual ~HeavyDog0510();
    virtual void Init();

    /* Creates the boss under task layer 14 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* 0x38 */ HeavyDog0510VarParam m_varParam;
};
CHECK_SIZEOF(HeavyDog0510, 0x240);

#endif

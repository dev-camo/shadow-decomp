#ifndef GAME_HEAVYDOG0410_H
#define GAME_HEAVYDOG0410_H

#include "Game/BossRootTask.h"
#include "Game/HeavyDogVarParam.h"
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

/* HeavyDog0410 (class name from RTTI; bases BossRootTask and Task): the boss task of the gadget
 * type "HEAVY_DOG_0410". It owns its variables, a HeavyDog0410VarParam. */
class HeavyDog0410 : public BossRootTask {
  public:
    HeavyDog0410(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag);
    virtual ~HeavyDog0410();
    virtual void Init();

    /* Creates the boss under task layer 14 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* 0x38 */ HeavyDog0410VarParam m_varParam;
};
CHECK_SIZEOF(HeavyDog0410, 0x238);

#endif

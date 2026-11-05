#ifndef GAME_EGGMECA310_H
#define GAME_EGGMECA310_H

#include "Game/BossRootTask.h"
#include "Game/EggMecaVarParam.h"
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

/* EggMeca310 (class name from RTTI; bases BossRootTask and Task): the boss task of the gadget type
 * "EGG_MECA_0310". It owns its variables, an EggMecaVarParam. */
class EggMeca310 : public BossRootTask {
  public:
    EggMeca310(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag);
    virtual ~EggMeca310();
    virtual void Init();

    /* Creates the boss under task layer 14 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* 0x38 */ EggMecaVarParam m_varParam;
};
CHECK_SIZEOF(EggMeca310, 0x288);

#endif

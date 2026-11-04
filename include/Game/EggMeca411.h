#ifndef GAME_EGGMECA411_H
#define GAME_EGGMECA411_H

#include "Game/BossRootTask.h"
#include "Game/EggMecaVarParam.h"
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

/* EggMeca411 (class name from RTTI; bases BossRootTask and Task): the boss task of the gadget type
 * "EGG_MECA_0411". It owns its variables, an EggMecaVarParam. */
class EggMeca411 : public BossRootTask {
  public:
    EggMeca411(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag);
    virtual ~EggMeca411();
    virtual void Init();

    /* Creates the boss under task layer 14 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* 0x38 */ EggMecaVarParam m_varParam;
};
CHECK_SIZEOF(EggMeca411, 0x288);

#endif

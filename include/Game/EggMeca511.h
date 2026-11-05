#ifndef GAME_EGGMECA511_H
#define GAME_EGGMECA511_H

#include "Game/BossRootTask.h"
#include "Game/EggMecaVarParam.h"
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

/* EggMeca511 (class name from RTTI; bases BossRootTask and Task): the boss task of the gadget type
 * "EGG_MECA_0511". It owns its variables, an EggMecaVarParam. */
class EggMeca511 : public BossRootTask {
  public:
    EggMeca511(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag);
    virtual ~EggMeca511();
    virtual void Init();

    /* Creates the boss under task layer 14 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* 0x38 */ EggMecaVarParam m_varParam;
};
CHECK_SIZEOF(EggMeca511, 0x288);

#endif

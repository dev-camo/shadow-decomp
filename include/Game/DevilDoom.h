#ifndef GAME_DEVILDOOM_H
#define GAME_DEVILDOOM_H

#include "Game/BossRootTask.h"
#include "Game/DevilDoomVarParam.h"
#include "layout_check.h"

class UnkGadgetOwner_800C9CD0;

/* DevilDoom (class name from RTTI; bases BossRootTask and Task): the boss task of the gadget type
 * "DEVIL_DOOM". It owns its variables, a DevilDoomVarParam. */
class DevilDoom : public BossRootTask {
  public:
    DevilDoom(Task *parent, UnkGadgetOwner_800C9CD0 *owner, bool flag);
    virtual ~DevilDoom();
    virtual void Init();

    /* Creates the boss under task layer 14 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* 0x38 */ DevilDoomVarParam m_varParam;
};
CHECK_SIZEOF(DevilDoom, 0x670);

#endif

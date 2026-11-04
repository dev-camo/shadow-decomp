#ifndef GAME_PJSLAUNCHER_H
#define GAME_PJSLAUNCHER_H

#include "Game/EffectLaunchInfoUSA.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Effect {
namespace PJS {

/* Class and base names come from RTTI. The constructor (0x8030531C) and the members are not used by
 * the reconstructed units yet; only the destructor is declared. */
class Launcher : public SonicteamUSA::System::Effect::EffectLaunchInfoUSA {
  public:
    virtual ~Launcher();

    /* 0x0C */ u32 field_0xC;
    /* 0x10 */ u32 field_0x10;
};
CHECK_SIZEOF(Launcher, 0x14);

} // namespace PJS
} // namespace Effect

/* Returns a copy of entry index of the table of shared pointers at lbl_80581394 (behaviour inferred
 * from its body). The element type is inferred: the functions reading the table pass its pointers to
 * code in Launcher's part of the executable and to EffectManager::Create as launch infos. */
extern "C" boost::shared_ptr<Effect::PJS::Launcher> fn_801D2CA4(int index);

#endif

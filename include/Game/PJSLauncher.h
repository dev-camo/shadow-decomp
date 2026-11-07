#ifndef GAME_PJSLAUNCHER_H
#define GAME_PJSLAUNCHER_H

#include "Game/EffectLaunchInfoUSA.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
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

namespace Effect {
namespace System {
class EffectElement;
} // namespace System
} // namespace Effect

extern "C" {
/* Returns a copy of entry index of the table of shared pointers at lbl_80581394 (behaviour
 * inferred from its body). The element type is inferred: the functions reading the table pass its
 * pointers to code in Launcher's part of the executable and to EffectManager::Create as launch
 * infos. */
boost::shared_ptr<Effect::PJS::Launcher> fn_801D2CA4(int index);
/* Entry index of the same table: for entries of the table at 0x804D0294 that have a range,
 * fn_801D2DE8 returns at once, without creating or counting, unless the word at 0xAC of
 * lbl_8057D6A0 is in the entry's range. Otherwise it creates the launcher from that table when
 * the entry is empty and then adds 1 to the count at 0x10 of the entry when it is set.
 * fn_801D2D14 subtracts 1 from that count and empties the entry when it reaches 0 (behaviour
 * from their bodies). Weapon::ChaosSpear::ChaosSpear calls the first with 2 before it copies
 * entry 2 and the second with 2 in its destructor. */
void fn_801D2DE8(int index);
void fn_801D2D14(int index);
/* Ask the effect manager's slot 3 for an effect with id and the launch info launcher and return
 * it as a weak pointer, which may be empty. Only when the returned effect locks, fn_80305900
 * keeps matrix as the effect's matrix source and fn_803055F8 copies the matrix into the effect
 * (fn_80425E68); both then clear bit 2 of the effect's flags and, when timeScale is not null,
 * store it as the effect's time-scale source (behaviour from their bodies). */
boost::weak_ptr<Effect::System::EffectElement> fn_80305900(Effect::PJS::Launcher *launcher,
                                                           const Fn8040Matrix34 *matrix, int id,
                                                           const float *timeScale);
boost::weak_ptr<Effect::System::EffectElement> fn_803055F8(Effect::PJS::Launcher *launcher,
                                                           const Fn8040Matrix34 *matrix, int id,
                                                           const float *timeScale);
/* Ask for an effect in the same way without a matrix. Only when the returned effect locks, both
 * clear bit 2 of its flags and store timeScale as above; fn_8030536C first passes position to
 * fn_80425E4C and the pitch and yaw that fn_8040F664 gives for the negated direction, with a zero
 * roll, to fn_80425D94 (behaviour from their bodies; the pointer spellings are inferred). */
boost::weak_ptr<Effect::System::EffectElement> fn_803056F8(Effect::PJS::Launcher *launcher, int id,
                                                           const float *timeScale);
boost::weak_ptr<Effect::System::EffectElement> fn_8030536C(Effect::PJS::Launcher *launcher,
                                                           const Fn8040Vector3 *position,
                                                           const Fn8040Vector3 *direction, int id,
                                                           const float *timeScale);
}

#endif

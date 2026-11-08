#ifndef GAME_PJSLAUNCHER_H
#define GAME_PJSLAUNCHER_H

#include "Game/EffectLaunchInfoUSA.h"
#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Effect {
namespace PJS {

/* Class and base names come from RTTI. fn_80304F40 creates each launcher for an effect file:
 * m_resourceKey (name inferred) is the key fn_800128D8 returned when it registered the file's
 * ResourceOneFile with the resource manager. fn_800128D8 returns -1 when all 100 keys are taken
 * (USA (GUPE8P) 0x80012970), and the result is stored without a test, so the key can be -1.
 * Loader fills field_0x4 (see Loader). fn_801D2DE8 adds 1 to
 * field_0x10 and fn_801D2D14 subtracts 1 (see below); the constructor clears it. */
class Launcher : public SonicteamUSA::System::Effect::EffectLaunchInfoUSA {
  public:
    Launcher(int key, ResourceOneFile *resource);
    virtual ~Launcher();

    /* 0x0C */ int m_resourceKey;
    /* 0x10 */ u32 field_0x10;
};
CHECK_SIZEOF(Launcher, 0x14);

/* Class name from RTTI; its task name is "Launcher Loader". fn_80304F40 creates one for each
 * launcher, under the parent task fn_801EB304 gives for layer 3. When fn_80012408 returns 3 for
 * the launcher's key, Update stores what fn_8001277C returns for that key and index 2 in the
 * launcher's field_0x4 and sets the task's flag 0x01; otherwise it does nothing (behaviour from
 * the bodies). */
class Loader : public Task {
  public:
    Loader(const boost::shared_ptr<Launcher> &launcher);
    virtual ~Loader();
    virtual void Update(float dt);

    /* 0x28 */ boost::shared_ptr<Launcher> m_launcher;
};
CHECK_SIZEOF(Loader, 0x30);

} // namespace PJS
} // namespace Effect

namespace Effect {
namespace System {
class EffectElement;
} // namespace System
} // namespace Effect

extern "C" {
/* Creates a launcher for the effect file at path: registers a new ResourceOneFile for it with the
 * resource manager (fn_800128D8), returns the launcher in a shared pointer whose deleter passes the
 * key to fn_800127F8 and deletes the launcher, and creates a Loader for it. Each object is
 * constructed only when its allocation succeeds (behaviour from its body; the const spelling is
 * inferred). */
boost::shared_ptr<Effect::PJS::Launcher> fn_80304F40(const char *path);
/* True when the launcher has its file data, that is when field_0x4 is not null (behaviour from its
 * body; fn_801D2C64 returns it for set entries of the table at lbl_80581394). */
bool fn_803052B4(const Effect::PJS::Launcher *launcher);
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
/* Ask the effect manager (the virtual function at vtable offset 0xC) for an effect with id and
 * the launch info launcher and return it as a weak pointer, which may be empty. Only when the
 * returned effect locks, fn_80305900 keeps matrix as the effect's matrix source and fn_803055F8
 * copies the matrix into the effect (fn_80425E68); both then clear bit 2 of the effect's flags
 * and, when timeScale is not null, store it as the effect's time-scale source (behaviour from
 * their bodies). */
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
/* Ask for an effect in the same way. Only when the returned effect locks, fn_803054D0 passes
 * position to fn_80425E4C and the x and y of rotation, with a zero z, to fn_80425D94, and
 * fn_803057EC keeps translation and rotation as the effect's translation and rotation sources;
 * both then clear bit 2 of the effect's flags and store timeScale as above (behaviour from their
 * bodies; the pointer spellings are inferred). */
boost::weak_ptr<Effect::System::EffectElement> fn_803054D0(Effect::PJS::Launcher *launcher,
                                                           const Fn8040Vector3 *position,
                                                           const UnkRotation3 *rotation, int id,
                                                           const float *timeScale);
boost::weak_ptr<Effect::System::EffectElement> fn_803057EC(Effect::PJS::Launcher *launcher,
                                                           const Fn8040Vector3 *translation,
                                                           const Fn8040Vector3 *rotation, int id,
                                                           const float *timeScale);
}

#endif

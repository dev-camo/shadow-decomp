#ifndef GAME_DIABLONEFFECT_H
#define GAME_DIABLONEFFECT_H

#include "Game/Diablon.h"
#include "Game/EffectElement.h"
#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace Effect {
namespace PJS {
class Launcher;
} // namespace PJS
} // namespace Effect

namespace Boss {
namespace Diablon {

/* Boss::Diablon::Effect (class name from RTTI, with Task as its only base): a task that holds the
 * launcher through which fn_802DC95C and fn_802DC998 request effects (0x38 bytes, the size that
 * 0x802D9ACC allocates). Its constructor (0x802DCA60) keeps the diablon and stores itself in the
 * diablon's m_effect, its destructor (0x802DC9D8) clears that pointer, and its Update (0x802DC9D4)
 * does nothing. */
class Effect : public Task {
  public:
    virtual ~Effect();
    virtual void Update(float dt);

    /* Ask the launcher (m_launcher) for the effect id, with the float after the one that the
     * diablon's CharInfo::field_0x60 points to as its time-scale source: fn_802DC95C through
     * fn_803056F8, fn_802DC998 at matrix through fn_80305900 (behaviour from their bodies; names
     * and the pointer spelling of matrix inferred). The weak pointer may be empty. */
    boost::weak_ptr< ::Effect::System::EffectElement> fn_802DC95C(int id);
    boost::weak_ptr< ::Effect::System::EffectElement> fn_802DC998(const Fn8040Matrix34 *matrix,
                                                                  int id);

    /* 0x28 */ boost::shared_ptr< ::Effect::PJS::Launcher> m_launcher; /* entry 1 (fn_801D2CA4) */
    /* 0x30 */ boost::shared_ptr<Diablon> m_diablon;
};
CHECK_SIZEOF(Boss::Diablon::Effect, 0x38);

} // namespace Diablon
} // namespace Boss

#endif

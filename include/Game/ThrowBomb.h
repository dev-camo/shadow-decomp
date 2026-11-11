#ifndef GAME_THROWBOMB_H
#define GAME_THROWBOMB_H

#include "Game/fn_80209B58.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class Task;

/* ThrowBomb (class name from RTTI): the interface of the 0xD4-byte ThrowBombImpl objects that
 * fn_80209908 and fn_80209B58 create. ThrowBombImpl's RTTI lists ThrowBomb as its base at offset
 * 0 and its other base at 4, so ThrowBomb holds only the vtable pointer. All six slots of
 * ThrowBomb's vtable (0x80544214) are null, so its functions are pure. The first three are
 * declared, named after ThrowBombImpl's implementations (placeholder names; USA (GUPE8P)
 * addresses); the other three are not declared yet. The return types are not recovered: the
 * calls of fn_8020BFBC and fn_8020BF9C use no result. */
class ThrowBomb {
  public:
    /* ThrowBombImpl's fn_8020BFBC calls it with a matrix that it builds with fn_8040B6F8 and
     * fn_8040B59C, and the implementation reads the twelve floats of a Fn8040Matrix34 from its
     * argument (pointee type inferred from both; const inferred). */
    virtual void fn_8020C194(const Fn8040Matrix34 *matrix) = 0;
    /* fn_80209B58 passes its params here, and HeavyDog0510CtrlUnit_Bomb's constructor a record
     * it builds from a position and a velocity (the const pointer is inferred). */
    virtual void fn_8020BFBC(const UnkParams_80145B58 *params) = 0;
    /* Called by HeavyDog0510CtrlUnit_Bomb's destructor without arguments. */
    virtual void fn_8020BF9C() = 0;
};
CHECK_SIZEOF(ThrowBomb, 0x4);

extern "C" {
/* Creates a ThrowBombImpl with kind and a 0x30-byte task under parent for it (0x80209CA8), as
 * fn_80209B58 does, and returns a weak pointer to the object (behaviour from its body). Each
 * constructor runs only when its own allocation succeeds; the weak pointer is returned either
 * way. The pointee type of the result is inferred: the code is the same for ThrowBomb and
 * ThrowBombImpl. The interface is chosen because fn_80209908 and fn_80209B58 both copy the
 * shared pointer that they build from the new object into a second one (fn_80209248), destroy
 * the first and use only the second, which a conversion from ThrowBombImpl to ThrowBomb
 * explains, and because HeavyDog0510CtrlUnit_Bomb, the one recovered user, calls only virtual
 * functions through it. */
boost::weak_ptr<ThrowBomb> fn_80209908(Task *parent, int kind);
}

#endif

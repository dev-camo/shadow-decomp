#ifndef GAME_PLAYERBARRIER_H
#define GAME_PLAYERBARRIER_H

#include "Game/Task.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace Player {

class PlayerBase;

/* Player::Barrier (class name and base from RTTI): a 0x30-byte task that Player::Effect's
 * implementation creates (Game/PlayerEffect.cpp). Its constructor (USA (GUPE8P) 0x8033DE30)
 * creates a 0x18-byte object at 0x28, to which the three methods below forward (fn_8033E328,
 * fn_8033E370 and fn_8033E3B8). This is a view: the object at 0x28 is not mapped. */
class Barrier : public Task {
  public:
    Barrier(Task *parent, const boost::shared_ptr<PlayerBase> &player);
    virtual ~Barrier();

    void fn_8033DCBC();
    void fn_8033DCE0();
    void fn_8033DD04(int arg0);

    /* 0x28 */ u8 field_0x28[0x30 - 0x28];

  private:
    Barrier(const Barrier &);
    Barrier &operator=(const Barrier &);
};
CHECK_SIZEOF(Barrier, 0x30);

} // namespace Player

#endif

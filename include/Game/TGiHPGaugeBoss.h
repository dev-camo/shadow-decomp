#ifndef GAME_TGIHPGAUGEBOSS_H
#define GAME_TGIHPGAUGEBOSS_H

#include "Game/Task.h"
#include "boost/scoped_ptr.hpp"
#include "layout_check.h"

/* The implementation object, defined in the task's file (class name from the RTTI name of its
 * draw callback's binder). */
class TGiHPGaugeBossImpl;

/* TGiHPGaugeBoss (class name from RTTI): the task of the in-game display that shows a boss's
 * gauge, the CSD project BossGauge. It owns an implementation object and updates it every frame.
 * TGameInfomation's constructor creates it. Member names are inferred. */
class TGiHPGaugeBoss : public Task {
  public:
    TGiHPGaugeBoss(Task *parent);
    virtual ~TGiHPGaugeBoss();
    virtual void Update(float dt);

    /* 0x28 */ boost::scoped_ptr<TGiHPGaugeBossImpl> m_impl;
};
CHECK_SIZEOF(TGiHPGaugeBoss, 0x30);

extern "C" {
/* The gauge's two settings (placeholder names; the parameter and return types are inferred from
 * the bodies). Each stores its argument and returns the value it replaces. fn_800BE2DC sets the
 * value the gauge shows: the scene's frame is the motion's end frame less 1, times 1 less this
 * value. fn_800BE2EC sets whether the gauge is updated and drawn. The implementation's
 * constructor sets them to 1 and false. The EggLastMech boss passes its health divided by 150 to
 * the first on every update, and true to the second from Init and false from its destructor. */
float fn_800BE2DC(float value);
bool fn_800BE2EC(bool value);
}

#endif

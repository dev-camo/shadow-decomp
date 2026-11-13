#ifndef GAME_THROWOBJECT_H
#define GAME_THROWOBJECT_H

#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/scoped_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;
class ThrowObjectImpl;

/* ThrowObject (class name from RTTI, with Task as its only direct base): a task that moves a
 * position under a constant acceleration, sweeps a sphere along each step and passes the sweep's
 * hit list, when there is one, with its attack object to fn_800C4190. ThrowContainer::Container,
 * ThrowVacuumBomb::Bomb and TEnemyVacuumBullet (EnemyVacuumBullet.cpp) derive from it (their RTTI
 * base lists). Its state is a ThrowObjectImpl (class name from the RTTI name of its bound
 * callbacks) that it owns. Method and member names are inferred. */
class ThrowObject : public Task {
  public:
    /* Creates the task under the task manager's parent for layer 11. radius is the radius of the
     * swept sphere and range the summed step length above which Update calls OnRangeOver. When
     * owner has not expired and its fn_80078B44 returns true, the attack object is created for
     * what fn_800B75DC returns for owner, otherwise for owner. */
    ThrowObject(const boost::weak_ptr<CharInfo> &owner, const Fn8040Vector3 &position,
                const Fn8040Vector3 &velocity, const Fn8040Vector3 &acceleration, float radius,
                float range);
    /* When the effect has not expired, requests its stop and releases it. */
    virtual ~ThrowObject();
    /* Advances the position and the velocity by dt and sweeps the sphere along the step. When the
     * sweep returns a hit list, calls OnHit and sets bit 0 of m_flags. Then, when the summed step
     * length is greater than the range, calls OnRangeOver and sets the same bit. Both tests are
     * made at every update. Last, it keeps the position as the previous position. */
    virtual void Update(float dt);
    /* The two notifications of Update; ThrowObject's own do nothing. */
    virtual void OnRangeOver();
    virtual void OnHit();

    /* The owner that the constructor received. Declared non-const; the original qualifiers of
     * this getter and of GetPosition are not known. Declared const, this implementation loads
     * the implementation pointer before it saves the link register, where the original loads it
     * after (2 words each). */
    boost::weak_ptr<CharInfo> GetOwner();
    /* When the effect has not expired, requests its stop and releases it; then keeps the effect
     * that fn_801D2BEC returns for id, with the position and the direction angles as its
     * translation and rotation arguments. */
    void StartEffect(int id);
    Fn8040Vector3 GetPosition();

  private:
    /* 0x28 */ boost::scoped_ptr<ThrowObjectImpl> m_impl;
};
CHECK_SIZEOF(ThrowObject, 0x30);

#endif

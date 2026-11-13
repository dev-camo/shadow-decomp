#ifndef GAME_THROWCONTAINER_H
#define GAME_THROWCONTAINER_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"

class CharInfo;
class GadgetModelInfo;

/* The two functions that create a ThrowContainer::Container, the CONTAINER gadget's container
 * after it is thrown (class in Game/ThrowContainer.cpp). Their original names are unknown.
 *
 * Both allocate 0x80 bytes and, only when the allocation succeeds, construct the container: a
 * ThrowObject that starts at position with the given velocity and acceleration, a swept sphere
 * of radius 5 and the given range. It keeps model and, when the model is present, places it at
 * its own position at every update, rotated by a matrix built by multiplying the y, x and z
 * rotations in that order and scaled by scale; every update turns it further about its x axis
 * by spin times the update's dt. When the sweep
 * returns a hit list, the container passes model and modelIndex to fn_8033E644 and select and
 * value to fn_8033E828. Then, and also when it has moved further than its range, it passes its
 * owner, when the owner still exists, and eventId to fn_8020642C.
 *
 * The signatures are inferred: the bytes give the eight integer register arguments, the stack
 * words and the floats, but not the pointer or reference spellings or the const qualifiers.
 * The owner and the acceleration are passed on to ThrowObject's constructor, and the model is
 * assigned to a model pointer; fn_8027B3E8's caller passes the owner and the position as
 * temporaries. */
extern "C" {
/* The CONTAINER gadget's container calls it when it is thrown, with the thrower, its model, its
 * "model", "select" and "value" parameters, its character info's position, the angles of its
 * grab matrix, its character info's velocity, a scaled vector, an id and 0, 1 and 600. The
 * SECRET CONTAINER gadget's container (Game/ContainerSecret.h) calls it in the same way, with 0,
 * 6 and its "weapon" parameter in place of the three parameters. The result is not kept. */
void fn_8027B338(const boost::weak_ptr<CharInfo> &owner,
                 const boost::shared_ptr<GadgetModelInfo> &model, int modelIndex, int select,
                 int value, const Fn8040Vector3 *position, const UnkRotation3 *angles,
                 const Fn8040Vector3 *velocity, const Fn8040Vector3 &acceleration, int eventId,
                 float spin, float scale, float range);
/* The same with an acceleration of (0, 0, 0) and a range of 600; afterwards it passes the
 * container (null when the allocation failed) and 0x254 to ThrowObject::StartEffect.
 * VacuumContainer::Container's fn_80238184 calls it with the character info that pulls the
 * container in, the model, the container's parameters, the object's position and angles, its
 * argument, an id and two floats. */
void fn_8027B3E8(const boost::weak_ptr<CharInfo> &owner,
                 const boost::shared_ptr<GadgetModelInfo> &model, int modelIndex, int select,
                 int value, const Fn8040Vector3 &position, const UnkRotation3 *angles,
                 const Fn8040Vector3 *velocity, int eventId, float spin, float scale);
}

#endif

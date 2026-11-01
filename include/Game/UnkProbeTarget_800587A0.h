#ifndef GAME_UNKPROBETARGET_800587A0_H
#define GAME_UNKPROBETARGET_800587A0_H

#include "Game/CharInfo.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The probe object that the probe factories at 0x800C7A84-0x800C7EA8 return: the box probe
 * (fn_800C7C34), the sphere probes (fn_800C7DEC, fn_800C7EA8) and fn_800C7A84. The placeholder
 * name is the one its weak and shared pointers already use; the original class name is not known.
 * Only the layout up to the list of touched objects and the virtual methods that the gadgets
 * call (slots 3, 13, 14, 16 and 26) are mapped; the other slots up to 26 are unknown, and so is the
 * size. */
class UnkProbeTarget_800587A0 {
  public:
    virtual void UnkSlot0() = 0;
    virtual void UnkSlot1() = 0;
    virtual void UnkSlot2() = 0;
    /* A pointer to the probe's position (name and behaviour inferred from the CityLaser gadget,
     * which subtracts the nearest point of a segment from the vector it points to). */
    virtual const Fn8040Vector3 *GetPosition() = 0;
    virtual void UnkSlot4() = 0;
    virtual void UnkSlot5() = 0;
    virtual void UnkSlot6() = 0;
    virtual void UnkSlot7() = 0;
    virtual void UnkSlot8() = 0;
    virtual void UnkSlot9() = 0;
    virtual void UnkSlot10() = 0;
    virtual void UnkSlot11() = 0;
    virtual void UnkSlot12() = 0;
    /* Moves the probe to position (behaviour inferred). */
    virtual void SetPosition(const Fn8040Vector3 *position) = 0;
    /* Sets the probe's rotation (name inferred from RainEffect, which passes the address of its
     * rotation vector; the other gadgets do not call this slot). */
    virtual void SetRotation(const Fn8040Vector3 *rotation) = 0;
    virtual void UnkSlot15() = 0;
    /* Sets the probe's size (name inferred): TornadoCollision passes its radius as x and 0.0f as
     * y and z, GravityChangeCollision the three half extents of its box. Both pass -2 as arg1,
     * which is not understood. */
    virtual void SetSize(float x, int arg1, float y, float z) = 0;
    virtual void UnkSlot17() = 0;
    virtual void UnkSlot18() = 0;
    virtual void UnkSlot19() = 0;
    virtual void UnkSlot20() = 0;
    virtual void UnkSlot21() = 0;
    virtual void UnkSlot22() = 0;
    virtual void UnkSlot23() = 0;
    virtual void UnkSlot24() = 0;
    virtual void UnkSlot25() = 0;
    /* Called without arguments right after slot 16 by the GravityChangeCollision gadget (purpose
     * unknown). */
    virtual void UnkSlot26() = 0;

    /* The list of the objects the probe touched (name inferred). */
    UnkProbeList_80021FE4 *GetHits() {
        return &hits;
    }

    /* 0x04 */ u8 field_0x4[0xC - 0x4];
    /* 0x0C */ UnkProbeList_80021FE4 hits;
};

class UnkWorld_80021B08;
class UnkAttack_8005A840;

/* A sphere of a centre and a radius (placeholder name; layout from the constructor at 0x80021C04,
 * which copies the centre to 0x0 and stores the radius at 0xC). */
struct UnkSphere_80021C04 {
    UnkSphere_80021C04();
    UnkSphere_80021C04(const Fn8040Vector3 *center, float radius);

    /* 0x00 */ Fn8040Vector3 m_center;
    /* 0x0C */ float m_radius;
};
CHECK_SIZEOF(UnkSphere_80021C04, 0x10);

extern "C" {
/* The single world object that owns the probes, constructed on first use (behaviour inferred). */
UnkWorld_80021B08 *fn_80021B08(void);
/* Creates a probe sphere at position and returns it (behaviour inferred; flags, arg3, arg4 and
 * arg6 are not understood). */
boost::shared_ptr<UnkProbeTarget_800587A0>
fn_800C7EA8(UnkWorld_80021B08 *world, const Fn8040Vector3 *position, int flags, int arg3, int arg4,
            const boost::weak_ptr<CharInfo> &owner, const Fn8040Vector3 *arg6, float radius);
/* Creates a probe in the world that fn_80021B08 returns (placeholder name; the arguments are not
 * understood). */
boost::shared_ptr<UnkProbeTarget_800587A0> fn_800C7A84(UnkWorld_80021B08 *world,
                                                       const boost::weak_ptr<CharInfo> &owner,
                                                       float arg2, float arg3, int arg4, int arg5,
                                                       int arg6, const Fn8040Vector3 *arg7,
                                                       const Fn8040Vector3 *arg8, int arg9);
/* Creates a box probe for the character info that owner refers to, with the given half extents
 * (behaviour inferred from the bytes: it stores the three vectors arg6, halfExtents and arg7 at
 * 0x0, 0xC and 0x18 of a shape and wraps it like the other probes; arg4, arg5, arg6, arg7 and arg9
 * are not understood, and the world argument is unused). */
boost::shared_ptr<UnkProbeTarget_800587A0>
fn_800C7C34(UnkWorld_80021B08 *world, const boost::weak_ptr<CharInfo> &owner,
            const Fn8040Vector3 *halfExtents, int flags, int arg4, int arg5,
            const Fn8040Vector3 *arg6, const Fn8040Vector3 *arg7, int arg9);
/* Creates a probe sphere of the given radius around the character info that owner refers to, and
 * returns it (behaviour inferred; arg2, arg3, arg4 and arg5 are passed on and not understood). */
boost::shared_ptr<UnkProbeTarget_800587A0> fn_800C7DEC(UnkWorld_80021B08 *world,
                                                       const boost::weak_ptr<CharInfo> &owner,
                                                       int arg2, int arg3, int arg4,
                                                       const Fn8040Vector3 *arg5, float radius);
/* Creates an oriented box probe at position with the given half extents and rotation, and returns
 * it (behaviour inferred from the arguments only: RainEffect passes its position, half extents
 * and rotation; arg4 to arg6 and the five trailing arguments are not understood). The half
 * extents are passed by reference to a copy that the caller builds. */
boost::shared_ptr<UnkProbeTarget_800587A0>
fn_800C7D04(UnkWorld_80021B08 *world, const Fn8040Vector3 *position,
            const Fn8040Vector3 &halfExtents, const Fn8040Vector3 *rotation, int arg4, int arg5,
            int arg6, int arg7, const boost::weak_ptr<CharInfo> &owner, const Fn8040Vector3 *arg9,
            const Fn8040Vector3 *arg10, int arg11);
/* Whether the probe touches the sphere (behaviour inferred). It is false for an expired probe and
 * for one whose byte at 0x2A is zero; otherwise it calls the probe's virtual function at vtable
 * offset 0x7C without arguments and returns whether the one at offset 0x68 returns nonzero, called
 * with the sphere, arg3 and two zero arguments. The world argument is unused and arg3 is not
 * understood. */
bool fn_800C5878(UnkWorld_80021B08 *world, const UnkSphere_80021C04 *sphere,
                 const boost::weak_ptr<UnkProbeTarget_800587A0> &probe, int arg3);
/* Creates the attack object of a probe and returns it (behaviour inferred). */
boost::weak_ptr<UnkAttack_8005A840> fn_800C4374(UnkWorld_80021B08 *world,
                                                boost::shared_ptr<UnkProbeTarget_800587A0> probe);
/* True when one of the probe hits in the list is a character info whose virtual function at
 * vtable offset 0x24 reports true (behaviour inferred). */
bool fn_8015D2EC(UnkProbeList_80021FE4 *hits);
/* Resets the probe pointer (behaviour inferred; the world argument is unused and the pointer is
 * passed by address). */
void fn_800C78B0(UnkWorld_80021B08 *world, boost::shared_ptr<UnkProbeTarget_800587A0> *probe);
/* The character info that the probe belongs to, or an empty pointer (behaviour inferred). */
boost::weak_ptr<CharInfo> fn_800C4568(UnkWorld_80021B08 *world,
                                      boost::weak_ptr<UnkProbeTarget_800587A0> target);
/* Tests the attack along the segment and returns a newly allocated list of what it met, or 0
 * (behaviour inferred); arg2 and arg3 are forwarded with unresolved meanings. */
UnkProbeList_80021FE4 *fn_800C5D74(UnkWorld_80021B08 *world, const UnkSegment_80020A20 *segment,
                                   int arg2, int arg3, UnkProbeList_80021FE4 *hits,
                                   UnkProbePredicate predicate, void *context,
                                   boost::weak_ptr<UnkAttack_8005A840> attack);
}

#endif

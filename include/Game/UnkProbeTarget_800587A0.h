#ifndef GAME_UNKPROBETARGET_800587A0_H
#define GAME_UNKPROBETARGET_800587A0_H

#include "Game/CharInfo.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The probe object that fn_800C7EA8 builds for the TornadoCollision gadget, with the placeholder
 * name its weak and shared pointers already use (in the original it is one of the character
 * collision shapes; which one is not known). Only the layout up to the list of touched objects
 * and the two virtual methods that the gadget calls are mapped; the 13 virtual methods before
 * SetPosition are unknown, and so is the size. */
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
    virtual void UnkSlot14() = 0;
    virtual void UnkSlot15() = 0;
    /* Sets the probe's radius (behaviour inferred from the gadget's "Radius" parameter; the
     * remaining arguments are unknown). */
    virtual void SetRadius(float radius, int arg1, float arg2, float arg3) = 0;

    /* The list of the objects the probe touched (name inferred). */
    UnkProbeList_80021FE4 *GetHits() {
        return &hits;
    }

    /* 0x04 */ u8 field_0x4[0xC - 0x4];
    /* 0x0C */ UnkProbeList_80021FE4 hits;
};

class UnkWorld_80021B08;
class UnkAttack_8005A840;

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
/* Creates a probe sphere of the given radius around the character info that owner refers to, and
 * returns it (behaviour inferred; arg2, arg3, arg4 and arg5 are passed on and not understood). */
boost::shared_ptr<UnkProbeTarget_800587A0> fn_800C7DEC(UnkWorld_80021B08 *world,
                                                       const boost::weak_ptr<CharInfo> &owner,
                                                       int arg2, int arg3, int arg4,
                                                       const Fn8040Vector3 *arg5, float radius);
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
 * (behaviour inferred); arg2 and the four further arguments are passed on and are not
 * understood. */
UnkProbeList_80021FE4 *fn_800C5D74(UnkWorld_80021B08 *world, const UnkSegment_80020A20 *segment,
                                   int arg2, int arg3, int arg4, int arg5, int arg6,
                                   boost::weak_ptr<UnkAttack_8005A840> attack);
}

#endif

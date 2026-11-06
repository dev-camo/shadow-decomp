#ifndef GAME_UNKWORLD_80021B08_H
#define GAME_UNKWORLD_80021B08_H

#include "Game/CharaColliBase.h"
#include "Game/CharInfo.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The world object that owns the probes (placeholder name; see fn_80021B08). The functions below
 * create and query its probes, which are CharaColliBase objects. */
class UnkWorld_80021B08;
struct _stMtnPlayWork;
namespace SonicteamUSA {
namespace System {
namespace CharaColli {
class CharaColliAttack;
} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

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
boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase>
fn_800C7EA8(UnkWorld_80021B08 *world, const Fn8040Vector3 *position, int flags, int arg3, int arg4,
            const boost::weak_ptr<CharInfo> &owner, const Fn8040Vector3 *arg6, float radius);
/* Creates a probe in the world that fn_80021B08 returns (placeholder name; the arguments are not
 * understood). */
boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase>
fn_800C7A84(UnkWorld_80021B08 *world, const boost::weak_ptr<CharInfo> &owner, float arg2,
            float arg3, int arg4, int arg5, int arg6, const Fn8040Vector3 *arg7,
            const UnkRotation3 *arg8, int arg9);
/* Creates a box probe for the character info that owner refers to, with the given half extents
 * (behaviour inferred from the bytes: it stores the three vectors arg6, halfExtents and arg7 at
 * 0x0, 0xC and 0x18 of a shape and wraps it like the other probes; arg4, arg5, arg6, arg7 and arg9
 * are not understood, and the world argument is unused). halfExtents as a const reference is
 * inferred: the ElecCoreWall constructor builds a temporary vector for it after the owner argument
 * and passes its address, which a pointer to a local would also produce. */
boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase>
fn_800C7C34(UnkWorld_80021B08 *world, const boost::weak_ptr<CharInfo> &owner,
            const Fn8040Vector3 &halfExtents, int flags, int arg4, int arg5,
            const Fn8040Vector3 *arg6, const UnkRotation3 *arg7, int arg9);
/* Creates a probe for the character info that owner refers to from a shape that fn_80030A5C builds
 * from position, rotation, arg7 and the three floats (behaviour inferred; the ELEC_BAR gadget
 * passes half its bar length, 2 and 0.5 as the floats, the zero vector lbl_805E07A0 as position and
 * a quarter turn about y as rotation). The world argument is unused; flags, arg4, arg5 and arg7 are
 * passed on and not understood. */
boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase>
fn_800C78D4(UnkWorld_80021B08 *world, const boost::weak_ptr<CharInfo> &owner, int flags, int arg4,
            u16 arg5, const Fn8040Vector3 *position, const UnkRotation3 &rotation, int arg7,
            float arg8, float arg9, float arg10);
/* Creates a probe sphere of the given radius around the character info that owner refers to, and
 * returns it (behaviour inferred; arg2, arg3, arg4 and arg5 are passed on and not understood). */
boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase>
fn_800C7DEC(UnkWorld_80021B08 *world, const boost::weak_ptr<CharInfo> &owner, int arg2, int arg3,
            int arg4, const Fn8040Vector3 *arg5, float radius);
/* Creates an oriented box probe at position with the given half extents and rotation, and returns
 * it (behaviour inferred from the arguments only: RainEffect passes its position, half extents
 * and rotation; arg4 to arg6 and the five trailing arguments are not understood). The half
 * extents, the rotation and arg9 are passed by address; FallRoad builds all three as temporaries
 * in the call, after the weak pointer argument. The reference spelling is inferred: the bytes do
 * not distinguish pointers from references, and the reference form reproduces the constructor
 * (named-local pointer forms differ by 36 words). */
boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase>
fn_800C7D04(UnkWorld_80021B08 *world, const Fn8040Vector3 *position,
            const Fn8040Vector3 &halfExtents, const UnkRotation3 &rotation, int arg4, int arg5,
            int arg6, int arg7, const boost::weak_ptr<CharInfo> &owner, const Fn8040Vector3 &arg9,
            const UnkRotation3 *arg10, int arg11);
/* Whether the probe touches the sphere (behaviour inferred). It is false for an expired probe and
 * for one whose byte at 0x2A is zero; otherwise it calls the probe's virtual function at vtable
 * offset 0x7C without arguments and returns whether the one at offset 0x68 returns nonzero, called
 * with the sphere, arg3 and two zero arguments. The world argument is unused and arg3 is not
 * understood. */
bool fn_800C5878(UnkWorld_80021B08 *world, const UnkSphere_80021C04 *sphere,
                 const boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> &probe,
                 int arg3);
/* Creates the attack object of a probe and returns it (behaviour inferred). */
boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack>
fn_800C4374(UnkWorld_80021B08 *world,
            boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> probe);
/* The attack object of a probe that is a CharaColliBaseCharInfo (its member at 0x38), or an empty
 * pointer for any other probe (behaviour inferred; the world argument is unused). */
boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack>
fn_800C42D0(UnkWorld_80021B08 *world,
            boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> probe);
/* True when one of the probe hits in the list is a character info whose virtual function at
 * vtable offset 0x24 reports true (behaviour inferred). */
bool fn_8015D2EC(SonicteamUSA::System::Collision::CollisionResultList *hits);
/* Resets the probe pointer (behaviour inferred; the world argument is unused and the pointer is
 * passed by address). */
void fn_800C78B0(UnkWorld_80021B08 *world,
                 boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> *probe);
/* The character info that the probe belongs to, or an empty pointer (behaviour inferred). */
boost::weak_ptr<CharInfo>
fn_800C4568(UnkWorld_80021B08 *world,
            boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> target);
/* Tests the attack along the segment and returns a newly allocated list of what it met, or 0
 * (behaviour inferred); arg2 and arg3 are forwarded with unresolved meanings. */
SonicteamUSA::System::Collision::CollisionResultList *
fn_800C5D74(UnkWorld_80021B08 *world, const UnkSegment_80020A20 *segment, int arg2, int arg3,
            SonicteamUSA::System::Collision::CollisionResultList *hits, UnkProbePredicate predicate,
            void *context,
            boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> attack);
/* Creates a probe for the character info that owner refers to from the loaded collision data
 * collision and returns it (behaviour inferred; arg3 and arg4 are passed on and not understood). */
boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase>
fn_800C81F4(UnkWorld_80021B08 *world, const boost::weak_ptr<CharInfo> &owner, void *collision,
            int arg3, int arg4);
/* Builds one of two probe objects (100 or 108 bytes, chosen by the word at 0x14 of the loaded
 * collision data collision) at position and rotation for the character info that owner refers
 * to, registers it with the world and returns it (behaviour inferred; arg4, arg5 and arg6 are
 * passed on and not understood). */
boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase>
fn_800C86A8(UnkWorld_80021B08 *world, void *collision, const Fn8040Vector3 *position,
            const UnkRotation3 *rotation, int arg4, u16 arg5, int arg6,
            const boost::weak_ptr<CharInfo> &owner);
/* Tests the segment from start to end against the world; when it meets something, stores the
 * point in hit (unless hit is null) and returns true (behaviour inferred from its body). */
bool fn_8015D61C(Fn8040Vector3 *hit, const Fn8040Vector3 &start, const Fn8040Vector3 &end);
/* Builds a 0x70-byte probe (constructor at 0x800C3B4C) for the character info that owner refers
 * to from the collision data collision and the motion control motion, registers it with the world
 * and returns it (behaviour inferred from its body; arg4, which the constructor receives as a
 * 16-bit value, and arg5 are passed on and not understood). The TurnOver gadget passes the file
 * that GadgetModelInfo's fn_801616CC returns, the motion of the same part, 3 and 0. Passing owner
 * by const reference is inferred, as for fn_800C86A8. */
boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase>
fn_800C7F7C(UnkWorld_80021B08 *world, const boost::weak_ptr<CharInfo> &owner, void *collision,
            _stMtnPlayWork *motion, u16 arg4, int arg5);
}

#endif

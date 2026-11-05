#ifndef GAME_UNKWORLDPROBE_80021FE4_H
#define GAME_UNKWORLDPROBE_80021FE4_H

#include "Game/DebugDisp.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include "rw/rwcore.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The world probe that the TestNakazawa task calls, with placeholder names: fn_80020D40 returns
 * a world object, and fn_80021FE4 asks it for the objects along a line segment, returning a newly
 * allocated CollisionResultList of 0x1C bytes (nullable). The list is a doubly linked list of
 * nodes whose first eight bytes are a weak pointer to the object found; the original class names
 * of the world, the nodes and the iterator are unknown. */
class UnkWorld_80020D40;
namespace SonicteamUSA {
namespace System {
namespace CharaColli {
class CharaColliBase;
} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA
class UnkProbeHit_8001E9BC;
struct UnkSphere_80021C04;

/* The record supplied to probe predicates (names inferred from its producers). */
struct UnkProbeFilterRecord {
    /* 0x00 */ RwV3d **vertices;
    /* 0x04 */ u32 field_0x4;
    /* 0x08 */ SonicteamUSA::System::CharaColli::CharaColliBase *target;
    /* 0x0C */ u32 flags;
    /* 0x10 */ void *context;
};
CHECK_SIZEOF(UnkProbeFilterRecord, 0x14);
typedef bool (*UnkProbePredicate)(const UnkProbeFilterRecord *record);

/* The object that a node refers to at 0x0C (placeholder name; the class is not identified). The
 * node's destructor calls its vtable slot at 0x2C with 1 (a deleting destructor, inferred) when bit
 * 0x2 of the node's word at 0x20 is clear, the node's word at 0x10 is 0, the pointer is not null
 * and the object's word at 0x14 is -1. The word at 0x1C is a flags word: Meteors tests it with the
 * same bits as a record's flags. It may be the word that CharaColliBase::UnkSlot20 stores at 0x1C
 * of the object at the probe's 0x5C (inferred). Only that word is declared: the checks cover this
 * 0x20-byte view, and the size of the complete object is unknown. */
struct UnkProbeObject_800265A4 {
    /* 0x00 */ u8 field_0x0[0x1C];
    /* 0x1C */ u32 flags;
};
CHECK_OFFSETOF(UnkProbeObject_800265A4, flags, 0x1C);
CHECK_SIZEOF(UnkProbeObject_800265A4, 0x20);

struct UnkProbeNode_80021FE4 {
    /* 0x800265A4: releases the hit's objects and the weak pointer; delete also frees the node with
     * the allocator's own function (behaviour inferred from its body). The list's destructor calls
     * it for each node, and SecurityLaser.cpp and SetWeapon.cpp, like most callers of fn_80021E24,
     * delete the node that function returns with it. */
    ~UnkProbeNode_80021FE4();

    /* 0x00 */ boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> target;
    /* 0x08 */ u8 field_0x8[0xC - 0x8];
    /* 0x0C */ UnkProbeObject_800265A4 *field_0xC;
    /* 0x10: a non-owning pointer to the object of the weak pointer at 0x00. The node constructor
     * (0x800C53F0) copies that weak pointer and, unless it has expired, locks it, stores the raw
     * pointer here and destroys the temporary shared pointer; otherwise it stores 0. */
    /* 0x10 */ SonicteamUSA::System::CharaColli::CharaColliBase *field_0x10;
    /* 0x14 */ UnkProbeHit_8001E9BC *hit;
    /* 0x18 */ UnkProbeNode_80021FE4 *field_0x18;
    /* 0x1C */ UnkProbeNode_80021FE4 *next;
    /* 0x20: a flag word. The node allocator (0x80026518) stores 0 or 1 here, depending on where
     * it took the memory. When bit 0x2 is set, the destructor leaves the objects at 0x0C and 0x14
     * alone (inferred: the node does not own them). What sets that bit is not known. */
    /* 0x20 */ u32 field_0x20;
};
/* Both producers allocate 0x24 bytes per node (0x80021890, 0x800C5C3C). */
CHECK_SIZEOF(UnkProbeNode_80021FE4, 0x24);

/* An iterator over the nodes of the list (behaviour inferred). It has a constructor because the
 * postfix step returns the previous value through a hidden pointer, which MWCC does for a class
 * with a constructor but not for a plain struct of one pointer. */
struct UnkProbeIterator_80027374 {
    UnkProbeIterator_80027374(UnkProbeNode_80021FE4 *start) : node(start) {}

    /* 0x0 */ UnkProbeNode_80021FE4 *node;
};
CHECK_SIZEOF(UnkProbeIterator_80027374, 0x4);

namespace SonicteamUSA {
namespace System {
namespace Collision {

/* The list returned by fn_80021FE4. Its class name comes from the type name
 * boost::detail::sp_counted_base_impl<SonicteamUSA::System::Collision::CollisionResultList *,
 * boost::checked_deleter<SonicteamUSA::System::Collision::CollisionResultList>> (0x804B6C90),
 * which the type info at 0x805E4E20 references. That type info heads the vtable at 0x80522F00,
 * which the count constructor at 0x800CFF5C installs; the counter's dispose function (0x800D1150)
 * deletes the list through 0x800CFF34, which calls this class's destructor (0x80026FB4). */
class CollisionResultList {
  public:
    /* Destroys the nodes (behaviour inferred); delete also frees the list. */
    ~CollisionResultList();
    /* The first node and the end marker, which iteration compares against (behaviour inferred). */
    UnkProbeNode_80021FE4 *fn_8001EE04();
    UnkProbeNode_80021FE4 *fn_8001EDEC();

    /* 0x00 */ u32 field_0x0;
    /* 0x04 */ s32 field_0x4;
    /* 0x08 */ UnkProbeNode_80021FE4 *first;
    /* 0x0C */ UnkProbeNode_80021FE4 *end;
    /* 0x10 */ u8 field_0x10[0x1C - 0x10];
};
CHECK_SIZEOF(CollisionResultList, 0x1C);

} // namespace Collision
} // namespace System
} // namespace SonicteamUSA

extern "C" {
/* The single world object, constructed on first use (behaviour inferred). */
UnkWorld_80020D40 *fn_80020D40(void);
/* Collects the objects along the segment; flags and the three further arguments are passed on to
 * the world's collision tests (behaviour inferred). */
SonicteamUSA::System::Collision::CollisionResultList *
fn_80021FE4(UnkWorld_80020D40 *world, const UnkSegment_80020A20 *segment, int flags, int arg3,
            UnkProbePredicate predicate, void *context);
/* Tests the segment like fn_80021FE4 but keeps only one newly allocated node: each hit replaces
 * the previous one and shortens the segment to its point, so the result is the hit nearest to the
 * segment's start, or null (behaviour inferred from its body). */
UnkProbeNode_80021FE4 *fn_80021E24(UnkWorld_80020D40 *world, const UnkSegment_80020A20 *segment,
                                   int flags, int arg3, UnkProbePredicate predicate, void *context);

/* Collects sphere hits; negativeHits optionally receives negative-distance hits (inferred). */
SonicteamUSA::System::Collision::CollisionResultList *
fn_80021CA4(UnkWorld_80020D40 *world, const UnkSphere_80021C04 *sphere, int flags, int arg3,
            UnkProbePredicate predicate, void *context,
            SonicteamUSA::System::Collision::CollisionResultList *negativeHits);
/* Collects the hits of the sphere moved along motion (behaviour inferred): bits 0 and 1 of flags
 * select its two tests. Bit 0 runs a loop over 0xA8 entries that passes arg5 and !arg8 to
 * fn_80021480; bit 1 calls fn_800C4DE0 with arg4 and, when arg8 is set, arg5 (otherwise 0). The
 * meanings of arg4, arg5 and arg8 are unknown. */
SonicteamUSA::System::Collision::CollisionResultList *
fn_80021934(UnkWorld_80020D40 *world, const UnkSphere_80021C04 *sphere, const Fn8040Vector3 *motion,
            int flags, int arg4, int arg5, UnkProbePredicate predicate, void *context, bool arg8);
Fn8040Vector3 *fn_8001E9C4(UnkProbeHit_8001E9BC *hit);
int fn_8001E9F0(UnkProbeHit_8001E9BC *hit);
void fn_8001E9F8(SonicteamUSA::System::Collision::CollisionResultList *hits);
u32 fn_80026588(UnkProbeNode_80021FE4 *node);
Fn8040Vector3 fn_8001E858(UnkWorld_80020D40 *world, UnkProbeHit_8001E9BC *hit,
                          const Fn8040Vector3 *center, float radius);

/* Moves the iterator to the next pointer of its node (behaviour inferred). */
void fn_80027398(UnkProbeIterator_80027374 *iterator);
/* The postfix form of the same step: returns the iterator's previous value (behaviour inferred
 * from the stores in its body; arg1 is the dummy int of a postfix operator). */
UnkProbeIterator_80027374 fn_80027374(UnkProbeIterator_80027374 *iterator, int arg1);
/* The vector stored at 0xC of the hit (behaviour inferred). */
Fn8040Vector3 *fn_8001E9BC(UnkProbeHit_8001E9BC *hit);
/* A weak pointer to the object a node refers to, built from the weak pointer at the node's start
 * (behaviour inferred). */
boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliBase>
fn_800587A0(const boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> *source);
}

#endif

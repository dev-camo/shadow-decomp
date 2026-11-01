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
 * allocated list of 0x1C bytes (nullable). The list is a doubly linked list of nodes whose first
 * eight bytes are a weak pointer to the object found; the original class names are unknown. */
class UnkWorld_80020D40;
class UnkProbeTarget_800587A0;
class UnkProbeHit_8001E9BC;
struct UnkSphere_80021C04;

/* The record supplied to probe predicates (names inferred from its producers). */
struct UnkProbeFilterRecord {
    /* 0x00 */ RwV3d **vertices;
    /* 0x04 */ u32 field_0x4;
    /* 0x08 */ UnkProbeTarget_800587A0 *target;
    /* 0x0C */ u32 flags;
    /* 0x10 */ void *context;
};
CHECK_SIZEOF(UnkProbeFilterRecord, 0x14);
typedef bool (*UnkProbePredicate)(const UnkProbeFilterRecord *record);

struct UnkProbeNode_80021FE4 {
    /* 0x00 */ boost::weak_ptr<UnkProbeTarget_800587A0> target;
    /* 0x08 */ u8 field_0x8[0x14 - 0x8];
    /* 0x14 */ UnkProbeHit_8001E9BC *hit;
    /* 0x18 */ UnkProbeNode_80021FE4 *field_0x18;
    /* 0x1C */ UnkProbeNode_80021FE4 *next;
};
CHECK_SIZEOF(UnkProbeNode_80021FE4, 0x20);

/* An iterator over the nodes of the list (behaviour inferred). It has a constructor because the
 * postfix step returns the previous value through a hidden pointer, which MWCC does for a class
 * with a constructor but not for a plain struct of one pointer. */
struct UnkProbeIterator_80027374 {
    UnkProbeIterator_80027374(UnkProbeNode_80021FE4 *start) : node(start) {}

    /* 0x0 */ UnkProbeNode_80021FE4 *node;
};
CHECK_SIZEOF(UnkProbeIterator_80027374, 0x4);

/* The list returned by fn_80021FE4. */
class UnkProbeList_80021FE4 {
  public:
    /* Destroys the nodes (behaviour inferred); delete also frees the list. */
    ~UnkProbeList_80021FE4();
    /* The first node and the end marker, which iteration compares against (behaviour inferred). */
    UnkProbeNode_80021FE4 *fn_8001EE04();
    UnkProbeNode_80021FE4 *fn_8001EDEC();

    /* 0x00 */ u32 field_0x0;
    /* 0x04 */ s32 field_0x4;
    /* 0x08 */ UnkProbeNode_80021FE4 *first;
    /* 0x0C */ UnkProbeNode_80021FE4 *end;
    /* 0x10 */ u8 field_0x10[0x1C - 0x10];
};
CHECK_SIZEOF(UnkProbeList_80021FE4, 0x1C);

extern "C" {
/* The single world object, constructed on first use (behaviour inferred). */
UnkWorld_80020D40 *fn_80020D40(void);
/* Collects the objects along the segment; flags and the three further arguments are passed on to
 * the world's collision tests (behaviour inferred). */
UnkProbeList_80021FE4 *fn_80021FE4(UnkWorld_80020D40 *world, const UnkSegment_80020A20 *segment,
                                   int flags, int arg3, UnkProbePredicate predicate, void *context);

/* Collects sphere hits; negativeHits optionally receives negative-distance hits (inferred). */
UnkProbeList_80021FE4 *fn_80021CA4(UnkWorld_80020D40 *world, const UnkSphere_80021C04 *sphere,
                                   int flags, int arg3, UnkProbePredicate predicate, void *context,
                                   UnkProbeList_80021FE4 *negativeHits);
Fn8040Vector3 *fn_8001E9C4(UnkProbeHit_8001E9BC *hit);
int fn_8001E9F0(UnkProbeHit_8001E9BC *hit);
void fn_8001E9F8(UnkProbeList_80021FE4 *hits);
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
boost::weak_ptr<UnkProbeTarget_800587A0>
fn_800587A0(const boost::weak_ptr<UnkProbeTarget_800587A0> *source);
}

#endif

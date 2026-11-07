#ifndef GAME_UNKHANDLEBTARGET_8015BE64_H
#define GAME_UNKHANDLEBTARGET_8015BE64_H

#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;
class GadgetModelInfo;
class UnkPair_8015C9DC;
namespace SonicteamUSA {
namespace System {
namespace CharaColli {
class CharaColliBase;
} // namespace CharaColli
namespace Collision {
class CollisionResultList;
} // namespace Collision
} // namespace System
} // namespace SonicteamUSA

/* What a collision callback receives (class name from RTTI; GadgetColliDetectArgs derives from it
 * at offset 0). */
class ColliDetectArgs {
  public:
    /* Inline: the constructions at 0x80112AFC-0x80112B20, in GadgetColliDetectArgs's constructor
     * (0x8015C0A4), in the Door gadget's IsTouched and in the NOBI_KUSA gadget's UpdateBody
     * (0x8014C108) store the vtable and clear both members in place. */
    ColliDetectArgs() : m_hits(0), field_0x4(0) {}

    /* 0x00 */ SonicteamUSA::System::Collision::CollisionResultList *m_hits;
    /* GadgetModelInfo::fn_80162FB0 stores its own address here (0x80163064); fn_8015BE9C and the
     * other known constructions leave it 0. */
    /* 0x04 */ GadgetModelInfo *field_0x4;
    /* 0x08: vtable */
    /* Inline: GadgetColliDetectArgs's destructor (0x8015C048), the Door gadget's IsTouched and
     * the NOBI_KUSA gadget's UpdateBody (at the end of the scope) restore the vtable in place;
     * 0x80112C74 is an out-of-line copy. */
    virtual ~ColliDetectArgs() {}
};
CHECK_OFFSETOF(ColliDetectArgs, m_hits, 0x0);
CHECK_OFFSETOF(ColliDetectArgs, field_0x4, 0x4);
CHECK_SIZEOF(ColliDetectArgs, 0xC);

/* GadgetModelInfo's collision callbacks (fn_80162FB0 calls them with a ColliDetectArgs it builds
 * on its stack). */
typedef boost::function1<void, ColliDetectArgs &> ColliDetectCallback;
CHECK_SIZEOF(ColliDetectCallback, 0xC);

/* What the handle target passes to its callbacks (class name from RTTI, whose base list names
 * ColliDetectArgs at offset 0). fn_8015BE9C builds one on its stack: the constructor (0x8015C0A4)
 * stores the base's vtable, clears both members and stores the class's own vtable (0x8052AF0C),
 * and 0x8015C048 is the virtual destructor. Neither touches a byte past the base, so no member of
 * its own is assumed (inferred). */
class GadgetColliDetectArgs : public ColliDetectArgs {
  public:
    virtual ~GadgetColliDetectArgs();
};
CHECK_SIZEOF(GadgetColliDetectArgs, 0xC);

/* The handle target's callbacks, a separate instantiation from ColliDetectCallback: the original
 * has two copies of function1's members (the call operator at 0x801656F0 and 0x8015CB14, the
 * destructor at 0x800D0CEC and 0x800F5B08) and of the list1 subscript of the bound calls
 * (0x800D05D0 and 0x800F5238). GadgetModelInfo and the gadgets that set its callbacks use the
 * first copies, the handle target and the gadgets that set its callbacks the second, and
 * fn_8015BE9C, which calls the handle target's callbacks, passes them a GadgetColliDetectArgs. */
typedef boost::function1<void, GadgetColliDetectArgs &> GadgetColliDetectCallback;
CHECK_SIZEOF(GadgetColliDetectCallback, 0xC);

/* The class of the handle target's callback members (placeholder name: the binary does not name
 * it). Its constructor (0x8015C378) only runs function1's default constructor, and its destructor
 * (0x8015C16C) only destroys the function1 at offset 0, passing -1 as for a member: the function1
 * is its one member. */
class UnkCallbackSlot_800F5980 {
  public:
    UnkCallbackSlot_800F5980();
    ~UnkCallbackSlot_800F5980();

    /* Assigns callback to the member and returns the slot (behaviour inferred: it calls function1's
     * assignment with the slot as this). */
    UnkCallbackSlot_800F5980 &fn_800F5980(const GadgetColliDetectCallback &callback);

    /* 0x00 */ GadgetColliDetectCallback m_callback;
};
CHECK_SIZEOF(UnkCallbackSlot_800F5980, 0xC);

/* The array at 0x34 of the handle target (placeholder name): fn_8015C040 reads its count at 0x4.
 * Its constructor (0x8015C2A0) zeroes the count and the buffer pointer. Its clear (0x8015C208,
 * through 0x8015CA3C) destroys the 8-byte elements of the buffer at 0x8 from the last one and
 * zeroes the count. Its destructor (0x8015C1B8, through 0x8015CA94) clears it and frees the buffer
 * (0x8015CAE0). The element type is not recovered. */
class UnkArray_8015C2A0 {
  public:
    UnkArray_8015C2A0();
    ~UnkArray_8015C2A0();
    /* Destroys the elements (name inferred). */
    void Clear();

    /* 0x00 */ u32 field_0x0;
    /* 0x04 */ u32 m_count;
    /* 0x08 */ UnkPair_8015C9DC *m_data;
};
CHECK_SIZEOF(UnkArray_8015C2A0, 0xC);

/* The 0x40-byte object that fn_8015B8C4 creates and returns in a shared pointer (placeholder
 * name); many gadgets call fn_8015B8C4, the entrance among them. Each frame, fn_8015BE9C passes
 * the probe's hits to the callbacks (behaviour inferred): while the probe has hits it calls
 * field_0xC once (setting bit 2 of the flags) and field_0x0 every time, and once the hits are gone
 * it calls field_0x18 and clears bit 2. */
class UnkHandleBTarget_8015BE64 {
  public:
    /* Constructs the members, stores probe and clears the flags and the timer (0x8015C228). */
    UnkHandleBTarget_8015BE64(
        boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> probe);
    /* Clears the array and, when there is a probe, passes it to fn_800C78B0 with the world object
     * of fn_80021B08 before the members are destroyed (0x8015C0CC). */
    ~UnkHandleBTarget_8015BE64();

    /* Matching workaround: as in Effect::System::EffectElement::ClearFlags, calling this inline
     * keeps the smart pointer's operator-> out of line, as in the original. The original accessor is
     * unknown. Revisit when it is identified.
     *
     * Clears the bits of mask in the flags word (name inferred). */
    void ClearFlags(u32 mask) {
        m_flags &= ~mask;
    }

    /* Sets the bits of mask in the flags word (name inferred). Called through this inline, the
     * smart pointer's operator-> stays out of line, as in the original. */
    void SetFlags(u32 mask) {
        m_flags |= mask;
    }

    /* Suspends the processing of the probe for time (name inferred). While bit 0 or bit 1 of the
     * flags is set, fn_8015BE9C skips the probe's hits; while bit 0 is set it counts the float at
     * 0x30 down by its dt and clears bit 0 when it runs out. The Coaster gadget passes 0.4. */
    void SuspendProbe(float time) {
        m_flags |= 1;
        field_0x30 = time;
    }

    /* 0x00 */ UnkCallbackSlot_800F5980 field_0x0;
    /* 0x0C */ UnkCallbackSlot_800F5980 field_0xC;
    /* 0x18 */ UnkCallbackSlot_800F5980 field_0x18;
    /* 0x24 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
    /* 0x2C */ u32 m_flags;
    /* 0x30 */ float field_0x30; /* the time left of the suspension (inferred) */
    /* 0x34 */ UnkArray_8015C2A0 field_0x34;
};
CHECK_OFFSETOF(UnkHandleBTarget_8015BE64, field_0x18, 0x18);
CHECK_OFFSETOF(UnkHandleBTarget_8015BE64, m_probe, 0x24);
CHECK_OFFSETOF(UnkHandleBTarget_8015BE64, m_flags, 0x2C);
CHECK_OFFSETOF(UnkHandleBTarget_8015BE64, field_0x34, 0x34);
CHECK_SIZEOF(UnkHandleBTarget_8015BE64, 0x40);

/* What the walks below that take a callback call for each character info they take from the hits
 * (name inferred). context is the pointer the caller passed to the walk, forwarded unchanged
 * without being read; each callback interprets it as its caller intends, or ignores it. */
typedef void (*ColliHitCallback)(boost::shared_ptr<CharInfo> hit, void *context);

extern "C" {
/* Calls callback with the character infos taken from the hits of args whose fn_80078B4C (vtable
 * offset 0x28) returns true, passing context through (behaviour inferred). Only hits whose
 * collision object and character info still lock are tested, and callback runs only when it is
 * not null. */
void fn_8015AA84(ColliDetectArgs *args, ColliHitCallback callback, void *context);
/* Calls callback with each character info taken from the hits of args whose fn_80078B44 reports
 * true, passing context through (behaviour inferred; fn_8015AA84 makes the same walk, with the
 * same lock and null-callback conditions, with fn_80078B4C). */
void fn_8015AE44(ColliDetectArgs *args, ColliHitCallback callback, void *context);
/* Wraps a probe in a handle target (behaviour inferred). */
boost::shared_ptr<UnkHandleBTarget_8015BE64>
fn_8015B8C4(boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> probe);
void fn_8015BE64(UnkHandleBTarget_8015BE64 *target);
void fn_8015BE9C(UnkHandleBTarget_8015BE64 *target, float dt);
/* Passes the array at 0x34 and an 8-byte pair holding arg0 and arg1 to fn_8015BE00 (purpose
 * unknown; the Fan gadget passes 1 and 2). */
void fn_8015BDC4(UnkHandleBTarget_8015BE64 *target, int arg0, int arg1);
/* True when one of the hits of args whose collision object and character info still lock belongs
 * to a character info whose fn_80078B4C reports true (behaviour inferred; the counterpart of
 * fn_8015D2EC in UnkWorld_80021B08.h for the next virtual function, taking the collision
 * callback's arguments). */
bool fn_8015AC84(ColliDetectArgs &args);
/* The same as fn_8015AC84 except for the filter: true when one of the hits belongs to a character
 * info whose fn_80078B2C (vtable offset 0x18) reports true (behaviour inferred). */
bool fn_8015B1C4(ColliDetectArgs &args);
/* The same as fn_8015AA84 except for the filter: callback runs only for the character infos whose
 * fn_80078B2C (vtable offset 0x18) returns true, where fn_8015AA84 tests fn_80078B4C (0x28); what
 * that function reports is not recovered (behaviour inferred; the CircusBall, DashPanel, DashRing,
 * JumpBoard, LongSpring and Spring gadgets pass their hit callbacks to it). */
void fn_8015B004(ColliDetectArgs *args, ColliHitCallback callback, void *context);
/* The same as fn_8015B004 except for the filter: callback runs only for the hits whose collision
 * object and character info still lock and whose character info's fn_80073634 (vtable offset
 * 0x2C) returns true, and only when callback is not null (behaviour inferred from its body; the
 * HEALING_UNIT gadget's blast passes its hit callback to it). */
void fn_8015B384(ColliDetectArgs *args, ColliHitCallback callback, void *context);
/* True when one of the hits of args whose collision object and character info still lock belongs
 * to a character info whose word at 0x5C equals arg1 and whose word at 0x50 equals arg2; no
 * virtual filter function is tested (behaviour inferred from its body). The reference and the s32
 * arguments are inferred. */
bool fn_8015B544(ColliDetectArgs &args, s32 arg1, s32 arg2);
/* For each hit of args whose collision object and character info still lock, calls callback
 * (when it is not null) with that character info and context if the character info's word at
 * 0x5C equals value; no virtual filter function is tested (behaviour inferred from its body; the
 * CAGE gadget passes 4, and the CAPTURE_CAGE gadget passes 3 (call at 0x800F3570)). The void
 * return and the s32 value are inferred. */
void fn_8015B70C(ColliDetectArgs *args, ColliHitCallback callback, void *context, s32 value);
}

#endif

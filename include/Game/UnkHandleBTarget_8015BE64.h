#ifndef GAME_UNKHANDLEBTARGET_8015BE64_H
#define GAME_UNKHANDLEBTARGET_8015BE64_H

#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class UnkProbeList_80021FE4;
class UnkProbeTarget_800587A0;

/* What a collision callback receives (class name from the callback's RTTI; only the list of the
 * touched objects at 0x0 is mapped, so the size is not known and only that offset is checked). */
class ColliDetectArgs {
  public:
    /* 0x00 */ UnkProbeList_80021FE4 *m_hits;
};
CHECK_OFFSETOF(ColliDetectArgs, m_hits, 0x0);

typedef boost::function1<void, ColliDetectArgs &> ColliDetectCallback;
CHECK_SIZEOF(ColliDetectCallback, 0xC);

/* A callback that a UnkHandleBTarget_8015BE64 calls with the collision query results (placeholder
 * name: the original class is unknown). fn_800F5980 stores a copy of the callback and returns the
 * slot (behaviour inferred); the three words after the callback are not mapped. */
class UnkCallbackSlot_800F5980 {
  public:
    UnkCallbackSlot_800F5980 &fn_800F5980(const ColliDetectCallback &callback);

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

    /* 0x00 */ ColliDetectCallback m_callback;
    /* 0x0C */ UnkProbeTarget_800587A0 *m_probe; /* the Coaster gadget moves it (inferred) */
    /* 0x10 */ u32 field_0x10;
    /* 0x14 */ u32 m_flags; /* bit 1 is cleared by the entrance gadget (inferred) */
};
CHECK_SIZEOF(UnkCallbackSlot_800F5980, 0x18);

/* The object that fn_8015B8C4 returns in a shared pointer (placeholder name); many gadgets call
 * fn_8015B8C4, the entrance among them. It holds two callback slots; only what the entrance reads
 * is mapped, so its size is not known and only the offset of the second slot is checked. */
class UnkHandleBTarget_8015BE64 {
  public:
    /* Suspends the processing of the second slot's probe for time (name inferred). While bit 0
     * or bit 1 of the second slot's flags is set, fn_8015BE9C skips the probe's hits; while bit 0
     * is set it counts the float at 0x30 down by its dt and clears bit 0 when it runs out. The
     * Coaster gadget passes 0.4. */
    void SuspendProbe(float time) {
        field_0x18.m_flags |= 1;
        field_0x30 = time;
    }

    /* 0x00 */ UnkCallbackSlot_800F5980 field_0x0;
    /* 0x18 */ UnkCallbackSlot_800F5980 field_0x18;
    /* 0x30 */ float field_0x30; /* the time left of the suspension (inferred) */
};
CHECK_OFFSETOF(UnkHandleBTarget_8015BE64, field_0x18, 0x18);
CHECK_OFFSETOF(UnkHandleBTarget_8015BE64, field_0x30, 0x30);

extern "C" {
/* Wraps a probe in a handle target (behaviour inferred). */
boost::shared_ptr<UnkHandleBTarget_8015BE64>
fn_8015B8C4(boost::shared_ptr<UnkProbeTarget_800587A0> probe);
void fn_8015BE64(UnkHandleBTarget_8015BE64 *target);
void fn_8015BE9C(UnkHandleBTarget_8015BE64 *target, float dt);
}

#endif

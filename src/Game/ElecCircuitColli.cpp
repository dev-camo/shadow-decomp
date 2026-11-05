/* ElecCircuitColli (class name from RTTI; the gadget's descriptor is "ELEC_CIRCUITCOLLI"): a gadget
 * that creates a box probe with the flags 0x40000 around its character info and, while a character
 * touches the box, passes 0x202 and -1 to the probe's UnkSlot20 unless fn_800B78B0 is true for that
 * character; once nothing touches it, it passes 0x40000 and -1 (behaviour inferred). File name
 * inferred from the class. Method names marked "(name inferred)" are not original. Definitions are
 * in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to the handle target are boost::function objects built with
 * boost::bind(&ElecCircuitColli::method, this, _1); their manager, invoker and copy functions come
 * from the boost headers. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h
 * (alphabetical order would reverse them). */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/ElecCircuitColli.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkCharInfo_800999C0.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

using SonicteamUSA::System::CharaColli::CharaColliBase;

/* What the hit callback receives: the gadget's character info and the gadget itself (placeholder
 * name). */
struct UnkHitContext_8033BB10 {
    /* 0x00 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ ElecCircuitColli *m_owner;
};
CHECK_SIZEOF(UnkHitContext_8033BB10, 0xC);

const char *ElecCircuitColli::s_name = "ElecCircuitColli";

/* The half extents of the box probe (fn_800C7C34's halfExtents), built at startup. */
static Fn8040Vector3 lbl_80585778(100.0f, 100.0f, 45.0f);

/* The callback that fn_8015AA84 calls for each character info in the collision results, with the
 * context of OnCollision. */
static void fn_8033C470(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_8033BB10 *context = static_cast<UnkHitContext_8033BB10 *>(arg);

    if (hit.get() != 0 && context != 0) {
        if (!fn_800B78B0(hit)) {
            context->m_owner->m_handle->m_probe->UnkSlot20(0x202, -1);
        }
    }
}

ElecCircuitColli::ElecCircuitColli(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x5A, 4, "ElecCircuitColli");

    boost::shared_ptr<CharaColliBase> probe =
        fn_800C7C34(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), lbl_80585778, 0x40000, 10,
                    0, &lbl_805E07A0, &lbl_805E07B8, 0);
    m_handle = fn_8015B8C4(probe);

    m_handle->field_0x0.fn_800F5980(
        ColliDetectCallback(boost::bind(&ElecCircuitColli::OnCollision, this, _1)));
    m_handle->field_0x18.fn_800F5980(
        ColliDetectCallback(boost::bind(&ElecCircuitColli::OnCollisionEnd, this, _1)));

    m_handle->ClearFlags(2);
    CopyFromOwner(m_owner);
}

ElecCircuitColli::~ElecCircuitColli() {}

void ElecCircuitColli::UpdateBody(float dt) {
    fn_8015BE9C(m_handle.get(), dt);
}

void ElecCircuitColli::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
}

void ElecCircuitColli::fn_800CBC1C() {
    fn_8033BC78();
}

void ElecCircuitColli::fn_8033BC78() {
    if (m_handle.get() != 0) {
        fn_8015BE64(m_handle.get());
    }
}

void ElecCircuitColli::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void ElecCircuitColli::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_8033BB10 context;

    context.m_charInfo = m_charInfo;
    context.m_owner = this;
    fn_8015AA84(&args, fn_8033C470, &context);
}

void ElecCircuitColli::OnCollisionEnd(ColliDetectArgs &args) {
    m_handle->m_probe->UnkSlot20(0x40000, -1);
}

extern "C" void fn_8033BAD0(void) {}

extern "C" void fn_8033BACC(void) {}

void ElecCircuitColli::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecCircuitColli(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_8055D638 = {
    "ELEC_CIRCUITCOLLI",
    fn_8033BAD0,
    fn_8033BACC,
    ElecCircuitColli::Create,
    0,
    0,
    0x7EC,
    1,
    0,
    10,
    0,
};

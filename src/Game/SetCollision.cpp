/* The SetCollision gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to the handle target are boost::function objects built with
 * boost::bind(&SetCollision::method, this, _1); their manager, invoker and copy functions come
 * from the boost headers. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/SetCollision.h"
#include "Game/CharInfo.h"
#include "Game/DebugMissionClearCollision.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

namespace SetCollision {

const char *SetCollision::s_name = "SetCollision";

SetCollision::SetCollision(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = 0;
    m_charInfo = fn_8015CB88(0, 4, "SetCollision");
    CreateCollision();
    CopyFromOwner(owner);

    if (m_params.appear == 1 && m_handle.get() != 0) {
        m_handle->m_probe->UnkSlot2(-1);
    }
}

SetCollision::~SetCollision() {}

void SetCollision::UpdateBody(float dt) {
    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }

    switch (m_state) {
    case 0:
        if (m_params.appear == 0) {
            m_state = 2;
        } else {
            m_state = 1;
        }
        break;
    case 1:
        if (fn_8016904C(fn_800CB69C(), m_params.id) == 1) {
            m_state = 2;
            if (m_handle.get() != 0) {
                m_handle->m_probe->UnkSlot1(-1);
            }
        }
        break;
    case 2:
        if (m_params.appear == 0 && fn_8016904C(fn_800CB69C(), m_params.id) == 1) {
            m_state = 3;
        }
        break;
    case 3:
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }
}

void SetCollision::fn_800CBC1C(float) {
    if (m_handle.get() != 0) {
        fn_8015BE64(m_handle.get());
    }
}

void SetCollision::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    s32 form = m_params.form;
    s32 kind = m_params.kind;

    CopyFromOwner(owner);

    if (form != m_params.form || kind != m_params.kind) {
        m_handle.reset();
        CreateCollision();
    }

    if (m_handle.get() != 0) {
        m_handle->m_probe->SetSize(m_params.x, -2, m_params.y, m_params.z);
    }
}

void SetCollision::CreateCollision() {
    /* Copied from a temporary through the copy constructor, as the original does. */
    Fn8040Vector3 halfExtents = Fn8040Vector3(1.0f, 1.0f, 1.0f);

    boost::shared_ptr<CharaColliBase> probe;
    int flags = 0;
    int arg = 7;           /* passed after the flags (meaning unknown) */
    s32 charInfoValue = 0; /* stored at 0x50 of the character info */

    switch (m_params.kind) {
    case 0:
        flags = 0x102;
        arg = 7;
        break;
    case 1:
    case 2:
    case 11:
    case 12:
        flags = 0x40100;
        arg = 10;
        break;
    case 3:
        flags = 0x104;
        arg = 7;
        break;
    case 4:
        flags = 0x120;
        arg = 7;
        break;
    case 5:
        flags = 0x40100;
        arg = 10;
        charInfoValue = 0x6F;
        break;
    case 6:
        flags = 0x40100;
        arg = 10;
        charInfoValue = 0x70;
        break;
    case 7:
        flags = 0x40100;
        arg = 10;
        charInfoValue = 0x71;
        break;
    case 8:
        flags = 0x40100;
        arg = 10;
        charInfoValue = 0x72;
        break;
    case 9:
        flags = 0x40100;
        arg = 10;
        charInfoValue = 0x73;
        break;
    case 10:
        flags = 0x40100;
        arg = 10;
        charInfoValue = 0x74;
        break;
    }

    switch (m_params.form) {
    case 0:
        probe = fn_800C7DEC(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), flags, arg, 0,
                            &lbl_805E07A0, 1.0f);
        break;
    case 1:
        probe = fn_800C7C34(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), halfExtents,
                            flags, arg, 0, &lbl_805E07A0, &lbl_805E07B8, 0);
        break;
    case 2:
        probe = fn_800C7A84(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 1.0f, 1.0f, flags,
                            arg, 0, &lbl_805E07A0, &lbl_805E07B8, 0);
        break;
    }

    if (m_charInfo.get() != 0) {
        m_charInfo->SetField0x50(charInfoValue);
    }

    m_handle = fn_8015B8C4(probe);

    switch (m_params.kind) {
    case 1:
        m_handle->field_0x0.fn_800F5980(GadgetColliDetectCallback(
            boost::bind(&SetCollision::OnCollisionNotAvailable, this, _1)));
        break;
    case 2:
        m_handle->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&SetCollision::OnCollisionStart, this, _1)));
        break;
    case 11:
        m_handle->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&SetCollision::OnCollisionGoal, this, _1)));
        break;
    case 12:
        m_handle->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&SetCollision::OnCollisionOff, this, _1)));
        break;
    }

    m_handle->m_probe->SetSize(m_params.x, -2, m_params.y, m_params.z);
}

void SetCollision::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    if (m_charInfo.get() != 0) {
        fn_800C9F14(owner, &m_charInfo->m_position);
        fn_800C9ED4(owner, &m_charInfo->m_rotation);
    }

    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void SetCollision::OnCollisionNotAvailable(ColliDetectArgs &args) {}

void SetCollision::OnCollisionStart(ColliDetectArgs &args) {
    if (m_state == 2 && fn_8015D3E8(args.m_hits)) {
        fn_801690D0(fn_800CB69C(), m_params.id, 1);
        m_state = 3;
    }
}

void SetCollision::OnCollisionGoal(ColliDetectArgs &args) {
    if (m_state == 2 && fn_8015D3E8(args.m_hits)) {
        fn_80175EEC(fn_80074470(), 6, false);
        m_state = 3;
    }
}

void SetCollision::OnCollisionOff(ColliDetectArgs &args) {
    if (m_state == 2 && fn_8015D3E8(args.m_hits)) {
        fn_801690D0(fn_800CB69C(), m_params.id, 0);
        m_state = 3;
    }
}

} // namespace SetCollision

void SetCollision::SetCollision::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetCollision(parent, owner);
}

/* The names of the values of the "kind", "form" and "appear" parameters, and the labels of "X",
 * "Y" and "Z" for each form. */
static const char *s_kindLabels[13] = {
    "Guard",
    "Not Available",
    "Start",
    "Damage",
    "Dead",
    "ChaosControlCancelOn",
    "ChaosControlCancelOff",
    "ChaosControlStop",
    "FlyEnd",
    "LaunchContinue",
    "NoInput",
    "Goal",
    "Off",
};
static const char *s_formLabels[3] = {"Sphere", "Box", "Cylinder"};
static const char *s_xLabels[3] = {"Radius", "Width", "Radius"};
static const char *s_yLabels[3] = {"", "Height", ""};
static const char *s_zLabels[3] = {"", "Depth", "Height"};
static const char *s_appearLabels[3] = {"Disappear", "Appear", "No Use"};

/* The label callback of the descriptor's "kind", "form", "X", "Y", "Z" and "appear" entries: the
 * labels of "X", "Y" and "Z" follow the form last shown (behaviour inferred). The kind last shown
 * is stored but not read. */
static const char *fn_8011CB54(UnkParamCursor_800CBC20 *cursor) {
    static s32 s_kind = 0;
    static s32 s_form = 0;

    if (cursor->m_index == 0) {
        s_kind = cursor->m_value;
        return s_kindLabels[s_kind];
    } else if (cursor->m_index == 1) {
        s_form = cursor->m_value;
        return s_formLabels[s_form];
    } else if (cursor->m_index == 2) {
        return s_xLabels[s_form];
    } else if (cursor->m_index == 3) {
        return s_yLabels[s_form];
    } else if (cursor->m_index == 4) {
        return s_zLabels[s_form];
    } else if (cursor->m_index == 6) {
        return s_appearLabels[cursor->m_value];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "kind", 0, 0, 12, 0.0f, 0.0f, 0.0f, fn_8011CB54, 0},
    {1, "form", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_8011CB54, 0},
    {4, "X", 0, 0, 0, 10.0f, 0.1f, 65536.0f, fn_8011CB54, 0},
    {4, "Y", 0, 0, 0, 10.0f, 0.1f, 65536.0f, fn_8011CB54, 0},
    {4, "Z", 0, 0, 0, 10.0f, 0.1f, 65536.0f, fn_8011CB54, 0},
    {1, "Id", 0, 0, 0xFF, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "appear", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_8011CB54, 0},
    {0},
};

GadgetTypeInfo lbl_80527440 = {
    "SETCOLLISION", 0, 0, SetCollision::SetCollision::Create, 0, 0, 0x50, 1, 0, 10, s_paramInfo,
};

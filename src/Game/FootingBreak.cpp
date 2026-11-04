/* FootingBreak (class name from RTTI; the gadget's descriptor has the same name): a footing that
 * hovers until the state of its com id becomes 1, waits, and then moves one part by
 * "MoveLength" while the other part stays and hovers; for type 0 (Burst) it then breaks with the
 * clump A10603FOOTINGBREAKFLOAT_BRK.DFF, for type 1 (Hover) both parts keep hovering (behaviour
 * inferred). File name inferred from the class. Method names marked "(name inferred)" are not
 * original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include <cmath>
#include <cstring>
/* TNumEdit.h comes before CharaColliBase.h, in the order their names appear in the unit's .rodata
 * (alphabetical order would reverse them). */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBase.h"
#include "Game/FootingBreak.h"
#include "Game/CharCommandUnit.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/SharedConstants.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

/* Places position on the way from start by delta, eased with a half cosine over the duration; at
 * the end once the time reaches the duration or the duration is 0 (name inferred). */
static void ApplyMove(Fn8040Vector3 *position, const Fn8040Vector3 *start,
                      const Fn8040Vector3 *delta, float time, float duration) {
    if (time >= duration || duration == 0.0f) {
        Fn8040Vector3 end;

        fn_8040CDD0(&end, start, delta);
        fn_800091D0(position, &end);
    } else {
        float phase = time / duration;
        float wave = std::cos(phase * lbl_805FAD38);
        Fn8040Vector3 moved;
        Fn8040Vector3 offset;

        fn_8040CDA8(&offset, delta, 0.5f * (1.0f - wave));
        fn_8040CDD0(&moved, start, &offset);
        fn_800091D0(position, &moved);
    }
}

/* Places position at base, raised by half the height times the sine of the time over the period
 * (scaled to half a turn); just at base when the period is about 0 (name inferred). */
static void ApplyHover(Fn8040Vector3 *position, const Fn8040Vector3 *base, float height, float time,
                       float period) {
    if (fn_8000A950(period)) {
        fn_800091D0(position, base);
    } else {
        float phase = time / period;
        float wave = std::sin(phase * lbl_805FAD38);
        float offset = 0.5f * (wave * height);

        fn_800091D0(position, base);
        position->y += offset;
    }
}

/* The clump that is shown when the footing breaks (name inferred). */
static const char *s_breakClumpName = "A10603FOOTINGBREAKFLOAT_BRK.DFF";

const char *FootingBreak::s_name = "FootingBreak";

FootingBreak::FootingBreak(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_alpha(1.0f), m_alpha2(1.0f), m_state(0) {
    m_name = s_name;

    m_charInfo = fn_8015CD90(0x6B, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&FootingBreak::OnCommand, this, _1));

    m_charInfo2 = fn_8015CD90(0x6C, 4);
    fn_8015CF7C(m_charInfo2.get());
    m_charInfo2->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&FootingBreak::OnCommand2, this, _1));

    ReadOwner(owner);
    m_model = fn_8015F800(1, "FOOTINGBREAK.BIN");
    m_model2 = fn_8015F800(1, "FOOTINGBREAK.BIN");
    SetState(1);
}

FootingBreak::~FootingBreak() {
    m_sound.fn_801D455C();
}

extern "C" void fn_80262DF0(void) {
    fn_8006B474("FOOTINGBREAK.BIN", 1);
}

extern "C" void fn_80262DEC(void) {}

void FootingBreak::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new FootingBreak(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void FootingBreak::UpdateBody(float dt) {
    switch (m_state) {
    case 1:
        m_hoverTime += dt;
        ApplyHover(&m_charInfo->m_position, &m_hoverBase, m_params.hoverLength, m_hoverTime,
                   m_params.hoverSec);
        if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
            SetState(2);
        }
        break;

    case 2:
        m_timer += dt;
        m_hoverTime += dt;
        ApplyHover(&m_charInfo->m_position, &m_hoverBase, m_params.hoverLength, m_hoverTime,
                   m_params.hoverSec);
        if (m_timer >= m_params.waitSec) {
            fn_801D54F4(fn_80053164(), 0x773E, &m_charInfo->m_position, 0, 1);
            SetState(3);
        }
        break;

    case 3:
        if (m_target.get() == 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
            m_target->fn_80167448(-1);
        }
        m_timer += dt;
        if (m_timer >= 2.0f) {
            fn_801D54F4(fn_80053164(), 0x773E, &m_charInfo->m_position, 0, 1);
            SetState(4);
            UpdateModel2Matrix();
            m_model2->fn_80162FB0(dt);
        }
        break;

    case 4:
        if (m_target.get() == 0) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;

            weakTarget.ptr = m_target;
            m_model2->fn_8015FA78(fn_80169D00, &weakTarget);
            m_target->fn_80167448(-1);
        }
        m_timer += dt;
        ApplyMove(&m_charInfo->m_position, &m_moveStart, &m_moveDelta, m_timer, m_params.moveSec);
        m_hoverTime += dt;
        ApplyHover(&m_charInfo2->m_position, &m_hoverBase, m_params.hoverLength, m_hoverTime,
                   m_params.hoverSec);
        if (m_timer >= m_params.moveSec) {
            if (m_params.type == 0) {
                SetState(5);
            } else {
                SetState(7);
            }
        }
        UpdateModel2Matrix();
        m_model2->fn_80162FB0(dt);
        break;

    case 5:
        m_timer += dt;
        m_hoverTime += dt;
        ApplyHover(&m_charInfo2->m_position, &m_hoverBase, m_params.hoverLength, m_hoverTime,
                   m_params.hoverSec);
        if (m_timer >= 1.0f) {
            fn_801D54F4(fn_80053164(), 0x773E, &m_charInfo->m_position, 0, 1);
            SetState(6);
        }
        UpdateModel2Matrix();
        m_model2->fn_80162FB0(dt);
        break;

    case 6:
        m_hoverTime += dt;
        ApplyHover(&m_charInfo2->m_position, &m_hoverBase, m_params.hoverLength, m_hoverTime,
                   m_params.hoverSec);
        UpdateModel2Matrix();
        m_model2->fn_80162FB0(dt);
        break;

    case 7:
        m_hoverTime2 += dt;
        ApplyHover(&m_charInfo->m_position, &m_hoverBase2, m_params.hoverLength, m_hoverTime2,
                   m_params.hoverSec);
        m_hoverTime += dt;
        ApplyHover(&m_charInfo2->m_position, &m_hoverBase, m_params.hoverLength, m_hoverTime,
                   m_params.hoverSec);
        UpdateModel2Matrix();
        m_model2->fn_80162FB0(dt);
        break;
    }

    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void FootingBreak::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    FootingBreakParams previous = m_params;
    u8 previousComId = m_comId;
    Fn8040Vector3 previousPosition;

    fn_800091EC(&previousPosition, &m_charInfo->m_position);
    ReadOwner(owner);
    if (memcmp(&previous, &m_params, sizeof(m_params)) != 0 || previousComId != m_comId ||
        !fn_8000AD08(&previousPosition, &m_charInfo->m_position)) {
        SetState(1);
    }

    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();

    /* Draws debug lines along the move and from half the hover length below the position up to
     * the position (behaviour inferred). */
    Fn8040Vector3 hoverLow;
    Fn8040Vector3 hoverHigh;
    Fn8040Vector3 moveEnd;
    Fn8040Vector3 move(m_params.moveLengthX, m_params.moveLengthY, m_params.moveLengthZ);

    fn_8040CDD0(&moveEnd, &m_charInfo->m_position, &move);

    UnkSegment_80020A20 moveLine(m_charInfo->m_position, moveEnd);

    fn_802E0AA0(moveLine, lbl_805F06D8, 5.0f);

    fn_800091EC(&hoverLow, &m_charInfo->m_position);
    fn_800091EC(&hoverHigh, &m_charInfo->m_position);
    hoverLow.y -= 0.5f * m_params.hoverLength;
    hoverHigh.y = hoverLow.y + 0.5f * m_params.hoverLength;

    UnkSegment_80020A20 hoverLine(hoverLow, hoverHigh);

    fn_802E0AA0(hoverLine, UnkDebugColor_802E0E84(0xC0, 0xFF, 0xFF, 0x00), 5.0f);
}

void FootingBreak::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_comId);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

void FootingBreak::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    m_model->fn_801633DC(&matrix);
}

void FootingBreak::UpdateModel2Matrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo2->m_position);
    fn_8040B848(&matrix, m_charInfo2->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo2->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo2->m_rotation.z);
    m_model2->fn_801633DC(&matrix);
}

void FootingBreak::SetState(int state) {
    switch (m_state) {
    case 3:
    case 5:
        m_sound.fn_801D455C();
        break;
    }

    switch (state) {
    case 1:
        m_hoverTime = 0.0f;
        fn_800C9F14(m_owner, &m_hoverBase);
        break;

    case 2:
        m_timer = 0.0f;
        break;

    case 3:
        m_timer = 0.0f;
        m_sound = fn_801D3E58(0x773D, &m_charInfo->m_position, this);
        break;

    case 4: {
        Fn8040Vector3 end;
        Fn8040Vector3 move(m_params.moveLengthX, m_params.moveLengthY, m_params.moveLengthZ);
        Fn8040Vector3 delta;

        fn_800091D0(&m_moveStart, &m_charInfo->m_position);
        fn_800C9F14(m_owner, &end);
        fn_8040CFF8(&end, &move);
        fn_8040CD74(&delta, &end, &m_moveStart);
        fn_800091D0(&m_moveDelta, &delta);
        m_timer = 0.0f;
        fn_800091D0(&m_charInfo2->m_position, &m_charInfo->m_position);
        m_charInfo2->m_rotation = m_charInfo->m_rotation;
        break;
    }

    case 5:
        m_timer = 0.0f;
        m_sound = fn_801D3E58(0x773D, &m_charInfo->m_position, this);
        break;

    case 7:
        fn_800C9F14(m_owner, &m_hoverBase2);
        m_hoverBase2.x += m_params.moveLengthX;
        m_hoverBase2.y += m_params.moveLengthY;
        m_hoverBase2.z += m_params.moveLengthZ;
        m_hoverTime2 = 0.0f;
        break;
    }

    SetupModels(m_state, state);
    m_state = state;
}

void FootingBreak::SetupModels(int previousState, int state) {
    switch (previousState) {
    case 1:
    case 2:
        m_model->fn_80161880();
        m_model->fn_80162804();
        break;

    case 3:
        m_model->fn_801604FC();
        m_model->fn_80161880();
        m_model->fn_80162804();
        if (m_target.get() != 0) {
            m_target->fn_80166F88();
            m_target.reset();
        }
        break;

    case 5:
        m_model->fn_801604FC();
        m_model->fn_80161880();
        m_model->fn_80162804();
        m_model2->fn_80161880();
        m_model2->fn_80162804();
        break;

    case 4:
        if (m_target.get() != 0) {
            m_target->fn_80166F88();
            m_target.reset();
        }
        m_model->fn_80161880();
        m_model->fn_80162804();
        m_model2->fn_80161880();
        m_model2->fn_80162804();
        break;

    case 7:
        m_model->fn_80161880();
        m_model->fn_80162804();
        m_model2->fn_80161880();
        m_model2->fn_80162804();
        break;

    case 6:
        m_model2->fn_80161880();
        m_model2->fn_80162804();
        break;
    }

    switch (state) {
    case 1:
    case 2:
        m_model->fn_80162A68(0, 1);
        m_model->fn_80161FC8(0, m_charInfo);
        m_model->fn_80161FC8(1, m_charInfo);
        break;

    case 3:
        m_model->fn_80162A68(0, 1);
        m_model->fn_80160C28(0);
        m_model->fn_8016082C(0);
        m_model->fn_80161BD8(0, m_charInfo, 0);
        m_model->fn_80161BD8(1, m_charInfo, 0);
        m_model->fn_80160B18(0);
        m_target.reset();
        break;

    case 5:
        m_model->fn_80162A68(1, 1);
        m_model->fn_80160C28(1);
        m_model->fn_8016082C(1);
        m_model->fn_80161BD8(2, m_charInfo, 1);
        m_model->fn_80161BD8(3, m_charInfo, 1);
        m_model->fn_80160B18(1);
        m_model2->fn_80162A68(2, 1);
        m_model2->fn_80161FC8(4, m_charInfo2);
        break;

    case 4:
        m_model->fn_80162A68(1, 1);
        m_model->fn_80161FC8(2, m_charInfo);
        m_model->fn_80161FC8(3, m_charInfo);
        m_model2->fn_80162A68(2, 1);
        m_model2->fn_80161FC8(4, m_charInfo2);
        m_target.reset();
        break;

    case 7:
        m_model->fn_80162A68(1, 0);
        m_model->fn_80161FC8(2, m_charInfo);
        m_model->fn_80161FC8(3, m_charInfo);
        m_model2->fn_80162A68(2, 0);
        m_model2->fn_80161FC8(4, m_charInfo2);
        break;

    case 6: {
        m_model2->fn_80162A68(2, 0);
        m_model2->fn_80161FC8(4, m_charInfo2);

        UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

        effect.field_0x44 = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, s_breakClumpName));
        effect.field_0x40 = m_charInfo->m_position.y;
        effect.field_0x34 = 1.0f;
        effect.field_0x4A = 0;
        effect.field_0x54 = 1;
        fn_8015D93C(&effect);
        break;
    }
    }
}

void FootingBreak::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x17: {
        SetTransparentCommand &transparent = static_cast<SetTransparentCommand &>(command);

        m_alpha = transparent.m_alpha;
        if (transparent.m_enable) {
            fn_8015FDD8(m_model.get(), m_alpha);
        } else {
            fn_8015FD10(m_model.get());
        }
        break;
    }
    }
    command.m_flags |= 2;
}

void FootingBreak::OnCommand2(CharCommand &command) {
    switch (command.m_type) {
    case 0x17: {
        SetTransparentCommand &transparent = static_cast<SetTransparentCommand &>(command);

        m_alpha2 = transparent.m_alpha;
        if (transparent.m_enable) {
            fn_8015FDD8(m_model2.get(), m_alpha2);
        } else {
            fn_8015FD10(m_model2.get());
        }
        break;
    }
    }
    command.m_flags |= 2;
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "Type 0:Burst 1:Hover", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "WaitSec", 0, 0, 0, 2.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "HoverLength point", 0, 0, 0, 20.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "HoverSec", 0, 0, 0, 6.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MoveLengthX point", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MoveLengthY point", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MoveLengthZ point", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MoveSec", 0, 0, 0, 5.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8054BE38 = {
    "FootingBreak", fn_80262DF0, fn_80262DEC, FootingBreak::Create, 0, 0, 0x189E, 1, 0, 10,
    s_paramInfo,
};

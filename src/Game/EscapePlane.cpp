/* The ESCAPE_PLANE gadget (classes described in Game/EscapePlane.h): EscapePlane::SetEscapePlane
 * and the functions that create and destroy it together with its
 * EscapePlaneController::Controller. File name inferred from the namespace. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first).
 *
 * The attack object's callback is a boost::function object built with
 * boost::bind(&SetEscapePlane::OnAttack, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/EscapePlane.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliAttack.h"
#include "Game/EscapePlaneController.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/Target.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkFrameSearch_80169E8C.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/UnkManager_80109D64.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The model file of SetEscapePlane; the descriptor's load hook passes the same string. */
static const char *s_fileName = "ESCAPEPLANE.BIN";

/* The ids of the frames that FindFrames keeps (name inferred). */
static s32 s_frameIds[3] = {111, 222, 333};

/* The offset that UpdateModelMatrix negates for the second model's matrix (name inferred). The
 * file's start-up function initialises it. */
static Fn8040Vector3 s_modelOffset(0.186f, -59.141f, 6.969f);

/* The value that FindFrames stores in each Target's float at 0x20, and what UpdateModelMatrix adds
 * to the y of the character info's position for field_0x24 (names inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original initialiser
 * expression. The original sets both values with its start-up function in .ctors (a constant
 * float initialiser would be emitted as initialised .sdata data instead), and the shared stand-in
 * reproduces that function. Revisit when the original initialiser is found. */
static const float s_targetField20 = UnkStartupFloat_800E2570(1000.0f).value;
static const float s_objectHeight = UnkStartupFloat_800E2570(-50.0f).value;

namespace EscapePlane {

SetEscapePlane::SetEscapePlane() : m_health(800.0f) {}

SetEscapePlane::~SetEscapePlane() {
    ReleaseFrames();
}

void SetEscapePlane::Initialize() {
    m_charInfo = fn_8015CD90(0x7F, 4);

    m_model = fn_8015F800(1, s_fileName);
    m_model->fn_80162A68(0, 0);
    field_0x14 = fn_8015F800(1, s_fileName);
    field_0x14->fn_80162A68(2, 0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> attackRef = m_model->fn_80161428(0);

    if (boost::shared_ptr<CharaColliAttack> attack = attackRef.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 =
            AttackCallback(boost::bind(&SetEscapePlane::OnAttack, this, _1));
    }

    FindFrames();

    field_0x24 = fn_802B4254(0, 1);
    fn_802B4510(field_0x24.get(), 1);

    UpdateModelMatrix();

    m_timer = 0.0f;
    m_hitTimer = 0.0f;
    m_state = STATE_START;
    field_0x3C = 0;
    m_flags.Clear();

    if (m_model.get() != 0) {
        field_0x1C = fn_80166AF4();

        UnkWeakHolder_800CC6C8 weakTarget;
        weakTarget.ptr = field_0x1C;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
    }
    if (field_0x1C.get() != 0) {
        field_0x1C->fn_80167010(-1);
    }
}

void SetEscapePlane::UnkSlot3() {}

void SetEscapePlane::Update(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (field_0x14.get() != 0) {
        field_0x14->fn_80162FB0(dt);
    }
    if (field_0x1C.get() != 0) {
        field_0x1C->fn_8016761C(dt);
    }

    m_hitTimer -= dt;
    if (m_hitTimer < 0.0f) {
        m_hitTimer = 0.0f;
    }

    switch (m_state) {
    case STATE_START:
        m_state = STATE_STAGE0;
        break;

    case STATE_STAGE0:
        if (m_health < 600.0f) {
            SetStage(0);
            m_state = STATE_STAGE1;

            int voice = fn_801FAAC4(fn_8006D6A8()->field_0x20);
            if (voice != -1) {
                fn_801FAF00(fn_80109D64(), 0x4F2, voice, 0, 0);
            }
        }
        break;

    case STATE_STAGE1:
        if (m_health < 400.0f) {
            SetStage(1);
            m_state = STATE_STAGE2;

            int voice = fn_801FAAC4(fn_8006D6A8()->field_0x20);
            if (voice != -1) {
                fn_801FAF00(fn_80109D64(), 0x4F3, voice, 0, 0);
            }
        }
        break;

    case STATE_STAGE2:
        if (m_health < 200.0f) {
            SetStage(2);
            m_state = STATE_STAGE3;

            int voice = fn_801FAAC4(fn_8006D6A8()->field_0x20);
            if (voice != -1) {
                fn_801FAF00(fn_80109D64(), 0x4F4, voice, 0, 0);
            }
        }
        break;

    case STATE_STAGE3:
        if (m_health <= 0.0f) {
            SetStage(3);
            m_state = STATE_DEFEATED;
        }
        break;

    case STATE_DEFEATED:
        m_timer = 4.0f;
        m_state = STATE_COUNTDOWN;
        break;

    case STATE_COUNTDOWN:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_flags.Set(FLAG_FINISHED);
            m_state = STATE_FINISHED;
        }
        break;

    case STATE_FINISHED:
        break;
    }

    if (m_flags.Test(FLAG_STAGE_SOUND)) {
        fn_801D53C4(fn_80053164(), 0x7834, &m_charInfo->m_position, this, 3, 0, 2);
    }
}

void SetEscapePlane::UpdateModelMatrix() {
    if (field_0x14.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040B6F8(&matrix);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B59C(&matrix, fn_80016284(s_modelOffset));
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        field_0x14->fn_801633DC(&matrix);
    }

    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040B6F8(&matrix);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }

    for (int i = 0; i < 3; i++) {
        if (m_frames[i] != 0) {
            RwMatrix *ltm = RwFrameGetLTM(m_frames[i]);
            fn_800CCE38(m_targets[i].get(), Fn8040Vector3(ltm->pos.x, ltm->pos.y, ltm->pos.z));
        }
    }

    if (field_0x24.get() != 0) {
        Fn8040Vector3 position(m_charInfo->m_position);

        position.y += s_objectHeight;
        fn_802B44C8(field_0x24.get(), &position);
    }
}

bool SetEscapePlane::IsFinished() {
    return m_flags.Test(FLAG_FINISHED);
}

void SetEscapePlane::UnkSlot7() {}

void SetEscapePlane::UnkSlot8() {
    if (m_health < 200.0f) {
        if (field_0x1C.get() != 0) {
            field_0x1C->fn_80167010(0x4B);
        }
    } else if (m_health < 400.0f) {
        if (field_0x1C.get() != 0) {
            field_0x1C->fn_80167010(0x32);
        }
    } else if (m_health < 600.0f) {
        if (field_0x1C.get() != 0) {
            field_0x1C->fn_80167010(0x19);
        }
    }
}

void SetEscapePlane::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;

    if (m_hitTimer > 0.0f) {
        return;
    }

    if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
        if ((attack->field_0x40 & 0x3D0) == 0) {
            return;
        }

        m_health -= param.field_0x50;
        m_hitTimer = 0.5f;

        if (param.field_0x14 != 0) {
            field_0x3C = param.field_0x14->field_0x20;
        } else {
            field_0x3C = 0;
        }

        if (m_health > 0.0f) {
            param.field_0x54 = 1;
            fn_801D54F4(fn_80053164(), 0x7830, &m_charInfo->m_position, 0, 1);
        } else {
            param.field_0x54 = 3;
        }
    }
}

void SetEscapePlane::SetStage(int stage) {
    switch (stage) {
    case 0:
        field_0x1C.reset();
        ReleaseFrames();
        m_model->fn_80162804();
        m_model->fn_80162A68(1, 0);
        FindFrames();
        UpdateModelMatrix();

        if (m_model.get() != 0) {
            field_0x1C = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = field_0x1C;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (field_0x1C.get() != 0) {
            field_0x1C->fn_80167010(0x19);
        }
        fn_801D54F4(fn_80053164(), 0x7832, &m_charInfo->m_position, 0, 1);
        break;

    case 1:
        if (field_0x1C.get() != 0) {
            field_0x1C->fn_80166E20(0x19);
            field_0x1C->fn_80167010(0x32);
        }
        fn_801D54F4(fn_80053164(), 0x7832, &m_charInfo->m_position, 0, 1);
        break;

    case 2:
        if (field_0x1C.get() != 0) {
            field_0x1C->fn_80166E20(0x32);
            field_0x1C->fn_80167010(0x4B);
        }
        fn_801D54F4(fn_80053164(), 0x7832, &m_charInfo->m_position, 0, 1);
        break;

    case 3:
        if (field_0x1C.get() != 0) {
            field_0x1C->fn_80166E20(0x4B);
            field_0x1C->fn_80167448(0x64);
        }
        fn_801D54F4(fn_80053164(), 0x7832, &m_charInfo->m_position, 0, 1);
        break;
    }

    switch (stage) {
    case 0:
    case 1:
    case 2:
        if (field_0x1C.get() != 0) {
            switch (field_0x3C) {
            case 1:
                field_0x1C->fn_80167448(2);
                break;
            case 3:
                field_0x1C->fn_80167448(4);
                break;
            case 5:
                field_0x1C->fn_80167448(6);
                break;
            }
        }
        break;
    }

    m_flags.Set(FLAG_STAGE_SOUND);
}

void SetEscapePlane::FindFrames() {
    UnkFrameSearch_80169E8C search;

    for (int i = 0; i < 3; i++) {
        search.id = s_frameIds[i];
        search.frame = 0;
        m_model->fn_8015FA78(fn_80169E8C, &search);
        m_frames[i] = search.frame;

        m_targets[i] = fn_801E72DC(0x800);
        m_targets[i]->SetField20(s_targetField20);
    }
}

void SetEscapePlane::ReleaseFrames() {
    for (int i = 0; i < 3; i++) {
        m_frames[i] = 0;
        if (m_targets[i].get() != 0) {
            fn_801E7108(m_targets[i]);
        }
    }
}

} // namespace EscapePlane

/* The controller of the current SetEscapePlane (name inferred).
 * Matching workaround: defined here rather than at the top of the file. The original's .sbss has
 * it after the three one-byte objects of the boost function templates that Initialize
 * instantiates (USA 0x805EFFCC-0x805EFFD4) and before the one of the shared_ptr constructor that
 * fn_80255074 uses (0x805EFFDC); defined at the top it comes before the three. Revisit if the
 * original order of the file's definitions is found. */
static EscapePlaneController::Controller *s_controller;

extern "C" void fn_80255074(void) {
    boost::shared_ptr<PathObject> object =
        boost::shared_ptr<PathObject>(new EscapePlane::SetEscapePlane());

    Task *parent = fn_801EB304(fn_8001C4F8(), 6);

    s_controller = new EscapePlaneController::Controller(parent, object);
}

extern "C" void fn_8025502C(void) {
    if (s_controller != 0) {
        delete s_controller;
        s_controller = 0;
    }
}

extern "C" void fn_80255000(void) {
    if (s_controller != 0) {
        s_controller->fn_8025F054();
    }
}

extern "C" void fn_80254FB4(void) {
    if (s_controller != 0) {
        delete s_controller;
        s_controller = 0;
    }
    fn_80255074();
}

/* The descriptor's load hook: adds the model file to the list of resources to load (name
 * inferred). */
extern "C" void fn_80254F88(void) {
    fn_8006B474("ESCAPEPLANE.BIN", 1);
}

extern "C" void fn_80254F84(void) {}

namespace EscapePlane {

boost::weak_ptr<CharInfo> SetEscapePlane::GetCharInfo() {
    return m_charInfo;
}

} // namespace EscapePlane

int fn_80254EFC(void) {
    fn_80255074();
    return 1;
}

int fn_80254ED8(void) {
    fn_80255000();
    return 1;
}

int fn_80254EB4(void) {
    fn_80254FB4();
    return 1;
}

int fn_80254E90(void) {
    fn_8025502C();
    return 1;
}

GadgetTypeInfo lbl_80549B28 = {
    "ESCAPE_PLANE", fn_80254F88, fn_80254F84, 0, 0, 0, 0x13EC, 1, 0x80, 10, 0,
};

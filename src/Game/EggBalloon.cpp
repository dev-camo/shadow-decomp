/* The EGG_BALLOON gadget (classes described in Game/EggBalloon.h): EggBalloon::SetEggBalloon and
 * the functions that create and destroy it together with its EggBalloonController::Controller.
 * File name inferred from the namespace. Definitions are in reverse address order (deferred
 * inlining emits functions last-defined first).
 *
 * The attack object's callback is a boost::function object built with
 * boost::bind(&SetEggBalloon::OnAttack, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/EggBalloon.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommand.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliAttack.h"
#include "Game/EggBalloonController.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetCommand.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/Target.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkFrameSearch_80169E8C.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/UnkManager_80109D64.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkParams_80051D10.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_800E9CEC.h"
#include "Game/fn_80038E84.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8012773C.h"
#include "Game/fn_80209B58.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rpworld.h"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The model file of SetEggBalloon; the descriptor's load hook passes the same string. */
static const char *s_fileName = "EGGBALLOON.BIN";

/* Half a turn. The file's start-up function (USA 0x80147AC0) initialises it and nothing in the
 * file reads it; the declaration that produced it is unknown. */
static const float s_halfTurn = DegreesToRadians(180.0f);

/* What FindFrames and FindMatrixFrames pass to the part callback below (placeholder name): the
 * id of a frame to look for below the frame of the part whose word at 0x8 is partId, and the
 * frame found. Its constructor sets both ids to -1 and the frame to null. */
struct UnkPartChildFrameSearch {
    UnkPartChildFrameSearch() : frameId(-1), partId(-1), frame(0) {}

    /* 0x0 */ s32 frameId;
    /* 0x4 */ s32 partId;
    /* 0x8 */ RwFrame *frame;
};
CHECK_SIZEOF(UnkPartChildFrameSearch, 0xC);

/* The part callback (placeholder name): for the part whose word at 0x8 equals the searched part
 * id, looks up the searched frame id below the frame of its clump (fn_80038E84) and stops the walk
 * once a frame is found (behaviour inferred). */
static UnkModelPart_8015FA78 *fn_8014788C(UnkModelPart_8015FA78 *part, void *userData) {
    UnkPartChildFrameSearch *search = static_cast<UnkPartChildFrameSearch *>(userData);

    if (search != 0 && part->clump != 0 && part->field_0x8 == search->partId) {
        search->frame =
            fn_80038E84(static_cast<RwFrame *>(part->clump->object.parent), search->frameId);
        if (search->frame != 0) {
            return 0;
        }
    }
    return part;
}

namespace EggBalloon {

SetEggBalloon::SetEggBalloon() : m_health(400.0f) {}

SetEggBalloon::~SetEggBalloon() {
    ReleaseFrames();
}

void SetEggBalloon::Initialize() {
    m_charInfo = fn_8015CB88(0x3D, 4, "SetEggBalloon");

    m_model = fn_8015F800(1, s_fileName);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160C28(0);
    m_model->fn_8015FF60();
    m_model->fn_80160A84(0, 1);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    boost::weak_ptr<CharaColliAttack> attackRef = m_model->fn_80161428(0);

    if (boost::shared_ptr<CharaColliAttack> attack = attackRef.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 =
            AttackCallback(boost::bind(&SetEggBalloon::OnAttack, this, _1));
    }

    FindFrames(0);

    field_0x24 = fn_802B4254(2, 0);
    fn_802B4510(field_0x24.get(), 1);
    fn_802B4498(field_0x24.get(), 300.0f);

    FindMatrixFrames(0);
    UpdateModelMatrix();

    m_hitTimer = 0.0f;
    m_fireTime = 0.0f;
    m_shotTimer = 0.0f;
    m_shotInterval = 1.0f;
    m_timer = 0.0f;
    m_state = STATE_START;
    m_flags.Clear();

    for (int i = 0; i < 6; i++) {
        fn_8040E970(&m_angles[i]);
    }
}

void SetEggBalloon::UnkSlot3() {}

void SetEggBalloon::Update(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (field_0x14.get() != 0) {
        field_0x14->fn_8016761C(dt);
    }
    if (field_0x1C.get() != 0) {
        field_0x1C->fn_8016761C(dt);
    }

    bool running = true;
    if (fn_80169CD0()) {
        running = false;
    }

    m_hitTimer -= dt;
    if (m_hitTimer < 0.0f) {
        m_hitTimer = 0.0f;
    }

    if (running) {
        m_fireTime -= dt;
        if (m_fireTime < 0.0f) {
            m_fireTime = 0.0f;
        }
        m_shotTimer -= dt;
        if (m_shotTimer < 0.0f) {
            m_shotTimer = 0.0f;
        }
    }

    switch (m_state) {
    case STATE_START:
        if (m_model.get() != 0) {
            field_0x1C = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = field_0x1C;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }
        if (field_0x1C.get() != 0) {
            field_0x1C->fn_80167010(0x29A);
        }
        m_state = STATE_STAGE0;
        break;

    case STATE_STAGE0:
        if (m_health < 300.0f) {
            SetStage(0);
            m_state = STATE_STAGE1;

            int voice = fn_801FAAC4(fn_8006D6A8()->field_0x20);
            if (voice != -1) {
                fn_801FAF00(fn_80109D64(), 0x430, voice, 0, 0);
            }
        }
        break;

    case STATE_STAGE1:
        if (m_health < 200.0f) {
            SetStage(1);
            m_state = STATE_STAGE2;

            int voice = fn_801FAAC4(fn_8006D6A8()->field_0x20);
            if (voice != -1) {
                fn_801FAF00(fn_80109D64(), 0x431, voice, 0, 0);
            }
        }
        break;

    case STATE_STAGE2:
        if (m_health < 100.0f) {
            SetStage(2);
            m_state = STATE_STAGE3;

            int voice = fn_801FAAC4(fn_8006D6A8()->field_0x20);
            if (voice != -1) {
                fn_801FAF00(fn_80109D64(), 0x432, voice, 0, 0);
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
        m_timer = 2.0f;
        m_state = STATE_COUNTDOWN;
        break;

    case STATE_COUNTDOWN:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_timer = 0.0f;
            m_state = STATE_FINISHED;
            m_flags.Set(FLAG_FINISHED);
            m_flags.Reset(FLAG_FIRE_3TO5);
            m_flags.Reset(FLAG_FIRE_0TO2);
            m_flags.Reset(FLAG_DROP);
            m_flags.Reset(FLAG_KIND3);
        }
        break;

    case STATE_FINISHED:
        break;
    }

    if (running) {
        Aim();
        if (m_fireTime > 0.0f) {
            Fire();
        }
    }
}

void SetEggBalloon::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }

    for (int i = 0; i < 6; i++) {
        fn_800E9CEC(m_aimFrames[i], 0, m_angles[i].y);
        fn_800E9C64(m_aimFrames[i], 2, m_angles[i].x);
        fn_804876E8(m_aimFrames[i], &m_aimMatrices[i], 2);
    }

    if (field_0x24.get() != 0) {
        Fn8040Vector3 position(m_charInfo->m_position);

        position.y += -105.0f;
        fn_802B44C8(field_0x24.get(), &position);
    }

    for (int i = 0; i < 6; i++) {
        if (m_targets[i].get() != 0) {
            RwMatrix *ltm = RwFrameGetLTM(m_aimFrames[i]);
            Fn8040Vector3 position(ltm->pos.x, ltm->pos.y, ltm->pos.z);

            fn_800CCE38(m_targets[i].get(), position);
        }
    }
}

bool SetEggBalloon::IsFinished() {
    return m_flags.Test(FLAG_FINISHED);
}

void SetEggBalloon::UnkSlot7() {}

void SetEggBalloon::UnkSlot8() {
    if (m_health < 100.0f) {
        if (field_0x14.get() != 0) {
            field_0x14->fn_80167010(0x4B);
        }
    } else if (m_health < 200.0f) {
        if (field_0x14.get() != 0) {
            field_0x14->fn_80167010(0x32);
        }
    } else if (m_health < 300.0f) {
        if (field_0x14.get() != 0) {
            field_0x14->fn_80167010(0x19);
        }
    }

    if (field_0x1C.get() != 0) {
        field_0x1C->fn_80167010(0x29A);
    }
}

void SetEggBalloon::OnAttack(AttackCallbackParam &param) {
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

        if (m_health > 0.0f) {
            param.field_0x54 = 1;

            if (m_flags.Test(FLAG_HIT_SOUND)) {
                fn_801D54F4(fn_80053164(), 0x754F, &m_charInfo->m_position, 0, 1);
                m_flags.Reset(FLAG_HIT_SOUND);
            } else {
                fn_801D54F4(fn_80053164(), 0x754E, &m_charInfo->m_position, 0, 1);
                m_flags.Set(FLAG_HIT_SOUND);
            }
        } else {
            param.field_0x54 = 3;
        }
    }
}

void SetEggBalloon::SetStage(int stage) {
    switch (stage) {
    case 0:
        ReleaseFrames();
        field_0x14.reset();
        field_0x1C.reset();

        m_model->fn_801604FC();
        m_model->fn_80162804();
        m_model->fn_80162A68(1, 0);
        m_model->fn_80160C28(1);
        m_model->fn_8015FF60();
        m_model->fn_80160A84(1, 1);
        FindFrames(1);
        FindMatrixFrames(1);
        UpdateModelMatrix();

        if (m_model.get() != 0) {
            UnkWeakHolder_800CC6C8 weakTarget;

            field_0x14 = fn_80166AF4();
            weakTarget.ptr = field_0x14;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);

            field_0x1C = fn_80166AF4();
            weakTarget.ptr = field_0x1C;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        }

        if (field_0x14.get() != 0) {
            field_0x14->fn_80167448(0x19);
            field_0x14->fn_80167010(0x19);
        }
        if (field_0x1C.get() != 0) {
            field_0x1C->fn_80167010(0x29A);
        }
        fn_801D54F4(fn_80053164(), 0x7550, &m_charInfo->m_position, 0, 1);
        break;

    case 1:
        if (field_0x14.get() != 0) {
            field_0x14->fn_80167448(0x19);
            field_0x14->fn_80167448(0x32);
            field_0x14->fn_80167010(0x32);
        }
        fn_801D54F4(fn_80053164(), 0x7550, &m_charInfo->m_position, 0, 1);
        break;

    case 2:
        if (field_0x14.get() != 0) {
            field_0x14->fn_80167448(0x19);
            field_0x14->fn_80167448(0x32);
            field_0x14->fn_80167448(0x4B);
            field_0x14->fn_80167010(0x4B);
        }
        fn_801D54F4(fn_80053164(), 0x7550, &m_charInfo->m_position, 0, 1);
        break;

    case 3:
        if (field_0x14.get() != 0) {
            field_0x14->fn_80167448(0x64);
        }
        fn_801D54F4(fn_80053164(), 0x7550, &m_charInfo->m_position, 0, 1);
        break;
    }
}

/* Matching workaround: the tables below are defined here, before the functions that use them,
 * not at the top of the file. The original's .rodata has the name of the bound attack callback's
 * type (created when Initialize, defined above, is parsed) before these tables; defined at the
 * top they come before it. Revisit if the original order of the file's definitions is found. */
/* The ids of the drop frames (name inferred). */
static const s32 s_dropFrameIds[2] = {0x1E, 0x14};
/* The ids of the parts whose frame 10 FindFrames keeps as shot frames, by set (name inferred). */
static const s32 s_shotPartIds[2][6] = {{1, 2, 3, 4, 5, 6}, {8, 9, 10, 11, 12, 13}};
/* The ids of the frames that FindFrames keeps as sight frames (name inferred). */
static const s32 s_sightFrameIds[6] = {1, 2, 3, 4, 5, 6};

void SetEggBalloon::FindFrames(int set) {
    UnkFrameSearch_80169E8C dropSearch;

    for (int i = 0; i < 2; i++) {
        dropSearch.id = s_dropFrameIds[i];
        dropSearch.frame = 0;
        m_model->fn_8015FA78(fn_80169E8C, &dropSearch);
        m_dropFrames[i] = dropSearch.frame;
    }

    UnkPartChildFrameSearch shotSearch;

    for (int i = 0; i < 6; i++) {
        shotSearch.frameId = 10;
        shotSearch.partId = s_shotPartIds[set][i];
        shotSearch.frame = 0;
        m_model->fn_8015FA78(fn_8014788C, &shotSearch);
        m_shotFrames[i] = shotSearch.frame;
    }

    UnkFrameSearch_80169E8C sightSearch;

    for (int i = 0; i < 6; i++) {
        sightSearch.id = s_sightFrameIds[i];
        sightSearch.frame = 0;
        m_model->fn_8015FA78(fn_80169E8C, &sightSearch);
        m_sightFrames[i] = sightSearch.frame;
    }

    for (int i = 0; i < 6; i++) {
        m_targets[i] = fn_801E72DC(0x2800);
    }
}

void SetEggBalloon::Aim() {
    if (m_flags.Test(FLAG_AIM_3TO5)) {
        AimAt(3, 6);
    } else if (m_flags.Test(FLAG_AIM_0TO2)) {
        AimAt(0, 3);
    } else {
        float step = DegreesToRadians(0.5f);

        for (int i = 0; i < 6; i++) {
            m_angles[i].y = fn_8012773C(0.0f, m_angles[i].y, step);
            m_angles[i].x = fn_8012773C(0.0f, m_angles[i].x, step);
        }
    }
}

void SetEggBalloon::AimAt(int first, int last) {
    Fn8040Vector3 target(0.0f, 0.0f, 0.0f);
    boost::weak_ptr<CharInfo> nearest = fn_800A8A7C(fn_80074194(), &m_charInfo->m_position);

    if (!nearest.expired()) {
        boost::shared_ptr<CharInfo> character = nearest.lock();

        target = character->m_position;
    }

    /* Matching workaround: the limits are declared before the step. Declared after it (y limits,
     * step, x limits), the step and the x limits take f31, f30 and f29 instead of the original's
     * f29, f31 and f30 (9 instructions differ). Revisit with the original's source order. */
    float minY = DegreesToRadians(-40.0f);
    float maxY = DegreesToRadians(40.0f);
    float minX = DegreesToRadians(-40.0f);
    float maxX = DegreesToRadians(40.0f);
    float step = DegreesToRadians(0.5f);

    for (int i = first; i < last; i++) {
        RwMatrix *ltm = RwFrameGetLTM(m_sightFrames[i]);
        Fn8040Matrix34 matrix;

        matrix.values[0][0] = ltm->right.x;
        matrix.values[0][1] = ltm->up.x;
        matrix.values[0][2] = ltm->at.x;
        matrix.values[0][3] = ltm->pos.x;
        matrix.values[1][0] = ltm->right.y;
        matrix.values[1][1] = ltm->up.y;
        matrix.values[1][2] = ltm->at.y;
        matrix.values[1][3] = ltm->pos.y;
        matrix.values[2][0] = ltm->right.z;
        matrix.values[2][1] = ltm->up.z;
        matrix.values[2][2] = ltm->at.z;
        matrix.values[2][3] = ltm->pos.z;

        Fn8040Matrix34 inverse;

        fn_8040BEF8(&inverse, &matrix);

        Fn8040Vector3 local = inverse * target;
        UnkRotation3 angles = fn_8040F7F8(&local);

        if (angles.y < minY) {
            angles.y = minY;
        } else if (angles.y > maxY) {
            angles.y = maxY;
        }
        if (angles.x < minX) {
            angles.x = minX;
        } else if (angles.x > maxX) {
            angles.x = maxX;
        }

        m_angles[i].y = fn_8012773C(angles.y, m_angles[i].y, step);
        m_angles[i].x = fn_8012773C(angles.x, m_angles[i].x, step);
    }
}

void SetEggBalloon::Fire() {
    if (m_flags.Test(FLAG_FIRE_3TO5)) {
        if (m_shotTimer > 0.0f) {
            return;
        }
        m_shotTimer = FireShots();
    } else if (m_flags.Test(FLAG_FIRE_0TO2)) {
        if (m_shotTimer > 0.0f) {
            return;
        }
        m_shotTimer = FireShots();
    } else if (m_flags.Test(FLAG_DROP)) {
        if (m_shotTimer > 0.0f) {
            return;
        }
        m_shotTimer = Drop();
    } else if (m_flags.Test(FLAG_KIND3)) {
        /* The original tests the flag and does nothing for it (the body, if there was one, is
         * unknown; revisit if found). */
    }
}

/* The ids of the parts whose frame 0xC9 FindMatrixFrames keeps as aim frames, by set (name
 * inferred). */
static const s32 s_aimPartIds[2][6] = {{1, 2, 3, 4, 5, 6}, {8, 9, 10, 11, 12, 13}};

void SetEggBalloon::FindMatrixFrames(int set) {
    UnkPartChildFrameSearch search;

    for (int i = 0; i < 6; i++) {
        search.frameId = 0xC9;
        search.partId = s_aimPartIds[set][i];
        search.frame = 0;
        m_model->fn_8015FA78(fn_8014788C, &search);
        m_aimFrames[i] = search.frame;
        m_aimMatrices[i] = search.frame->modelling;
    }
}

void SetEggBalloon::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x303: {
        GadgetCommand::AttackStartCommand &start =
            static_cast<GadgetCommand::AttackStartCommand &>(command);

        switch (start.m_kind) {
        case 0:
            m_flags.Reset(FLAG_FIRE_0TO2);
            m_flags.Reset(FLAG_AIM_0TO2);
            m_flags.Reset(FLAG_DROP);
            m_flags.Reset(FLAG_KIND3);
            if (m_flags.Test(FLAG_FIRE_3TO5)) {
                m_fireTime += start.m_time;
            } else {
                m_fireTime = start.m_time;
                m_flags.Set(FLAG_FIRE_3TO5);
                m_flags.Set(FLAG_AIM_3TO5);
            }
            m_shotInterval = start.m_interval;
            break;

        case 1:
            m_flags.Reset(FLAG_FIRE_3TO5);
            m_flags.Reset(FLAG_AIM_3TO5);
            m_flags.Reset(FLAG_DROP);
            m_flags.Reset(FLAG_KIND3);
            if (m_flags.Test(FLAG_FIRE_0TO2)) {
                m_fireTime += start.m_time;
            } else {
                m_fireTime = start.m_time;
                m_flags.Set(FLAG_FIRE_0TO2);
                m_flags.Set(FLAG_AIM_0TO2);
            }
            m_shotInterval = start.m_interval;
            break;

        case 2:
            m_flags.Reset(FLAG_FIRE_3TO5);
            m_flags.Reset(FLAG_AIM_3TO5);
            m_flags.Reset(FLAG_FIRE_0TO2);
            m_flags.Reset(FLAG_AIM_0TO2);
            m_flags.Reset(FLAG_KIND3);
            if (m_flags.Test(FLAG_DROP)) {
                m_fireTime += start.m_time;
            } else {
                m_fireTime = start.m_time;
                m_flags.Set(FLAG_DROP);
            }
            m_shotInterval = start.m_interval;
            break;

        case 3:
            m_flags.Reset(FLAG_FIRE_3TO5);
            m_flags.Reset(FLAG_AIM_3TO5);
            m_flags.Reset(FLAG_FIRE_0TO2);
            m_flags.Reset(FLAG_AIM_0TO2);
            m_flags.Reset(FLAG_DROP);
            if (m_flags.Test(FLAG_KIND3)) {
                m_fireTime += start.m_time;
            } else {
                m_fireTime = start.m_time;
                m_flags.Set(FLAG_KIND3);
            }
            break;
        }
        break;
    }

    case 0x304:
        m_flags.Reset(FLAG_FIRE_3TO5);
        m_flags.Reset(FLAG_AIM_3TO5);
        m_flags.Reset(FLAG_FIRE_0TO2);
        m_flags.Reset(FLAG_AIM_0TO2);
        m_flags.Reset(FLAG_DROP);
        m_flags.Reset(FLAG_KIND3);
        m_fireTime = 0.0f;
        break;
    }
}

float SetEggBalloon::Drop() {
    if (m_dropFrames[0] != 0) {
        RwMatrix *ltm = RwFrameGetLTM(m_dropFrames[0]);
        Fn8040Vector3 position(ltm->pos.x, ltm->pos.y, ltm->pos.z);
        Fn8040Vector3 direction(0.0f, -1.0f, 0.0f);
        UnkParams_80145B58 params;

        params.field_0x0 = position;
        params.field_0xC = direction;
        params.field_0x18 = 5.0f;
        params.field_0x20 = false;
        fn_80209B58(fn_801EB304(fn_8001C4F8(), 11), 0, &params);
    }

    return m_shotInterval <= 0.0f ? 2.0f : m_shotInterval;
}

/* The direction of a shot in the space of its frame (name inferred). */
static const RwV3d s_shotDirection = {0.0f, 0.0f, 1.0f};

float SetEggBalloon::FireShots() {
    int first;
    int last;

    if (m_flags.Test(FLAG_FIRE_3TO5)) {
        first = 3;
        last = 6;
    } else if (m_flags.Test(FLAG_FIRE_0TO2)) {
        first = 0;
        last = 3;
    } else {
        return 1.0f;
    }

    for (int i = first; i < last; i++) {
        if (m_shotFrames[i] != 0) {
            RwMatrix *ltm = RwFrameGetLTM(m_shotFrames[i]);
            RwV3d origin = ltm->pos;
            RwV3d forward = s_shotDirection;

            fn_80482504(&forward, &forward, ltm);

            Fn8040Vector3 position(origin.x, origin.y, origin.z);
            Fn8040Vector3 direction(forward.x, forward.y, forward.z);
            UnkParams_80051D10 params(boost::weak_ptr<CharInfo>(m_charInfo), 1, position,
                                      direction);
            fn_80051C34(0x11, &params);
        }
    }

    return m_shotInterval <= 0.0f ? 1.0f : m_shotInterval;
}

} // namespace EggBalloon

/* The controller of the current SetEggBalloon (name inferred).
 * Matching workaround: defined here rather than at the top of the file. The original's .sbss has
 * it after the three one-byte objects that the boost function code emitted for Initialize uses
 * and before the one of the shared_ptr constructor that fn_8014565C uses; defined at the top it
 * comes before them. Revisit if the original order of the file's definitions is found. */
static EggBalloonController::Controller *s_controller;

extern "C" void fn_8014565C(void) {
    boost::shared_ptr<PathObject> object =
        boost::shared_ptr<PathObject>(new EggBalloon::SetEggBalloon());

    Task *parent = fn_801EB304(fn_8001C4F8(), 6);

    s_controller = new EggBalloonController::Controller(parent, object);
}

extern "C" void fn_80145614(void) {
    if (s_controller != 0) {
        delete s_controller;
        s_controller = 0;
    }
}

extern "C" void fn_801455E8(void) {
    if (s_controller != 0) {
        s_controller->fn_802742D4();
    }
}

extern "C" void fn_8014559C(void) {
    if (s_controller != 0) {
        delete s_controller;
        s_controller = 0;
    }
    fn_8014565C();
}

/* The descriptor's load hook: adds the model file to the list of resources to load (name
 * inferred). */
extern "C" void fn_80145570(void) {
    fn_8006B474("EGGBALLOON.BIN", 1);
}

extern "C" void fn_8014556C(void) {}

namespace EggBalloon {

boost::weak_ptr<CharInfo> SetEggBalloon::GetCharInfo() {
    return m_charInfo;
}

} // namespace EggBalloon

GadgetTypeInfo lbl_80529AF4 = {
    "EGG_BALLOON", fn_80145570, fn_8014556C, 0, 0, 0, 0x1450, 1, 0x80, 10, 0,
};

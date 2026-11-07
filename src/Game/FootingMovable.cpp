/* The MOVE_FOOTING gadget (class described in Game/FootingMovable.h). File name inferred from the
 * class. Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first).
 *
 * The character info's command handler and the attack callback of the model entry's attack object
 * are boost::function objects built with boost::bind(&FootingMovable::method, this, _1); their
 * managers, invokers and copy functions come from the boost headers. */
#include <cmath>
/* Matching workaround: TNumEdit.h and CharaColliBaseCharInfo.h are included only for the RTTI
 * records that their never-called stand-in functions emit (see the notes there); no code in this
 * file uses TNumEdit or CharaColliBaseCharInfo. TNumEdit.h comes first, in the order their names
 * appear in the unit's .rodata (alphabetical order would reverse them). Revisit when the original
 * inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/FootingMovable.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharaColliBase.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8006_resource_list.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

/* The length of the character info's velocity (field_0x18) above which UpdateSound requests the
 * move sound (name inferred). Matching workaround: the original stores 0.5 here at startup (USA
 * (GUPE8P) 0x8014BD08), which the stand-in class of UnkStartupFloat_800E2570.h reproduces; a plain
 * float initialiser would place the value in .sdata. Revisit when the original wrapper type is
 * found. */
static const float s_moveSoundSpeed = UnkStartupFloat_800E2570(0.5f).value;

/* value modulo divisor rounded to the nearest integer, or 0 unless divisor >= 0.5 (so also for
 * NaN) (name inferred). */
static inline u32 ModuloRounded(u32 value, float divisor) {
    return divisor >= 0.5f ? value % (u32)(0.5f + divisor) : 0;
}

namespace FootingMovable {

const char *FootingMovable::s_name = "FootingMovable";

FootingMovable::FootingMovable(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;

    m_charInfo = fn_8015CD90(0x20, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&FootingMovable::OnCommand, this, _1));

    ReadOwner(owner);
    m_model = fn_8015F800(1, "FOOTINGMOVABLE.BIN");
    m_modelIndex = m_params.model;
    m_model->fn_80162A68(m_modelIndex, 1);
    m_model->fn_80161FC8(m_modelIndex, boost::weak_ptr<CharInfo>(m_charInfo));

    m_flags.Reset(FLAG_SOUND);
    m_flags.Reset(FLAG_SOUND_START);
    if (m_model->field_0x30 != 0) {
        UnkResourceRecord_80208848 *record = fn_80208848(m_model->field_0x30, m_modelIndex);

        if (record != 0) {
            m_moveSound = record->field_0x8;
            m_flags.Set(FLAG_SOUND);
            m_flags.Set(FLAG_SOUND_START);
        }
    }

    UpdateModelMatrix();
}

FootingMovable::~FootingMovable() {
    m_sound.fn_801D455C();
}

void FootingMovable::UpdateWaitSwitch(float dt) {
    if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
        if (m_type == 0) {
            m_state = STATE_NORMAL_START;
        } else if (m_type == 1) {
            m_state = STATE_PATH_START;
        } else if (m_type != 2) {
            if (m_type == 3) {
                m_state = STATE_ONE_WAY;
            } else if (m_type == 6) {
                m_state = STATE_NORMAL_PAUSE_START;
            }
        }
    }
}

void FootingMovable::UpdateNormalStart(float dt) {
    /* The start phase from "spd" (see the class comment). */
    m_angle = ModuloRounded(lbl_8057D6A0.field_0x90, m_params.spd);
    m_state = STATE_NORMAL;
    SetupTarget();
}

void FootingMovable::UpdatePathStart(float dt) {
    if (fn_8001C7B4(fn_8001B494(), &m_path, m_params.param0)) {
        fn_80018304(&m_path);
        m_charInfo->m_position = fn_80018634(&m_path);
        m_charInfo->m_rotation = fn_80018434(&m_path, 0, 0);

        /* The start phase from "spd" (see the class comment). */
        m_angle = ModuloRounded(lbl_8057D6A0.field_0x90, m_params.spd);
        m_state = STATE_PATH;
    }
    SetupTarget();
}

void FootingMovable::UpdateLoopPathStart(float dt) {
    if (fn_8001C7B4(fn_8001B494(), &m_path, m_params.param0)) {
        fn_80018304(&m_path);
        m_charInfo->m_position = fn_80018634(&m_path);
        m_charInfo->m_rotation = fn_80018434(&m_path, 0, 0);
        m_angle = 0.0f;
        m_state = STATE_LOOP_PATH;
        m_charInfo->m_rotation.x = 0.0f;
        m_charInfo->m_rotation.z = 0.0f;
        UpdateModelMatrix();
    }
    SetupTarget();
}

void FootingMovable::UpdateNormal(float dt) {
    Fn8040Vector3 end(m_params.param0, m_params.param1, m_params.param2);

    /* Adds the start to the offset and subtracts it again, as the original does (it calls
     * fn_8040CFF8 and then fn_8040CD74). */
    fn_8040CFF8(&end, &m_start);

    Fn8040Vector3 direction = end - m_start;
    float halfLength = 0.5f * fn_8040CF50(&direction);

    direction = fn_8040CEE8(direction);
    m_angle += m_params.acc;
    fn_8040CE38(&direction, halfLength - halfLength * std::cos(fn_8000AB90(m_angle)));
    m_charInfo->m_position = direction + m_start;
    UpdateModelMatrix();
}

void FootingMovable::UpdatePath(float dt) {
    m_angle += m_params.acc;

    float halfLength = 0.5f * fn_800183D8(&m_path);
    float distance = halfLength - halfLength * std::cos(DegreesToRadians(m_angle));

    fn_80018304(&m_path);
    fn_8001C91C(fn_8001B494(), &m_path, 0, &m_charInfo->m_position, distance,
                &m_charInfo->m_rotation, 0);
    m_charInfo->m_rotation.x = 0.0f;
    m_charInfo->m_rotation.z = 0.0f;
    UpdateModelMatrix();
}

void FootingMovable::UpdateLoopPath(float dt) {
    m_angle += m_params.acc * dt;
    fn_80018304(&m_path);
    if (fn_8001C91C(fn_8001B494(), &m_path, 0, &m_charInfo->m_position, m_angle,
                    &m_charInfo->m_rotation, 1)) {
        m_angle -= fn_800183D8(&m_path);
    }
    m_charInfo->m_rotation.x = 0.0f;
    m_charInfo->m_rotation.z = 0.0f;
    UpdateModelMatrix();
}

void FootingMovable::UpdateOneWay(float dt) {
    m_angle += 0.001f * m_params.spd * dt;
    if (m_angle > 1.0f) {
        m_angle = 1.0f;
    }

    Fn8040Vector3 offset(m_params.param0, m_params.param1, m_params.param2);

    fn_8040CE38(&offset, m_angle);
    m_charInfo->m_position = m_start + offset;
    UpdateModelMatrix();
}

void FootingMovable::UpdateNormalSyncStart(float dt) {
    m_syncTime = lbl_8057D6A0.field_0x94;
    MoveNormalSync(m_syncTime);
    UpdateModelMatrix();
    m_state = STATE_NORMAL_SYNC;
    SetupTarget();
}

void FootingMovable::UpdateNormalSync(float dt) {
    m_syncTime += *m_charInfo->field_0x60;
    MoveNormalSync(m_syncTime);
    UpdateModelMatrix();
}

void FootingMovable::UpdatePathSyncStart(float dt) {
    if (fn_8001C7B4(fn_8001B494(), &m_path, m_params.param0)) {
        fn_80018304(&m_path);
        m_charInfo->m_position = fn_80018634(&m_path);
        m_charInfo->m_rotation = fn_80018434(&m_path, 0, 0);
        m_state = STATE_PATH_SYNC;
        UpdateModelMatrix();
    }
    m_syncTime = lbl_8057D6A0.field_0x94;
    SetupTarget();
}

void FootingMovable::UpdatePathSync(float dt) {
    m_syncTime += *m_charInfo->field_0x60;
    m_angle = 0.05f * (m_params.spd * (20.0f * m_syncTime)) + m_angleOffset;

    float halfLength = 0.5f * fn_800183D8(&m_path);
    float distance = halfLength - halfLength * std::cos(DegreesToRadians(m_angle));

    fn_80018304(&m_path);
    fn_8001C91C(fn_8001B494(), &m_path, 0, &m_charInfo->m_position, distance,
                &m_charInfo->m_rotation, 0);
    m_charInfo->m_rotation.x = 0.0f;
    m_charInfo->m_rotation.z = 0.0f;
    UpdateModelMatrix();
}

void FootingMovable::UpdateNormalPauseStart(float dt) {
    MovePause(dt);
    UpdateModelMatrix();
    m_flags.Set(FLAG_TO_END);
    m_state = STATE_NORMAL_PAUSE;
    SetupTarget();
}

void FootingMovable::UpdateNormalPause(float dt) {
    if (m_flags.Test(FLAG_PAUSE)) {
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_flags.Reset(FLAG_PAUSE);
        }
    } else {
        MovePause(dt);
    }
    UpdateModelMatrix();
}

void FootingMovable::MoveNormalSync(float time) {
    m_angle = 0.05f * (m_params.spd * (20.0f * time)) + m_angleOffset;

    Fn8040Vector3 end(m_params.param0, m_params.param1, m_params.param2);

    /* Adds the start to the offset and subtracts it again, as the original does (it calls
     * fn_8040CFF8 and then fn_8040CD74). */
    fn_8040CFF8(&end, &m_start);

    Fn8040Vector3 direction = end - m_start;
    float halfLength = 0.5f * fn_8040CF50(&direction);

    direction = fn_8040CEE8(direction);
    fn_8040CE38(&direction, halfLength - halfLength * std::cos(fn_8000AB90(m_angle)));
    m_charInfo->m_position = direction + m_start;
}

void FootingMovable::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    switch (m_state) {
    case STATE_WAIT_SWITCH:
        UpdateWaitSwitch(dt);
        break;
    case STATE_NORMAL_START:
        UpdateNormalStart(dt);
        break;
    case STATE_PATH_START:
        UpdatePathStart(dt);
        break;
    case STATE_LOOP_PATH_START:
        UpdateLoopPathStart(dt);
        break;
    case STATE_NORMAL:
        UpdateNormal(dt);
        break;
    case STATE_PATH:
        UpdatePath(dt);
        break;
    case STATE_LOOP_PATH:
        UpdateLoopPath(dt);
        break;
    case STATE_DAMAGE_PAUSE:
        m_timer -= dt;
        if (m_timer <= 0.0f) {
            m_timer = 0.0f;
            m_state = m_resumeState;
            m_flags.Set(FLAG_DAMAGE_PAUSE);
        }
        break;
    case STATE_ONE_WAY:
        UpdateOneWay(dt);
        break;
    case STATE_NORMAL_SYNC_START:
        UpdateNormalSyncStart(dt);
        break;
    case STATE_NORMAL_SYNC:
        UpdateNormalSync(dt);
        break;
    case STATE_PATH_SYNC_START:
        UpdatePathSyncStart(dt);
        break;
    case STATE_PATH_SYNC:
        UpdatePathSync(dt);
        break;
    case STATE_NORMAL_PAUSE_START:
        UpdateNormalPauseStart(dt);
        break;
    case STATE_NORMAL_PAUSE:
        UpdateNormalPause(dt);
        break;
    }

    UpdateSound(dt);
}

void FootingMovable::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    if (m_model.get() == 0) {
        return;
    }

    if (m_modelIndex != m_params.model) {
        int count = fn_8015FF44(m_model.get());

        m_modelIndex = m_params.model;
        if (m_modelIndex < 0) {
            m_modelIndex = 0;
        }
        if (m_modelIndex > count - 1) {
            m_modelIndex = count - 1;
        }

        m_target.reset();
        m_model->fn_80161880();
        m_model->fn_80162804();
        m_model->fn_80162A68(m_modelIndex, 1);
        m_model->fn_80161FC8(m_modelIndex, boost::weak_ptr<CharInfo>(m_charInfo));

        boost::weak_ptr<CharaColliAttack> attackWeak = m_model->fn_80161428(m_modelIndex);

        if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&FootingMovable::OnAttack, this, _1));
        }

        m_params.model = m_modelIndex;
        fn_800C9D68(owner, &m_params, sizeof(m_params));
    }

    UpdateModelMatrix();

    Fn8040Vector3 start = m_charInfo->m_position;
    Fn8040Vector3 end(m_params.param0, m_params.param1, m_params.param2);

    fn_8040CFF8(&end, &start);
    fn_802E0B58(UnkSegment_80020A20(start, end), lbl_805F06D8);
    m_model->fn_8016108C();
}

void FootingMovable::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E50(owner, &m_comId);
    m_angle = 0.0f;
    m_type = m_params.type;
    m_flags.Clear();
    m_target.reset();

    if (m_params.pauseDamage > 0.0f) {
        m_flags.Set(FLAG_DAMAGE_PAUSE);
    } else {
        m_flags.Reset(FLAG_DAMAGE_PAUSE);
    }

    switch (m_type) {
    case 0:
        m_state = STATE_NORMAL_START;
        break;
    case 1:
        m_state = STATE_PATH_START;
        break;
    case 2:
        m_state = STATE_LOOP_PATH_START;
        break;
    case 4:
        m_state = STATE_NORMAL_SYNC_START;
        break;
    case 5:
        m_state = STATE_PATH_SYNC_START;
        break;
    case 6:
        m_state = STATE_NORMAL_PAUSE_START;
        break;
    default:
        m_state = STATE_WAIT_SWITCH;
        break;
    }

    switch (m_type) {
    case 0:
    case 3:
    case 6:
        if (m_comId > 0) {
            m_state = STATE_WAIT_SWITCH;
        }
        break;
    }

    m_start = m_charInfo->m_position;
    m_angleOffset = 180.0f - 180.0f * m_params.pos;
    m_syncTime = 0.0f;
}

void FootingMovable::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }
}

void FootingMovable::MovePause(float dt) {
    m_angle += dt * (20.0f * m_params.acc);

    Fn8040Vector3 offset(m_params.param0, m_params.param1, m_params.param2);
    /* Matching workaround: end is never read. The original computes it all the same (the calls
     * to fn_8040CDD0 and the Fn8040Vector3 copy constructor at USA (GUPE8P) 0x80149E60 and
     * 0x80149E6C); without it the function is 93 words with two calls fewer instead of 100.
     * Revisit when the original's use of end is understood. */
    Fn8040Vector3 end = m_start + offset;
    float halfLength = 0.5f * fn_8040CF50(&offset);

    offset = fn_8040CEE8(offset);
    if (m_flags.Test(FLAG_TO_END)) {
        if (m_angle > 180.0f) {
            m_angle = 180.0f;
            m_flags.Reset(FLAG_TO_END);
            m_flags.Set(FLAG_TO_START);
            m_flags.Set(FLAG_PAUSE);
            m_timer = m_params.pause;
        }
    } else if (m_flags.Test(FLAG_TO_START)) {
        if (m_angle > 360.0f) {
            m_angle = 0.0f;
            m_flags.Reset(FLAG_TO_START);
            m_flags.Set(FLAG_TO_END);
            m_flags.Set(FLAG_PAUSE);
            m_timer = m_params.pause;
        }
    }

    fn_8040CE38(&offset, halfLength - halfLength * std::cos(fn_8000AB90(m_angle)));
    m_charInfo->m_position = offset + m_start;
}

void FootingMovable::UpdateSound(float dt) {
    if (m_flags.Test(FLAG_SOUND)) {
        if (m_flags.Test(FLAG_SOUND_START)) {
            m_sound.fn_801D455C();
            m_lastPosition = m_charInfo->m_position;
            m_flags.Reset(FLAG_SOUND_START);
        } else {
            if (fn_8000A950(dt)) {
                m_charInfo->field_0x18 = lbl_805E07A0;
            } else {
                m_charInfo->field_0x18 = (m_charInfo->m_position - m_lastPosition) * (1.0f / dt);
            }

            if (fn_8040CF50(&m_charInfo->field_0x18) > s_moveSoundSpeed) {
                if (m_sound.field_0x0 == 0) {
                    m_sound = fn_801D3C38(m_moveSound, &m_charInfo->m_position, this);
                }
            } else if (m_sound.field_0x0 != 0) {
                m_sound.fn_801D455C();
            }

            m_lastPosition = m_charInfo->m_position;
        }
    }
}

void FootingMovable::OnAttack(AttackCallbackParam &param) {
    if (m_flags.Test(FLAG_DAMAGE_PAUSE)) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 0x140) == 0) {
                param.field_0x54 = 5;
                return;
            }

            m_resumeState = m_state;
            m_state = STATE_DAMAGE_PAUSE;
            m_timer = m_params.pauseDamage;
            m_flags.Reset(FLAG_DAMAGE_PAUSE);
            param.field_0x54 = 1;
            return;
        }
    }

    param.field_0x54 = 5;
}

void FootingMovable::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 0x17: {
        SetTransparentCommand &transparent = static_cast<SetTransparentCommand &>(command);

        if (transparent.m_enable) {
            if (m_model.get() != 0) {
                fn_8015FDD8(m_model.get(), transparent.m_alpha);

                boost::shared_ptr<CharaColliBase> probe = m_model->fn_80161614(m_modelIndex).lock();

                if (probe.get() != 0) {
                    int count = probe->UnkSlot0();

                    if (count > 16) {
                        count = 16;
                    }
                    for (int i = 0; i < count; i++) {
                        int flags = probe->UnkSlot10(i);

                        m_probeFlags[i] = flags;
                        flags |= s_transparentFlag;
                        probe->UnkSlot20(flags, i);
                    }
                }
            }
        } else if (m_model.get() != 0) {
            fn_8015FD10(m_model.get());

            boost::shared_ptr<CharaColliBase> probe = m_model->fn_80161614(m_modelIndex).lock();

            if (probe.get() != 0) {
                int count = probe->UnkSlot0();

                if (count > 16) {
                    count = 16;
                }
                for (int i = 0; i < count; i++) {
                    probe->UnkSlot20(m_probeFlags[i], i);
                }
            }
        }
        break;
    }
    }
    command.m_flags |= 2;
}

void FootingMovable::SetupTarget() {
    if (m_target.get() == 0 && m_model.get() != 0) {
        m_target = fn_80166AF4();

        UnkWeakHolder_800CC6C8 weakTarget;

        weakTarget.ptr = m_target;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
    }
    if (m_target.get() != 0) {
        m_target->fn_80167010(-1);
    }
}

} // namespace FootingMovable

extern "C" void fn_801498B4(void) {
    fn_8006B474("FOOTINGMOVABLE.BIN", 1);
}

extern "C" void fn_801498B0(void) {}

void FootingMovable::FootingMovable::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new FootingMovable(fn_801EB304(fn_8001C4F8(), 11), owner);
}

/* The names of the values of the "type" parameter, and the labels of "spd", "acc", "pause",
 * "pos", "param0", "param1" and "param2" for each type. */
static const char *s_typeLabels[7] = {
    "type : normal",       "type : path",       "type : loop path",     "type : one way",
    "type : normal(sync)", "type : path(sync)", "type : normal(pause)",
};
static const char *s_spdLabels[7] = {"spd", "spd", "N/A", "spd", "spd", "spd", "N/A"};
static const char *s_accLabels[7] = {"acc", "acc", "acc", "N/A", "N/A", "N/A", "acc"};
static const char *s_pauseLabels[7] = {"N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "pause(sec)"};
static const char *s_posLabels[7] = {
    "N/A", "N/A", "N/A", "N/A", "Init Pos(0.0-1.0)", "Init Pos(0.0-1.0)", "N/A",
};
static const char *s_param0Labels[7] = {"X", "path id", "path id", "X", "X", "path id", "X"};
static const char *s_param1Labels[7] = {"Y", "N/A", "N/A", "Y", "Y", "N/A", "Y"};
static const char *s_param2Labels[7] = {"Z", "N/A", "N/A", "Z", "Z", "N/A", "Z"};

/* The label callback of the descriptor's "type", "spd", "acc", "pause", "pos", "param0", "param1"
 * and "param2" entries: the labels of the parameters after "type" follow the value of the last
 * "type" label request (behaviour inferred). */
static const char *fn_80149744(UnkParamCursor_800CBC20 *cursor) {
    static s32 s_type = 0;

    if (cursor->m_index == 1) {
        s_type = cursor->m_value;
        return s_typeLabels[cursor->m_value];
    } else if (cursor->m_index == 2) {
        return s_spdLabels[s_type];
    } else if (cursor->m_index == 3) {
        return s_accLabels[s_type];
    } else if (cursor->m_index == 4) {
        return s_pauseLabels[s_type];
    } else if (cursor->m_index == 5) {
        return s_posLabels[s_type];
    } else if (cursor->m_index == 6) {
        return s_param0Labels[s_type];
    } else if (cursor->m_index == 7) {
        return s_param1Labels[s_type];
    } else if (cursor->m_index == 8) {
        return s_param2Labels[s_type];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "model", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "type", 0, 0, 6, 0.0f, 0.0f, 0.0f, fn_80149744, 0},
    {4, "spd", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_80149744, 0},
    {4, "acc", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_80149744, 0},
    {4, "pause", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_80149744, 0},
    {4, "pos", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_80149744, 0},
    {4, "param0", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_80149744, 0},
    {4, "param1", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_80149744, 0},
    {4, "param2", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_80149744, 0},
    {4, "pause damage", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8052A0B0 = {
    "MOVE_FOOTING",
    fn_801498B4,
    fn_801498B0,
    FootingMovable::FootingMovable::Create,
    0,
    0,
    0xF,
    1,
    0,
    10,
    s_paramInfo,
};

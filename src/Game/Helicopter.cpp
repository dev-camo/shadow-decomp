/* The Helicopter gadget (class described in Game/Helicopter.h). File name inferred from the
 * class. Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first).
 *
 * The character info's command handler, the callbacks of the two bar handle targets and the
 * attack callback of the model's attack object are boost::function objects built with
 * boost::bind(&Helicopter::method, this, _1); their managers, invokers and copy functions come
 * from the boost headers. */
#include <cmath>
/* Matching workaround: TNumEdit.h and CharaColliBaseCharInfo.h are included only for the RTTI
 * records that their never-called stand-in functions emit (see the notes there); no code in this
 * file uses TNumEdit or CharaColliBaseCharInfo. TNumEdit.h comes first, in the order their names
 * appear in the unit's .rodata (alphabetical order would reverse them). Revisit when the original
 * inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/Helicopter.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8006_resource_list.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

/* Half the length of each bar along z (name inferred): the bar segments end this far from the bar
 * centres, and the constructor passes it to fn_800C78D4 as the first float for both bar probes,
 * where the ELEC_BAR gadget passes half its bar length. */
static const float s_barHalfLength = 13.5f;

/* The centres of the two bars and the bars as segments along z, in the model's space (names
 * inferred: the bar probes sit at the centres, the bar Targets follow them and OnCollision hangs
 * the character on the segments).
 *
 * The original's .bss for this file is 0x168 bytes (0x80584B78-0x80584CE0 in USA (GUPE8P)) and
 * holds these four objects at 0xC, 0x18, 0x24 and 0x3C. The static initialiser forms the pooled
 * base address 0x80584B78, the first byte of the 0xC bytes before them, but no instruction in the
 * file reads or writes those 0xC bytes or the 0x114 bytes after the objects, and what occupies
 * them is unknown. This source's .bss is the four objects alone (0x48 bytes, at 0x0, 0xC, 0x18
 * and 0x30), so the displacements from the pooled base are 0xC lower: nine words of the static
 * initialiser differ (the other functions reach the objects through their own symbols and match).
 * Revisit when the original declarations of the objects around them are found. */
static Fn8040Vector3 s_barCenterA(13.25f, -19.3f, 0.325f);
static Fn8040Vector3 s_barCenterB(-13.25f, -19.3f, 0.325f);
static UnkSegment_80020A20 s_barA(s_barCenterA + Fn8040Vector3(0.0f, 0.0f, s_barHalfLength),
                                  s_barCenterA + Fn8040Vector3(0.0f, 0.0f, -s_barHalfLength));
static UnkSegment_80020A20 s_barB(s_barCenterB + Fn8040Vector3(0.0f, 0.0f, s_barHalfLength),
                                  s_barCenterB + Fn8040Vector3(0.0f, 0.0f, -s_barHalfLength));

const char *Helicopter::s_name = "Helicopter";

Helicopter::Helicopter(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_states(0), m_hangAngles(0.0f, 0.0f, 0.0f) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x53, 4);
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Helicopter::OnCommand, this, _1));
    ReadOwner(owner);

    m_target = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0xA800);
    m_barTargetA = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x8400);
    m_barTargetB = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x8400);

    m_model = fn_8015F800(1, "HELICOPTER.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160C28(0);
    m_model->fn_8016082C(0);
    m_model->fn_8015FF60();
    m_model->fn_80160B18(0);
    m_model->fn_80161BD8(0, boost::weak_ptr<CharInfo>(m_charInfo), 0);

    {
        boost::shared_ptr<CharaColliAttack> attack = m_model->fn_80161428(0).lock();

        if (attack.get() != 0) {
            attack->fn_80096B64(1, AttackCallback(boost::bind(&Helicopter::OnAttack, this, _1)));
        }
    }
    {
        boost::shared_ptr<CharaColliBase> probeA =
            fn_800C78D4(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0x10000002, 7, 0,
                        &s_barCenterA, lbl_805E07B8, 0, s_barHalfLength, 1.0f);

        probeA->SetField0x2C(1);
        m_barHandleA = fn_8015B8C4(probeA);
        m_barHandleA->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&Helicopter::OnCollisionBarA, this, _1)));

        boost::shared_ptr<CharaColliBase> probeB =
            fn_800C78D4(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0x10000002, 7, 0,
                        &s_barCenterB, lbl_805E07B8, 0, s_barHalfLength, 1.0f);

        probeB->SetField0x2C(1);
        m_barHandleB = fn_8015B8C4(probeB);
        m_barHandleB->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&Helicopter::OnCollisionBarB, this, _1)));
    }

    UpdateModelMatrix();

    m_effectTarget = fn_80166AF4();
    {
        UnkWeakHolder_800CC6C8 weakTarget;

        weakTarget.ptr = m_effectTarget;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        m_effectTarget->fn_80167010(-1);
    }

    m_sound = fn_801D3DC8(0x713B, &m_charInfo->m_position, this);
}

Helicopter::~Helicopter() {
    m_sound.fn_801D455C();
}

extern "C" void fn_802F8594(void) {
    fn_8006B474("HELICOPTER.BIN", 1);
}

extern "C" void fn_802F8590(void) {}

void Helicopter::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Helicopter(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void Helicopter::UpdateBody(float dt) {
    if (m_states.Test(STATE_DESTROYED)) {
        if (m_effectTarget.get() != 0) {
            m_effectTarget->fn_80166F88();
            m_effectTarget.reset();
        }
        m_model->fn_80162790();
        m_model->fn_80161880();

        UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

        effect.field_0x44 =
            static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, m_model->fn_8015F7BC(1000)->name));
        effect.field_0x40 = m_charInfo->m_position.y;
        effect.field_0x34 = 1.0f;
        effect.field_0x4A = 0;
        effect.field_0x54 = 1;
        fn_8015D93C(&effect);

        fn_8000D664(fn_80009548(), 0, 0.7f, 1.6f, 8.0f);
        ReleaseRider();
        m_sound.fn_801D455C();
        fn_801D54F4(fn_80053164(), 0x713C, &m_charInfo->m_position, 0, 1);
        fn_800C9AD4(m_owner);
        m_flags |= 1;
    } else {
        Fn8040Vector3 movement = m_charInfo->m_position;

        switch (m_params.moveType) {
        case MOVE_STRAIGHT:
            MoveStraight(dt);
            break;
        case MOVE_CIRCLE:
            MoveCircle(dt);
            break;
        }

        movement = m_charInfo->m_position - movement;
        if (fn_8000A950(dt)) {
            m_charInfo->field_0x18 = lbl_805E07A0;
        } else {
            m_charInfo->field_0x18 = movement * (1.0f / dt);
        }
        UpdateTilt(dt);
    }

    if (m_states.Test(STATE_RIDDEN)) {
        boost::shared_ptr<CharInfo> rider = m_rider.lock();

        if (rider.get() != 0) {
            SendTouchHang(rider);
        } else {
            ReleaseRider();
        }
    }

    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
    fn_8015BE9C(m_barHandleA.get(), dt);
    fn_8015BE9C(m_barHandleB.get(), dt);
    if (m_effectTarget.get() != 0) {
        m_effectTarget->fn_8016761C(dt);
    }
}

void Helicopter::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();

    switch (m_params.moveType) {
    case MOVE_STRAIGHT: {
        UnkSegment_80020A20 line(m_charInfo->m_position,
                                 m_charInfo->m_position + Fn8040Vector3(m_params.moveParam0,
                                                                        m_params.moveParam1,
                                                                        m_params.moveParam2));

        fn_802E0AA0(line, lbl_805F06D8, 5.0f);
        break;
    }
    case MOVE_CIRCLE: {
        Fn8040Vector3 center;

        fn_800C9F14(owner, &center);

        fn_802E0C60(center, UnkRotation3(0.5f * lbl_805FAD38, 0.0f, 0.0f), 0, 2.5f,
                    m_params.moveParam0);
        break;
    }
    }
}

void Helicopter::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    m_charInfo->m_rotation.x = 0.0f;
    m_charInfo->m_rotation.z = 0.0f;
    fn_8040CE60(&m_charInfo->field_0x18);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    if (m_params.moveType == MOVE_CIRCLE) {
        Fn8040Vector3 offset = Fn8040Vector3(0.0f, 0.0f, -m_params.moveParam0);

        offset = fn_8040D0AC(offset, DegreesToRadians(m_params.moveParam2));
        AddVector(&m_charInfo->m_position, offset);

        if (!fn_8000A950(m_params.moveParam0) && !fn_8000A950(m_params.moveParam1)) {
            Fn8040Vector3 forward = fn_8040CFB8(offset, Fn8040Vector3(0.0f, 1.0f, 0.0f));

            fn_8040CE74(&forward);
            if (m_params.moveParam1 < 0.0f) {
                fn_8040CE38(&forward, -1.0f);
            }

            UnkRotation3 angles = fn_8040F7F8(&forward);

            m_charInfo->m_rotation.y = angles.y;
        }
    }

    m_states.Clear();
    m_moveState = 0;
    m_moveBack = 0;
    m_moveTimer = 0.0f;
    m_health = 10.0f;
    m_turnSpeed = 0.0f;
}

void Helicopter::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    m_model->fn_801633DC(&matrix);

    fn_800CCE38(m_target.get(), m_charInfo->m_position);
    fn_800CCE38(m_barTargetA.get(), matrix * s_barCenterA);
    fn_800CCE38(m_barTargetB.get(), matrix * s_barCenterB);

    if (m_states.Test(STATE_RIDDEN)) {
        m_hangPosition = matrix * m_hangOffset;

        Fn8040Vector3 direction = m_hangPosition - m_charInfo->m_position;

        direction.y = 0.0f;
        fn_8040CE74(&direction);
        m_hangAngles = fn_8040F7F8(&direction);
    }
}

void Helicopter::MoveStraight(float dt) {
    m_moveTimer += dt;

    switch (m_moveState) {
    case 0:
        if (m_moveTimer < m_params.moveSec && !fn_8000A950(m_params.moveSec)) {
            float phase = m_moveTimer / m_params.moveSec;
            float wave = std::cos(phase * lbl_805FAD38);
            float progress = 0.5f * (1.0f - wave);
            Fn8040Vector3 move(m_params.moveParam0, m_params.moveParam1, m_params.moveParam2);

            fn_800C9F14(m_owner, &m_charInfo->m_position);
            if (m_moveBack == 0) {
                AddVector(&m_charInfo->m_position, move * progress);
            } else {
                m_charInfo->m_position = m_charInfo->m_position + move * (1.0f - progress);
            }
        } else {
            m_moveTimer = 0.0f;
            m_moveState = 1;
        }
        break;

    case 1:
        if (m_moveTimer > m_params.pauseSec) {
            m_moveTimer = 0.0f;
            if (m_moveBack == 0) {
                m_moveBack = 1;
            } else {
                m_moveBack = 0;
            }
            m_moveState = 0;
        }
        break;
    }
}

void Helicopter::MoveCircle(float dt) {
    if (!fn_8000A950(m_params.moveParam1)) {
        Fn8040Vector3 center;

        fn_800C9F14(m_owner, &center);

        float angle = m_params.moveParam1 * dt / m_params.moveParam0;
        Fn8040Vector3 offset = m_charInfo->m_position - center;

        fn_8040CE74(&offset);
        offset = fn_8040D0AC(offset, angle);
        fn_8040CE38(&offset, m_params.moveParam0);
        m_charInfo->m_position = center + offset;
    }
}

/* Whether the squared length of vector is below 0.0001 (name inferred).
 * Matching workaround: this inline function stands for an unknown original helper. The original
 * computes this test in place but keeps the character info pointer's operator-> out of line where
 * UpdateTilt reads field_0x18 for it (USA (GUPE8P) 0x802F79C8), as an inline call that takes the
 * vector does; written as an expression on the member, the operator is expanded in place and
 * UpdateTilt is 2 words shorter. Revisit when the original helper is identified. */
static inline bool IsShort(const Fn8040Vector3 &vector) {
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z < 0.0001f;
}

void Helicopter::UpdateTilt(float dt) {
    if (!IsShort(m_charInfo->field_0x18)) {
        /* Matching workaround: horizontal is declared before up. The original keeps up at the
         * lower stack offset (0x60, horizontal at 0x6C), which this compiler gives to the
         * later-declared local; declared the other way round, 12 words of UpdateTilt differ (the
         * stack offsets). Revisit if the original order of these declarations is found. */
        Fn8040Vector3 horizontal;
        Fn8040Vector3 up(0.0f, 1.0f, 0.0f);

        fn_8040F990(0, &horizontal, &m_charInfo->field_0x18, &up);
        if (!IsShort(horizontal)) {
            Fn8040Vector3 forward(0.0f, 0.0f, 1.0f);

            forward = fn_8040D0AC(forward, m_charInfo->m_rotation.y);

            float angle = fn_8040F8FC(&forward, &horizontal);

            if (!fn_8000A950(angle)) {
                m_turnSpeed += dt * DegreesToRadians(10.0f);
                m_turnSpeed = fn_80070460(m_turnSpeed, 0.0f, DegreesToRadians(15.0f));
                angle = fn_80070460(angle, 0.0f, m_turnSpeed * dt);

                Fn8040Vector3 axis = fn_8040CFB8(forward, horizontal);

                if (IsShort(axis)) {
                    axis = up;
                }

                Fn8040Matrix34 turn;

                fn_8040FD8C(&turn, &axis, angle);
                forward = turn * forward;
                m_charInfo->m_rotation.y = fn_8040F7F8(&forward).y;
            } else {
                m_turnSpeed = 0.0f;
            }

            float lean = fn_8040CF50(&horizontal) / 60.0f;

            lean = fn_80070460(lean, 0.0f, 1.0f);
            m_charInfo->m_rotation.x = lean * DegreesToRadians(16.0f);
        } else {
            m_charInfo->m_rotation.x = 0.0f;
        }
    }
}

void Helicopter::SendTouchHang(boost::shared_ptr<CharInfo> rider) {
    rider->fn_801DC980(new Player::TouchHangCommand(boost::weak_ptr<CharInfo>(m_charInfo),
                                                    &m_hangPosition, &m_hangAngles, 3, 4));
}

void Helicopter::OnCollisionBarA(ColliDetectArgs &args) {
    OnCollision(args, true);
}

void Helicopter::OnCollisionBarB(ColliDetectArgs &args) {
    OnCollision(args, false);
}

void Helicopter::OnCollision(ColliDetectArgs &args, bool barA) {
    if (args.m_hits == 0) {
        return;
    }

    boost::shared_ptr<CharInfo> player = fn_800A8B74(fn_80074194(), lbl_805EE588).lock();

    for (UnkProbeIterator_80027374 iterator(args.m_hits->fn_8001EE04());
         iterator.node != args.m_hits->fn_8001EDEC(); fn_80027374(&iterator, 0)) {
        boost::shared_ptr<CharInfo> charInfo =
            fn_800C4568(fn_80021B08(), fn_800587A0(&iterator.node->target)).lock();

        if (charInfo.get() != 0 && player == charInfo) {
            Fn8040Matrix34 matrix;

            fn_8040C148(&matrix, &lbl_805E0720);
            fn_8040B59C(&matrix, m_charInfo->m_position);
            fn_8040B848(&matrix, m_charInfo->m_rotation.y);
            fn_8040B884(&matrix, m_charInfo->m_rotation.x);

            /* The bar in the gadget's current placement. */
            const UnkSegment_80020A20 &bar = barA ? s_barA : s_barB;
            /* Copy-initialised: the original copies the constructed segment with the segment's
             * out-of-line copy constructor. */
            UnkSegment_80020A20 line = UnkSegment_80020A20(matrix * bar.start, matrix * bar.end);

            m_hangPosition = fn_8040FC3C(&line, &m_charInfo->m_position);

            /* The original copies the inverse into a second stack matrix (fn_8040C148) and
             * transforms with the copy, as an inline function returning the inverse by value
             * would (the function at USA (GUPE8P) 0x802388B8 has the same shape). Written as such
             * a function, the call is kept out of line here (the local matrix's inline default
             * constructor counts against the inlining limit), so the source transforms with the
             * inverse directly; see the unit's entry in configure.py. */
            Fn8040Matrix34 inverse;

            fn_8040BEF8(&inverse, &matrix);
            m_hangOffset = inverse * m_hangPosition;
            SendTouchHang(player);
            m_states.Set(STATE_TOUCHED);
            return;
        }
    }
}

void Helicopter::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 11:
        if (m_states.Test(STATE_TOUCHED)) {
            m_states.Reset(STATE_TOUCHED);
            m_rider = static_cast<RideCommand &>(command).m_rider;
            m_barHandleA->SetFlags(2);
            m_barHandleB->SetFlags(2);
            m_states.Set(STATE_RIDDEN);
        }
        break;
    case 12:
        ReleaseRider();
        break;
    }
    command.m_flags |= 2;
}

void Helicopter::ReleaseRider() {
    m_states.Reset(STATE_RIDDEN);
    m_states.Reset(STATE_TOUCHED);
    m_barHandleA->ClearFlags(2);
    m_barHandleB->ClearFlags(2);
    m_barHandleA->SuspendProbe(1.0f);
    m_barHandleB->SuspendProbe(1.0f);
    m_rider.reset();
}

void Helicopter::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;

    boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock();

    if (attack.get() != 0 && (attack->field_0x40 & 2) == 0) {
        param.field_0x54 = 1;
        if (!m_states.Test(STATE_DESTROYED)) {
            m_health -= param.field_0x50;
            if (m_health < 0.0f || fn_8000A950(m_health)) {
                param.field_0x54 = 3;

                boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);

                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x39);
                }
                m_states.Set(STATE_DESTROYED);
            }
        }
    }
}

/* The labels of "MoveType" and of the parameters after it, for "Straight" and "Circle". */
static const char *s_moveTypeLabels[2] = {"Straight", "Circle"};
static const char *s_moveParam0Labels[2] = {"MoveLengthX", "CircleRadius"};
static const char *s_moveParam1Labels[2] = {"MoveLengthY", "MoveSpd (point/sec)"};
static const char *s_moveParam2Labels[2] = {"MoveLengthZ", "InitPos (deg)"};
static const char *s_moveSecLabels[2] = {"MoveSec", "Not Used"};
static const char *s_pauseSecLabels[2] = {"PauseSec", "Not Used"};

/* The label callback of the descriptor's entries: the labels of the parameters after "MoveType"
 * follow the value of the last "MoveType" label request (behaviour inferred). */
static const char *fn_802F72A4(UnkParamCursor_800CBC20 *cursor) {
    static s32 s_moveType = 0;

    if (cursor->m_index == 0) {
        s_moveType = cursor->m_value;
        return s_moveTypeLabels[s_moveType];
    } else if (cursor->m_index == 1) {
        return s_moveParam0Labels[s_moveType];
    } else if (cursor->m_index == 2) {
        return s_moveParam1Labels[s_moveType];
    } else if (cursor->m_index == 3) {
        return s_moveParam2Labels[s_moveType];
    } else if (cursor->m_index == 4) {
        return s_moveSecLabels[s_moveType];
    } else if (cursor->m_index == 5) {
        return s_pauseSecLabels[s_moveType];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "MoveType", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_802F72A4, 0},
    {4, "MoveParam0", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_802F72A4, 0},
    {4, "MoveParam1", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_802F72A4, 0},
    {4, "MoveParam2", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_802F72A4, 0},
    {4, "MoveSec", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_802F72A4, 0},
    {4, "PauseSec", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_802F72A4, 0},
    {0},
};

GadgetTypeInfo lbl_8055A508 = {
    "Helicopter", fn_802F8594, fn_802F8590, Helicopter::Create, 0, 0, 0xFA2, 1, 0, 10, s_paramInfo,
};

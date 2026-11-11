/* Meteor (class name from RTTI; the gadget's descriptor is "METEOR_L"): a meteor that waits until a
 * character comes within its range, then flies along its direction while turning, shifts its
 * model colour in the last second of its lifetime and breaks up into MeteorsTask fragments at up
 * to three model parts, with an effect and a sound, when the lifetime runs out or attacks have used
 * up its health (behaviour inferred). File name inferred from the class. Method names marked
 * "(name inferred)" are not original. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first). */
/* Matching workaround: TNumEdit.h and CharaColliBaseCharInfo.h are included for the RTTI data that
 * their never-called stand-in functions emit (see TNumEdit.h); this file uses neither class.
 * TNumEdit.h comes first, in the order their names appear in the unit's .rodata (alphabetical order
 * would reverse them). Revisit when the original inline functions behind that data are found. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/AngleConversion.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/Meteor.h"
#include "Game/Meteors.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkModelAtomicState.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The ids of the model parts at which the meteor breaks into fragments (name inferred). */
static const s32 s_partIds[3] = {1, 2, 3};

/* The model colour at the start and at the end of the last second of the lifetime (names
 * inferred). The first byte is 0xFF in both and the update interpolates the other three, which
 * suggests alpha, red, green and blue (inferred). */
static UnkDebugColor_802E0E84 s_startColor(0xFF, 0x4B, 0x1B, 0x1B);
static UnkDebugColor_802E0E84 s_endColor(0xFF, 0xFF, 0x73, 0x00);

/* The fragments' move speed (meaning inferred from Meteors' use of the setting).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of this variable.
 * The original's start-up function in .ctors (0x8021DD64) loads the value and stores it to this
 * .sbss word (0x805EFD5C); a constant float initialiser would be emitted as initialised .sdata data
 * instead, and the shared stand-in reproduces that store. The original type is unknown. Revisit
 * when the original wrapper type is found. */
static UnkStartupFloat_800E2570 lbl_805EFD5C(100.0f);

const char *Meteor::s_name = "Meteor";

Meteor::Meteor(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner), m_state(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x61, 4);
    ReadOwner(owner);
    m_object = fn_801E72DC(0xA800);
    m_model = fn_8015F800(1, "METEOR.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80162424(0);
    UnkModelAtomicState *state = m_model->fn_80162298(0);
    state->color = s_startColor;
    UpdateModelMatrix();

    m_target = fn_80166AF4();
    {
        UnkWeakHolder_800CC6C8 weakTarget;
        weakTarget.ptr = m_target;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        m_target->fn_80167010(-1);
    }

    m_model->fn_80161FC8(0, m_charInfo);

    boost::shared_ptr<CharaColliAttack> attack(m_model->fn_80161428(0).lock());
    if (attack.get() != 0) {
        attack->fn_80096B64(1, AttackCallback(boost::bind(&Meteor::OnAttack, this, _1)));
        attack->fn_80054DD0(4, 1, AttackCallback(boost::bind(&Meteor::fn_8021CA5C, this, _1)),
                            AttackCallback(boost::bind(&Meteor::fn_8021CA58, this, _1)), 5.0f);
    }
}

Meteor::~Meteor() {}

extern "C" void fn_8021D330(void) {
    fn_8006B474("METEOR.BIN", 1);
    fn_8006B474("METEORS.BIN", 1);
}

extern "C" void fn_8021D32C(void) {}

void Meteor::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Meteor(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "RANGE m", 0, 0, 0, 50.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MOVE_SPD m/s", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "DIRECTION_X deg", 0, 0, 0, 0.0f, -180.0f, 180.0f, fn_800CBC20, 0},
    {4, "DIRECTION_Y deg", 0, 0, 0, 0.0f, -180.0f, 180.0f, fn_800CBC20, 0},
    {4, "ROT_SPD_X deg/s", 0, 0, 0, 0.0f, -1080.0f, 1080.0f, fn_800CBC20, 0},
    {4, "ROT_SPD_Z deg/s", 0, 0, 0, 0.0f, -1080.0f, 1080.0f, fn_800CBC20, 0},
    {4, "ROT_SPD_Y deg/s", 0, 0, 0, 0.0f, -1080.0f, 1080.0f, fn_800CBC20, 0},
    {4, "LIFETIME s", 0, 0, 0, 5.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80545BCC = {
    "METEOR_L", fn_8021D330, fn_8021D32C, Meteor::Create, 0, 0, 0x138A, 1, 0, 10, s_paramInfo,
};

void Meteor::UpdateBody(float dt) {
    switch (m_state) {
    case 0: {
        boost::shared_ptr<CharInfo> nearest(
            fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock());
        if (nearest.get() != 0) {
            Fn8040Vector3 offset = nearest->m_position - m_charInfo->m_position;
            if (fn_8040CF50(offset) < m_params.range) {
                m_state = 1;
            }
        }
        break;
    }
    case 1: {
        m_charInfo->field_0x18 = m_direction * (m_params.moveSpeed * dt);
        AddVector(&m_charInfo->m_position, m_charInfo->field_0x18);

        m_timer -= dt;
        if (m_timer < 0.0f) {
            m_state = 2;
        } else if (m_timer < 1.0f) {
            float t = 1.0f - m_timer;
            float red;
            float green;
            float blue;

            red = t * (s_endColor.field_0x1 - s_startColor.field_0x1);
            green = t * (s_endColor.field_0x2 - s_startColor.field_0x2);
            blue = t * (s_endColor.field_0x3 - s_startColor.field_0x3);
            red += s_startColor.field_0x1;
            red = fn_80070460(red, 0.0f, 255.0f);
            green += s_startColor.field_0x2;
            green = fn_80070460(green, 0.0f, 255.0f);
            blue += s_startColor.field_0x3;
            blue = fn_80070460(blue, 0.0f, 255.0f);

            UnkModelAtomicState *state = m_model->fn_80162298(0);
            u8 blueByte = blue;
            u8 greenByte = green;
            u8 redByte = red;

            state->color.field_0x0 = 0xFF;
            state->color.field_0x1 = redByte;
            state->color.field_0x2 = greenByte;
            state->color.field_0x3 = blueByte;
        }
        break;
    }
    case 2: {
        m_model->fn_80161A24(0);
        if (m_target.get() != 0) {
            m_target->fn_80166F88();
            m_target.reset();
        }
        m_model->fn_80162790();
        fn_801D54F4(fn_80053164(), 0x772F, &m_charInfo->m_position, 0, 1);

        for (u32 i = 0; i < 3; i++) {
            Fn8040Matrix34 matrix;

            if (fn_80169EF0(&matrix, m_model, 0, s_partIds[i])) {
                Fn8040Vector3 position(fn_8040FB08(&matrix));
                Fn8040Vector3 direction(position - m_charInfo->m_position);

                fn_8040CE74(&direction);

                UnkRotation3 angles(0.0f, 0.0f, 0.0f);
                fn_800494F8(&angles, &matrix);

                UnkRotation3 rotateSpeed(m_params.rotateSpeedX, m_params.rotateSpeedY,
                                         m_params.rotateSpeedZ);
                UnkMeteorsParams_8021FCE4 params(0.0f, lbl_805EFD5C.value, direction, rotateSpeed,
                                                 3.0f);
                new MeteorsTask(fn_801EB304(fn_8001C4F8(), 11), position, angles, params, 1);
            }
        }

        UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(1);
        parameters.field_0x44 = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, entry->name));
        parameters.field_0x40 = m_charInfo->m_position.y;
        parameters.field_0x34 = 1.0f;
        parameters.field_0x4A = 0;
        parameters.field_0x54 = 1;
        fn_8015D93C(&parameters);

        fn_800C9AD4(m_owner);
        m_flags |= 1;
        return;
    }
    }

    m_charInfo->m_rotation.x += m_params.rotateSpeedX * dt;
    m_charInfo->m_rotation.y += m_params.rotateSpeedY * dt;
    m_charInfo->m_rotation.z += m_params.rotateSpeedZ * dt;
    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void Meteor::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();

    /* Draws the range around the meteor and a line along its path, at least 100 long. */
    fn_802E0E84(&m_charInfo->m_position, lbl_805F06D8, m_params.range);

    float length = m_params.lifetime * m_params.moveSpeed;
    if (length < 100.0f) {
        length = 100.0f;
    }

    Fn8040Vector3 path = m_direction * length;
    UnkSegment_80020A20 segment(m_charInfo->m_position, m_charInfo->m_position + path);

    fn_802E0AA0(segment, UnkDebugColor_802E0E84(0xFF, 0xFF, 0xFF, 0), 10.0f);
}

void Meteor::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_params.range = 10.0f * m_params.range;
    m_params.moveSpeed = 10.0f * m_params.moveSpeed;
    m_params.directionX = DegreesToRadians(m_params.directionX);
    m_params.directionY = DegreesToRadians(m_params.directionY);
    m_params.rotateSpeedX = DegreesToRadians(m_params.rotateSpeedX);
    m_params.rotateSpeedZ = DegreesToRadians(m_params.rotateSpeedZ);
    m_params.rotateSpeedY = DegreesToRadians(m_params.rotateSpeedY);
    m_timer = m_params.lifetime;

    /* The direction: the z axis turned about the x axis, then about the y axis. */
    Fn8040Vector3 forward(0.0f, 0.0f, 1.0f);

    m_direction = forward;
    m_direction = fn_8040D118(m_direction, m_params.directionX);
    m_direction = fn_8040D0AC(m_direction, m_params.directionY);

    m_state = 0;
    m_health = 1.0f;
}

void Meteor::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    m_model->fn_801633DC(&matrix);
    fn_800CCE38(m_object.get(), m_charInfo->m_position);
}

void Meteor::fn_8021CA5C(AttackCallbackParam &param) {
    param.field_0x34 = m_direction;
    param.field_0x4C = m_params.moveSpeed;
    param.field_0x50 = 5.0f;
}

void Meteor::fn_8021CA58(AttackCallbackParam &param) {}

void Meteor::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 1;
    if (m_state != 2) {
        m_health -= param.field_0x50;
        if (m_health < 0.0f || fn_8000A950(m_health)) {
            param.field_0x54 = 3;

            boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);
            if (attacker.get() != 0) {
                fn_8020642C(attacker, 0x4A);
            }
            m_state = 2;
        }
    }
}

/* True when the character info's word at 0x5C is 1 and its word at 0x50 is 1, 2 or 3 (behaviour
 * inferred). Meteor does not call it; other files do. It lies between the end of ElecPillar.cpp's
 * code and Meteor's own functions and has no data of its own, so it is placed here as the last
 * definition of this file (the original file boundary at this function is not certain). */
extern "C" bool fn_8021C958(const boost::shared_ptr<CharInfo> &info) {
    if (info->field_0x5C == 1 &&
        (info->field_0x50 == 1 || info->field_0x50 == 2 || info->field_0x50 == 3)) {
        return true;
    }
    return false;
}

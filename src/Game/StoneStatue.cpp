/* StoneStatue (class name from RTTI; the gadget's descriptor is "STONE_STATUE"): a stone statue
 * that breaks when attacks have used up its health and, when it is active, turns to the nearest
 * character once one has come into its range (behaviour inferred). StoneStatueSetState (class
 * name from RTTI) is the state that the owner remembers between creations. File name inferred
 * from the class. Method names marked "(name inferred)" in the header are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/StoneStatue.h"
#include "Game/AngleConversion.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliAttack.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SetWeapon.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

/* How far below the statue's position (m_charInfo) the range cylinder starts (name inferred from
 * its uses).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of this variable.
 * The original's start-up function (0x800DD770) loads the float and stores it to the .sbss value at
 * 0x805EE824 (a constant float initialiser would be emitted as initialised .sdata data instead),
 * and the shared stand-in reproduces that function. Revisit when the original type is found. */
static UnkStartupFloat_800E2570 s_rangeDepth(50.0f);

/* The file of the break effect, by the parameter MIRROR (both entries name the same file). */
static const char *s_breakFiles[] = {"CN0000STONESTATUEBRK.DFF", "CN0000STONESTATUEBRK.DFF"};

const char *StoneStatue::s_name = "StoneStatue";

extern "C" void fn_800DD604(void) {
    fn_8006B474("STONESTATUE.BIN", 1);
}

extern "C" void fn_800DD600(void) {}

void StoneStatue::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new StoneStatue(fn_801EB304(fn_8001C4F8(), 11), owner);
}

StoneStatue::StoneStatue(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_tracking(false) {
    m_name = s_name;

    m_charInfo = fn_8015CD90(0x2F, 4);
    m_charInfo->field_0x30 = 1;

    if (owner->m_adapterState == 0) {
        fn_800C9930(owner, new StoneStatueSetState);
    }

    ReadOwner(owner);

    m_model = fn_8015F800(1, "STONESTATUE.BIN");
    m_model->fn_80162A68(m_params.mirror, 0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    {
        boost::weak_ptr<CharaColliBase> probe(m_model->fn_80161614(0));

        m_attack = fn_800C4374(fn_80021B08(), probe.lock());

        boost::shared_ptr<CharaColliAttack> attack(m_attack.lock());

        if (attack.get() != 0) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&StoneStatue::OnAttack, this, _1));
        }
    }

    m_object = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x2400);
    m_object->SetFlags(1);
    UpdateModelMatrix();

    m_target = fn_80166AF4();

    UnkWeakHolder_800CC6C8 weakTarget;
    weakTarget.ptr = m_target;
    m_model->fn_8015FA78(fn_80169D00, &weakTarget);
    m_target->fn_80167010(-1);
}

StoneStatue::~StoneStatue() {
    m_sound.fn_801D455C();
}

/* Whether two angles are equal within the zero tolerance of fn_8000B044 (name inferred). */
static bool IsSameAngle(float a, float b) {
    return fn_8000B044(a - b);
}

void StoneStatue::UpdateBody(float dt) {
    if (m_params.active != 0) {
        if (!m_tracking && IsCharacterInRange()) {
            fn_801D54F4(fn_80053164(), 0x7336, &m_charInfo->m_position, 0, 1);
            m_tracking = true;
        }

        if (m_tracking) {
            UnkRotation3 previousRotation = m_charInfo->m_rotation;

            TurnToCharacter(dt);

            if (IsSameAngle(previousRotation.y, m_charInfo->m_rotation.y)) {
                m_sound.fn_801D455C();
            } else if (m_sound.field_0x0 == 0) {
                m_sound = fn_801D3E58(0x7337, &m_charInfo->m_position, this);
            }
        }
    }

    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);

    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void StoneStatue::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    StoneStatueParams previous = m_params;

    ReadOwner(owner);

    if (m_params.mirror != previous.mirror) {
        m_model->fn_801625D8(m_params.mirror, 0);
    }

    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
    DrawRange();
}

void StoneStatue::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_params.rangeRadius = 10.0f * m_params.rangeRadius;
    m_params.rangeHeight = 10.0f * m_params.rangeHeight;
    m_tracking = false;

    dynamic_cast<StoneStatueSetState *>(m_owner->m_adapterState)->Reset();
}

void StoneStatue::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }

    if (m_object.get() != 0) {
        fn_800CCE38(m_object.get(), &m_charInfo->m_position);
    }
}

bool StoneStatue::IsCharacterInRange() {
    boost::shared_ptr<CharInfo> character =
        fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();

    if (character.get() == 0) {
        return false;
    }

    Fn8040Vector3 statuePosition;
    Fn8040Vector3 characterPosition;
    Fn8040Vector3 offset;
    Fn8040Vector3 offsetTemporary;

    fn_800091EC(&statuePosition, &m_charInfo->m_position);
    fn_800091EC(&characterPosition, &character->m_position);
    statuePosition.y = 0.0f;
    characterPosition.y = 0.0f;
    fn_8040CD74(&offsetTemporary, &characterPosition, &statuePosition);
    fn_800091EC(&offset, &offsetTemporary);

    if (fn_8040CF50(&offset) > m_params.rangeRadius) {
        return false;
    }

    float height = character->m_position.y - (m_charInfo->m_position.y - s_rangeDepth.value);

    if (height < 0.0f || height > m_params.rangeHeight) {
        return false;
    }

    return true;
}

void StoneStatue::TurnToCharacter(float dt) {
    boost::shared_ptr<CharInfo> character =
        fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();

    if (character.get() != 0) {
        Fn8040Vector3 statuePosition;
        Fn8040Vector3 characterPosition;
        Fn8040Vector3 facing;
        Fn8040Vector3 forward;
        Fn8040Vector3 axis;
        Fn8040Vector3 facingTemporary;
        Fn8040Vector3 turnedTemporary;
        Fn8040Vector3 axisTemporary;

        fn_800091EC(&statuePosition, &m_charInfo->m_position);
        fn_800091EC(&characterPosition, &character->m_position);
        statuePosition.y = 0.0f;
        characterPosition.y = 0.0f;
        fn_8040CD74(&facingTemporary, &characterPosition, &statuePosition);
        fn_800091EC(&facing, &facingTemporary);
        forward.Set(0.0f, 0.0f, 1.0f);
        fn_8040D0AC(&turnedTemporary, &forward, m_charInfo->m_rotation.y);
        fn_800091D0(&forward, &turnedTemporary);
        fn_8040CE74(&facing);
        fn_8040CFB8(&axisTemporary, &facing, &forward);
        fn_800091EC(&axis, &axisTemporary);

        float angle = fn_8040F8FC(&facing, &forward);
        float maxTurn = dt * DegreesToRadians(40.0f);

        if (fn_8000A980(angle) < DegreesToRadians(5.0f)) {
            return;
        }

        /* This adds angle whichever side the character is on, as the original does: angle is an
         * arc cosine and never negative, so this last step turns the wrong way when axis.y > 0. */
        if (fn_8000A980(angle) < maxTurn) {
            m_charInfo->m_rotation.y += angle;
        } else if (axis.y > 0.0f) {
            m_charInfo->m_rotation.y -= maxTurn;
        } else {
            m_charInfo->m_rotation.y += maxTurn;
        }
    }
}

void StoneStatue::OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param) {
    StoneStatueSetState *state = dynamic_cast<StoneStatueSetState *>(m_owner->m_adapterState);

    state->m_health -= param.field_0x50;

    if (state->m_health < 0.0001f) {
        if (fn_8004EAC0(this)) {
            {
                boost::shared_ptr<CharInfo> attacker(fn_8015D570(&param));

                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x41);
                }
            }

            Break();
        }

        param.field_0x54 = 3;
    } else {
        param.field_0x54 = 1;
    }
}

void StoneStatue::Break() {
    fn_801D54F4(fn_80053164(), 0x7335, &m_charInfo->m_position, 0, 1);

    {
        UnkEffectParams_8015F0B8 breakEffect(m_charInfo, 0);

        breakEffect.field_0x44 =
            static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, s_breakFiles[m_params.mirror]));
        breakEffect.field_0x40 = m_charInfo->m_position.y;
        breakEffect.field_0x34 = 1.0f;
        breakEffect.field_0x4A = 0;
        breakEffect.field_0x54 = 1;
        fn_8015D93C(&breakEffect);
    }

    if (m_params.active != 0) {
        fn_8011E938(0x1F, &m_charInfo->m_position, &m_charInfo->m_rotation);
    }

    fn_800C9B34(m_owner);
    m_flags |= 1;
}

void StoneStatue::DrawRange() {
    Fn8040Vector3 center;

    fn_800091EC(&center, &m_charInfo->m_position);
    center.y -= s_rangeDepth.value;

    float halfHeight = 0.5f * m_params.rangeHeight;

    center.y += halfHeight;

    UnkRotation3 rotation(0.5f * lbl_805FAD38, 0.0f, 0.0f);

    fn_802E0C60(&center, &rotation, 0, halfHeight, m_params.rangeRadius);
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "ACTIVE 0:off 1:On", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {2, "MIRROR 0:off 1:On", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "RANGE_RADIUS m", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "RANGE_HEIGHT m", 0, 0, 0, 6.5f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80523BE0 = {
    "STONE_STATUE", fn_800DD604, fn_800DD600, StoneStatue::Create, 0, 0, 0x838, 1, 0, 10,
    s_paramInfo,
};

/* ArkCannon (class name from RTTI; the gadget's descriptor is "ARKCANNON"). File name inferred from
 * the class. Method names marked "(name inferred)" in the header are not original. Definitions are
 * in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/ArkCannon.h"
#include "Game/CharaColliAttack.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/LockonTarget.h"
#include "Game/SharedConstants.h"
#include "Game/AngleConversion.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "layout_check.h"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The object at 0x24C of the character (placeholder name; the original class is unknown).
 * fn_80206574 returns a weak pointer to the UnkLockonObject_8005FDF4 that it holds at 0xC (see
 * LockonTarget.h). */
class UnkTargetSource_80206574 {
  public:
    /* Returns a weak pointer built from the member at 0xC (fn_80206598; behaviour inferred). */
    boost::weak_ptr<UnkLockonObject_8005FDF4> fn_80206574();
};

/* The object that fn_80095388 returns (placeholder name; the original class is unknown). Only the
 * pointer at 0x24C is used here. fn_80095388 has the shape of boost::static_pointer_cast (it passes
 * an empty tag byte and copies the pointer and its count unchanged), so this is probably a class
 * derived from CharInfo at offset 0, which field_0x0 covers (lead, not verified). */
class UnkCharacter_80095388 {
  public:
    /* 0x000 */ u8 field_0x0[0x24C];
    /* 0x24C */ UnkTargetSource_80206574 *field_0x24C;
};
/* Checks only the recovered view; the complete native extent remains unknown. */
CHECK_SIZEOF(UnkCharacter_80095388, 0x250);

/* The 0x48-byte block that the constructor at 0x80051D10 builds (placeholder name; the original
 * class is unknown). The constructor copies the owner and the lock-on target, stores the
 * translation of the matrix at 0x10, the matrix's rotation applied to a constant vector at 0x1C, a
 * copy of arg4 at 0x28 and arg2 at 0x34 (behaviour inferred); the destructor is at 0x8005C410. */
class UnkParams_80051D10 {
  public:
    UnkParams_80051D10(const boost::weak_ptr<CharInfo> &owner,
                       const boost::shared_ptr<Weapon::LockonTarget> &target, int arg2,
                       const Fn8040Matrix34 *matrix, const Fn8040Vector3 *arg4);
    ~UnkParams_80051D10();

    /* 0x00 */ boost::weak_ptr<CharInfo> m_owner;
    /* 0x08 */ boost::shared_ptr<Weapon::LockonTarget> m_target;
    /* 0x10 */ Fn8040Vector3 field_0x10;
    /* 0x1C */ Fn8040Vector3 field_0x1C;
    /* 0x28 */ Fn8040Vector3 field_0x28;
    /* 0x34 */ int field_0x34;
    /* An object that fn_80051EB4 constructs and fn_80051DBC destroys (type unknown). */
    /* 0x38 */ u8 field_0x38[0x10];
};
CHECK_SIZEOF(UnkParams_80051D10, 0x48);

/* The object that the callback at 0xC of UnkFactory_8051DDC8 returns (placeholder name; the
 * original class is unknown). fn_80051B58 constructs it when there is no callback. */
class UnkResult_80051B58;

/* A 0x14-byte record whose function at 0x8 allocates a 0x6C-byte object from the parameters
 * (placeholder names; behaviour inferred from fn_80056B8C, the function lbl_8051DDC8 holds). This
 * record holds null at 0x0, 0x4 and 0xC; other records in the same table hold functions there,
 * which fn_80051CC8, fn_80051C80 and fn_80051AF0 call when they are set. */
struct UnkFactory_8051DDC8 {
    /* fn_80051CC8 calls it without setting arguments; the known callees take none. */
    /* 0x00 */ void (*field_0x0)();
    /* fn_80051C80 calls it without setting arguments; the known callees take none. */
    /* 0x04 */ void (*field_0x4)();
    /* 0x08 */ void (*create)(const UnkParams_80051D10 *params);
    /* Called by fn_80051AF0 with the parameters and a float; its result is returned by value. */
    /* 0x0C */ UnkResult_80051B58 (*field_0xC)(const UnkParams_80051D10 *params, float arg);
    /* 0x10 */ float field_0x10;
};
CHECK_SIZEOF(UnkFactory_8051DDC8, 0x14);

extern UnkFactory_8051DDC8 lbl_8051DDC8;

extern "C" {
/* Returns the character as an UnkCharacter_80095388 (behaviour inferred: the pointer and its count
 * are copied unchanged). */
boost::shared_ptr<UnkCharacter_80095388> fn_80095388(const boost::shared_ptr<CharInfo> &character);
/* Returns a when a < b, otherwise b (behaviour inferred from its body). */
const float &fn_800704AC(const float &a, const float &b);
}

/* Converts metres to game units, as for the "RANGE m" parameter (name inferred). */
static inline float MetresToUnits(float metres) {
    return 10.0f * metres;
}

/* The break models of the two types. */
static const char *s_breakFiles[2] = {"A10000ARKCANNON_BRK.DFF", "A10000ARKCANNONAIR_BRK.DFF"};

/* The number of objects that stand around the cannon (name inferred). */
static const u32 kObjectCount = 2;

/* The ids of the model frames that the four shots are fired from (name inferred). */
static const int s_shotFrames[4] = {2, 4, 5, 3};

const char *ArkCannon::s_name = "ArkCannon";

/* The distance of the objects from the cannon (name inferred). The original stores it in its
 * static initialiser, as the shared startup float type does (see its header). */
static UnkStartupFloat_800E2570 s_objectDistance(7.5f);

ArkCannon::ArkCannon(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_state(0), m_attackFlags(0) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x64, 4);
    InitFromOwner(owner);
    m_object = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0xA400);
    m_objects.reserve(kObjectCount);
    for (u32 i = 0; i < kObjectCount; i++) {
        m_objects.push_back(fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x8800));
    }
    m_model = fn_8015F800(1, "ARKCANNON.BIN");
    SetupModel();
    UpdateModelMatrix();
}

ArkCannon::~ArkCannon() {}

extern "C" void fn_80227DB8(void) {
    fn_8006B474("ARKCANNON.BIN", 1);
}

extern "C" void fn_80227DB4(void) {}

void ArkCannon::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ArkCannon(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void ArkCannon::UpdateBody(float dt) {
    if (m_attackFlags.Test(1)) {
        m_model->fn_80161A24(m_params.type);
        m_model->fn_80162790();
        if (m_params.type == 0) {
            UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

            effect.field_0x44 =
                static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, s_breakFiles[m_params.type]));
            effect.field_0x40 = m_charInfo->m_position.y;
            effect.field_0x34 = 1.0f;
            effect.field_0x4A = 1;
            effect.field_0x54 = 1;
            fn_8015D93C(&effect);
        } else if (m_params.type == 1) {
            for (u32 i = 0; i <= m_params.type; i++) {
                UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

                effect.field_0x44 =
                    static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, s_breakFiles[i]));
                effect.field_0x40 = m_charInfo->m_position.y;
                effect.field_0x34 = 1.0f;
                effect.field_0x4A = 0;
                effect.field_0x54 = 1;
                fn_8015D93C(&effect);
            }
        }
        m_state = 4;
        m_attackFlags.Reset(1);
        fn_800C9B04(m_owner);
        m_flags |= 1;
        return;
    }

    switch (m_state) {
    case 0:
        if (IsTargetInRange()) {
            SetState(1);
        }
        break;

    case 1: {
        boost::shared_ptr<CharInfo> target = m_target.lock();

        if (target.get() != 0) {
            if (TurnToTarget(target, dt)) {
                SetState(2);
            }
        } else {
            SetState(3);
        }
        break;
    }

    case 2: {
        boost::shared_ptr<CharInfo> target = m_target.lock();

        if (target.get() != 0) {
            if (m_shotTimer < 0.0f || fn_8000A950(m_shotTimer)) {
                Fire(m_shotCount, target);
                m_shotCount++;
                if (m_shotCount >= 4) {
                    SetState(3);
                } else {
                    m_shotTimer = 0.5f;
                }
            } else {
                m_shotTimer -= dt;
            }
        } else {
            SetState(3);
        }
        break;
    }

    case 3:
        if (IsTargetInRange()) {
            if (m_reloadTimer < 0.0f || fn_8000A950(m_reloadTimer)) {
                SetState(1);
            } else {
                m_reloadTimer -= dt;
            }
        } else {
            SetState(0);
        }
        break;
    }

    m_model->fn_80162FB0(dt);
}

void ArkCannon::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ArkCannonParams previous = m_params;

    InitFromOwner(owner);

    if (m_params.type != previous.type) {
        m_model->fn_80162804();
        m_model->fn_80161880();
        SetupModel();
    }

    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
    fn_802E0E84(&m_charInfo->m_position, lbl_805F06D8, m_params.range);
}

void ArkCannon::InitFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_params.range = MetresToUnits(m_params.range);
    m_health = 16.0f;
    m_reloadTimer = 0.0f;
    m_shotTimer = 0.0f;
    m_shotCount = 0;
}

void ArkCannon::SetupModel() {
    m_model->fn_80162A68(m_params.type, 0);
    m_model->fn_80161FC8(m_params.type, m_charInfo);

    boost::shared_ptr<CharaColliAttack> attack = m_model->fn_80161428(m_params.type).lock();

    if (attack.get() != 0) {
        attack->fn_80096B64(1, AttackCallback(boost::bind(&ArkCannon::OnAttack, this, _1)));
    }
}

void ArkCannon::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    m_model->fn_801633DC(&matrix);
    fn_800CCE38(m_object.get(), &m_charInfo->m_position);

    /* The objects stand around the cannon at even angles. */
    std::vector<boost::shared_ptr<Target> >::iterator it = m_objects.begin();
    /* Matching workaround: the distance is read once, before the loop, as the original does.
     * Reading s_objectDistance.value in the loop reloads it on every pass (0x174 bytes instead
     * of 0x184), and reading it after step changes four instruction words. Revisit with the
     * original type of s_objectDistance. */
    const float distance = s_objectDistance.value;
    const float step = DegreesToRadians(360.0f) / kObjectCount;

    for (u32 i = 0; it != m_objects.end(); ++i, ++it) {
        Fn8040Vector3 offset(distance, 0.0f, 0.0f);
        Fn8040Vector3 rotated;
        Fn8040Vector3 placed;

        fn_8040D044(&rotated, &offset, step * i);
        fn_800091D0(&offset, &rotated);
        fn_8040B67C(&placed, &matrix, &offset);
        fn_800091D0(&offset, &placed);
        fn_800CCE38(it->get(), &offset);
    }
}

bool ArkCannon::IsTargetInRange() {
    boost::shared_ptr<CharInfo> nearest =
        fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();

    if (nearest.get() != 0) {
        Fn8040Vector3 distance;
        Fn8040Vector3 difference;

        fn_8040CD74(&difference, &nearest->m_position, &m_charInfo->m_position);
        fn_800091EC(&distance, &difference);

        if (fn_8040CF50(&distance) < m_params.range) {
            return true;
        }
    }

    return false;
}

bool ArkCannon::TurnToTarget(boost::shared_ptr<CharInfo> target, float dt) {
    Fn8040Vector3 up(0.0f, 1.0f, 0.0f);
    Fn8040Vector3 forward(0.0f, 0.0f, -1.0f);
    bool facing = false;
    Fn8040Vector3 direction;
    Fn8040Vector3 ownerRotation;
    Fn8040Vector3 axis;
    Fn8040Matrix34 ownerMatrix;
    Fn8040Matrix34 matrix;
    Fn8040Matrix34 turn;

    Fn8040Vector3 offset;
    fn_8040CD74(&offset, &target->m_position, &m_charInfo->m_position);
    fn_800091EC(&direction, &offset);
    fn_8040CE74(&direction);

    /* The owner's rotation turns the up direction. */
    ownerRotation.Set(0.0f, 0.0f, 0.0f);
    fn_800C9ED4(m_owner, &ownerRotation);
    fn_8040C148(&ownerMatrix, &lbl_805E0720);
    fn_8040B848(&ownerMatrix, ownerRotation.y);
    fn_8040B884(&ownerMatrix, ownerRotation.x);
    fn_8040B80C(&ownerMatrix, ownerRotation.z);

    Fn8040Vector3 turnedUp;
    fn_8040B67C(&turnedUp, &ownerMatrix, &up);
    fn_800091D0(&up, &turnedUp);

    /* The cannon's rotation turns the forward direction. */
    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);

    Fn8040Vector3 turnedForward;
    fn_8040B67C(&turnedForward, &matrix, &forward);
    fn_800091D0(&forward, &turnedForward);

    /* Both directions are compared in the plane perpendicular to the up direction. */
    fn_8040F990(0, &forward, &forward, &up);
    fn_8040F990(0, &direction, &direction, &up);
    fn_8040CE74(&forward);
    fn_8040CE74(&direction);

    /* Turn by at most 45 degrees a second.
     * Matching workaround: the limit is written out twice. The original stores it for the call
     * only after the comparison; computing it once into a local (const or not) stores it before
     * the comparison and changes 11 instruction words. Revisit if the original form of this
     * limit is found. */
    float angle = fn_8040F8FC(&forward, &direction);
    if (angle < DegreesToRadians(45.0f * dt)) {
        facing = true;
    }
    angle = fn_800704AC(angle, DegreesToRadians(45.0f * dt));

    Fn8040Vector3 cross;
    fn_8040CFB8(&cross, &forward, &direction);
    fn_800091EC(&axis, &cross);
    fn_8040CE74(&axis);

    Fn8040Matrix34 axisTurn;
    fn_80049778(&axisTurn, &axis, angle);
    fn_8040C148(&turn, &axisTurn);

    Fn8040Matrix34 turned;
    fn_8001E044(&turned, &turn, &matrix);
    fn_8040C148(&matrix, &turned);
    fn_800494F8(&m_charInfo->m_rotation, &matrix);

    UpdateModelMatrix();
    return facing;
}

void ArkCannon::Fire(u32 index, boost::shared_ptr<CharInfo> target) {
    Fn8040Matrix34 matrix;

    if (fn_80169EF0(&matrix, m_model, 0, s_shotFrames[index])) {
        boost::shared_ptr<Weapon::LockonTarget> lockon(new Weapon::LockonTarget());
        boost::shared_ptr<UnkCharacter_80095388> character = fn_80095388(target);

        if (character->field_0x24C != 0) {
            lockon->fn_80061054(character->field_0x24C->fn_80206574());
            lockon->fn_80060F70();

            UnkParams_80051D10 params(boost::weak_ptr<CharInfo>(m_charInfo), lockon, 1, &matrix,
                                      &lbl_805E07A0);
            lbl_8051DDC8.create(&params);
        }
    }
}

void ArkCannon::SetState(int state) {
    switch (state) {
    case 1:
        m_target = fn_800A8A7C(fn_80074194(), &m_charInfo->m_position);
        break;
    case 2:
        m_shotTimer = 0.0f;
        m_shotCount = 0;
        break;
    case 3:
        m_reloadTimer = 5.0f;
        m_target.reset();
        break;
    }
    m_state = state;
}

void ArkCannon::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 1;

    if (m_state != 4) {
        m_health -= param.field_0x50;
        m_attackFlags.Set(0);

        if (m_health < 0.0f || fn_8000A950(m_health)) {
            param.field_0x54 = 3;

            boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);

            if (attacker.get() != 0) {
                fn_8020642C(attacker, 0x4B);
            }
            m_attackFlags.Set(1);
        }
    }
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "0:GRAND 1:AIR", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "RANGE m", 0, 0, 0, 50.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8054669C = {
    "ARKCANNON", fn_80227DB8, fn_80227DB4, ArkCannon::Create, 0, 0, 0x138E, 1, 0, 10, s_paramInfo,
};

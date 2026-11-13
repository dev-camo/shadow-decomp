/* Katana (class name from RTTI, with Weapon::WeaponStickBase as its only direct base): the weapon
 * stick of the two weapon types "Katana01" and "Katana02". It strikes along the line between the
 * positions of its model's frames 0 and 1 and can keep a Test::EffLocusMultiVtx task (the trail;
 * name inferred) on that line. StartAttack also passes bullet parameters to fn_80051C34 with the
 * weapon type as the bullet kind unless the stick is already attacking. The file name and the
 * class's unnamed namespace come from the RTTI name "@unnamed@WeaponSpecialKatana_cpp@::Katana".
 * Member names and the names of the static and non-virtual methods are inferred from the code.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/AngleConversion.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/EffLocusMultiVtx.h"
#include "Game/Task.h"
#include "Game/UnkParams_80051D10.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWeaponModel_800631FC.h"
#include "Game/WeaponItem.h"
#include "Game/WeaponStickBase.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace {

/* The two weapon types of the katana, passed to WeaponStickBase and fn_8005B760. RequestBullet
 * passes the weapon's type member to fn_80051C34 as the bullet kind, so the kinds that the two
 * hooks below pass to fn_80051CC8 and fn_80051C80 are written with the same constants
 * (inferred). */
const int kKatana01Type = 0x39;
const int kKatana02Type = 0x3A;

/* The model descriptions of the weapon and of its item, shared by both weapon types (placeholder
 * names, see UnkWeaponModel_800631FC.h). */
const UnkWeaponModelPart_8006465C s_weaponParts[2] = {
    {-1, 0x2328, 0x4B, -1, 0, 0},
    {0x233C, -1, 0x4C, -1, 0, 0},
};

const int s_weaponNumbers[2] = {0x233C, 0x233D};

const UnkWeaponModelDesc_8006465C s_weaponModel = {s_weaponParts, 2, s_weaponNumbers, 2};

const UnkWeaponModelPart_8006465C s_itemParts[2] = {
    {-1, 0x801, 0x4B, -1, 0, 0},
    {0x233C, -1, 0x4C, -1, 0, 0},
};

const int s_itemNumbers[2] = {0x2328, 0x801};

const UnkWeaponModelDesc_8006465C s_itemModel = {s_itemParts, 2, s_itemNumbers, 2};

/* The names of the two weapon type descriptions. */
const char *s_katana01Name = "Katana01";
const char *s_katana02Name = "Katana02";

/* The colour that StartAttack passes to the trail. */
UnkDebugColor_802E0E84 s_trailColor(0xA5, 0, 0xFF);

class Katana : public Weapon::WeaponStickBase {
  public:
    /* type and arg2 are passed on to WeaponStickBase. The original has no out-of-line constructor;
     * the two Create functions expand it. */
    Katana(const boost::weak_ptr<CharInfo> &owner, int type, float arg2)
        : Weapon::WeaponStickBase(owner, type, arg2) {
        field_0x3C = false;
        field_0x1C = new UnkWeaponModel_800631FC(owner, &s_weaponModel);
        m_trail = 0;
    }
    virtual ~Katana();
    /* Also passes position and rotation to fn_800632E8 of the model. */
    virtual void fn_8005CFF0(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* Also passes dt to fn_800632A0 of the model and calls UpdateLine. */
    virtual void Update(float dt);
    /* Pass the model to fn_80063130 or fn_80063154 when it exists, as the base implementations do,
     * call the trail's fn_802DE9BC or fn_802DE9CC when there is a trail, and clear or set
     * field_0x3C. */
    virtual void fn_8005CF64();
    virtual void fn_8005CF38();
    /* StartAttack first creates the trail when there is none and field_0x3C is clear, then calls
     * RequestBullet unless the stick is already attacking; EndAttack releases the trail first. */
    virtual void StartAttack();
    virtual void EndAttack();
    /* Plays sound 0xE024 + index (as a 16-bit id) at the weapon's position. */
    virtual void PlaySound(int index);

    /* The functions of the two weapon type descriptions. */
    static Weapon::WeaponBase *CreateKatana01(const boost::weak_ptr<CharInfo> &owner);
    static Weapon::Item *CreateKatana01Item();
    static Weapon::WeaponBase *CreateKatana02(const boost::weak_ptr<CharInfo> &owner);
    static Weapon::Item *CreateKatana02Item();

    /* Sets the stick's line to the positions of the model's frames 0 and 1. When there is a trail,
     * copies them to m_points while fn_8004EAC0 returns true for the trail task, and otherwise
     * forgets the trail. */
    void UpdateLine();
    /* Passes bullet parameters to fn_80051C34 with field_0x14 (the weapon type) as the bullet
     * kind. fn_80069540 builds them from a matrix at the owner's position, rotated by the owner's
     * rotation in the owner's rotation order and then turned half a turn by fn_8040B848. The
     * locked owner is not tested for null. */
    void RequestBullet();

    /* Calls the trail's fn_802DE9DC, which stops it reading m_points, and forgets it, if there is
     * one. The destructor makes the same store of null after the call as EndAttack, although
     * nothing reads m_trail after it there; that both expand one inline is inferred from that. */
    void ReleaseTrail() {
        if (m_trail != 0) {
            m_trail->fn_802DE9DC();
            m_trail = 0;
        }
    }

    /* 0x3C */ bool field_0x3C;           /* set by fn_8005CF38, cleared by fn_8005CF64 */
    /* 0x40 */ Fn8040Vector3 m_points[2]; /* the trail's points (its constructor gets them) */
    /* 0x58 */ Test::EffLocusMultiVtx *m_trail;
};
CHECK_SIZEOF(Katana, 0x5C);

Katana::~Katana() {
    delete field_0x1C;
    ReleaseTrail();
}

void Katana::fn_8005CFF0(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    Weapon::WeaponStickBase::fn_8005CFF0(position, rotation);
    fn_800632E8(field_0x1C, position, rotation);
}

void Katana::Update(float dt) {
    Weapon::WeaponStickBase::Update(dt);
    fn_800632A0(field_0x1C, dt);
    UpdateLine();
}

void Katana::fn_8005CF64() {
    if (field_0x1C != 0) {
        fn_80063130(field_0x1C);
    }
    if (m_trail != 0) {
        m_trail->fn_802DE9BC();
    }
    field_0x3C = false;
}

void Katana::fn_8005CF38() {
    if (field_0x1C != 0) {
        fn_80063154(field_0x1C);
    }
    if (m_trail != 0) {
        m_trail->fn_802DE9CC();
    }
    field_0x3C = true;
}

void Katana::StartAttack() {
    if (m_trail == 0 && !field_0x3C) {
        m_trail = new Test::EffLocusMultiVtx(10, 2, m_points, 0, s_trailColor);
        UpdateLine();
    }
    if (!IsAttacking()) {
        RequestBullet();
    }
    Weapon::WeaponStickBase::StartAttack();
}

void Katana::EndAttack() {
    ReleaseTrail();
    Weapon::WeaponStickBase::EndAttack();
}

void Katana::PlaySound(int index) {
    fn_801D54F4(fn_80053164(), index + 0xE024, &m_position, 0, 1);
}

void Katana::RequestBullet() {
    boost::shared_ptr<CharInfo> owner(m_owner.lock());

    Fn8040Matrix34 matrix(fn_8040B034(owner->m_position));

    matrix = fn_8040FFBC(&matrix, &owner->m_rotation, fn_801DBD00(owner->field_0x30));
    fn_8040B848(&matrix, DegreesToRadians(180.0f));

    fn_80051C34(field_0x14, fn_80069540(this, &matrix));
}

void Katana::UpdateLine() {
    Fn8040Vector3 start = fn_8040FB08(fn_80063234(field_0x1C, 0));
    Fn8040Vector3 end = fn_8040FB08(fn_80063234(field_0x1C, 1));

    SetLine(start, end);

    if (m_trail != 0) {
        if (!fn_8004EAC0(m_trail)) {
            m_trail = 0;
        } else {
            m_points[0] = start;
            m_points[1] = end;
        }
    }
}

} // namespace

/* Passes the two numbers of the model descriptions' parts (field_0x8 of each part) to
 * fn_8006B50C, and each weapon type to fn_80051CC8 as a bullet kind and to fn_8005B760. Apart from
 * the name it prints first, Weapon::HeavyShot::Initialize makes the same calls for its numbers,
 * kind and type; that this is the katana's load hook is inferred from that. fn_80275AF8 calls it
 * first, before five other functions. */
extern "C" void fn_80275D40(void) {
    fn_8006B50C(0x4B, 0);
    fn_8006B50C(0x4C, 0);
    fn_80051CC8(kKatana01Type);
    fn_80051CC8(kKatana02Type);
    fn_8005B760(kKatana01Type);
    fn_8005B760(kKatana02Type);
}

/* Passes each bullet kind to fn_80051C80, as Weapon::HeavyShot::Finalize does for its kind; that
 * this is the unload hook is inferred from that. fn_80275AC4 calls it first, before five other
 * functions. */
extern "C" void fn_80275D14(void) {
    fn_80051C80(kKatana01Type);
    fn_80051C80(kKatana02Type);
}

namespace {

/* Each description has its own pair of functions in the original. The two Create functions differ
 * in the type and the float that they pass to the constructor; the two item functions have the
 * same body. */
Weapon::WeaponBase *Katana::CreateKatana01(const boost::weak_ptr<CharInfo> &owner) {
    return new Katana(owner, kKatana01Type, 4.0f);
}

Weapon::Item *Katana::CreateKatana01Item() {
    return new Weapon::Item(&s_itemModel);
}

Weapon::WeaponBase *Katana::CreateKatana02(const boost::weak_ptr<CharInfo> &owner) {
    return new Katana(owner, kKatana02Type, 8.0f);
}

Weapon::Item *Katana::CreateKatana02Item() {
    return new Weapon::Item(&s_itemModel);
}

} // namespace

/* The weapon type descriptions of the two katanas (layout in WeaponItem.h).
 * Matching workaround: the original's start-up function (0x80276224, USA (GUPE8P)) loads 1.0
 * and 5.0 from .sdata2 once each and stores them at 0x10 and 0x14 of both descriptions, where its
 * .data has zeros; the other members are initialised data, apart from the names, which the same
 * function copies. A constant initialiser gives no store. UnkStartupFloat_800E2570 stands in for
 * the original initialiser expression, which is unknown. It is written in each record because
 * named static floats get stores and .sbss variables of their own, which the original does not
 * have. Revisit when the original initialiser is found. */
UnkWeaponDescription_8005B9D4 lbl_8054D2C0 = {
    s_katana01Name,
    Katana::CreateKatana01,
    Katana::CreateKatana01Item,
    0,
    UnkStartupFloat_800E2570(1.0f).value,
    UnkStartupFloat_800E2570(5.0f).value,
    4,
    3,
    0x2001,
    0.0f,
    0.0f,
    0.0f,
};

UnkWeaponDescription_8005B9D4 lbl_8054D2F0 = {
    s_katana02Name,
    Katana::CreateKatana02,
    Katana::CreateKatana02Item,
    0,
    UnkStartupFloat_800E2570(1.0f).value,
    UnkStartupFloat_800E2570(5.0f).value,
    8,
    3,
    0x2001,
    0.0f,
    0.0f,
    0.0f,
};

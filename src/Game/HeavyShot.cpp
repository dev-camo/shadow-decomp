/* Weapon::HeavyShot (class name from RTTI, with Weapon::WeaponGunBase as its only direct base): a
 * gun that requests bullets of kind 0xB at its model's frame 0. File name inferred from the class.
 * The names of the static and non-virtual methods are inferred from the code. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include <cstdio>
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkParams_80051D10.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWeaponModel_800631FC.h"
#include "Game/WeaponGunBase.h"
#include "Game/WeaponItem.h"
#include "Game/fn_8005E418.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace Weapon {

/* The weapon type of the heavy shot (passed to WeaponGunBase as its type). */
static const int kHeavyShotType = 0xB;

/* The bullet kind passed to fn_80051C34, fn_80051CC8 and fn_80051C80. */
static const int kHeavyShotBulletKind = 0xB;

class HeavyShot : public WeaponGunBase {
  public:
    HeavyShot(const boost::weak_ptr<CharInfo> &owner);
    virtual ~HeavyShot();
    /* Also passes position and rotation to fn_800632E8 of the model. */
    virtual void fn_8005CFF0(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* Also passes dt to fn_800632A0 of the model. */
    virtual void Update(float dt);
    /* Pass the bullet parameters that fn_80069424 (Shoot) or fn_80069298 (ShootAt) builds at the
     * model's frame 0 to fn_80051C34 with the bullet kind, call PlayShotEffects, add 1 to
     * field_0x38 and return 0.125. */
    virtual float Shoot(const Fn8040Vector3 &arg);
    virtual float ShootAt(const Fn8040Vector3 &arg, const Fn8040Vector3 &target);

    /* The functions of the weapon type's description and descriptor. Initialize and Finalize
     * print "<s_name>::Initialize" and "<s_name>::Finalize". */
    static WeaponBase *Create(const boost::weak_ptr<CharInfo> &owner);
    static Item *CreateItem();
    static void Finalize();
    static void Initialize();

    /* Passes sound 0x2012 at the position of the model's frame 0 to fn_801D53C4 and creates
     * effect 0x287 at that frame's matrix with fn_8005E418. */
    void PlayShotEffects();

    static const char *s_name;
};
CHECK_SIZEOF(Weapon::HeavyShot, 0x3C);

const char *HeavyShot::s_name = "HeavyShot";

/* The model descriptions of the weapon and of its item (placeholder names, see
 * UnkWeaponModel_800631FC.h). */
static const UnkWeaponModelSubPart_80063828 s_partNumbers[1] = {{0x6E, 0, 0}};

static const UnkWeaponModelPart_8006465C s_weaponParts[2] = {
    {-1, 0x2328, 0x13, -1, 0, 0},
    {0x251C, -1, 0x1E, 0x5F, s_partNumbers, 1},
};

static const int s_weaponNumbers[1] = {0x2329};

static const UnkWeaponModelDesc_8006465C s_weaponModel = {s_weaponParts, 2, s_weaponNumbers, 1};

static const UnkWeaponModelPart_8006465C s_itemParts[2] = {
    {-1, 0x801, 0x13, -1, 0, 0},
    {0x251C, -1, 0x1E, 0x5F, s_partNumbers, 1},
};

static const int s_itemNumbers[2] = {0x2328, 0x801};

static const UnkWeaponModelDesc_8006465C s_itemModel = {s_itemParts, 2, s_itemNumbers, 2};

/* inline: the original has no out-of-line constructor; Create expands it. */
inline HeavyShot::HeavyShot(const boost::weak_ptr<CharInfo> &owner)
    : WeaponGunBase(owner, kHeavyShotType) {
    field_0x1C = new UnkWeaponModel_800631FC(owner, &s_weaponModel);
}

HeavyShot::~HeavyShot() {
    delete field_0x1C;
}

void HeavyShot::fn_8005CFF0(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    WeaponGunBase::fn_8005CFF0(position, rotation);
    fn_800632E8(field_0x1C, position, rotation);
}

void HeavyShot::Update(float dt) {
    WeaponGunBase::Update(dt);
    fn_800632A0(field_0x1C, dt);
}

float HeavyShot::Shoot(const Fn8040Vector3 &arg) {
    fn_80051C34(kHeavyShotBulletKind, fn_80069424(this, fn_80063234(field_0x1C, 0), arg));
    PlayShotEffects();
    fn_8005EB94(1);
    return 0.125f;
}

float HeavyShot::ShootAt(const Fn8040Vector3 &arg, const Fn8040Vector3 &target) {
    fn_80051C34(kHeavyShotBulletKind, fn_80069298(this, fn_80063234(field_0x1C, 0), arg, target));
    PlayShotEffects();
    fn_8005EB94(1);
    return 0.125f;
}

void HeavyShot::PlayShotEffects() {
    Fn8040Vector3 position = fn_8040FB08(fn_80063234(field_0x1C, 0));

    fn_801D53C4(fn_80053164(), 0x2012, &position, 0, 1, 0, 0);
    fn_8005E418(fn_80063234(field_0x1C, 0), 0x287, m_owner);
}

void HeavyShot::Initialize() {
    printf("%s::Initialize\n", s_name);
    fn_8006B50C(0x13, 0);
    fn_8006B50C(0x1E, 0);
    fn_8006B50C(0x5F, 0);
    fn_8006B50C(0x6E, 0);
    fn_80051CC8(kHeavyShotBulletKind);
    fn_8005B760(kHeavyShotType);
}

void HeavyShot::Finalize() {
    printf("%s::Finalize\n", s_name);
    fn_80051C80(kHeavyShotBulletKind);
}

WeaponBase *HeavyShot::Create(const boost::weak_ptr<CharInfo> &owner) {
    return new HeavyShot(owner);
}

Item *HeavyShot::CreateItem() {
    return new Item(&s_itemModel);
}

} // namespace Weapon

GadgetTypeInfo lbl_80559E60 = {
    Weapon::HeavyShot::s_name,
    Weapon::HeavyShot::Initialize,
    Weapon::HeavyShot::Finalize,
    0,
    0,
    0x80,
    0xD2,
    4,
    0,
    0,
    0,
};

UnkWeaponDescription_8005B9D4 lbl_80559E84 = {
    Weapon::HeavyShot::s_name,
    Weapon::HeavyShot::Create,
    Weapon::HeavyShot::CreateItem,
    0,
    1.0f,
    5.0f,
    0x1E,
    3,
    0x2003,
    0.0f,
    0.2f,
    0.0f,
};

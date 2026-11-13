/* ShadowRifle (class name from RTTI, with Weapon::WeaponGunBase as its only direct base): a gun that
 * requests bullets of its weapon type at its model's frame 0. The file name and the class's unnamed
 * namespace come from the RTTI name "@unnamed@WeaponSpecialShadowRifle_cpp@::ShadowRifle". The
 * names of the static and non-virtual methods are inferred from the code. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/UnkParams_80051D10.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWeaponModel_800631FC.h"
#include "Game/WeaponGunBase.h"
#include "Game/WeaponItem.h"
#include "Game/fn_8005E418.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace {

/* The weapon type of the shadow rifle, passed to WeaponGunBase and fn_8005B760. Shoot and ShootAt
 * pass the weapon's type member to fn_80051C34 as the bullet kind, so the kind that the two hooks
 * below pass to fn_80051CC8 and fn_80051C80 is written with the same constant (inferred). */
const int kShadowRifleType = 0x43;

class ShadowRifle : public Weapon::WeaponGunBase {
  public:
    ShadowRifle(const boost::weak_ptr<CharInfo> &owner);
    virtual ~ShadowRifle();
    /* Also passes position and rotation to fn_800632E8 of the model. */
    virtual void fn_8005CFF0(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* Also passes dt to fn_800632A0 of the model. */
    virtual void Update(float dt);
    /* Pass the bullet parameters that fn_80069424 (Shoot) or fn_80069298 (ShootAt) builds at the
     * model's frame 0 to fn_80051C34 with field_0x14 (the weapon type) as the bullet kind, call
     * PlayShotEffects, add 1 to field_0x38 and return 0.25. */
    virtual float Shoot(const Fn8040Vector3 &arg);
    virtual float ShootAt(const Fn8040Vector3 &arg, const Fn8040Vector3 &target);

    /* The functions of the weapon type's description. */
    static Weapon::WeaponBase *Create(const boost::weak_ptr<CharInfo> &owner);
    static Weapon::Item *CreateItem();

    /* Passes sound 0xE030 at the position of the model's frame 0 to fn_801D53C4 and creates
     * effect 0x27D at that frame's matrix with fn_8005E418. */
    void PlayShotEffects();

    static const char *s_name;
};
CHECK_SIZEOF(ShadowRifle, 0x3C);

const char *ShadowRifle::s_name = "ShadowRifle";

/* The model descriptions of the weapon and of its item (placeholder names, see
 * UnkWeaponModel_800631FC.h). */
const UnkWeaponModelSubPart_80063828 s_partNumbers[1] = {{0x76, 0, 0}};

const UnkWeaponModelPart_8006465C s_weaponParts[4] = {
    {-1, 0x2328, 0x50, -1, 0, 0},
    {0x3E8, -1, 0x52, -1, 0, 0},
    {0x3E8, -1, 0x53, -1, 0, 0},
    {0x3E9, -1, 0x51, 0x65, s_partNumbers, 1},
};

const int s_weaponNumbers[1] = {0x2329};

const UnkWeaponModelDesc_8006465C s_weaponModel = {s_weaponParts, 4, s_weaponNumbers, 1};

const UnkWeaponModelPart_8006465C s_itemParts[4] = {
    {-1, 0x801, 0x50, -1, 0, 0},
    {0x3E8, -1, 0x52, -1, 0, 0},
    {0x3E8, -1, 0x53, -1, 0, 0},
    {0x3E9, -1, 0x51, 0x65, s_partNumbers, 1},
};

const int s_itemNumbers[2] = {0x2328, 0x801};

const UnkWeaponModelDesc_8006465C s_itemModel = {s_itemParts, 4, s_itemNumbers, 2};

/* inline: the original has no out-of-line constructor; Create expands it. */
inline ShadowRifle::ShadowRifle(const boost::weak_ptr<CharInfo> &owner)
    : Weapon::WeaponGunBase(owner, kShadowRifleType) {
    field_0x1C = new UnkWeaponModel_800631FC(owner, &s_weaponModel);
}

ShadowRifle::~ShadowRifle() {
    delete field_0x1C;
}

void ShadowRifle::fn_8005CFF0(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    Weapon::WeaponGunBase::fn_8005CFF0(position, rotation);
    fn_800632E8(field_0x1C, position, rotation);
}

void ShadowRifle::Update(float dt) {
    Weapon::WeaponGunBase::Update(dt);
    fn_800632A0(field_0x1C, dt);
}

float ShadowRifle::Shoot(const Fn8040Vector3 &arg) {
    fn_80051C34(field_0x14, fn_80069424(this, fn_80063234(field_0x1C, 0), arg));
    PlayShotEffects();
    fn_8005EB94(1);
    return 0.25f;
}

float ShadowRifle::ShootAt(const Fn8040Vector3 &arg, const Fn8040Vector3 &target) {
    fn_80051C34(field_0x14, fn_80069298(this, fn_80063234(field_0x1C, 0), arg, target));
    PlayShotEffects();
    fn_8005EB94(1);
    return 0.25f;
}

void ShadowRifle::PlayShotEffects() {
    /* Matching workaround: the frame's matrix is a named local. The original copies the pointer
     * that fn_80063234 returns to r0 before it sets up the fn_8040FB08 call (0x80276F74, USA
     * (GUPE8P)), as MWCC does for a named variable; passing the call's result directly, also
     * through an inline helper, moves it straight to r4 and gives 156 bytes against 160
     * (measured). The second fn_80063234 result below goes straight to r4 in the original
     * (0x80276FC0). Whether the original names the pointer is not known. Revisit when another
     * form with the same code is found. */
    Fn8040Matrix34 *matrix = fn_80063234(field_0x1C, 0);
    Fn8040Vector3 position = fn_8040FB08(matrix);

    fn_801D53C4(fn_80053164(), 0xE030, &position, 0, 1, 0, 0);
    fn_8005E418(fn_80063234(field_0x1C, 0), 0x27D, m_owner);
}

} // namespace

/* Passes the six numbers of the model descriptions' parts (field_0x8 of each part, field_0xC of
 * the last one and the first word of the sub-part record) to fn_8006B50C, the bullet kind to
 * fn_80051CC8 and the weapon type to fn_8005B760. Apart from the name it prints first,
 * Weapon::HeavyShot::Initialize makes the same calls for its numbers, kind and type; that this is
 * the shadow rifle's load hook is inferred from that. fn_80275AF8 calls it after five other
 * functions. */
extern "C" void fn_80276EE0(void) {
    fn_8006B50C(0x50, 0);
    fn_8006B50C(0x52, 0);
    fn_8006B50C(0x53, 0);
    fn_8006B50C(0x51, 0);
    fn_8006B50C(0x65, 0);
    fn_8006B50C(0x76, 0);
    fn_80051CC8(kShadowRifleType);
    fn_8005B760(kShadowRifleType);
}

/* Passes the bullet kind to fn_80051C80, as Weapon::HeavyShot::Finalize does for its kind; that
 * this is the unload hook is inferred from that. fn_80275AC4 calls it after five other
 * functions. */
extern "C" void fn_80276EBC(void) {
    fn_80051C80(kShadowRifleType);
}

namespace {

Weapon::WeaponBase *ShadowRifle::Create(const boost::weak_ptr<CharInfo> &owner) {
    return new ShadowRifle(owner);
}

Weapon::Item *ShadowRifle::CreateItem() {
    return new Weapon::Item(&s_itemModel);
}

} // namespace

/* The shadow rifle's weapon type description (layout in WeaponItem.h). The first word of the
 * record of weapon type 0x43 in the weapon type table at 0x8051DF38 (the word at 0x8051E368, USA
 * (GUPE8P)) points to it.
 * Matching workaround: the original's start-up function (0x802771E4, USA (GUPE8P)) loads 1.0
 * and 5.0 from .sdata2 and stores them at 0x10 and 0x14 of the description, where its .data has
 * zeros; the other members are initialised data, apart from the name, which the same function
 * copies. A constant initialiser gives no store. UnkStartupFloat_800E2570 stands in for the
 * original initialiser expression, which is unknown; it is written in the record, as in
 * WeaponSpecialKatana.cpp. Revisit when the original initialiser is found. */
UnkWeaponDescription_8005B9D4 lbl_8054D518 = {
    ShadowRifle::s_name,
    ShadowRifle::Create,
    ShadowRifle::CreateItem,
    0,
    UnkStartupFloat_800E2570(1.0f).value,
    UnkStartupFloat_800E2570(5.0f).value,
    0x14,
    3,
    0x2002,
    0.0f,
    0.0f,
    0.0f,
};

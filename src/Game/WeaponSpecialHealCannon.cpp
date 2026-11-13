/* HealCannon (class name from RTTI, with Weapon::WeaponCannonBase as its only direct base): the
 * cannon of the two weapon types 0x41 and 0x42, "HealCannon01" and "HealCannon02". The file name
 * and the class's unnamed namespace come from the RTTI name
 * "@unnamed@WeaponSpecialHealCannon_cpp@::HealCannon". The names of the static methods and of
 * the static members are inferred from the code. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first), except fn_8005C0DC, whose in-class
 * definition is emitted before all of them. */
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWeaponModel_800631FC.h"
#include "Game/WeaponCannonBase.h"
#include "Game/WeaponItem.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace {

/* The two weapon types of the heal cannon, passed to WeaponCannonBase and fn_8005B760. */
const int kHealCannon01Type = 0x41;
const int kHealCannon02Type = 0x42;

class HealCannon : public Weapon::WeaponCannonBase {
  public:
    HealCannon(const boost::weak_ptr<CharInfo> &owner, int type);
    /* Defined out of class although it is empty: the original's destructor is the last function
     * before the start-up function, and an implicit or in-class one is emitted before
     * fn_8005C0DC instead. */
    virtual ~HealCannon();
    /* Returns 1 where Weapon::WeaponBase's returns 0. */
    virtual int fn_8005C0DC() {
        return 1;
    }
    /* Passes sound 0xE02D at the position of the model's frame 0 to fn_801D54F4. */
    virtual void PlayShotEffects();

    /* The functions of the two weapon types' descriptions. */
    static Weapon::WeaponBase *Create01(const boost::weak_ptr<CharInfo> &owner);
    static Weapon::Item *CreateItem01();
    static Weapon::WeaponBase *Create02(const boost::weak_ptr<CharInfo> &owner);
    static Weapon::Item *CreateItem02();

    /* The names of the two weapon types; the start-up function copies them into the
     * descriptions. */
    static const char *s_name01;
    static const char *s_name02;
};
CHECK_SIZEOF(HealCannon, 0x3C);

/* The model descriptions of the weapon and of its item (placeholder names, see
 * UnkWeaponModel_800631FC.h); both weapon types use them. */
const UnkWeaponModelPart_8006465C s_weaponParts[3] = {
    {-1, 0x2328, 0x48, -1, 0, 0},
    {0x2328, -1, 0x4A, -1, 0, 0},
    {0x2328, -1, 0x49, -1, 0, 0},
};

const int s_weaponNumbers[1] = {0x2329};

const UnkWeaponModelDesc_8006465C s_weaponModel = {s_weaponParts, 3, s_weaponNumbers, 1};

const UnkWeaponModelPart_8006465C s_itemParts[3] = {
    {-1, 0x801, 0x48, -1, 0, 0},
    {0x2328, -1, 0x4A, -1, 0, 0},
    {0x2328, -1, 0x49, -1, 0, 0},
};

const int s_itemNumbers[2] = {0x2328, 0x801};

const UnkWeaponModelDesc_8006465C s_itemModel = {s_itemParts, 3, s_itemNumbers, 2};

const char *HealCannon::s_name01 = "HealCannon01";
const char *HealCannon::s_name02 = "HealCannon02";

/* inline: the original has no out-of-line constructor; Create01 and Create02 expand it. That the
 * weapon type is a constructor parameter is inferred from the two factories, which differ only in
 * the type they pass to WeaponCannonBase. */
inline HealCannon::HealCannon(const boost::weak_ptr<CharInfo> &owner, int type)
    : Weapon::WeaponCannonBase(owner, type, &s_weaponModel) {}

HealCannon::~HealCannon() {}

void HealCannon::PlayShotEffects() {
    /* Matching workaround: the frame's matrix is a named local. The original copies the pointer
     * that fn_80063234 returns to r0 before it sets up the fn_8040FB08 call (0x802759F0, USA
     * (GUPE8P)), as MWCC does for a named variable; passing the call's result directly, also
     * through an inline helper, moves it straight to r4 and gives 92 bytes against 96
     * (measured). Whether the original names the pointer is not known. Revisit when
     * another form with the same code is found. */
    Fn8040Matrix34 *matrix = fn_80063234(field_0x1C, 0);
    Fn8040Vector3 position = fn_8040FB08(matrix);

    fn_801D54F4(fn_80053164(), 0xE02D, &position, 0, 1);
}

} // namespace

/* Passes the three numbers of the model descriptions' parts (field_0x8 of each part) to
 * fn_8006B50C and the two weapon types to fn_8005B760. Apart from the name it prints first and its
 * fn_80051CC8 call, Weapon::HeavyShot::Initialize makes the same calls for its numbers and type;
 * that this is the heal cannon's load hook is inferred from that. fn_80275AF8 calls it after
 * four other functions. */
extern "C" void fn_80275988(void) {
    fn_8006B50C(0x48, 0);
    fn_8006B50C(0x49, 0);
    fn_8006B50C(0x4A, 0);
    fn_8005B760(kHealCannon01Type);
    fn_8005B760(kHealCannon02Type);
}

/* Does nothing. fn_80275AC4 calls it after four other functions. */
extern "C" void fn_80275984(void) {}

namespace {

Weapon::WeaponBase *HealCannon::Create01(const boost::weak_ptr<CharInfo> &owner) {
    return new HealCannon(owner, kHealCannon01Type);
}

Weapon::Item *HealCannon::CreateItem01() {
    return new Weapon::Item(&s_itemModel);
}

Weapon::WeaponBase *HealCannon::Create02(const boost::weak_ptr<CharInfo> &owner) {
    return new HealCannon(owner, kHealCannon02Type);
}

Weapon::Item *HealCannon::CreateItem02() {
    return new Weapon::Item(&s_itemModel);
}

} // namespace

/* The descriptions of the two weapon types (layout in WeaponItem.h). The first words of the
 * records of weapon types 0x41 and 0x42 in the weapon type table at 0x8051DF38 (the words at
 * 0x8051E348 and 0x8051E358, USA (GUPE8P)) point to them. They differ in the name, the two creators
 * and field_0x18.
 * Matching workaround: the original's start-up function (0x80275A90, USA (GUPE8P)) loads 1.0
 * and 5.0 from .sdata2 once each and stores them at 0x10 and 0x14 of both descriptions, where its
 * .data has zeros; the other members are initialised data, apart from the names, which the same
 * function copies. A constant initialiser gives no store. UnkStartupFloat_800E2570 stands in for
 * the original initialiser expression, which is unknown; it is written in each record, as in
 * WeaponSpecialKatana.cpp. Revisit when the original initialiser is found. */
UnkWeaponDescription_8005B9D4 lbl_8054D200 = {
    HealCannon::s_name01,
    HealCannon::Create01,
    HealCannon::CreateItem01,
    0,
    UnkStartupFloat_800E2570(1.0f).value,
    UnkStartupFloat_800E2570(5.0f).value,
    0xA,
    2,
    0x2002,
    1.0f,
    1.0f,
    1.0f,
};

UnkWeaponDescription_8005B9D4 lbl_8054D230 = {
    HealCannon::s_name02,
    HealCannon::Create02,
    HealCannon::CreateItem02,
    0,
    UnkStartupFloat_800E2570(1.0f).value,
    UnkStartupFloat_800E2570(5.0f).value,
    0x14,
    2,
    0x2002,
    1.0f,
    1.0f,
    1.0f,
};

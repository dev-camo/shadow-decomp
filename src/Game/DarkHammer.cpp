/* Weapon::DarkHammer (class name from RTTI, with Weapon::WeaponStickBase as its only direct base):
 * a weapon stick that strikes along the line between the positions of its model's frames 0 and 1
 * and can keep a Test::EffLocusMultiVtx task (the trail; name inferred) on that line.
 * StartAttack creates the trail only when there is none and field_0x54 is clear, and starts the
 * base attack in either case. UpdateLine copies the line to m_points, which the trail reads, while
 * fn_8004EAC0 returns true for the trail task, and otherwise forgets the trail without releasing
 * it; EndAttack and the destructor release it. File name inferred from the class. Member names
 * and the names of the static and non-virtual methods are inferred from the code. Definitions are
 * in reverse address order (deferred inlining emits functions last-defined first). */
#include <cstdio>
#include "Game/DebugDisp.h"
#include "Game/EffLocusMultiVtx.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/Task.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWeaponModel_800631FC.h"
#include "Game/WeaponItem.h"
#include "Game/WeaponStickBase.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace Weapon {

class DarkHammer : public WeaponStickBase {
  public:
    DarkHammer(const boost::weak_ptr<CharInfo> &owner);
    virtual ~DarkHammer();
    /* Also passes position and rotation to fn_800632E8 of the model and keeps copies of them. */
    virtual void fn_8005CFF0(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* Also passes dt to fn_800632A0 of the model and calls UpdateLine. */
    virtual void Update(float dt);
    /* Pass the model to fn_80063130 or fn_80063154 when it exists, as the base implementations do,
     * call the trail's fn_802DE9BC or fn_802DE9CC when there is a trail, and clear or set
     * field_0x54. */
    virtual void fn_8005CF64();
    virtual void fn_8005CF38();
    /* StartAttack also creates the trail when there is none and field_0x54 is clear; EndAttack
     * releases the trail first. */
    virtual void StartAttack();
    virtual void EndAttack();

    /* The functions of the weapon type's description and descriptor. Initialize and Finalize
     * print "<s_name>::Initialize" and "<s_name>::Finalize". */
    static WeaponBase *Create(const boost::weak_ptr<CharInfo> &owner);
    static Item *CreateItem();
    static void Finalize();
    static void Initialize();

    /* Sets the stick's line to the positions of the model's frames 0 and 1. When there is a trail,
     * copies them to m_points while fn_8004EAC0 returns true for the trail task, and otherwise
     * forgets the trail. */
    void UpdateLine();

    /* Calls the trail's fn_802DE9DC, which stops it reading m_points, and forgets it, if there is
     * one. */
    void ReleaseTrail() {
        if (m_trail != 0) {
            m_trail->fn_802DE9DC();
            m_trail = 0;
        }
    }

    static const char *s_name;

    /* 0x3C */ Fn8040Vector3 field_0x3C;  /* the position last passed to fn_8005CFF0 */
    /* 0x48 */ UnkRotation3 field_0x48;   /* the rotation last passed to fn_8005CFF0, or zero */
    /* 0x54 */ bool field_0x54;           /* set by fn_8005CF38, cleared by fn_8005CF64 */
    /* 0x58 */ Fn8040Vector3 m_points[2]; /* the trail's points (its constructor gets them) */
    /* 0x70 */ Test::EffLocusMultiVtx *m_trail;
};
CHECK_SIZEOF(Weapon::DarkHammer, 0x74);

const char *DarkHammer::s_name = "DarkHammer";

/* The weapon type of the dark hammer (passed to WeaponStickBase as its type). */
static const int kDarkHammerType = 0x20;

/* The model descriptions of the weapon and of its item (placeholder names, see
 * UnkWeaponModel_800631FC.h). */
static const UnkWeaponModelSubPart_80063828 s_partNumbers[1] = {{0x6D, 0, 0}};

static const UnkWeaponModelPart_8006465C s_weaponParts[2] = {
    {-1, 0x2328, 0x1C, -1, 0, 0},
    {0x233C, -1, 0x1D, 0x5E, s_partNumbers, 1},
};

static const int s_weaponNumbers[2] = {0x233C, 0x233D};

static const UnkWeaponModelDesc_8006465C s_weaponModel = {s_weaponParts, 2, s_weaponNumbers, 2};

static const UnkWeaponModelPart_8006465C s_itemParts[2] = {
    {-1, 0x801, 0x1C, -1, 0, 0},
    {0x233C, -1, 0x1D, 0x5E, s_partNumbers, 1},
};

static const int s_itemNumbers[2] = {0x2328, 0x801};

static const UnkWeaponModelDesc_8006465C s_itemModel = {s_itemParts, 2, s_itemNumbers, 2};

DarkHammer::DarkHammer(const boost::weak_ptr<CharInfo> &owner)
    : WeaponStickBase(owner, kDarkHammerType, 6.0f), field_0x48(0.0f, 0.0f, 0.0f) {
    field_0x54 = false;
    field_0x1C = new UnkWeaponModel_800631FC(owner, &s_weaponModel);
    m_trail = 0;
}

DarkHammer::~DarkHammer() {
    delete field_0x1C;
    ReleaseTrail();
}

void DarkHammer::fn_8005CFF0(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    WeaponStickBase::fn_8005CFF0(position, rotation);
    fn_800632E8(field_0x1C, position, rotation);
    field_0x3C = *position;
    field_0x48 = *rotation;
}

void DarkHammer::Update(float dt) {
    WeaponStickBase::Update(dt);
    fn_800632A0(field_0x1C, dt);
    UpdateLine();
}

void DarkHammer::fn_8005CF64() {
    if (field_0x1C != 0) {
        fn_80063130(field_0x1C);
    }
    if (m_trail != 0) {
        m_trail->fn_802DE9BC();
    }
    field_0x54 = false;
}

void DarkHammer::fn_8005CF38() {
    if (field_0x1C != 0) {
        fn_80063154(field_0x1C);
    }
    if (m_trail != 0) {
        m_trail->fn_802DE9CC();
    }
    field_0x54 = true;
}

void DarkHammer::StartAttack() {
    if (m_trail == 0 && !field_0x54) {
        UnkDebugColor_802E0E84 color(0x80, 0x20, 0);
        m_trail = new Test::EffLocusMultiVtx(10, 2, m_points, 0, color);
        UpdateLine();
    }
    WeaponStickBase::StartAttack();
}

void DarkHammer::EndAttack() {
    ReleaseTrail();
    WeaponStickBase::EndAttack();
}

void DarkHammer::UpdateLine() {
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

void DarkHammer::Initialize() {
    printf("%s::Initialize\n", s_name);
    fn_8006B50C(0x1C, 0);
    fn_8006B50C(0x1D, 0);
    fn_8006B50C(0x5E, 0);
    fn_8006B50C(0x6D, 0);
    fn_8005B760(kDarkHammerType);
}

void DarkHammer::Finalize() {
    printf("%s::Finalize\n", s_name);
}

WeaponBase *DarkHammer::Create(const boost::weak_ptr<CharInfo> &owner) {
    return new DarkHammer(owner);
}

Item *DarkHammer::CreateItem() {
    return new Item(&s_itemModel);
}

} // namespace Weapon

GadgetTypeInfo lbl_80559DB0 = {
    Weapon::DarkHammer::s_name,
    Weapon::DarkHammer::Initialize,
    Weapon::DarkHammer::Finalize,
    0,
    0,
    0x80,
    0xFC,
    4,
    0,
    0,
    0,
};

/* The weapon type description (layout in WeaponItem.h).
 * Matching workaround: the original's start-up function (0x802F1768, USA (GUPE8P)) loads 1.0
 * and 5.0 from .sdata2 and stores them at 0x10 and 0x14 of the description, where its .data has
 * zeros; the other members are initialised data, apart from the name, which the same function
 * copies. A constant initialiser gives no store. UnkStartupFloat_800E2570 stands in for the
 * original initialiser expression, which is unknown; it is written in the record, as in
 * WeaponSpecialKatana.cpp. Revisit when the original initialiser is found. */
UnkWeaponDescription_8005B9D4 lbl_80559DD4 = {
    Weapon::DarkHammer::s_name,
    Weapon::DarkHammer::Create,
    Weapon::DarkHammer::CreateItem,
    0,
    UnkStartupFloat_800E2570(1.0f).value,
    UnkStartupFloat_800E2570(5.0f).value,
    6,
    3,
    0x2001,
    0.0f,
    0.0f,
    0.0f,
};

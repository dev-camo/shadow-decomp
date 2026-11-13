/* Player::Shadow::ShadowMotion (class name from RTTI): Shadow's motion task, its IK targets and node
 * callbacks, and the table of its motion states. File name inferred from the class. Definitions
 * are in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/PlayerShadowMotion.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/PickupCharInfo.h"
#include "Game/Player.h"
#include "Game/PlayerBase.h"
#include "Game/PlayerShadowMotionEye.h"
#include "Game/PlayerShadowMotionHand.h"
#include "Game/PlayerShadowMotionSub.h"
#include "Game/PlayerWeaponBase.h"
#include "Game/SetClumpWorld.h"
#include "Game/SharedConstants.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/WeaponItem.h"
#include "Game/fn_80015A7C.h"
#include "Game/fn_80015B60.h"
#include "Game/fn_80038198.h"
#include "Game/fn_80038E84.h"
#include "Game/fn_8003A840.h"
#include "Game/fn_8007A34C.h"
#include "Game/fn_80169350.h"
#include "Game/fn_8040C908.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_804157E8_motion_control.h"
#include "boost/bind.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rpworld.h"

namespace Player {
namespace Shadow {

/* The two state functions of a state (placeholder name): field_0x0 runs before the motion control
 * steps, field_0xC after it (fn_800B4804). */
struct UnkStateFunctions_80521ED4 {
    /* 0x00 */ ShadowMotion::StateFunction field_0x0;
    /* 0x0C */ ShadowMotion::StateFunction field_0xC;
};
CHECK_SIZEOF(UnkStateFunctions_80521ED4, 0x18);

/* A state of the state table (placeholder name). fn_8006CB7C starts the motion field_0x0 (counted
 * from 1 above the first motion; -1 keeps the motion), passes field_0x8 to MotionHand and installs
 * the state functions; fn_800B3CA4 starts the motion field_0x4 when the first one ends. */
struct UnkMotionState_804B38A4 {
    /* 0x0 */ int field_0x0;
    /* 0x4 */ int field_0x4;
    /* 0x8 */ int field_0x8;
    /* 0xC */ const UnkStateFunctions_80521ED4 *field_0xC;
};
CHECK_SIZEOF(UnkMotionState_804B38A4, 0x10);

/* A threshold of fn_800B3538 (placeholder name) and the offset that fn_800B42A8 turns by the
 * owner's rotation (placeholder name); both are constructed at startup. */
static const float lbl_805EE5E4 = UnkStartupFloat_800E2570(40.0f).value;
static Fn8040Vector3 lbl_80576928(1.4375f, 3.0f, 0.28125f);

/* The motion node ids that fn_800B4328 binds to PlayerMotion's 14 node records, for the three
 * groups of owner ids that it selects (placeholder name): -1 and -2 select PlayerMotion's matrices
 * at 0x70 and 0xA0. */
static const int lbl_804B37FC[3][14] = {
    {-1, -2, 0x1E, 0x18, 0x35, 0x1C, 0x49, 0x2C, 0x2B, 0x30, 0x2F, 0x28, -1, -1},
    {-1, -2, 0x1E, 0x18, 0x18, 0x1C, 0x1C, 0x2C, 0x2B, 0x30, 0x2F, 0x28, 0x2329, 0x2332},
    {-1, -2, 0x1E, 0x18, 0x18, 0x1C, 0x1C, 0x2C, 0x2B, 0x30, 0x2F, 0x28, 0x2329, -1},
};

/* The state functions of each state (placeholder names). */
static UnkStateFunctions_80521ED4 lbl_80521ED4 = {&ShadowMotion::fn_800B3D40,
                                                  &ShadowMotion::fn_800B3CA4};
static UnkStateFunctions_80521ED4 lbl_80521EF8 = {&ShadowMotion::fn_800B3AAC, 0};
static UnkStateFunctions_80521ED4 lbl_80521F1C = {&ShadowMotion::fn_800B38C8, 0};
static UnkStateFunctions_80521ED4 lbl_80521F40 = {&ShadowMotion::fn_800B36F8, 0};
static UnkStateFunctions_80521ED4 lbl_80521F64 = {&ShadowMotion::fn_800B3624, 0};
static UnkStateFunctions_80521ED4 lbl_80521F88 = {&ShadowMotion::fn_800B3538, 0};
static UnkStateFunctions_80521ED4 lbl_80521FAC = {&ShadowMotion::fn_800B34E0, 0};
static UnkStateFunctions_80521ED4 lbl_80521FD0 = {&ShadowMotion::fn_800B33D8, 0};
static UnkStateFunctions_80521ED4 lbl_80522000 = {&ShadowMotion::fn_800B334C,
                                                  &ShadowMotion::fn_800B32B4};
static UnkStateFunctions_80521ED4 lbl_80522024 = {&ShadowMotion::fn_800B31E0, 0};
static UnkStateFunctions_80521ED4 lbl_80522048 = {&ShadowMotion::fn_800B30F8, 0};
static UnkStateFunctions_80521ED4 lbl_8052206C = {&ShadowMotion::fn_800B2F48, 0};
static UnkStateFunctions_80521ED4 lbl_80522090 = {&ShadowMotion::fn_800B2EA4, 0};
static UnkStateFunctions_80521ED4 lbl_805220B4 = {&ShadowMotion::fn_800B2E28, 0};
static UnkStateFunctions_80521ED4 lbl_805220D8 = {&ShadowMotion::fn_800B2D38, 0};
static UnkStateFunctions_80521ED4 lbl_805220FC = {&ShadowMotion::fn_800B2C70, 0};
static UnkStateFunctions_80521ED4 lbl_80522120 = {&ShadowMotion::fn_800B2C0C, 0};
static UnkStateFunctions_80521ED4 lbl_80522144 = {&ShadowMotion::fn_800B2B48, 0};
static UnkStateFunctions_80521ED4 lbl_80522174 = {&ShadowMotion::fn_800B2B44,
                                                  &ShadowMotion::fn_800B2A84};
static UnkStateFunctions_80521ED4 lbl_80522198 = {&ShadowMotion::fn_800B2A18, 0};
static UnkStateFunctions_80521ED4 lbl_805221BC = {&ShadowMotion::fn_800B2844, 0};
static UnkStateFunctions_80521ED4 lbl_805221E0 = {&ShadowMotion::fn_800B2814, 0};
static UnkStateFunctions_80521ED4 lbl_80522204 = {&ShadowMotion::fn_800B27E4, 0};
static UnkStateFunctions_80521ED4 lbl_8052221C = {0, 0};

/* The state table (placeholder name), indexed by the state that fn_8006CB7C receives. */
static const UnkMotionState_804B38A4 lbl_804B38A4[48] = {
    {-1, -1, 2, &lbl_80521EF8},     {-1, -1, 1, &lbl_80521F1C},      {-1, -1, 1, &lbl_80521F40},
    {-1, -1, 0, &lbl_80521F88},     {-1, -1, 0, &lbl_80521FAC},      {-1, -1, 0, &lbl_80521F64},
    {0x53, -1, 0, &lbl_80521ED4},   {-1, -1, 0, &lbl_80521FD0},      {0x2A, -1, 0, &lbl_80521ED4},
    {0x3C, -1, 0, &lbl_80521ED4},   {0x3B, -1, 0, &lbl_80521ED4},    {0x49, -1, 0, &lbl_80521ED4},
    {-1, -1, 1, &lbl_80522000},     {0x2C, 0x37, 6, &lbl_80521ED4},  {-1, -1, 6, &lbl_80522024},
    {-1, -1, 8, &lbl_80522048},     {0x98, 0x53, 0, &lbl_80521ED4},  {0x9C, 0x9B, 0, &lbl_80521ED4},
    {0x9A, 0x99, 0, &lbl_80521ED4}, {0x9E, -1, 0, &lbl_80521ED4},    {0x9D, -1, 0, &lbl_80521ED4},
    {-1, -1, 0, &lbl_8052206C},     {-1, -1, 0, &lbl_80522090},      {0x65, -1, 0, &lbl_80521ED4},
    {-1, -1, 0, &lbl_805220B4},     {0x84, -1, 0, &lbl_80521ED4},    {0x42, -1, 0, &lbl_80521ED4},
    {0x41, -1, 0, &lbl_80521ED4},   {-1, -1, 10, &lbl_805220D8},     {-1, -1, 0, &lbl_805220FC},
    {0x29, -1, 4, &lbl_80521ED4},   {0x2B, -1, 0, &lbl_80521ED4},    {-1, -1, 0, &lbl_80522120},
    {0x73, -1, 0, &lbl_80521ED4},   {0x74, -1, 0, &lbl_80521ED4},    {0x78, -1, 0, &lbl_80522144},
    {0x26, -1, 11, &lbl_80521ED4},  {0x27, 0x28, 12, &lbl_80521ED4}, {0x53, -1, 0, &lbl_80521ED4},
    {0x51, -1, 0, &lbl_80521ED4},   {0x39, -1, 6, &lbl_80521ED4},    {0x43, -1, 0, &lbl_80522174},
    {-1, -1, 0, &lbl_80522198},     {-1, -1, 0, &lbl_805221BC},      {-1, -1, 0, &lbl_805221E0},
    {-1, -1, 0, &lbl_80522204},     {-1, -1, 0, &lbl_8052221C},      {-1, -1, 0, &lbl_8052221C},
};

/* The node callbacks that fn_800B44B4 registers with the motion control; they are defined below in
 * reverse address order. */
static void fn_800B1F8C(MotionNodeCallbackData *data);
static void fn_800B1F68(MotionNodeCallbackData *data);
static void fn_800B1F44(MotionNodeCallbackData *data);
static void fn_800B1B1C(MotionNodeCallbackData *data);
static void fn_800B1F20(MotionNodeCallbackData *data);
static void fn_800B1EFC(MotionNodeCallbackData *data);
static void fn_800B1ED8(MotionNodeCallbackData *data);
static void fn_800B1838(MotionNodeCallbackData *data);
static void fn_800B1808(MotionNodeCallbackData *data);
static void fn_800B1798(MotionNodeCallbackData *data);
static void fn_800B1760(MotionNodeCallbackData *data);
static void fn_800B1728(MotionNodeCallbackData *data);
static void fn_800B16B8(MotionNodeCallbackData *data);
static void fn_800B1680(MotionNodeCallbackData *data);
static void fn_800B1648(MotionNodeCallbackData *data);

ShadowMotion::ShadowMotion(Task *parent, const boost::shared_ptr<PlayerBase> &owner,
                           const UnkMotionResources_800B0D3C &resources)
    : PlayerMotion(parent, owner), field_0x458(0), field_0x7EC(resources) {
    field_0x45C = 0;
    field_0x460 = 0;
    field_0x46C = 0;
    field_0x7E0 = 0;
    field_0x7E4 = 0;
    field_0x7E8 = 0;
}

ShadowMotion::~ShadowMotion() {
    fn_800B442C();
}

void ShadowMotion::Update(float dt) {
    field_0x478 = dt;
    fn_800B4754();
    PlayerMotion::Update(field_0x478);
    if (fn_8006CD78(m_owner->field_0xA8, 0x1E) && fn_8006CD78(m_owner->field_0xA8, 0x3D)) {
        fn_800B4088();
    } else {
        fn_8006C098(0);
    }
    switch (field_0x458) {
    case 0:
        fn_800B4B44();
        break;
    case 1:
        fn_800B4804(field_0x478);
        break;
    }
}

void ShadowMotion::fn_8006CB7C(int arg0, float arg1) {
    if (!fn_8006CD78(m_owner->field_0xA8, 0x26) && field_0x42C != arg0) {
        field_0x428 = field_0x42C;
        field_0x42C = arg0;
        field_0x430 = arg1;
        field_0x43C = 1.0f;
        const UnkMotionState_804B38A4 *state = &lbl_804B38A4[field_0x42C];
        if (state->field_0x0 >= 0) {
            field_0x420[0] = state->field_0x0 + (field_0x3E0 - 1);
            field_0x420[1] = -1;
        }
        field_0x460 = state->field_0xC->field_0x0;
        field_0x46C = state->field_0xC->field_0xC;
        field_0x450 = 0;
        m_owner->field_0xB4.Clear();
        if (field_0x7E0 != 0) {
            field_0x7E0->fn_800B0148(state->field_0x8, field_0x430);
        }
    }
}

void ShadowMotion::UnkSlot3(std::vector<void *> &objects) {
    if (field_0x60.Test(2)) {
        RpClump *clump = field_0x45C->field_0x0;
        objects.push_back(clump);
        field_0x7E0->fn_800B01C0(objects);
        field_0x7E4->fn_800B5C0C(objects);
        if (field_0x7E8 != 0) {
            field_0x7E8->fn_8034F704(objects);
        }
    }
}

void ShadowMotion::UnkSlot13() {
    if (field_0x60.Test(3)) {
        field_0x60.Reset(3);
        if (field_0x45C != 0) {
            fn_80038F94(field_0x45C->field_0x0);
        }
        if (field_0x7E0 != 0) {
            field_0x7E0->fn_800B0014();
        }
        if (field_0x7E4 != 0) {
            field_0x7E4->fn_800B5BE8();
        }
        if (field_0x7E8 != 0) {
            field_0x7E8->fn_8034F6E0();
        }
        if (m_owner->field_0x238 != 0) {
            m_owner->field_0x238->UnkSlot14();
        }
    }
}

void ShadowMotion::UnkSlot14() {
    if (!field_0x60.Test(3)) {
        field_0x60.Set(3);
        if (field_0x45C != 0) {
            fn_80038FC0(field_0x45C->field_0x0);
        }
        if (field_0x7E0 != 0) {
            field_0x7E0->fn_800AFFF0();
        }
        if (field_0x7E4 != 0) {
            field_0x7E4->fn_800B5BC4();
        }
        if (field_0x7E8 != 0) {
            field_0x7E8->fn_8034F66C();
        }
        if (m_owner->field_0x238 != 0) {
            m_owner->field_0x238->UnkSlot13();
        }
    }
}

void ShadowMotion::fn_8006C090(int arg0) {
    if (field_0x7E0 != 0) {
        field_0x7E0->fn_800B019C(arg0);
    }
}

void ShadowMotion::fn_8006C094(const UnkExternalMotionArgs_8009A6A4 &arg0, float arg1) {
    /* State 0x2F is the state whose functions are both null. */
    fn_8006CB7C(0x2F, arg1);
    field_0x430 = arg1;
    field_0x420[0] = arg0.field_0x0;
    if (field_0x7E0 != 0) {
        field_0x7E0->fn_800B0124(arg0, arg1);
    }
}

void ShadowMotion::fn_8006C098(int arg0) {
    if (field_0x490[arg0].field_0xCC) {
        field_0x490[arg0].field_0xCC = false;
        field_0x490[arg0].field_0xC0.Clear();
        field_0x490[arg0].field_0xD0 = 0.0f;
    }
}

void ShadowMotion::fn_8006C0A0(int arg0, const Fn8040Matrix34 &matrix) {
    field_0x490[arg0].field_0xCC = true;
    field_0x490[arg0].field_0x90 = matrix;
    field_0x490[arg0].field_0xC0.Clear();
    field_0x490[arg0].field_0xD0 = 1.0f;
}

void ShadowMotion::fn_8006C09C(int arg0, const IKEventCallback &callback) {
    field_0x490[arg0].field_0xCC = true;
    field_0x490[arg0].field_0xC0 = callback;
    field_0x490[arg0].field_0xD0 = 1.0f;
}

void ShadowMotion::fn_8006C0A4(int arg0, float arg1) {
    field_0x490[arg0].field_0xD0 = arg1;
}

float ShadowMotion::fn_8006C0A8(int arg0) {
    return field_0x490[arg0].field_0xD0;
}

void ShadowMotion::fn_800AA748(_stMtnPlayWork *&first, _stMtnPlayWork *&second) {
    if (field_0x7E0 != 0) {
        field_0x7E0->fn_800B0038(first, second);
    }
}

int ShadowMotion::fn_800AA740() {
    if (field_0x7E4 != 0) {
        return field_0x7E4->fn_800B5B90();
    }
    return 0;
}

/* Waits until the owner's resources are loaded (fn_80012408 gives 3), then loads the model and the
 * motions, creates the hand, eye and sub tasks and enters the running mode with state 0x30
 * (behaviour inferred). */
void ShadowMotion::fn_800B4B44() {
    UnkResourceManager_800122D4 *manager = fn_800122D4();
    int key = fn_80078C4C(m_owner.get());
    if (fn_80012408(manager, key) == 3) {
        fn_800B44B4();
        field_0x7E0 = new MotionHand(this, m_owner, field_0x7EC);
        field_0x7E4 = new MotionEye(this, m_owner, field_0x7EC);
        field_0x7E8 = fn_8034F890(this, m_owner);
        fn_800B4328();
        field_0x458 = 1;
        fn_80076CA8(m_owner->field_0xA8, 0x47);
        fn_80076CA8(m_owner->field_0xA8, 0x46);
        int state = field_0x42C;
        if (state != 0x30) {
            field_0x42C = 0x30;
            fn_8006CB7C(state, 0.0f);
        }
    }
}

void ShadowMotion::fn_800B4804(float dt) {
    field_0x60.Reset(2);
    if (field_0x3E8 != 0) {
        field_0xA0 = m_owner->field_0x198;
        fn_8040B848(&field_0xA0, m_owner->m_rotation.y);
        fn_8040B884(&field_0xA0, m_owner->m_rotation.x);
        fn_8040B80C(&field_0xA0, m_owner->m_rotation.z);
        field_0x70 = field_0xA0;
        Fn8040Vector3 offset;
        offset = field_0xA0 * Fn8040Vector3(0.0f, -5.0f, 0.0f);
        field_0x48C = true;
        if (field_0x460) {
            (this->*field_0x460)();
        }
        Fn8040Vector3 root;
        fn_80415E34(field_0x3E8, &root);
        root = m_owner->m_position + offset - root;
        fn_80415BDC(field_0x3E8, &root);
        Fn8040Matrix34 matrix;
        fn_800163F4(&matrix, &field_0xA0);
        fn_8041734C(field_0x3E8, &matrix);
        Fn8040Vector3 head;
        head = field_0x70 * Fn8040Vector3(0.0f, 1.1999999f, -1.5999999f);
        fn_8040FA20(&field_0x70, m_owner->m_position + head);
        fn_8040FA20(&field_0xA0, m_owner->m_position);
        fn_800AA404();
        if (field_0x434 >= 0.0f) {
            stMtnRequestCurrentFrame(field_0x3E8, 0, field_0x434);
            field_0x7E0->fn_800B00F0(field_0x434);
            field_0x434 = -1.0f;
            field_0x44C = 0.0f;
        } else {
            field_0x44C = fn_800A9EF8();
        }
        stMtnRequestFrameSpeed(field_0x3E8, 0, field_0x44C);
        fn_80415EA4(field_0x3E8);
        if (field_0x45C != 0) {
            field_0x45C->fn_800159AC();
        }
        fn_800A9EA0();
        field_0x60.Set(2);
        if (field_0x48C) {
            Fn8040Vector3 position;
            fn_80415E34(field_0x3E8, &position);
            m_owner->field_0xC4 = position - offset - m_owner->m_position;
            m_owner->m_position = position - offset;
        } else {
            fn_8040CE60(&m_owner->field_0xC4);
        }
        fn_800AA0D4();
        if (field_0x46C) {
            (this->*field_0x46C)();
        }
    }
    field_0x7E0->fn_800B01E4();
    field_0x7E4->fn_800B5C30();
    if (field_0x7E8 != 0) {
        field_0x7E8->fn_8034F728();
    }
    fn_800A9E48();
    fn_80079860(m_owner.get());
    if (!fn_8006CD78(m_owner->field_0xA8, 0x27) && !fn_8006CD78(m_owner->field_0xA8, 0x28)) {
        fn_803563D0(m_owner->field_0x278, 4.0f);
    }
}

/* Calls UnkSlot13, or UnkSlot14 while the owner's fn_80073A20 is true and, while status bit 0x18
 * is set, on every other call (behaviour from the code). */
void ShadowMotion::fn_800B4754() {
    bool slot13 = true;
    if (m_owner->fn_80073A20()) {
        slot13 = false;
    }
    if (fn_8006CD78(m_owner->field_0xA8, 0x18)) {
        if (++field_0x3EC & 1) {
            slot13 = false;
        }
    }
    if (slot13) {
        UnkSlot13();
    } else {
        UnkSlot14();
    }
}

void ShadowMotion::fn_800B44B4() {
    UnkResourceManager_800122D4 *manager = fn_800122D4();
    fn_80078C4C(m_owner.get());
    void *pack =
        fn_80012684(manager, field_0x7EC.field_0x70.field_0x0, field_0x7EC.field_0x70.field_0x4);
    field_0x3E0 =
        fn_80012608(manager, field_0x7EC.field_0x68.field_0x0, field_0x7EC.field_0x68.field_0x4);
    RpClump *clump = static_cast<RpClump *>(
        fn_8001277C(manager, field_0x7EC.field_0x0.field_0x0, field_0x7EC.field_0x0.field_0x4));
    if (clump != 0) {
        clump = fn_80457090(clump);
        UnkManager_8003A840 *worlds = fn_8003A840();
        int index = fn_80169350(fn_80169364(clump));
        SetClumpWorld(fn_800593D0(worlds, index), clump);
        fn_80038198(clump, fn_800AA584());
    }
    /* The motion pack's resource value is the id of its bone data. */
    field_0x3E8 = fn_8041B908(reinterpret_cast<unsigned int>(pack), 2);
    if (field_0x3E8 != 0) {
        if (clump != 0) {
            field_0x45C = new Unk80015A7C(clump, field_0x3E8);
        }
        fn_80415808(field_0x3E8, 0x19, fn_800B1F8C, this);
        fn_80415808(field_0x3E8, 0x1A, fn_800B1F68, this);
        fn_80415808(field_0x3E8, 0x1B, fn_800B1F44, this);
        fn_80415808(field_0x3E8, 0x1C, fn_800B1B1C, this);
        fn_80415808(field_0x3E8, 0x15, fn_800B1F20, this);
        fn_80415808(field_0x3E8, 0x16, fn_800B1EFC, this);
        fn_80415808(field_0x3E8, 0x17, fn_800B1ED8, this);
        fn_80415808(field_0x3E8, 0x18, fn_800B1838, this);
        fn_80415808(field_0x3E8, 0x1E, fn_800B1808, this);
        fn_80415808(field_0x3E8, 0x29, fn_800B1798, this);
        fn_80415808(field_0x3E8, 0x2A, fn_800B1760, this);
        fn_80415808(field_0x3E8, 0x2B, fn_800B1728, this);
        fn_80415808(field_0x3E8, 0x2D, fn_800B16B8, this);
        fn_80415808(field_0x3E8, 0x2E, fn_800B1680, this);
        fn_80415808(field_0x3E8, 0x2F, fn_800B1648, this);
    }
}

void ShadowMotion::fn_800B442C() {
    RpClump *clump = 0;
    if (field_0x45C != 0) {
        clump = field_0x45C->field_0x0;
        delete field_0x45C;
    }
    if (field_0x3E8 != 0) {
        fn_8041B884(field_0x3E8);
    }
    if (clump != 0) {
        RpWorld *world = fn_8046256C(clump);
        if (world != 0) {
            fn_804623EC(world, clump);
        }
        fn_80457468(clump);
    }
    field_0x45C = 0;
    field_0x3E8 = 0;
}

void ShadowMotion::fn_800B4328() {
    if (field_0x3E8 != 0) {
        int group;
        switch (m_owner->GetField0x64()) {
        case 0xE:
        case 0xF:
            group = 0;
            break;
        case 0x10:
        case 0x11:
            group = 1;
            break;
        default:
            group = 2;
            break;
        }
        const int *ids = lbl_804B37FC[group];
        for (int i = 0; i < 14; i++) {
            int id = ids[i];
            bool owned = true;
            Fn8040Matrix34 *matrix;
            switch (id) {
            case -1:
                matrix = &field_0x70;
                break;
            case -2:
                matrix = &field_0xA0;
                break;
            default:
                matrix = &fn_80415E70(field_0x3E8, id)->matrix;
                if (matrix == 0) {
                    matrix = field_0x7E0->fn_800B0084(id);
                    owned = false;
                }
                break;
            }
            field_0xD0[i].field_0x0 = matrix;
            field_0xD0[i].field_0x34 = owned;
        }
    }
}

/* The callback of IK target 0 (fn_800B4088): the owner's position turned to the node's frame
 * (behaviour inferred). */
void ShadowMotion::fn_800B42A8(IKEventArgs &args) {
    Fn8040Matrix34 matrix;
    fn_80016370(&matrix, args.field_0x0->matrix);
    Fn8040Vector3 target;
    target = fn_80079628(m_owner.get(), &lbl_80576928);
    AddVector(&target, fn_8040FB08(&matrix));
    *args.field_0x4 = target;
}

void ShadowMotion::fn_800B4088() {
    fn_8006C09C(0, boost::bind(&ShadowMotion::fn_800B42A8, this, _1));
}

void ShadowMotion::fn_800B3D40() {
    switch (field_0x450) {
    case 0:
        if (field_0x440[0] == field_0x420[0]) {
            fn_800A9F9C(0.0f);
        }
        field_0x450++;
        break;
    }
}

void ShadowMotion::fn_800B3CA4() {
    switch (field_0x450) {
    case 1: {
        const UnkMotionState_804B38A4 *state = &lbl_804B38A4[field_0x42C];
        if (state->field_0x4 >= 0) {
            if (fn_800A9E1C(m_owner)) {
                field_0x420[0] = state->field_0x4 + (field_0x3E0 - 1);
                field_0x420[1] = -1;
                field_0x450++;
            }
        } else {
            field_0x450++;
        }
        break;
    }
    }
}

void ShadowMotion::fn_800B3AAC() {
    switch (field_0x450) {
    case 0:
        if (field_0x440[0] == field_0x3E0 + 0x4A || field_0x440[0] == field_0x3E0 + 0x4B) {
            field_0x450 = 2;
            break;
        }
    case 1: {
        int motion = 0x67;
        if (fn_8006CD78(m_owner->field_0xA8, 0x1F)) {
            switch (m_owner->field_0x238->UnkSlot6()) {
            case 5:
                motion = 0x69;
                break;
            default:
                motion = 0x6B;
                break;
            }
        }
        field_0x420[0] = motion + (field_0x3E0 - 1);
        field_0x450 = 3;
        break;
    }
    case 2:
        if (fn_800A9E1C(m_owner)) {
            field_0x450 = 1;
        }
    case 3:
        if (m_owner->field_0x238 != 0 && m_owner->field_0x238->UnkSlot17()) {
            if (fn_8005B904(m_owner->field_0x238->UnkSlot5()) > 0.7f) {
                field_0x420[0] = field_0x3E0 + 0x4A;
            } else {
                field_0x420[0] = field_0x3E0 + 0x4B;
            }
            if (field_0x450 == 2) {
                fn_800A9F9C(0.0f);
            }
            field_0x450 = 2;
        }
        break;
    }
    if (field_0x438 < 0.0f) {
        if (field_0x438 < -1.5f) {
            field_0x420[0] = field_0x3E0 + 0x69;
        } else {
            field_0x420[0] = field_0x3E0 + 0x67;
            if (field_0x7E0 != 0) {
                field_0x7E0->fn_800B0148(3, 0.0f);
            }
        }
        field_0x438 = 0.0f;
        field_0x450 = 2;
    }
    field_0x420[1] = -1;
    field_0x48C = false;
}

void ShadowMotion::fn_800B38C8() {
    /* The motions of the four speed ranges, by whether status bit 0x1E is set (placeholder name). */
    static const int motions[2][4][2] = {
        {{0x67, 0x93}, {0x93, 0x88}, {0x88, 0x91}, {0x91, 0x66}},
        {{0x85, 0x87}, {0x87, 0x88}, {0x88, 0x88}, {0x88, 0x88}},
    };
    int flag = fn_8006CD78(m_owner->field_0xA8, 0x1E) ? 1 : 0;
    float speed = field_0x438;
    int handState;
    int first;
    int second;
    int range;
    if (speed < 1.0f) {
        field_0x448 = speed;
        range = 1;
        field_0x43C = 1.0f;
        first = motions[flag][0][0];
        second = motions[flag][0][1];
        handState = 1;
    } else if (speed < 2.0f) {
        range = 2;
        field_0x448 = speed - 1.0f;
        handState = 1;
        first = motions[flag][1][0];
        field_0x43C = 1.0f;
        second = motions[flag][1][1];
    } else if (speed < 3.0f) {
        range = 3;
        field_0x448 = speed - 2.0f;
        handState = 0;
        first = motions[flag][2][0];
        field_0x43C = 1.0f;
        second = motions[flag][2][1];
    } else {
        field_0x448 = speed - 3.0f;
        first = motions[flag][3][0];
        second = motions[flag][3][1];
        field_0x448 = field_0x448 < 1.0f ? field_0x448 : 1.0f;
        range = 4;
        handState = 0;
        field_0x43C = 1.0f;
    }
    field_0x420[0] = first + (field_0x3E0 - 1);
    if (second >= 0) {
        field_0x420[1] = second + (field_0x3E0 - 1);
    } else {
        field_0x448 = 0.0f;
        field_0x420[1] = -1;
    }
    if (field_0x450 != range) {
        if (field_0x450 > 0) {
            field_0x430 = 0.0f;
            float fraction = fn_800AA064();
            fn_800AA404();
            fn_800A9F9C(fraction);
        }
        if (field_0x7E0 != 0) {
            field_0x7E0->fn_800B0148(handState, 10.0f);
        }
        field_0x450 = range;
    }
}

void ShadowMotion::fn_800B36F8() {
    Fn8040Vector3 forward(fn_800796DC(m_owner.get()));
    Fn8040Vector3 up(fn_8040CEE8(m_owner->field_0x18));
    Fn8040Vector3 side(fn_8040CEE8(fn_8040CFB8(m_owner->field_0x1C8, up)));
    float dot = fn_8040CF90(forward, up);
    int first;
    int second;
    if (dot > 0.0f) {
        first = 0x88;
        field_0x448 = 1.0f - dot;
        if (fn_8040CF90(forward, side) > 0.0f) {
            second = 0x8F;
        } else {
            second = 0x8D;
        }
    } else if (dot > -0.5f) {
        if (fn_8040CF90(forward, side) > 0.0f) {
            first = 0x8C;
            second = 0x8F;
        } else {
            first = 0x8B;
            second = 0x8D;
        }
        field_0x448 = 2.0f * (0.5f + dot);
    } else {
        first = 0x8A;
        if (fn_8040CF90(forward, side) > 0.0f) {
            second = 0x8C;
        } else {
            second = 0x8B;
        }
        field_0x448 = 2.0f * (1.0f + dot);
    }
    int motion0 = first + (field_0x3E0 - 1);
    int motion1 = second + (field_0x3E0 - 1);
    field_0x43C = field_0x438;
    if (field_0x420[0] != motion0 || field_0x420[1] != motion1) {
        field_0x420[0] = motion0;
        field_0x420[1] = motion1;
        float fraction = fn_800AA064();
        fn_800AA404();
        fn_800A9F9C(fraction);
    }
}

void ShadowMotion::fn_800B3624() {
    switch (field_0x450) {
    case 0: {
        if (fn_8006CD78(m_owner->field_0xA8, 0x1E)) {
            field_0x420[0] = field_0x3E0 + 0x7A;
            field_0x420[1] = -1;
        } else {
            field_0x420[0] = field_0x3E0 + 0x6D;
            field_0x420[1] = field_0x3E0 + 0x75;
        }
        Fn8040Vector3 forward(fn_800796DC(m_owner.get()));
        field_0x448 = fn_8040CF90(forward, m_owner->field_0x18) / 700.0f;
        if (field_0x448 > 1.0f) {
            field_0x448 = 1.0f;
        }
        field_0x450++;
        break;
    }
    }
}

void ShadowMotion::fn_800B3538() {
    switch (field_0x450) {
    case 0:
        if (field_0x420[0] == field_0x3E0 + 0x70) {
            field_0x450 = 100;
            break;
        }
    case 1: {
        UnkPlayerObject_803563D0 *object = m_owner->field_0x278;
        if (!object->field_0x24 || object->field_0x20 >= lbl_805EE5E4) {
            field_0x420[0] = field_0x3E0 + 0x71;
            field_0x450 = 500;
        } else {
            field_0x420[0] = field_0x3E0 + 0x6B;
        }
        break;
    }
    case 100:
        if (fn_800A9E1C(m_owner)) {
            field_0x450 = 1;
        }
        break;
    case 500:
        break;
    }
    field_0x420[1] = -1;
}

void ShadowMotion::fn_800B34E0() {
    if (field_0x450 == 0) {
        if (field_0x440[0] != field_0x3E0 + 0x28) {
            if (field_0x440[0] == field_0x3E0 + 0x71) {
                field_0x420[0] = field_0x3E0 + 0x74;
            } else {
                field_0x420[0] = field_0x3E0 + 0x6C;
            }
        }
        field_0x420[1] = -1;
        field_0x450++;
    }
}

void ShadowMotion::fn_800B33D8() {
    switch (field_0x450) {
    case 0: {
        Fn8040Vector3 direction;
        fn_8040F990(0, &direction, &m_owner->field_0x18, &m_owner->field_0xF4);
        fn_8040CE74(&direction);
        switch (fn_80093C60(m_owner, &direction)) {
        case 0:
        case 3:
            field_0x420[0] = field_0x3E0 + 0x6F;
            break;
        case 1:
            field_0x420[0] = field_0x3E0 + 0x8F;
            break;
        case 2:
            field_0x420[0] = field_0x3E0 + 0x8D;
            break;
        }
        field_0x420[1] = -1;
        field_0x450++;
        break;
    }
    case 1:
        if (fn_800A9E1C(m_owner)) {
            field_0x420[0] = field_0x3E0 + 0x6B;
            field_0x430 = 10.0f;
            field_0x450++;
        }
        break;
    }
}

void ShadowMotion::fn_800B334C() {
    if (field_0x450 == 0) {
        if (field_0x420[0] == field_0x3E0 + 0x70) {
            field_0x450 = 2;
            return;
        }
        if (field_0x420[0] == field_0x3E0 + 0x6D && field_0x420[1] == field_0x3E0 + 0x75) {
            field_0x420[0] = field_0x3E0 + 0x6E;
            field_0x420[1] = field_0x3E0 + 0x76;
        } else {
            field_0x420[0] = field_0x3E0 + 0x6E;
            field_0x420[1] = -1;
        }
        field_0x430 = 0.0f;
        field_0x43C = 1.0f;
        field_0x450 = 1;
    }
}

void ShadowMotion::fn_800B32B4() {
    switch (field_0x450) {
    case 1:
        if (field_0x440[0] == field_0x420[0] && fn_800A9E1C(m_owner)) {
            m_owner->field_0xB4.Reset(3);
            field_0x420[0] = field_0x3E0 + 0x70;
            field_0x420[1] = -1;
            field_0x430 = 0.0f;
            field_0x43C = 1.0f;
            field_0x450++;
        }
        break;
    }
}

void ShadowMotion::fn_800B31E0() {
    int motion = -1;
    switch (field_0x450) {
    case 0: {
        int start = 0x2C;
        if (field_0x420[0] == field_0x3E0 + 0x38) {
            start = 0x37;
        }
        motion = start;
        field_0x450++;
        break;
    }
    case 1:
        if (fn_800A9E1C(m_owner)) {
            motion = 0x37;
        }
        if (field_0x438 > 0.0f) {
            motion = 0x3A;
        } else if (field_0x438 < 0.0f) {
            motion = 0x38;
        }
        break;
    }
    if (motion >= 0) {
        field_0x420[0] = motion + (field_0x3E0 - 1);
    }
    field_0x420[1] = -1;
    field_0x438 = 0.0f;
}

void ShadowMotion::fn_800B30F8() {
    int motion = -1;
    switch (field_0x450) {
    case 0:
        motion = 0x30;
        field_0x450++;
        break;
    case 1:
        if (fn_800A9E1C(m_owner)) {
            motion = 0x35;
        }
        if (field_0x438 > 0.0f) {
            motion = 0x36;
        } else if (field_0x438 < 0.0f) {
            motion = 0x33;
            field_0x450 = 100;
        }
        break;
    case 100:
        if (field_0x438 < 0.0f) {
            break;
        }
        motion = 0x34;
        field_0x450 = 1;
        break;
    }
    if (motion >= 0) {
        field_0x420[0] = motion + (field_0x3E0 - 1);
    }
    field_0x420[1] = -1;
    field_0x438 = 0.0f;
}

void ShadowMotion::fn_800B2F48() {
    int motion = -1;
    switch (field_0x450) {
    case 0:
        if (field_0x438 < 0.0f) {
            if (field_0x428 == 0x20 || field_0x428 == 0x17) {
                field_0x450 = 300;
            } else {
                field_0x450 = 301;
                motion = 0x5D;
            }
        } else {
            if (field_0x428 == 0x20 || field_0x428 == 0x17) {
                field_0x450 = 100;
            } else {
                field_0x450 = 101;
                motion = 0x63;
            }
        }
        break;
    case 100:
        motion = 0x54;
    case 101:
        if (fn_800A9E1C(m_owner)) {
            field_0x450 = 100;
        }
        if (field_0x438 < 0.0f) {
            field_0x450 = 200;
        }
        break;
    case 200:
        field_0x430 = 0.0f;
        field_0x450 = 301;
        motion = 0x61;
        break;
    case 300:
        motion = 0x58;
    case 301:
        if (fn_800A9E1C(m_owner)) {
            field_0x450 = 300;
        }
        if (field_0x438 > 0.0f) {
            field_0x450 = 400;
        }
        break;
    case 400:
        field_0x430 = 0.0f;
        field_0x450 = 101;
        motion = 0x62;
        break;
    }
    if (motion > 0) {
        field_0x420[0] = motion + (field_0x3E0 - 1);
    }
    field_0x420[1] = -1;
}

void ShadowMotion::fn_800B2EA4() {
    int motion = field_0x420[0];
    switch (field_0x450) {
    case 0: {
        int start;
        if (field_0x438 > 0.0f) {
            start = 0x90;
        } else {
            start = 0x8E;
        }
        motion = start + (field_0x3E0 - 1);
        field_0x450++;
        break;
    }
    case 1:
        if (fn_800A9E1C(m_owner)) {
            field_0x450 = 200;
        }
        break;
    }
    field_0x420[0] = motion;
    field_0x420[1] = -1;
}

void ShadowMotion::fn_800B2E28() {
    switch (field_0x450) {
    case 0: {
        int motion = fn_8006CD78(m_owner->field_0xA8, 2) ? 0x86 : 0x7C;
        field_0x420[0] = motion + (field_0x3E0 - 1);
        field_0x420[1] = -1;
        field_0x450++;
        break;
    }
    }
}

void ShadowMotion::fn_800B2D38() {
    int motion = -1;
    switch (field_0x450) {
    case 0:
        motion = 0x4F;
        field_0x450++;
        break;
    case 1:
        if (fn_800A9E1C(m_owner)) {
            motion = 0x4D;
            field_0x450++;
        }
        break;
    case 2:
        m_owner->field_0xB4.Reset(3);
        if (field_0x438 != 0.0f) {
            if (field_0x438 > 0.0f) {
                motion = 0x50;
            } else {
                motion = 0x4E;
            }
            field_0x450++;
        }
        break;
    }
    if (motion >= 0) {
        field_0x420[0] = motion + (field_0x3E0 - 1);
        field_0x420[1] = -1;
    }
    field_0x438 = 0.0f;
}

void ShadowMotion::fn_800B2C70() {
    /* The two motions that the state picks between at random (placeholder name). */
    static const int motions[2] = {0x7F, 0x80};
    int motion = field_0x420[0];
    switch (field_0x450) {
    case 0:
        motion = motions[static_cast<int>(100.0f * fn_8040EE90()) & 1] + (field_0x3E0 - 1);
        fn_800A9F9C(0.0f);
        field_0x450++;
        break;
    case 1:
        if (fn_800A9E1C(m_owner)) {
            motion = field_0x3E0 + 0x6B;
            field_0x430 = 20.0f;
            field_0x450 = 200;
        }
        break;
    }
    field_0x420[0] = motion;
    field_0x420[1] = -1;
}

void ShadowMotion::fn_800B2C0C() {
    if (field_0x450 == 0) {
        if (field_0x420[0] == field_0x3E0 + 0x5C || field_0x420[0] == field_0x3E0 + 0x57 ||
            field_0x420[0] == field_0x3E0 + 0x59) {
            field_0x420[0] = field_0x3E0 + 0x59;
        } else {
            field_0x420[0] = field_0x3E0 + 0x55;
        }
        field_0x420[1] = -1;
        field_0x450++;
    }
}

void ShadowMotion::fn_800B2B48() {
    field_0x420[1] = -1;
    switch (field_0x450) {
    case 0:
        field_0x420[0] = field_0x3E0 + 0x77;
        fn_800A9F9C(0.0f);
        field_0x450++;
        break;
    case 1:
        if (fn_800A9E1C(m_owner)) {
            field_0x420[0] = field_0x3E0 + 0x78;
            field_0x450++;
        }
    case 2:
        if (field_0x438 != 0.0f) {
            field_0x450 = 0;
        }
        break;
    }
    field_0x438 = 0.0f;
}

void ShadowMotion::fn_800B2B44() {}

void ShadowMotion::fn_800B2A84() {
    switch (field_0x450) {
    case 0:
        if (fn_800A9E1C(m_owner)) {
            m_owner->field_0xB4.Reset(3);
            field_0x420[0] = field_0x3E0 + 0x43;
            field_0x450++;
        }
        break;
    case 1:
        m_owner->field_0xB4.Reset(3);
        if (fn_8006CD78(m_owner->field_0xA8, 2)) {
            field_0x420[0] = field_0x3E0 + 0x44;
            field_0x450++;
        }
        break;
    }
}

void ShadowMotion::fn_800B2A18() {
    if (fn_8006CD78(m_owner->field_0xA8, 2)) {
        field_0x420[0] = field_0x3E0 + 0x81;
        field_0x420[1] = -1;
    } else {
        field_0x420[0] = field_0x3E0 + 0x82;
        field_0x420[1] = -1;
    }
}

void ShadowMotion::fn_800B2844() {
    Fn8040Vector3 direction;
    fn_8040F990(0, &direction, &m_owner->field_0x18, &m_owner->field_0x1C8);
    float length = fn_8040CF50(direction);
    if (length <= 15.0f) {
        field_0x450 = 0;
    } else {
        field_0x450 = 1;
    }
    if (field_0x450 == 0) {
        field_0x430 = 30.0f;
        field_0x420[0] = field_0x3E4 + 0xC;
        field_0x420[1] = -1;
    } else {
        fn_8040CE74(&direction);
        Fn8040Vector3 forward(fn_800796DC(m_owner.get()));
        Fn8040Vector3 side(fn_8040CEE8(fn_8040CFB8(m_owner->field_0x1C8, direction)));
        float dot = fn_8040CF90(forward, direction);
        field_0x448 = 1.0f - fn_8000A980(dot);
        int first;
        if (dot > 0.0f) {
            if (length <= 100.0f) {
                first = 0xE;
            } else {
                first = 0x12;
            }
        } else {
            first = 0xF;
        }
        int second;
        if (fn_8040CF90(forward, side) > 0.0f) {
            second = 0x11;
        } else {
            second = 0x10;
        }
        int motion0 = first + (field_0x3E4 - 1);
        int motion1 = second + (field_0x3E4 - 1);
        if (field_0x420[0] != motion0 || field_0x420[1] != motion1) {
            field_0x430 = 30.0f;
            field_0x420[0] = motion0;
            field_0x420[1] = motion1;
            float fraction = fn_800AA064();
            fn_800AA404();
            fn_800A9F9C(fraction);
        }
    }
}

void ShadowMotion::fn_800B2814() {
    if (field_0x450 == 0) {
        field_0x420[0] = field_0x3E4 + 0xA;
        field_0x420[1] = -1;
        field_0x450++;
    }
}

void ShadowMotion::fn_800B27E4() {
    if (field_0x450 == 0) {
        field_0x420[0] = field_0x3E4 + 0xB;
        field_0x420[1] = -1;
        field_0x450++;
    }
}

/* Helpers of the IK target callback (placeholder names; their owning class is not recovered), the
 * counterparts of fn_800AA344-fn_800AA390 for Player::MotionEventCallback. */
extern "C" bool fn_800B27D0(const UnkIKCallback_800B12C0 *callback) {
    return !callback->m_function.empty();
}

extern "C" bool fn_800B27B0(const UnkIKCallback_800B12C0 *callback) {
    return fn_800B27D0(callback);
}

extern "C" void fn_800B2784(UnkIKCallback_800B12C0 *callback, IKEventArgs &args) {
    if (!callback->m_function.empty()) {
        callback->m_function(args);
    }
}

/* The node callback of the hand-side IK chains (behaviour inferred): blends the node towards the
 * solution of IK target index when the target is set, otherwise turns the node towards the owner's
 * target. Not matching: the original calls the owner pointer's operator-> out of line for the two
 * fn_8040BAC4 calls on the owner's field_0x198 (8 bytes longer), and its matrices take other stack
 * slots. */
static void fn_800B23B0(MotionNodeCallbackData *data, int index) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    UnkIKRecord_800B1264 &record = motion->field_0x490[index];
    if (record.field_0xD0 == 0.0f) {
        return;
    }
    if (record.field_0xCC) {
        Fn8040Vector3 target;
        if (fn_800B27B0(&record.field_0xC0)) {
            IKEventArgs args;
            args.field_0x0 = data;
            args.field_0x4 = &target;
            fn_800B2784(&record.field_0xC0, args);
        } else {
            target = fn_8040FB08(&record.field_0x90);
        }
        if (index == 1) {
            Fn8040Matrix34 frame = fn_8040B034(fn_8040FB08(data->matrix));
            fn_8040BAC4(&frame, &motion->m_owner->field_0x198);
            fn_8040B80C(&frame, motion->m_owner->m_rotation.z);
            fn_8040B884(&frame, motion->m_owner->m_rotation.x);
            fn_8040B848(&frame, motion->m_owner->m_rotation.y);
            fn_8040B80C(&frame, -40.0f * (lbl_805FAD38 / 180.0f));
            fn_80015DBC(&record.field_0x0[0], &record.field_0x0[1], &record.field_0x0[2], &target,
                        &frame, 1.4f, 0.5f, 1.35103f);
        } else {
            Fn8040Matrix34 frame = fn_8040B034(fn_8040FB08(data->matrix));
            fn_8040BAC4(&frame, &motion->m_owner->field_0x198);
            fn_8040B80C(&frame, motion->m_owner->m_rotation.z);
            fn_8040B884(&frame, motion->m_owner->m_rotation.x);
            fn_8040B848(&frame, motion->m_owner->m_rotation.y);
            fn_8040B848(&frame, 90.0f * (lbl_805FAD38 / 180.0f));
            fn_80015B60(&record.field_0x0[0], &record.field_0x0[1], &record.field_0x0[2], &target,
                        &frame, 1.4f, 0.5f, 1.35103f);
            Fn8040Matrix34 scale(lbl_805E0720);
            fn_8040B720(&scale, 1.0f, 1.0f, 1.2f * record.field_0xD0);
            fn_8040BAC4(&record.field_0x0[2], &scale);
        }
        Fn8040Matrix34 current;
        fn_80016370(&current, data->matrix);
        Fn8040Matrix34 result = fn_8040EFA4(&current, &record.field_0x0[0], record.field_0xD0);
        fn_800163F4(data->matrix, &result);
    } else {
        Fn8040Matrix34 matrix;
        fn_80016370(&matrix, data->matrix);
        Fn8040Vector3 target(motion->m_owner->field_0x24C->UnkSlot6());
        Fn8040Vector3 position(fn_8040FB08(&matrix));
        fn_8040FA20(&matrix, Fn8040Vector3(0.0f, 0.0f, 0.0f));
        Fn8040Vector3 forward(matrix * Fn8040Vector3(0.0f, 0.0f, 1.0f));
        Fn8040Vector3 direction;
        direction = target - position;
        fn_8040CE74(&direction);
        float dot = fn_8040CF90(forward, direction);
        if (dot < 0.99995f) {
            float angle = record.field_0xD0 * fn_80015D68(dot);
            Fn8040Vector3 axis(fn_8040CEE8(fn_8040CFB8(forward, direction)));
            Fn8040Matrix34 turn;
            fn_80049778(&turn, fn_8040CEE8(axis), angle);
            matrix = fn_8001E044(&turn, &matrix);
        }
        fn_8040FA20(&matrix, position);
        fn_800163F4(data->matrix, &matrix);
    }
}

/* The node callback of the middle joints of the IK chains (behaviour inferred; see fn_800B23B0). */
static void fn_800B21A8(MotionNodeCallbackData *data, int index) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    UnkIKRecord_800B1264 &record = motion->field_0x490[index];
    if (record.field_0xD0 == 0.0f) {
        return;
    }
    if (record.field_0xCC) {
        Fn8040Matrix34 current;
        fn_80016370(&current, data->matrix);
        Fn8040Matrix34 result = fn_8040EFA4(&current, &record.field_0x0[1], record.field_0xD0);
        fn_800163F4(data->matrix, &result);
    } else {
        Fn8040Matrix34 matrix;
        fn_80016370(&matrix, data->matrix);
        Fn8040Vector3 target(motion->m_owner->field_0x24C->UnkSlot6());
        Fn8040Vector3 position(fn_8040FB08(&matrix));
        fn_8040FA20(&matrix, Fn8040Vector3(0.0f, 0.0f, 0.0f));
        Fn8040Vector3 forward(matrix * Fn8040Vector3(0.0f, 0.0f, 1.0f));
        if (record.field_0xD0 > 0.0f) {
            Fn8040Vector3 direction;
            direction = target - position;
            fn_8040CE74(&direction);
            float dot = fn_8040CF90(forward, direction);
            if (dot < 0.99995f) {
                float angle = record.field_0xD0 * fn_80015D68(dot);
                Fn8040Vector3 axis(fn_8040CEE8(fn_8040CFB8(forward, direction)));
                Fn8040Matrix34 turn;
                fn_80049778(&turn, fn_8040CEE8(axis), angle);
                matrix = fn_8001E044(&turn, &matrix);
            }
        }
        fn_8040FA20(&matrix, position);
        fn_800163F4(data->matrix, &matrix);
    }
}

/* The node callback of the end joints of the IK chains (behaviour inferred; see fn_800B23B0). */
static void fn_800B1FB0(MotionNodeCallbackData *data, int index) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    UnkIKRecord_800B1264 &record = motion->field_0x490[index];
    if (record.field_0xD0 == 0.0f) {
        return;
    }
    if (record.field_0xCC) {
        Fn8040Matrix34 current;
        fn_80016370(&current, data->matrix);
        Fn8040Matrix34 result = fn_8040EFA4(&current, &record.field_0x0[2], record.field_0xD0);
        fn_800163F4(data->matrix, &result);
    } else {
        Fn8040Vector3 target(motion->m_owner->field_0x24C->UnkSlot6());
        Fn8040Matrix34 matrix;
        fn_80016370(&matrix, data->matrix);
        Fn8040Vector3 position(fn_8040FB08(&matrix));
        fn_8040FA20(&matrix, Fn8040Vector3(0.0f, 0.0f, 0.0f));
        Fn8040Vector3 forward(matrix * Fn8040Vector3(0.0f, 0.0f, 1.0f));
        Fn8040Vector3 direction;
        direction = target - position;
        fn_8040CE74(&direction);
        float dot = fn_8040CF90(forward, direction);
        if (dot < 0.99995f) {
            float angle = record.field_0xD0 * fn_80015D68(dot);
            Fn8040Vector3 axis(fn_8040CEE8(fn_8040CFB8(forward, direction)));
            Fn8040Matrix34 turn;
            fn_80049778(&turn, fn_8040CEE8(axis), angle);
            matrix = fn_8001E044(&turn, &matrix);
        }
        fn_8040FA20(&matrix, position);
        fn_800163F4(data->matrix, &matrix);
    }
}

/* The node callbacks of the two IK chains (nodes 0x15-0x17 and 0x19-0x1B). */
static void fn_800B1F8C(MotionNodeCallbackData *data) {
    fn_800B23B0(data, 1);
}

static void fn_800B1F68(MotionNodeCallbackData *data) {
    fn_800B21A8(data, 1);
}

static void fn_800B1F44(MotionNodeCallbackData *data) {
    fn_800B1FB0(data, 1);
}

static void fn_800B1F20(MotionNodeCallbackData *data) {
    fn_800B23B0(data, 0);
}

static void fn_800B1EFC(MotionNodeCallbackData *data) {
    fn_800B21A8(data, 0);
}

static void fn_800B1ED8(MotionNodeCallbackData *data) {
    fn_800B1FB0(data, 0);
}

/* Stores in result from turned by weight towards to: a copy of to for a weight of 1 or more,
 * otherwise a rotation interpolated between from's and to's (behaviour inferred; the translation
 * of the result is not set). */
static void fn_800B1D34(Fn8040Matrix34 *result, Fn8040Matrix34 *from, Fn8040Matrix34 *to,
                        float weight) {
    if (weight >= 1.0f) {
        fn_8040C148(result, to);
        return;
    }
    Fn8040Vector4 toTemporary;
    fn_8040C55C(&toTemporary, to);
    Fn8040Vector4 rotationTo;
    fn_8007A34C(&rotationTo, &toTemporary);
    Fn8040Vector3 up;
    up = *from * Fn8040Vector3(0.0f, 1.0f, 0.0f);
    Fn8040Vector3 back;
    back = *from * Fn8040Vector3(0.0f, 0.0f, -1.0f);
    UnkRotation3 rotation = fn_8040F364(fn_8040CEE8(back), fn_8040CEE8(up));
    Fn8040Matrix34 matrix(lbl_805E0720);
    fn_8040B848(&matrix, rotation.y);
    fn_8040B884(&matrix, rotation.x);
    fn_8040B80C(&matrix, rotation.z);
    Fn8040Vector4 fromTemporary;
    fn_8040C55C(&fromTemporary, &matrix);
    Fn8040Vector4 rotationFrom;
    fn_8007A34C(&rotationFrom, &fromTemporary);
    Fn8040Vector4 rotation2;
    fn_8040C3EC(&rotation2, &rotationFrom, &rotationTo, weight);
    fn_8040B228(result, &rotation2);
}

/* The node callback of node 0x1C (behaviour inferred): turns the node towards the owner's target
 * by the weight of IK target 1 when status bit 0x1F is set. */
static void fn_800B1B1C(MotionNodeCallbackData *data) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    if (fn_8006CD78(motion->m_owner->field_0xA8, 0x1F)) {
        Fn8040Matrix34 matrix;
        fn_80016370(&matrix, data->matrix);
        Fn8040Vector3 position(fn_8040FB08(&matrix));
        fn_8040FA20(&matrix, lbl_805E07A0);
        if (motion->field_0x490[1].field_0xD0 > 0.0f) {
            Fn8040Vector3 up;
            Fn8040Vector3 forward;
            Fn8040Vector3 side;
            Fn8040Vector3 target(motion->m_owner->field_0x24C->UnkSlot6());
            forward = fn_8040CEE8(position - target);
            up = fn_80016284(
                motion->m_owner->field_0x24C->UnkSlot9(Fn8040Vector3(0.0f, 1.0f, 0.0f)));
            side = fn_8040CFB8(up, forward);
            up = fn_8040CEE8(fn_8040CFB8(forward, side));
            UnkRotation3 rotation = fn_8040F364(forward, up);
            Fn8040Matrix34 turn;
            fn_8040B6F8(&turn);
            fn_8040B848(&turn, rotation.y);
            fn_8040B884(&turn, rotation.x);
            fn_8040B80C(&turn, rotation.z);
            Fn8040Matrix34 turned;
            fn_800B1D34(&turned, &matrix, &turn, motion->field_0x490[1].field_0xD0);
            matrix = turned;
        }
        fn_8040FA20(&matrix, position);
        fn_800163F4(data->matrix, &matrix);
    }
}

/* The node callback of node 0x18 (behaviour inferred): while status bit 0x1E is set and the owner
 * holds an object, turns the node towards the owner's facing by the weight of IK target 0 and
 * passes the node's frame to the held object's grab position handler. */
static void fn_800B1838(MotionNodeCallbackData *data) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    if (!fn_8006CD78(motion->m_owner->field_0xA8, 0x1E)) {
        return;
    }
    if (motion->m_owner->fn_80073604().expired()) {
        return;
    }
    Fn8040Matrix34 matrix;
    fn_80016370(&matrix, data->matrix);
    Fn8040Vector3 position(fn_8040FB08(&matrix));
    fn_8040FA20(&matrix, Fn8040Vector3(0.0f, 0.0f, 0.0f));
    if (motion->field_0x490[0].field_0xD0 > 0.0f) {
        Fn8040Vector3 axis;
        axis = fn_80016284(fn_80079698(motion->m_owner.get()));
        Fn8040Vector3 forward;
        forward = fn_800796DC(motion->m_owner.get());
        Fn8040Vector3 up;
        up = fn_8040CFB8(axis, forward);
        axis = fn_8040CEE8(fn_8040CFB8(forward, up));
        UnkRotation3 rotation = fn_8040F364(forward, axis);
        Fn8040Matrix34 turn;
        fn_8040B6F8(&turn);
        fn_8040B848(&turn, rotation.y);
        fn_8040B884(&turn, rotation.x);
        fn_8040B80C(&turn, rotation.z);
        Fn8040Matrix34 turned;
        fn_800B1D34(&turned, &matrix, &turn, motion->field_0x490[0].field_0xD0);
        matrix = turned;
    }
    fn_8040FA20(&matrix, position);
    fn_800163F4(data->matrix, &matrix);
    boost::shared_ptr<PickupCharInfo> held = motion->m_owner->fn_80073604().lock();
    Fn8040Vector3 axis;
    axis = fn_80079698(motion->m_owner.get());
    Fn8040Vector3 forward;
    forward = fn_800796DC(motion->m_owner.get());
    UnkRotation3 rotation = fn_8040F364(forward, axis);
    Fn8040Matrix34 frame = fn_8040B034(position);
    fn_8040B848(&frame, rotation.y);
    fn_8040B884(&frame, rotation.x);
    fn_8040B80C(&frame, rotation.z);
    held->fn_801E41C4(&frame);
}

/* The node callback of node 0x1E: turns the node by the rotation at 0x47C (fn_800B1308). */
static void fn_800B1808(MotionNodeCallbackData *data) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    motion->field_0x47C.fn_800B1308(data, motion->field_0x478);
}

/* The node callbacks of nodes 0x29-0x2B and 0x2D-0x2F (behaviour inferred): while IK target 2 or 3
 * is set, they store its joint matrices in the nodes, the first after solving the two-joint chain
 * towards the target matrix. */
static void fn_800B1798(MotionNodeCallbackData *data) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    UnkIKRecord_800B1264 &record = motion->field_0x490[2];
    if (record.field_0xCC) {
        fn_80015DE0(&record.field_0x0[0], &record.field_0x0[1], 1.6750001f, 2.525f,
                    fn_8040FB08(&record.field_0x90), data->matrix);
        fn_800163F4(data->matrix, &record.field_0x0[0]);
    }
}

static void fn_800B1760(MotionNodeCallbackData *data) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    if (motion->field_0x490[2].field_0xCC) {
        fn_800163F4(data->matrix, &motion->field_0x490[2].field_0x0[1]);
    }
}

static void fn_800B1728(MotionNodeCallbackData *data) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    if (motion->field_0x490[2].field_0xCC) {
        fn_800163F4(data->matrix, &motion->field_0x490[2].field_0x90);
    }
}

static void fn_800B16B8(MotionNodeCallbackData *data) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    UnkIKRecord_800B1264 &record = motion->field_0x490[3];
    if (record.field_0xCC) {
        fn_80015DE0(&record.field_0x0[0], &record.field_0x0[1], 1.6750001f, 2.525f,
                    fn_8040FB08(&record.field_0x90), data->matrix);
        fn_800163F4(data->matrix, &record.field_0x0[0]);
    }
}

static void fn_800B1680(MotionNodeCallbackData *data) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    if (motion->field_0x490[3].field_0xCC) {
        fn_800163F4(data->matrix, &motion->field_0x490[3].field_0x0[1]);
    }
}

static void fn_800B1648(MotionNodeCallbackData *data) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    if (motion->field_0x490[3].field_0xCC) {
        fn_800163F4(data->matrix, &motion->field_0x490[3].field_0x90);
    }
}

UnkRotation_800B1618::UnkRotation_800B1618() {
    fn_8040C908(&field_0x0);
}

/* Turns the node's rotation towards the owner's target, within limits, while bit 1 of the motion's
 * flags is set and the target object reports one (slot 2), and back towards the rest pose
 * otherwise, at most by a rate per second (behaviour inferred). Not matching: the original stores
 * the (0, 0, -1) vector before it calls fn_8040B228 and places its locals in other stack slots. */
void UnkRotation_800B1618::fn_800B1308(MotionNodeCallbackData *data, float dt) {
    ShadowMotion *motion = static_cast<ShadowMotion *>(data->argument);
    Fn8040Vector2 angles(0.0f, 0.0f);
    Fn8040Matrix34 matrix;
    fn_80016370(&matrix, data->matrix);
    Fn8040Vector4 target;
    float rate;
    if (motion->field_0x60.Test(1) && motion->m_owner->field_0x24C->UnkSlot2()) {
        Fn8040Vector3 position;
        position = motion->m_owner->field_0x24C->UnkSlot5();
        Fn8040Vector3 local;
        local = fn_8040BEF8(&matrix) * position;
        Fn8040Vector3 direction;
        direction = fn_8040CEE8(local);
        angles = fn_8040F760(&direction);
        angles.x = fn_80070460(angles.x, -40.0f * (lbl_805FAD38 / 180.0f),
                               45.0f * (lbl_805FAD38 / 180.0f));
        angles.y = fn_80070460(angles.y, -45.0f * (lbl_805FAD38 / 180.0f),
                               45.0f * (lbl_805FAD38 / 180.0f));
        Fn8040Matrix34 turn;
        fn_8040B6F8(&turn);
        fn_8040B848(&turn, angles.y);
        fn_8040B884(&turn, angles.x);
        Fn8040Vector4 targetTemporary;
        fn_8040C55C(&targetTemporary, &turn);
        fn_8007A34C(&target, &targetTemporary);
        rate = 200.0f * (lbl_805FAD38 / 180.0f);
    } else {
        fn_8040E988(&angles);
        fn_8040C908(&target);
        rate = 400.0f * (lbl_805FAD38 / 180.0f);
    }
    Fn8040Matrix34 current;
    fn_8040B228(&current, &field_0x0);
    Fn8040Vector3 back(current * Fn8040Vector3(0.0f, 0.0f, -1.0f));
    Fn8040Vector3 goal(0.0f, 0.0f, -1.0f);
    goal = fn_8040D118(goal, angles.x);
    goal = fn_8040D0AC(goal, angles.y);
    float angle = fn_80015D68(fn_8040CF90(back, goal));
    rate *= dt;
    float t;
    if (angle >= rate) {
        t = rate / angle;
        if (t >= 0.5f) {
            t = 0.5f;
        }
    } else {
        t = 0.5f;
    }
    Fn8040Vector4 rotation;
    fn_8040C3EC(&rotation, &field_0x0, &target, t);
    fn_8007A34C(&field_0x0, &rotation);
    fn_8040B8C0(&matrix, &field_0x0);
    fn_800163F4(data->matrix, &matrix);
}

UnkIKRecord_800B1264::UnkIKRecord_800B1264() {
    field_0xCC = false;
    field_0xD0 = 0.0f;
}

UnkIKRecord_800B1264::~UnkIKRecord_800B1264() {}

} // namespace Shadow
} // namespace Player

void fn_800B10CC(Task *parent, const boost::shared_ptr<Player::PlayerBase> &owner,
                 const Player::Shadow::UnkMotionResources_800B0D3C &resources) {
    new Player::Shadow::ShadowMotion(parent, owner, resources);
}

/* EggLastMech::EggLastMech (class name from RTTI; class described in Game/EggLastMech.h): the
 * character info and logic of the boss that the gadget's task, EggLastMech::EggLastMechTask (class
 * name from RTTI), owns. File name inferred from the class. Definitions are in reverse address
 * order (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to its command unit, its model wrapper and its attack object are
 * boost::function objects built with boost::bind(&EggLastMech::method, this, _1); their manager,
 * invoker and copy functions come from the boost headers. */
/* Matching workaround: CharaColliBaseCharInfo.h and TNumEdit.h are included only for the RTTI
 * records that their never-called stand-in functions emit (see the notes there); no code in this
 * file uses them (CharaColliBase itself comes with UnkWorld_80021B08.h). Without them the unit's
 * .data is 0x15C bytes instead of 0x18C, .sdata 0x88 instead of 0xB8 and .rodata 0x549 instead of
 * 0x5ED, and the .data offsets that fn_802FBDE4 loads move down by 0x30. CharaColliBaseCharInfo.h
 * comes first, in the order their names appear in the unit's .rodata (USA (GUPE8P) 0x8050A970:
 * CharaColliBase, then TNumEdit); with TNumEdit.h first, the names in .rodata and the RTTI records
 * in .sdata change places. Revisit when the original inline functions or their real callers are
 * recovered. */
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/TNumEdit.h"
#include "Game/EggLastMech.h"
#include "Game/CharCommand.h"
#include "Game/EffectElement.h"
#include "Game/EnemyDispFunc.h"
#include "Game/Gadget.h"
#include "Game/LockonTarget.h"
#include "Game/PlayerBase.h"
#include "Game/Target.h"
#include "Game/Unk802CD448.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80079808.h"
#include "Game/UnkManager_80109D64.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkParams_80051D10.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_801A35AC.h"
#include "Game/fn_801A4430.h"
#include "Game/fn_80209B58.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include <cstdio>

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

namespace EggLastMech {

/* For the reactions of field_0xEC: the bits of field_0x110 that decide between a reaction state
 * and fn_803123E8's state (-1: always the reaction state), and the message ids that fn_802FD384
 * plays. */
static const int s_reactionBits[8] = {6, 6, 4, 4, 3, 5, 7, -1};
static const int s_reactionMessages[8] = {4, 5, 6, 7, 8, 9, 10, -1};

/* Passes the message id of reaction index to fn_802FAB88. */
static void fn_802FDAC8(int index) {
    EggLastMech::fn_802FAB88(s_reactionMessages[index]);
}

boost::shared_ptr<EggLastMech> EggLastMech::Create(Task *task) {
    boost::shared_ptr<EggLastMech> mech = boost::shared_ptr<EggLastMech>(new EggLastMech(task));
    return mech;
}

EggLastMech::EggLastMech(Task *task)
    : EnemyCharInfo(0xAD), m_stateMachine(0), field_0xEC(0), field_0xF0(0), field_0xF4(0),
      field_0xF8(0), field_0xFC(0), m_task(task) {
    field_0x118 = 0.0f;
    field_0x11C = 0.0f;
    field_0xAC = 700.0f;
    field_0xB4 = 2.0f;
    m_health = 150.0f;
    field_0x10C = 0;
    m_player = fn_800A8B74(fn_80074194(), lbl_805EE588);
    field_0x104 = false;
    field_0x105 = false;
    field_0x114 = 0.0f;
}

EggLastMech::~EggLastMech() {
    m_task = 0;
    if (field_0xFC != 0) {
        delete field_0xFC;
        field_0xFC = 0;
    }
    if (field_0xF8 != 0) {
        delete field_0xF8;
        field_0xF8 = 0;
    }
    if (field_0xF0 != 0) {
        delete field_0xF0;
        field_0xF0 = 0;
    }
    if (field_0xEC != 0) {
        delete field_0xEC;
        field_0xEC = 0;
    }
    if (field_0xF4 != 0) {
        delete field_0xF4;
        field_0xF4 = 0;
    }
    if (m_stateMachine != 0) {
        delete m_stateMachine;
        m_stateMachine = 0;
    }
    if (m_probe) {
        fn_800C78B0(fn_80021B08(), &m_probe);
    }
    fn_800BE2EC(false);
}

void EggLastMech::Init(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_position);
    fn_800C9ED4(owner, &m_rotation);
    fn_802FCB10();
    fn_802FC574();
    fn_802FBDE4();

    m_target = fn_801E7128(fn_802FB430(), 0x800);
    m_target->SetFlags(1);

    m_stateMachine = new UnkStateMachine_802FD008(this);
    m_stateMachine->m_current = fn_803124BC();
    m_stateMachine->m_previous = fn_803124BC();
    m_stateMachine->fn_802FD008(fn_80311C5C(), 0);

    field_0xF4 = new Unk80313648(this);
    field_0xEC = new Unk803142C0(this);
    field_0xF0 = new Unk8032D6FC(this);
    field_0xF8 = new Unk80334C78(this);
    field_0xFC = new Unk80336F48(this);
    fn_800BE2EC(true);
}

/* When field_0xEC's reaction index (its field_0x4) is 0 or more: passes the reaction's message
 * to fn_802FAB88 (fn_802FDAC8), then enters the reaction state (fn_802FCEDC) when the reaction's
 * bit is -1 or set in field_0x110, and otherwise fn_803123E8's state with the bit as the
 * argument. */
void EggLastMech::fn_802FD384() {
    int index = field_0xEC->field_0x4;
    if (index >= 0) {
        fn_802FDAC8(index);
        int bit = s_reactionBits[index];
        if (bit == -1 || field_0x110.Test(bit)) {
            fn_802FCEDC(index);
        } else {
            fn_802FCFE4(fn_803123E8(), bit);
        }
    }
}

/* Unless bit 2 of field_0x110 is set, once the health is <= 0: sets bit 0 of field_0xC4, calls
 * field_0xEC's fn_80313C10 and field_0xFC's fn_80336DF4 with 5 (when there is one) and changes to
 * fn_8031017C's state. */
void EggLastMech::fn_802FD2F8() {
    if (!field_0x110.Test(2) && m_health <= 0.0f) {
        field_0xC4.Set(0);
        field_0xEC->fn_80313C10();
        if (field_0xFC != 0) {
            field_0xFC->fn_80336DF4(5);
        }
        fn_802FCFE4(fn_8031017C(), 0);
    }
}

void EggLastMech::Update(float dt) {
    if (fn_802DFA00()) {
        m_health = 0.0f;
    }
    field_0x118 -= fn_8019FFE0();
    /* Compared in the assignment: as a separate statement the field is loaded again for the test
     * (one more instruction than the original). */
    if ((field_0x114 -= dt) < 0.0f) {
        field_0x114 = 0.0f;
    }
    if (field_0xF8 != 0) {
        field_0xF8->fn_80334A70();
    }
    if (!field_0x104 && m_health < 150.0f) {
        field_0x104 = true;
        fn_802FAB88(1);
    }
    fn_802FD2F8();
    if (m_stateMachine != 0) {
        /* Runs the current state's Execute with m_updating set, when there is a current state. The
         * state machine's pointer is read once: with m_stateMachine written at each use MWCC
         * reloads it and Update is 0x268 bytes instead of 0x260. */
        UnkStateMachine_802FD008 *machine = m_stateMachine;
        if (machine->m_current != 0) {
            machine->m_updating = true;
            machine->m_current->Execute(machine->m_mech);
            machine->m_updating = false;
        }
    }

    field_0x6C = m_position;
    float speed;
    if (field_0x24.x * field_0x24.x + field_0x24.y * field_0x24.y + field_0x24.z * field_0x24.z <
        0.0001f) {
        speed = 8.0f;
    } else {
        speed = 1.0f;
    }
    fn_804103E8(&m_position, &field_0x18, &field_0x24, speed, fn_8019FFE0());
    fn_8040CE60(&field_0x24);

    m_model->fn_802CD728(m_position, m_rotation);
    m_model->fn_802CD824(dt);
    m_position = m_model->fn_802CD450();
    m_model->fn_802CD7E8();
    Unk801A35ACParameters *parameters = m_model->fn_802CD5AC()->fn_801A3634(0);
    m_model->fn_802CD5AC()->fn_801A35AC(*parameters, -1);
    field_0xEC->fn_80313CC0();
    fn_800CCE38(m_target.get(), m_model->fn_802CD508(kTargetFrame));
    fn_800BE2DC(m_health / 150.0f);
    field_0x105 = false;

    Fn8040Vector3 position(m_position);
    position.y = 0.1f;
    fn_80356400(position, Fn8040Vector3(0.0f, 1.0f, 0.0f), 25.0f);
}

void UnkStateMachine_802FD008::fn_802FD008(EggLastMechAIStateBase *state, int arg) {
    if (m_lock == 0 || m_updating) {
        if (m_current != 0) {
            m_previous = m_current;
            m_current->Exit(m_mech);
            m_current = state;
            m_current->Enter(m_mech, arg);
        }
    }
}

void EggLastMech::fn_802FCFE4(EggLastMechAIStateBase *state, int arg) {
    m_stateMachine->fn_802FD008(state, arg);
}

void EggLastMech::fn_802FCFD0() {
    m_stateMachine->m_lock++;
}

void EggLastMech::fn_802FCFBC() {
    m_stateMachine->m_lock--;
}

/* For kind 0 to 7, changes to fn_80311060's state with the reaction state of that kind as the
 * argument (the argument is an int, so the pointer is converted); for any other kind, changes to
 * fn_80311AE4's state with 0. */
void EggLastMech::fn_802FCEDC(int kind) {
    EggLastMechAIStateBase *state = 0;
    switch (kind) {
    case 0:
    case 1:
        state = fn_80310EA0();
        break;
    case 2:
    case 3:
        state = fn_80310C9C();
        break;
    case 4:
        state = fn_803107FC();
        break;
    case 5:
        state = fn_8031062C();
        break;
    case 6:
        state = fn_803104C0();
        break;
    case 7:
        state = fn_80310314();
        break;
    }
    if (state != 0) {
        fn_802FCFE4(fn_80311060(), reinterpret_cast<int>(state));
    } else {
        fn_802FCFE4(fn_80311AE4(), 0);
    }
}

/* Whether there is a current AI state, its GetId returns 4 and field_0x10C is 1. The first two
 * tests are stored as a bool before field_0x10C is tested (USA (GUPE8P) 0x802FCE9C-0x802FCEA8). */
bool EggLastMech::fn_802FCE60() {
    bool inState4;
    if (m_stateMachine->m_current != 0 && m_stateMachine->m_current->GetId() == 4) {
        inState4 = true;
    } else {
        inState4 = false;
    }
    if (inState4 && field_0x10C == 1) {
        return true;
    }
    return false;
}

/* The command handler. For command 0x17 it sets bit 1 of the command's flags. While bit 1 is
 * clear it passes the command a weak pointer to the boss (the command's fn_801D9CFC), and when
 * bit 1 is still clear after that it prints the "isn't Received" message with the command's id
 * and the string of its fn_801D9CA8. */
void EggLastMech::fn_802FCD20(CharCommand &command) {
    switch (command.m_type) {
    case 0x17:
        command.m_flags |= 2;
        break;
    }
    if (!(command.m_flags & 2)) {
        command.fn_801D9CFC(boost::weak_ptr<CharInfo>(fn_802FCE30()));
    }
    if (!(command.m_flags & 2)) {
        printf("EggLastMech Command isn't Received ! : (%d)%s\n", command.m_type,
               command.fn_801D9CA8());
    }
}

void EggLastMech::fn_802FCB10() {
    fn_8019FD64(CharCommandHandler(boost::bind(&EggLastMech::fn_802FCD20, this, _1)));
}

/* The model's parts: the part index and its parent frame id (-1 for none). */
static const Unk801A35ACEntry s_parts[8] = {
    {9, -1}, {10, 5001}, {11, 5001}, {14, 5001}, {12, 5001}, {4, 5001}, {6, 5001}, {8, 5001},
};

/* Builds the model wrapper from the resources of kResourceKey and, when that gives one, places it
 * at the position and rotation, passes motion 12 to its Unk801A4430's fn_801A4898, calls its
 * update with 0 and installs fn_802FB670 as its motion event callback. */
void EggLastMech::fn_802FC574() {
    void *resourceData = fn_80012684(fn_800122D4(), kResourceKey, 0xF);
    u32 resourceValue = fn_80012608(fn_800122D4(), kResourceKey, 0x19);
    Unk801A35ACDescriptor parts;
    parts.m_entries = s_parts;
    parts.m_count = 8;
    m_model = fn_802CDB08(kResourceKey, &parts, resourceData, resourceValue);
    if (m_model) {
        m_model->fn_802CD638(1.0f);
        m_model->fn_802CD728(m_position, m_rotation);
        m_model->fn_802CD588()->fn_801A4898(12, 0, 0.0f);
        m_model->fn_802CD824(0.0f);
        m_model->fn_802CD7E8();
        m_model->fn_802CD270(boost::function1<void, EnemyMotionEventArg &>(
            boost::bind(&EggLastMech::fn_802FB670, this, _1)));
    }
}

/* First looks up the resource of kResourceKey and index 0x11 (fn_8019A890, USA (GUPE8P)
 * 0x802FBE08). That call comes before the model wrapper is tested, so it runs, and stores 0x202 in
 * the resource's entries when it finds the resource, even when there is no model wrapper. Only
 * when there is both a model wrapper and a resource does it build the probe (fn_800C7F7C) from the
 * resource and the model wrapper's motion control, set the probe's byte at 0x2C and, when the
 * probe's attack object still exists, install fn_802FBA28 and fn_802FBA24 (fn_80054DD0) and
 * fn_802FB8DC (fn_80096B64) as its callbacks. */
void EggLastMech::fn_802FBDE4() {
    void *collision = fn_8019A890(kResourceKey, 0x11);
    if (m_model && collision != 0) {
        m_probe =
            fn_800C7F7C(fn_80021B08(), fn_802FB430(), collision, m_model->fn_802CD420(), 5, 0);
        m_probe->SetField0x2C(1);

        boost::weak_ptr<CharaColliAttack> attackWeak = fn_800C4374(fn_80021B08(), m_probe);
        if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
            attack->fn_80054DD0(
                1, 1, AttackCallback(boost::bind(&EggLastMech::fn_802FBA28, this, _1)),
                AttackCallback(boost::bind(&EggLastMech::fn_802FBA24, this, _1)), 1.0f);
            attack->fn_80096B64(1,
                                AttackCallback(boost::bind(&EggLastMech::fn_802FB8DC, this, _1)));
        }
    }
}

/* Locks the owner (m_owner) of the attack object that param.field_0x0 locks to (that lock is used
 * without a test). When the owner still exists, sets param.field_0x30 to 1 if field_0x114 > 0 and
 * the owner's field_0x5C is 1, and stores param.field_0x18 minus the owner's position in
 * param.field_0x34; otherwise zeroes param.field_0x34 (fn_8040CE60). */
void EggLastMech::fn_802FBA28(AttackCallbackParam &param) {
    boost::shared_ptr<CharInfo> owner = param.field_0x0.lock()->m_owner.lock();
    if (owner) {
        if (field_0x114 > 0.0f && owner->field_0x5C == 1) {
            param.field_0x30 = 1;
        }
        param.field_0x34 = param.field_0x18 - owner->m_position;
        return;
    }
    fn_8040CE60(&param.field_0x34);
}

void EggLastMech::fn_802FBA24(AttackCallbackParam &param) {}

/* Halves field_0x18. When param.field_0x0 has not expired and the field_0x40 of the attack object
 * it locks to has one of the bits 0xA10 set, sets param.field_0x54 to 1 and passes 0x6002
 * (fn_8040EA90(2) nonzero) or 0x6003 to field_0xFC's fn_80336B58; otherwise, when param.field_0x14
 * is not null and its field_0x20 is kTargetFrame, sets param.field_0x54 to 1 and passes 0x7F36.
 * Then, when param.field_0x54 is 1, it takes the damage (fn_802FB7BC) and passes 0x225,
 * param.field_0x18 and the angles of param.field_0x40 (fn_8003B9E8) to fn_801D2AFC; for any other
 * value it sets param.field_0x54 to 5. field_0xFC is used without a test. */
void EggLastMech::fn_802FB8DC(AttackCallbackParam &param) {
    /* Read before the call: the original loads param.field_0x14 before fn_8040CE38. */
    SonicteamUSA::System::CharaColli::UnkAttackPart_80192F14 *part = param.field_0x14;
    fn_8040CE38(&field_0x18, 0.5f);
    if (!param.field_0x0.expired() && (param.field_0x0.lock()->field_0x40 & 0xA10)) {
        param.field_0x54 = 1;
        if (fn_8040EA90(2) != 0) {
            field_0xFC->fn_80336B58(0x6002);
        } else {
            field_0xFC->fn_80336B58(0x6003);
        }
    } else if (part != 0 && part->field_0x20 == kTargetFrame) {
        param.field_0x54 = 1;
        field_0xFC->fn_80336B58(0x7F36);
    }

    if (param.field_0x54 == 1) {
        fn_802FB7BC(param);
        Fn8040Vector3 angles(lbl_805E07A0);
        fn_8003B9E8(&param.field_0x40, &angles.x, &angles.y);
        fn_801D2AFC(0, &param.field_0x18, &angles, 0x225, field_0x60 + 1);
    } else {
        param.field_0x54 = 5;
    }
}

/* Subtracts param.field_0x50 from the health and sets it to 0 when the result is below 0, passes
 * 100 times the health taken (converted to an int) to fn_8016F0CC with index 0 when fn_80079808
 * returns a manager, and halves field_0x18. When bit 1 of field_0x110 is clear and the health is
 * below 75, creates object 12 at the position (fn_8034DE08) and changes to fn_80312274's state
 * with 1; otherwise fn_80336888 creates its object under m_task with the parameters of the
 * model's part 0. */
void EggLastMech::fn_802FB7BC(AttackCallbackParam &param) {
    float health = m_health;
    /* Compared in the assignment, as in Update. */
    if ((m_health -= param.field_0x50) < 0.0f) {
        m_health = 0.0f;
    }
    UnkManager_80079808 *manager = fn_80079808();
    if (manager != 0) {
        fn_8016F0CC(&manager->field_0x70C, 0, 100.0f * (health - m_health));
    }
    fn_8040CE38(&field_0x18, 0.5f);
    if (!field_0x110.Test(1) && m_health < 75.0f) {
        fn_8034DE08(12, &m_position, -2);
        fn_802FCFE4(fn_80312274(), 1);
    } else {
        fn_80336888(m_task, m_model->fn_802CD5AC()->fn_801A3634(0), 1.0f, 0.0f, 0.0f, 1.0f);
    }
}

/* The model's motion events; the payload is read as words. Event 0x5000 with the first word 1
 * calls field_0xEC's fn_80314004 with 0 when entry 0 of fn_80079808's table exists and its
 * field_0x20 is 0, and with 1 otherwise; with the first word 2 it fires a shot (fn_802FADB4) of
 * field_0xEC's field_0x18 from the frame the second word selects. Events 0x5003 and 0xD004 pass
 * 0x7F2E to field_0xFC's fn_80336B58 for the first word 0x24 (without a test of field_0xFC) and
 * create object 7 (fn_8034DF48) for 0x21. Events without a payload are ignored. */
void EggLastMech::fn_802FB670(EnemyMotionEventArg &event) {
    switch (event.field_0x0) {
    case 0x5000: {
        const int *data = static_cast<const int *>(event.field_0x4);
        if (data != 0) {
            switch (data[0]) {
            case 1: {
                UnkEntry_80178B5C *entry = fn_80178B5C(fn_80079808(), 0);
                if (entry != 0 && fn_8016EF84(&entry->field_0x20) == 0) {
                    field_0xEC->fn_80314004(field_0xEC->fn_80313F48(), 0, 2);
                } else {
                    field_0xEC->fn_80314004(field_0xEC->fn_80313F48(), 1, 2);
                }
                break;
            }
            case 2:
                fn_802FADB4(field_0xEC->field_0x18, data[1]);
                break;
            }
        }
        break;
    }
    case 0x5003:
    case 0xD004: {
        const int *data = static_cast<const int *>(event.field_0x4);
        if (data != 0) {
            switch (data[0]) {
            case 0x24:
                field_0xFC->fn_80336B58(0x7F2E);
                break;
            case 0x21:
                fn_8034DF48(7, -2);
                break;
            }
        }
        break;
    }
    }
}

void EggLastMech::fn_802FB644(int arg0) {
    if (field_0xFC != 0) {
        field_0xFC->fn_80336DF4(arg0);
    }
}

void EggLastMech::fn_802FB618() {
    if (field_0xFC != 0) {
        field_0xFC->fn_80336D38();
    }
}

Unk801A4430 *EggLastMech::fn_802FB5F4() {
    return m_model->fn_802CD588();
}

Fn8040Matrix34 *EggLastMech::fn_802FB5C0(unsigned int nodeId) {
    return fn_802FB5F4()->fn_801A46E4(nodeId);
}

/* The vector from the boss to m_player's position, or the zero vector when m_player no longer
 * exists. */
Fn8040Vector3 EggLastMech::fn_802FB504() {
    if (boost::shared_ptr<CharInfo> player = fn_802FB1B8().lock()) {
        return player->m_position - m_position;
    }
    return lbl_805E07A0;
}

/* The frames that the shots start from, by shot index. */
static const int s_shotFrames[6] = {0x13BB, 0x13C5, 0x13BC, 0x13C6, 0x13BD, 0x13C7};

/* Fires a shot through lbl_8051DDDC from the model frame that index selects. For kind 1 the owner
 * is the boss and the lock-on target is the Target that the object at 0x24C of m_player returns
 * (fn_80206574), when m_player still exists, is a Player::PlayerBase and has that object. For any
 * other kind the owner is what fn_800B75DC returns for m_player and the target is the boss's own
 * Target. Nothing is fired when the owner or the target has expired, or when the LockonTarget
 * allocation fails; a fired shot also passes 0x201E to field_0xFC's fn_80336B58 (without a test)
 * and sound 0x201D to fn_801D560C. */
void EggLastMech::fn_802FADB4(int kind, int index) {
    boost::weak_ptr<CharInfo> owner;
    boost::weak_ptr<Target> target;
    if (kind == 1) {
        owner = fn_802FB430();
        boost::shared_ptr<CharInfo> player = fn_802FB1B8().lock();
        boost::shared_ptr<Player::PlayerBase> playerBase =
            boost::dynamic_pointer_cast<Player::PlayerBase>(player);
        if (playerBase && playerBase->field_0x24C != 0) {
            target = playerBase->field_0x24C->fn_80206574();
        }
    } else {
        owner = fn_800B75DC(fn_802FB1B8());
        target = m_target;
    }

    if (!owner.expired() && !target.expired()) {
        boost::shared_ptr<Weapon::LockonTarget> lockon =
            boost::shared_ptr<Weapon::LockonTarget>(new Weapon::LockonTarget());
        if (lockon) {
            lockon->fn_80061054(target);
            lockon->fn_80060F70();

            Fn8040Matrix34 *frame = m_model->fn_802CD560(s_shotFrames[index]);
            Fn8040Vector3 position;
            fn_8040FAA0(&position, frame);
            Fn8040Vector3 direction(0.0f, 0.0f, 1.0f);
            direction = fn_8040B618(*frame, direction);
            /* The argument is an int: passing the bool kind == 1 masks it to 8 bits (rlwinm)
             * where the original only shifts the comparison result (srwi). */
            UnkParams_80051D10 params(owner, lockon, kind == 1 ? 1 : 0, position, direction,
                                      lbl_805E07A0);
            lbl_8051DDDC.create(&params);
            field_0xFC->fn_80336B58(0x201E);
            fn_801D560C(fn_80053164(), 0x201D, 0, 0);
        }
    }
}

/* Passes kind 2 and parameters (the position of the model's frame kTargetFrame, the offset
 * (0, 100, 0) plus half of field_0x18 when arg is 1, 10, true and 20 in field_0x1C) to
 * fn_80209B58 under task layer 14, and plays sound 0x6037 at that position. */
void EggLastMech::fn_802FACE8(int arg) {
    Fn8040Vector3 offset(0.0f, 100.0f, 0.0f);
    if (arg == 1) {
        AddVector(&offset, field_0x18 * 0.5f);
    }
    Fn8040Vector3 position = m_model->fn_802CD508(kTargetFrame);
    UnkParams_80145B58 params(position, offset, 10.0f, true);
    params.field_0x1C = 20.0f;
    fn_80209B58(fn_801EB304(fn_8001C4F8(), 14), 2, &params);
    fn_801D54F4(fn_80053164(), 0x6037, &position, 0, 1);
}

/* Whether the squared lengths of field_0x18 and field_0x24 are both below 0.0001. The conditional
 * expression converts the result to bool once more (clrlwi, subic, subfe), as the original does;
 * a plain return of the && expression returns it with mr. */
bool EggLastMech::fn_802FAC7C() {
    return (field_0x18.x * field_0x18.x + field_0x18.y * field_0x18.y +
                    field_0x18.z * field_0x18.z <
                0.0001f &&
            field_0x24.x * field_0x24.x + field_0x24.y * field_0x24.y +
                    field_0x24.z * field_0x24.z <
                0.0001f)
               ? true
               : false;
}

void EggLastMech::fn_802FAB88(int id) {
    if (id == -2) {
        fn_801FABE4(fn_80109D64());
    } else if (id != -1) {
        bool paused = false;
        if (fn_80109D64()->field_0x24 != 0) {
            paused = true;
            fn_801FAAA4(fn_80109D64());
        }
        switch (lbl_8057D6A0.field_0xA8.field_0x4) {
        case 37:
            fn_801FAF00(fn_80109D64(), id + 0x11E, 0, 0, 0);
            break;
        case 39:
            fn_801FAF00(fn_80109D64(), id + 0x149, 0, 0, 0);
            break;
        case 40:
            fn_801FAF00(fn_80109D64(), id + 0x15D, 0, 0, 0);
            break;
        }
        if (paused) {
            fn_801FAAB4(fn_80109D64());
        }
    }
}

} // namespace EggLastMech

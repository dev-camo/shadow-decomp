#ifndef GAME_EGGLASTMECH_H
#define GAME_EGGLASTMECH_H

#include "Game/CharaColliAttack.h"
#include "Game/EnemyCharInfo.h"
#include "Game/Task.h"
#include "Game/EnemyMotionEventArg.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "boost/enable_shared_from_this.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharCommand;
class Target;
class Unk801A4430;
class Unk802CD448;
class Unk80336A58;
class UnkGadgetOwner_800C9CD0;
struct Unk801A35ACParameters;

namespace SonicteamUSA {
namespace System {
namespace CharaColli {
class CharaColliBase;
} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

namespace EggLastMech {

class EggLastMech;

/* The resource key under which the gadget's load hook registers EggLastMechData.one; the boss
 * builds its model wrapper and its probe from resources under it (name inferred). */
static const int kResourceKey = 0x16;
/* The model frame whose position the boss's Target follows. fn_802FACE8's effect starts there, and
 * an attack callback whose part (param.field_0x14) has this frame id hurts the boss (name
 * inferred). */
static const int kTargetFrame = 0x36;

/* EggLastMech::EggLastMechAIStateBase (class name from RTTI): the base of the boss's AI states
 * (EggLastMech::AIState::*, defined in another file). Its vtable (0x8055BBC0, USA (GUPE8P)) has the
 * destructor and four null slots, which the states fill. Only those slots are declared here; their
 * names are inferred from the state changes and the update (UnkStateMachine_802FD008). */
class EggLastMechAIStateBase {
  public:
    virtual ~EggLastMechAIStateBase();
    /* Called with the boss and the state change's argument when the state is entered. The boss
     * passes numbers (0, 1 or a bit index of field_0x110), except that fn_802FCEDC passes
     * fn_80311060's state a reaction state's pointer converted to int. */
    virtual void Enter(EggLastMech *mech, int arg) = 0;
    /* Called with the boss on every update while the state is current. */
    virtual void Execute(EggLastMech *mech) = 0;
    /* Called with the boss when the state is left. */
    virtual void Exit(EggLastMech *mech) = 0;
    /* Returns a number; fn_802FCE60 compares it with 4. */
    virtual int GetId() = 0;
};
CHECK_SIZEOF(EggLastMechAIStateBase, 0x4);

/* The boss's current and previous AI state (placeholder name; the original class is unknown).
 * EggLastMech's Init allocates it with plain new and constructs it inline. */
class UnkStateMachine_802FD008 {
  public:
    UnkStateMachine_802FD008(EggLastMech *mech)
        : m_mech(mech), m_current(0), m_previous(0), m_lock(0), m_updating(false) {}
    /* EggLastMech's destructor keeps delete's own null test after its own one (USA (GUPE8P)
     * 0x802FD6A4), which MWCC emits only for a class with a declared destructor. */
    ~UnkStateMachine_802FD008() {}

    /* When m_lock is 0 or m_updating is set, and there is a current state: makes the current state
     * the previous one, calls its Exit, makes state current and calls its Enter with arg.
     * Otherwise nothing changes and the request is dropped (behaviour from its body). */
    void fn_802FD008(EggLastMechAIStateBase *state, int arg);

    /* 0x00 */ EggLastMech *m_mech;
    /* 0x04 */ EggLastMechAIStateBase *m_current;
    /* 0x08 */ EggLastMechAIStateBase *m_previous;
    /* fn_802FCFD0 counts it up and fn_802FCFBC down; while it is not 0, fn_802FD008 changes the
     * state only during the current state's Execute (m_updating set). */
    /* 0x0C */ int m_lock;
    /* 0x10 */ bool m_updating;
};
CHECK_SIZEOF(UnkStateMachine_802FD008, 0x14);

/* The five parts the boss owns (placeholder names after their constructors; the original classes
 * are not known). Each is constructed with the boss and deleted by the destructor; the sizes are
 * the ones Init allocates. Only the members that this file uses are declared; the rest of each
 * layout is not recovered. */
class Unk803142C0 {
  public:
    Unk803142C0(EggLastMech *mech);
    ~Unk803142C0();
    void fn_80313C10();
    void fn_80313CC0();
    int fn_80313F48();
    void fn_80314004(int arg0, int arg1, int arg2);

    /* 0x00 */ u8 field_0x0[0x4];
    /* fn_802FD384 picks a reaction for it when it is 0 or more. */
    /* 0x04 */ int field_0x4;
    /* 0x08 */ u8 field_0x8[0x18 - 0x8];
    /* The motion event 0x5000 with payload 2 passes it to fn_802FADB4 as the kind. */
    /* 0x18 */ int field_0x18;
    /* 0x1C */ u8 field_0x1C[0x6C - 0x1C];
};
CHECK_SIZEOF(Unk803142C0, 0x6C);

class Unk8032D6FC {
  public:
    Unk8032D6FC(EggLastMech *mech);
    ~Unk8032D6FC();

    /* 0x00 */ u8 field_0x0[0x60];
};
CHECK_SIZEOF(Unk8032D6FC, 0x60);

class Unk80313648 {
  public:
    Unk80313648(EggLastMech *mech);
    ~Unk80313648();

    /* 0x00 */ u8 field_0x0[0x60];
};
CHECK_SIZEOF(Unk80313648, 0x60);

class Unk80334C78 {
  public:
    Unk80334C78(EggLastMech *mech);
    ~Unk80334C78();
    void fn_80334A70();

    /* 0x00 */ u8 field_0x0[0x2C];
};
CHECK_SIZEOF(Unk80334C78, 0x2C);

class Unk80336F48 {
  public:
    Unk80336F48(EggLastMech *mech);
    ~Unk80336F48();
    void fn_80336B58(int id);
    void fn_80336D38();
    void fn_80336DF4(int arg0);

    /* 0x00 */ u8 field_0x0[0x54];
};
CHECK_SIZEOF(Unk80336F48, 0x54);

/* EggLastMech::EggLastMech (class name from RTTI, with EnemyCharInfo and
 * boost::enable_shared_from_this<EggLastMech::EggLastMech> as its bases): the character info and
 * logic of the boss that the gadget's task, EggLastMech::EggLastMechTask, creates. Method names are
 * placeholders unless marked "(name inferred)"; the behaviour descriptions are inferred from the
 * code. */
class EggLastMech : public EnemyCharInfo, public boost::enable_shared_from_this<EggLastMech> {
  public:
    EggLastMech(Task *task);
    virtual ~EggLastMech();

    /* Allocates the boss for task and returns the shared pointer that owns it (name inferred). */
    static boost::shared_ptr<EggLastMech> Create(Task *task);

    /* For id -2, calls fn_801FABE4 on the message manager (fn_80109D64); -1 does nothing. Any
     * other id is passed to fn_801FAF00 plus 0x11E, 0x149 or 0x15D for the stages 37, 39 and
     * 40 (the word at 0x4 of lbl_8057D6A0's record at 0xA8; other stages pass nothing), with
     * fn_801FAAA4 called before and fn_801FAAB4 after when the manager's word at 0x24 is set.
     * It takes no object. That it is a static member of the boss is inferred, and so is its place
     * in this file: it is the first function after the stage message file (Game/fn_802FA9C0.cpp),
     * and nothing in the code places it on one side of that boundary. Its callers outside this
     * file are at USA (GUPE8P) 0x80311BD0, 0x80312090 and 0x8032EC7C. */
    static void fn_802FAB88(int id);

    /* Reads the position and rotation from the owner, installs the command handler, builds the
     * model wrapper, the probe, the Target, the state machine and the parts, and enters
     * fn_80311C5C's state (name inferred). */
    void Init(UnkGadgetOwner_800C9CD0 *owner);
    void Update(float dt);

    bool fn_802FAC7C();
    void fn_802FACE8(int arg);
    void fn_802FADB4(int kind, int index);
    Fn8040Vector3 fn_802FB504();
    Fn8040Matrix34 *fn_802FB5C0(unsigned int nodeId);
    Unk801A4430 *fn_802FB5F4();
    void fn_802FB618();
    void fn_802FB644(int arg0);
    void fn_802FB670(EnemyMotionEventArg &event);
    void fn_802FB7BC(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void fn_802FB8DC(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void fn_802FBA24(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void fn_802FBA28(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void fn_802FBDE4();
    void fn_802FC574();
    void fn_802FCB10();
    void fn_802FCD20(CharCommand &command);
    bool fn_802FCE60();
    void fn_802FCEDC(int kind);
    void fn_802FCFBC();
    void fn_802FCFD0();
    void fn_802FCFE4(EggLastMechAIStateBase *state, int arg);
    void fn_802FD2F8();
    void fn_802FD384();

    /* Inline members whose out-of-line copies the original emits in this file (USA (GUPE8P)
     * 0x802FB1B8, 0x802FB430 and 0x802FCE30) and other files call: they call inline functions
     * (the weak_ptr copy and shared_from_this), so MWCC does not expand them. They return a copy of
     * m_player, a weak pointer to the boss and a shared pointer to the boss. */
    boost::weak_ptr<CharInfo> fn_802FB1B8() const {
        return m_player;
    }
    boost::weak_ptr<EggLastMech> fn_802FB430() {
        return shared_from_this();
    }
    boost::shared_ptr<EggLastMech> fn_802FCE30() {
        return shared_from_this();
    }

    /* 0xC4 */ UnkBitFlags_80014E04 field_0xC4;
    /* The model wrapper that fn_802CDB08 builds (name inferred). */
    /* 0xC8 */ boost::shared_ptr<Unk802CD448> m_model;
    /* 0xD0 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
    /* Created by fn_801E7128 with 0x800 and flag 1 (names inferred, as in the gadgets). */
    /* 0xD8 */ boost::shared_ptr<Target> m_target;
    /* The character info that fn_800A8B74 returns for lbl_805EE588 when the boss is built (name
     * inferred: fn_802FADB4 casts it to Player::PlayerBase). */
    /* 0xE0 */ boost::weak_ptr<CharInfo> m_player;
    /* 0xE8 */ UnkStateMachine_802FD008 *m_stateMachine;
    /* 0xEC */ Unk803142C0 *field_0xEC;
    /* 0xF0 */ Unk8032D6FC *field_0xF0;
    /* 0xF4 */ Unk80313648 *field_0xF4;
    /* 0xF8 */ Unk80334C78 *field_0xF8;
    /* 0xFC */ Unk80336F48 *field_0xFC;
    /* The task that created the boss (the constructor's argument); fn_80336888 creates its
     * object under it. */
    /* 0x100 */ Task *m_task;
    /* Set by the first update in which the health is below 150, which also passes 1 to
     * fn_802FAB88. */
    /* 0x104 */ bool field_0x104;
    /* Cleared on every update. */
    /* 0x105 */ bool field_0x105;
    /* Starts at 150; fn_802FB7BC subtracts the attack's field_0x50 and sets it to 0 when the
     * result is below 0; Update sets it to 0 when fn_802DFA00 returns true (name inferred). */
    /* 0x108 */ float m_health;
    /* 0x10C */ int field_0x10C;
    /* 0x110 */ UnkBitFlags_80014E04 field_0x110;
    /* Every update subtracts the time and sets it to 0 when the result is below 0;
     * fn_802FBA28 tests whether it is > 0. */
    /* 0x114 */ float field_0x114;
    /* Every update subtracts fn_8019FFE0's result from it (no lower limit). */
    /* 0x118 */ float field_0x118;
    /* 0x11C */ float field_0x11C;
    /* Not touched by this file; the constructor does not initialise it. */
    /* 0x120 */ u8 field_0x120[0x128 - 0x120];
};
CHECK_SIZEOF(EggLastMech, 0x128);

} // namespace EggLastMech

extern "C" {
/* Called by the boss but defined in files that have no header yet (placeholder names; the
 * descriptions come from their bodies, the parameter and return types are inferred).
 * fn_802DFA00 returns false. fn_8019A890 returns the resource that fn_8001277C gives for key and
 * index, or null, after storing 0x202 in the word at 0x8 of each of its 0x10-byte entries up to
 * the one whose word at 0x4 is 0; the boss passes it to fn_800C7F7C as the probe's collision data.
 * fn_800BE2DC and fn_800BE2EC store their argument in .sbss and return the previous value; the
 * boss passes its health divided by 150 to the first on every update, and true to the second from
 * Init and false from its destructor. fn_80356400 is passed a position, a direction and a size. */
bool fn_802DFA00(void);
void *fn_8019A890(int key, int index);
float fn_800BE2DC(float value);
bool fn_800BE2EC(bool value);
void fn_80356400(const Fn8040Vector3 &position, const Fn8040Vector3 &direction, float size);
/* Allocates a 0x40-byte object (constructor fn_80336A58) under parent with the parameters and the
 * four floats and returns it, null when the allocation fails (behaviour from its body). The boss
 * passes the parameters of its model's part 0 and (1, 0, 0, 1) when an attack hurts it. */
Unk80336A58 *fn_80336888(Task *parent, const Unk801A35ACParameters *parameters, float arg2,
                         float arg3, float arg4, float arg5);

/* Return AI states; defined in the AI state file (placeholder names, return types inferred from
 * the boss's use). */
EggLastMech::EggLastMechAIStateBase *fn_8031017C(void);
EggLastMech::EggLastMechAIStateBase *fn_80310314(void);
EggLastMech::EggLastMechAIStateBase *fn_803104C0(void);
EggLastMech::EggLastMechAIStateBase *fn_8031062C(void);
EggLastMech::EggLastMechAIStateBase *fn_803107FC(void);
EggLastMech::EggLastMechAIStateBase *fn_80310C9C(void);
EggLastMech::EggLastMechAIStateBase *fn_80310EA0(void);
EggLastMech::EggLastMechAIStateBase *fn_80311060(void);
EggLastMech::EggLastMechAIStateBase *fn_80311AE4(void);
EggLastMech::EggLastMechAIStateBase *fn_80311C5C(void);
EggLastMech::EggLastMechAIStateBase *fn_80312274(void);
EggLastMech::EggLastMechAIStateBase *fn_803123E8(void);
EggLastMech::EggLastMechAIStateBase *fn_803124BC(void);
}

#endif

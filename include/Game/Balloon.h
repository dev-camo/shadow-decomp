#ifndef GAME_BALLOON_H
#define GAME_BALLOON_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/VacuumObject.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class CharCommand;

namespace Balloon {

/* The values of the "type" parameter, named after the descriptor's value labels "straight",
 * "roundtrip" and "circle". */
enum { TYPE_STRAIGHT = 0, TYPE_ROUNDTRIP = 1, TYPE_CIRCLE = 2 };

/* The gadget parameters in the order of the BALLOON descriptor's table, whose entry names give the
 * member names. Only ReadOwner (called by the constructor and by ResetFromOwner) writes them, and
 * only when the owner's parameter block is 0x1C bytes (fn_800C9DBC compares the size at
 * 0x800C9DD4). */
struct BalloonParams {
    /* 0x00 */ s32 type;     /* TYPE_ values */
    /* 0x04 */ s32 item;     /* the item given (an index into the item labels; 0-7 in the table) */
    /* 0x08 */ float sec;    /* the duration of "straight", the period of the other types */
    /* 0x0C */ float radius; /* the radius of "circle" */
    /* "X", "Y" and "Z": the offset from the owner's position that "straight" moves to and
     * "roundtrip" moves to and back. */
    /* 0x10 */ float x;
    /* 0x14 */ float y;
    /* 0x18 */ float z;
};
CHECK_SIZEOF(BalloonParams, 0x1C);

/* Balloon::Balloon (class name from RTTI; the gadget's descriptor is "BALLOON"): the model 0xC2 on
 * a character info, with a Target, that gives an item to the character that touches it, attacks it
 * or pulls it in (behaviour inferred). ReadOwner and the constructor set m_state to STATE_START;
 * the first update after that starts the movement that "type" selects and resets m_timer to 0. In
 * the three moving states each update adds dt to m_timer first. A "straight" balloon
 * (STATE_STRAIGHT) sits at the owner's position plus m_velocity times m_timer and goes to
 * STATE_DONE in the update in which m_timer becomes > "sec" (the position is not changed in that
 * update; a NaN never compares greater). A "roundtrip" balloon (STATE_ROUNDTRIP) goes from the
 * owner's position to the owner's position plus X, Y, Z and back, and a "circle" balloon
 * (STATE_CIRCLE, for every "type" other than TYPE_STRAIGHT and TYPE_ROUNDTRIP) circles the
 * owner's position at "radius" about the y axis; both reset m_timer to 0 when it becomes > "sec".
 * STATE_DONE passes the owner to fn_800C9B34 when there is one (fn_8015A794) and sets bit 0 of the
 * task flags. OnCollision, OnAttack and OnCommand set STATE_DONE when the balloon is used (see
 * each). Method and member names are inferred from the code. */
class Balloon : public Gadget {
  public:
    /* m_state values (names inferred from what each state does). */
    enum State { STATE_START, STATE_STRAIGHT, STATE_ROUNDTRIP, STATE_CIRCLE, STATE_DONE };

    Balloon(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Balloon();
    /* Advances the model by dt when there is one, acts on m_state (see the class comment) and then
     * calls UpdateModels. The model update comes first and runs the model's collision callbacks,
     * so when OnCollision sets STATE_DONE there, the state switch of the same update acts on it. */
    virtual void UpdateBody(float dt);
    /* Calls ReadOwner and UpdateModels; then, when there is a model, calls its fn_8016108C and
     * queues a debug shape: for TYPE_CIRCLE a cylinder at the character info's position, turned 90
     * degrees about x, with half length 5 and the "radius" parameter, and for any other "type" a
     * line from the character info's position to that position plus X, Y, Z. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11; nothing is constructed when the allocation fails. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info, copies the owner's
     * parameter block into m_params when its size is 0x1C (see BalloonParams) and sets m_state to
     * STATE_START. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When there is a model, gives it the identity matrix moved to the character info's position
     * and turned about y by the character info's y rotation; then, when there is a Target, moves it
     * to the character info's position. */
    void UpdateModels();
    /* The model's collision callback: whatever m_state is, passes the hits to fn_8015AA84 with the
     * give-item callback (fn_800E8438) and a context holding the character info's position and
     * the "item" parameter. fn_8015AA84 calls the callback for each character info it takes from
     * the hits whose collision object and character info still exist and whose fn_80078B4C
     * returns true; the first such call processes the item request and sets the context's done,
     * which makes the later calls do nothing. When done is set, m_state becomes STATE_DONE. */
    void OnCollision(ColliDetectArgs &args);
    /* The attack callback of the model's attack object "col". In the three moving states: when
     * the attack object exists and its word at 0x40 has neither bit 0x40 nor bit 0x200 set, the
     * parameter's word at 0x54 becomes 5 and nothing else happens. Otherwise that word becomes 3;
     * then, when the attack object and its owner still exist, the owner is passed to the give-item
     * callback with the character info's position and the "item" parameter; and m_state becomes
     * STATE_DONE whether or not that happened. In every other state the word at 0x54 becomes 5. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The character info's command handler. Only command 0x14 (a VacuumCommand) is handled: when
     * there is a model, it calls the model's fn_80161880, creates a VacuumBalloon with the command,
     * a copy of the model pointer and the "item" parameter at the character info's position and
     * rotation (fn_800E63E8) and resets m_model; then, with or without a model, m_state becomes
     * STATE_DONE. */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<Target> m_target;
    /* 0x48 */ BalloonParams m_params;
    /* 0x64 */ s32 m_state; /* a State */
    /* For "straight", set by the first update in STATE_START: the unit vector of X, Y, Z (the zero
     * vector when its length is below 1e-6) times the length of X, Y, Z divided by "sec", or times
     * that length itself when "sec" is 0. */
    /* 0x68 */ Fn8040Vector3 m_velocity;
    /* 0x74 */ float m_timer;
    /* For "roundtrip" and "circle", set by the first update in STATE_START: 360 divided by "sec" (0
     * when "sec" is 0), in degrees per second. */
    /* 0x78 */ float m_angularSpeed;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(Balloon, 0x80);

/* Balloon::VacuumBalloon (class name from RTTI, with VacuumObject as its only base): the balloon's
 * model while a character pulls it in (inferred from the names). Update places the model, when
 * there is one, at the object's position and rotation, shrunk as the pull progresses, and
 * fn_80238180 gives the item to the character info that pulls it in (behaviour inferred). */
class VacuumBalloon : public VacuumObject {
  public:
    VacuumBalloon(VacuumCommand *command, boost::shared_ptr<GadgetModelInfo> model, int item);
    virtual ~VacuumBalloon();
    /* Advances the base object; then, when the model is not null, places the model at the
     * object's position (fn_802382B0) turned by its rotation (fn_80238248: y, then x, then z) and
     * scaled on all three axes by 1 - (1 - e) * t, where e is lbl_805F72B0 / 15 (computed at
     * startup) and t is fn_8023800C clamped to [0, 1] (fn_80070460), and advances the model by
     * dt. */
    virtual void Update(float dt);
    /* Locks fn_80238058's weak pointer and, when it is not null, passes that character info and a
     * context with the object's position (fn_802382B0) and m_item to the give-item callback. */
    virtual void fn_80238180();

    /* 0x08 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x10 */ s32 m_item;
};
CHECK_SIZEOF(VacuumBalloon, 0x14);

} // namespace Balloon

#endif

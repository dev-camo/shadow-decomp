#ifndef GAME_ELEVATOR_H
#define GAME_ELEVATOR_H

#include "Game/CharCommand.h"
#include "Game/CrashInfoCtrl.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class ColliDetectArgs;
class UnkResource_80012700;

namespace Elevator {

/* The gadget's parameters, in the order of the descriptor's table (names from its strings). The
 * constructor stores 0 in all of them; the table gives "distance" 10, "speed" 0.5 and "range" 100
 * as their first floats (their defaults, inferred). */
struct ElevatorParams {
    ElevatorParams() : model(0), distance(0.0f), speed(0.0f), range(0.0f) {}

    /* 0x0 */ s32 model;      /* the id of the model in the model file's resource */
    /* 0x4 */ float distance; /* the signed y offset of the destination from the start */
    /* 0x8 */ float speed;    /* 30 * speed is the movement per second */
    /* 0xC */ float range;    /* the distance at which the nearest character counts as away */
};
CHECK_SIZEOF(ElevatorParams, 0x10);

/* Elevator::Elevator (class name from RTTI; the gadget's descriptor is "ELEVATOR"): a platform of
 * ELEVATOR.BIN that moves along y between its start and a destination "distance" from it
 * (behaviour inferred):
 * - In STATE_START (set by ReadOwner, which the constructor and ResetFromOwner call) the next
 *   update enters STATE_WAITING, to wait for the switch of its com id, when the id is > 0,
 *   otherwise STATE_AT_START.
 * - In STATE_AT_START or STATE_TO_START, command 11 enters STATE_TO_DESTINATION.
 * - In STATE_TO_DESTINATION and STATE_AT_DESTINATION, an update in which the nearest character is
 *   "range" or more away horizontally (IsNearestAway) enters STATE_TO_START and ends there without
 *   moving; the movement back to the start begins on a later update.
 * Method, member and constant names are inferred, and so are the parameters' pointer, reference
 * and const spellings. */
class Elevator : public Gadget {
  public:
    /* The values of m_state (names inferred from what each state does). */
    enum State {
        STATE_START = 0,
        STATE_WAITING = 1,        /* until the com id's switch reports 1 */
        STATE_AT_START = 2,       /* at the start */
        STATE_AT_DESTINATION = 3, /* at the destination */
        STATE_TO_START = 4,       /* moving back to the start */
        STATE_TO_DESTINATION = 5  /* moving to the destination */
    };

    /* The bits of m_hitFlags (name inferred). */
    enum Flag {
        /* Set by OnCollision for a hit for which fn_8015AC84 is true; cleared at the start of
         * every update, before the model and the target object are advanced and the state runs.
         * STATE_WAITING enters STATE_TO_DESTINATION instead of STATE_AT_START when it is set as
         * the switch reports 1. */
        FLAG_TOUCHED = 0
    };

    /* Builds the character info with its command unit and handler (OnCommand), reads the owner
     * (ReadOwner), loads the model and the model file's resource; when GadgetBuilder::fn_80208BB8
     * rejects the model id it passes the gadget to fn_8015A794 and sets Task flag 0x01, otherwise
     * it sets up the model (SetupModel) and calls UpdateModelMatrix. */
    Elevator(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Elevator();
    /* Clears FLAG_TOUCHED, advances the model and the target object by dt (each when there is one)
     * and runs the state (see the class comment). When there is a model and no target object yet,
     * STATE_START also tries to create the target object (fn_80166AF4 gives a null pointer when its
     * allocation fails) and passes fn_80169D00 and a weak pointer to it to the model's
     * fn_8015FA78; then it calls the target's fn_80167010(-1) when there is a target object.
     * STATE_TO_DESTINATION changes the character info's y by dt * 30 * "speed", subtracting it
     * when "distance" < 0 and adding it otherwise; STATE_TO_START changes it the other way. When y
     * goes past the target y (strictly: < it while subtracting, > it while adding; the target is
     * start y + "distance" or the start y; NaN never passes), it is set to it, the state
     * becomes STATE_AT_DESTINATION or STATE_AT_START and m_endSound is passed to fn_801D54F4;
     * otherwise m_loopSound is passed to fn_801D53C4. Both then call UpdateModelMatrix. */
    virtual void UpdateBody(float dt);
    /* Rereads the owner (ReadOwner) and places the model; without a model it returns there. When
     * the "model" parameter has changed, it releases the target object and the model
     * (ReleaseModel) and returns when GadgetBuilder::fn_80208BB8 rejects the new id; otherwise it
     * sets the model up again. Then it calls the debug helpers fn_802E0B58 with the segment from
     * the character info's position to that position with "distance" added to y, and fn_802E0C60
     * at the position with flag 0, half length 10, radius "range" and the character info's
     * rotation with 90 degrees added to its x component (each queues its new shape, or a null
     * pointer when the shape's allocation fails), and calls the model's fn_8016108C. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 when its allocation succeeds. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation (converted to radians) into the character info,
     * the owner's parameters into m_params only when their size is sizeof(ElevatorParams), 0x10
     * (fn_800C9DBC), and its com id into m_comId (fn_800C9E50); enters STATE_START and keeps the
     * position in m_start. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When there is a model, places it at the character info's position, turned about y by its
     * rotation. */
    void UpdateModelMatrix();
    /* Whether the nearest character info (fn_800A8A7C) still exists and is "range" or more away
     * from the character info, measured without the y components (the comparison of the squared
     * distance with the square of "range" is true for equality). False when there is none. */
    bool IsNearestAway();
    /* The character info's command handler: command 11 enters STATE_TO_DESTINATION from
     * STATE_AT_START or STATE_TO_START. Other commands and states are ignored. */
    void OnCommand(CharCommand &command);
    /* Gets the model's crash info for the "model" parameter (GadgetBuilder::fn_80208978) and
     * passes its fn_802087D8 name to the model's fn_801629D8, which returns false for a null name
     * or one that none of the model's entries has. When the crash info's fn_802087BC returns a
     * node name, it passes the name and a weak pointer to the character info to the model's
     * fn_80161F08 (false when the pointer has expired or no node has the name) and installs
     * OnCollision as the model's collision callback; neither result is tested, so the callback is
     * installed whatever fn_80161F08 returns. Sets m_endSound and m_loopSound to 0xFFFF and, when
     * the crash info's fn_80208768 returns a name, to the low 16 bits of the word at 0x8 of the
     * resource records "<name>_end" and "<name>_loop" (fn_80208888), each only when its record
     * exists. fn_801D53C4 and fn_801D54F4 return without playing an id of 0xFFFF. */
    void SetupModel();
    /* Calls the model's fn_80161880 and fn_80162804 without testing the model pointer and
     * releases the crash info. */
    void ReleaseModel();
    /* The model's collision callback: sets FLAG_TOUCHED when fn_8015AC84 is true for the hits. */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<GadgetBuilder::CrashInfoCtrl> m_crashInfo;
    /* 0x48 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x50 */ UnkResource_80012700 *m_resource; /* the model file's resource */
    /* The "model" parameter at construction or at the last accepted reload (never read here). */
    /* 0x54 */ s32 m_loadedModelIndex;
    /* 0x58 */ ElevatorParams m_params;
    /* 0x68 */ s32 m_state;                     /* a State */
    /* 0x6C */ Fn8040Vector3 m_start;           /* the position ReadOwner read */
    /* 0x78 */ s32 m_comId;                     /* the owner's com id (fn_800C9E50) */
    /* 0x7C */ u16 m_endSound;                  /* passed to fn_801D54F4 on arrival */
    /* 0x7E */ u16 m_loopSound;                 /* passed to fn_801D53C4 while moving */
    /* 0x80 */ UnkBitFlags_80014E04 m_hitFlags; /* Flag bits */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(Elevator, 0x88);

} // namespace Elevator

#endif

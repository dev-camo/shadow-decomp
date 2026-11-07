#ifndef GAME_FOOTINGMOVABLE_H
#define GAME_FOOTINGMOVABLE_H

#include "Game/CharCommand.h"
#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/PathCtrl.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace FootingMovable {

/* The gadget's parameters, in the order of the descriptor's table (names from its strings). The
 * constructor stores 0 in all of them. The meaning of "spd", "acc", "pos" and the three params
 * depends on "type" (see the labels at the end of FootingMovable.cpp and the class comment). */
struct FootingMovableParams {
    FootingMovableParams()
        : model(0), type(0), spd(0.0f), acc(0.0f), pause(0.0f), pos(0.0f), param0(0.0f),
          param1(0.0f), param2(0.0f), pauseDamage(0.0f) {}

    /* 0x00 */ s32 model;
    /* 0 to 6: "normal", "path", "loop path", "one way", "normal(sync)", "path(sync)" and
     * "normal(pause)" (the table's labels). */
    /* 0x04 */ s32 type;
    /* 0x08 */ float spd;
    /* 0x0C */ float acc;
    /* 0x10 */ float pause;       /* "pause(sec)" for type 6 */
    /* 0x14 */ float pos;         /* "Init Pos(0.0-1.0)" for types 4 and 5 */
    /* 0x18 */ float param0;      /* "X" or "path id" */
    /* 0x1C */ float param1;      /* "Y" */
    /* 0x20 */ float param2;      /* "Z" */
    /* 0x24 */ float pauseDamage; /* "pause damage" */
};
CHECK_SIZEOF(FootingMovableParams, 0x28);

/* FootingMovable::FootingMovable (class name from RTTI; the gadget's descriptor is "MOVE_FOOTING"):
 * a footing of FOOTINGMOVABLE.BIN that moves by its "type" (behaviour inferred):
 * - Types 0, 4 and 6 ("normal") place it on the line from its start to the start plus the offset
 *   (X, Y, Z), at half the offset's length times 1 - cos of an angle in degrees: at the start for
 *   0 and 360, at the offset for 180. The direction (for types 0 and 4 computed as (start +
 *   offset) - start) goes through fn_8040CEE8, which returns the zero vector for a length below
 *   1e-6; such a footing stays at its start at every angle. Type 0 adds "acc" to the angle at
 *   every update, type 4 derives it from a sync time, type 6 adds dt * 20 * "acc" and pauses at
 *   180 and 360.
 * - Types 1 and 5 ("path") place it along the path "path id" (fn_8001C91C) at the same half cosine
 *   of half the value fn_800183D8 returns for the path (its length, inferred). Type 2 ("loop
 *   path") places it along the path at a distance that grows by "acc" * dt per update and loses
 *   the value fn_800183D8 returns whenever fn_8001C91C reports the end.
 * - Type 3 ("one way") places it at the start plus a fraction of the offset. The fraction starts
 *   at 0; every update adds 0.001 * "spd" * dt to it and sets it to 1 when it is then above 1
 *   (there is no lower limit). With "spd" 0, the default, the footing stays at its start.
 * For types 0 and 1, "spd" only sets the start: their start states (type 1's once fn_8001C7B4
 * finds the path) set the angle to the word at 0x90 of lbl_8057D6A0 modulo the rounded "spd",
 * or to 0 unless "spd" >= 0.5 (so also for NaN) (purpose unknown).
 * Type 3 waits for the switch of its com id, types 0 and 6 do when the id is > 0. While "pause
 * damage" is > 0, some attacks on the model's attack object stop it for that many seconds. Only
 * ResetFromOwner binds that attack callback (after a change of the "model" parameter); the
 * constructor never does, so a gadget that is only constructed ignores attacks.
 * Method, member and constant names are inferred, and so are the parameters' pointer, reference
 * and const spellings. */
class FootingMovable : public Gadget {
  public:
    /* The values of m_state (names inferred from what each state does). */
    enum State {
        STATE_WAIT_SWITCH = 0,         /* until the com id's switch reports 1 */
        STATE_NORMAL_START = 1,        /* type 0 */
        STATE_PATH_START = 2,          /* type 1 */
        STATE_LOOP_PATH_START = 3,     /* type 2 */
        STATE_NORMAL = 4,              /* type 0 */
        STATE_PATH = 5,                /* type 1 */
        STATE_LOOP_PATH = 6,           /* type 2 */
        STATE_DAMAGE_PAUSE = 7,        /* stopped by an attack for "pause damage" seconds */
        STATE_ONE_WAY = 9,             /* type 3 */
        STATE_NORMAL_SYNC_START = 10,  /* type 4 */
        STATE_NORMAL_SYNC = 11,        /* type 4 */
        STATE_PATH_SYNC_START = 12,    /* type 5 */
        STATE_PATH_SYNC = 13,          /* type 5 */
        STATE_NORMAL_PAUSE_START = 14, /* type 6 */
        STATE_NORMAL_PAUSE = 15        /* type 6 */
    };

    /* The bits of m_flags (names inferred from their set and clear points). ReadOwner clears all
     * of them, FLAG_SOUND and FLAG_SOUND_START included. */
    enum Flag {
        /* Set by ReadOwner when "pause damage" is > 0 (otherwise cleared) and when a damage pause
         * ends; cleared when an attack starts one. OnAttack acts only while it is set. */
        FLAG_DAMAGE_PAUSE = 0,
        /* Set by MovePause when the angle passes 180 (with FLAG_TO_END) or 360 (with
         * FLAG_TO_START), together with m_timer = "pause". While it is set, STATE_NORMAL_PAUSE
         * counts m_timer down by dt instead of moving and clears it once m_timer is <= 0. */
        FLAG_PAUSE = 1,
        /* Set by STATE_NORMAL_PAUSE_START (after its first MovePause) and by MovePause when the
         * angle passes 360; cleared by MovePause when the angle passes 180. */
        FLAG_TO_END = 2,
        /* Set by MovePause when the angle passes 180, cleared when it passes 360. */
        FLAG_TO_START = 3,
        /* Set by the constructor when the model's object at 0x30 is not null and fn_80208848
         * finds a record for the model id in it; the sound update runs only while it is set. */
        FLAG_SOUND = 4,
        /* Set with FLAG_SOUND; the first sound update clears it instead of comparing positions. */
        FLAG_SOUND_START = 5
    };

    /* Builds the character info with its command unit and handler (OnCommand), reads the owner,
     * loads the model, selects the "model" entry (fn_80162A68) and attaches the character info to
     * the part with that id (fn_80161FC8). Clears FLAG_SOUND and FLAG_SOUND_START; when the
     * model's object at 0x30 is not null and has a record for that id (fn_80208848), keeps the
     * low 16 bits of the record's word at 0x8 as m_moveSound and sets both flags. */
    FootingMovable(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    /* Stops m_sound. */
    virtual ~FootingMovable();
    /* Advances the model and the target object by dt when they exist, runs the state, then the
     * sound update (UpdateSound). */
    virtual void UpdateBody(float dt);
    /* Rereads the owner; without a model it returns there. When the "model" parameter differs
     * from m_modelIndex, it clamps the parameter to the model's entry count (first to at least 0,
     * then to at most fn_8015FF44 - 1, so -1 when there are none) as the new m_modelIndex,
     * releases the target object, calls the model's fn_80161880 and fn_80162804,
     * selects the entry and attaches the character info to its part as the constructor does,
     * binds OnAttack to the part's attack object (fn_80161428) only when that still exists, and
     * writes the clamped value back to the owner (fn_800C9D68). Then it places the model, queues a
     * debug line from the character info's position to that position plus (param0, param1,
     * param2) and calls the model's fn_8016108C. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 when its allocation succeeds. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation (converted to radians) into the character info,
     * its parameters into m_params only when their size is sizeof(FootingMovableParams), 0x28
     * (fn_800C9DBC), and its com id into m_comId (fn_800C9E50). Clears m_angle, all flags and the
     * target object, sets FLAG_DAMAGE_PAUSE by "pause damage" and picks the first state by
     * "type" (STATE_WAIT_SWITCH for type 3 and values outside 0 to 6, and for types 0 and 6 when
     * the com id is > 0). Keeps the position in m_start, sets m_angleOffset to 180 - 180 * "pos"
     * and m_syncTime to 0. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When there is a model, places it at the character info's position, turned by its
     * rotation. */
    void UpdateModelMatrix();
    /* When there is no target object but a model, attempts to create the target object
     * (fn_80166AF4 returns an empty pointer when its allocation fails) and passes a weak pointer to
     * it, possibly empty, to the model's parts (fn_8015FA78 with fn_80169D00); then calls the
     * target object's fn_80167010(-1) when there is one. */
    void SetupTarget();
    /* The character info's command handler: a SetTransparentCommand (id 0x17) with m_enable set
     * passes m_alpha to fn_8015FDD8 and, for up to 16 ranges of the selected model entry's probe
     * (when it still exists), keeps each UnkSlot10 word in m_probeFlags and stores it back with
     * bit 0x100 set; with m_enable clear it calls fn_8015FD10 and stores the kept words back.
     * Both need a model. Every command gets bit 1 (0x2) of its flags. */
    void OnCommand(CharCommand &command);
    /* The attack callback of the model entry's attack object: while FLAG_DAMAGE_PAUSE is set, an
     * attack whose attack object still exists and has a bit of 0x140 set in its word at 0x40
     * enters STATE_DAMAGE_PAUSE for "pause damage" seconds (keeping the state in
     * m_resumeState), clears FLAG_DAMAGE_PAUSE and gets the result 1. Any other attack gets 5.
     * Only ResetFromOwner binds it (after a change of the "model" parameter); the constructor
     * never does, so a gadget that is only constructed ignores attacks. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* While FLAG_SOUND is set: the first call (FLAG_SOUND_START) stops m_sound, keeps the position
     * and clears FLAG_SOUND_START. Later calls set the character info's velocity (field_0x18) to
     * the zero vector when fn_8000A950 accepts dt as 0, otherwise to the movement since the last
     * call divided by dt. When its length is above 0.5, they request m_moveSound on the character
     * info's position (fn_801D3C38) into m_sound unless m_sound's owner word is set; otherwise
     * they stop m_sound when its owner word is set. Then they keep the position. */
    void UpdateSound(float dt);

    /* The states' updates (see State); each takes the update's time step. */
    void UpdateWaitSwitch(float dt);
    void UpdateNormalStart(float dt);
    void UpdatePathStart(float dt);
    void UpdateLoopPathStart(float dt);
    void UpdateNormal(float dt);
    void UpdatePath(float dt);
    void UpdateLoopPath(float dt);
    void UpdateOneWay(float dt);
    void UpdateNormalSyncStart(float dt);
    void UpdateNormalSync(float dt);
    void UpdatePathSyncStart(float dt);
    void UpdatePathSync(float dt);
    void UpdateNormalPauseStart(float dt);
    void UpdateNormalPause(float dt);
    /* Sets m_angle to 0.05 * ("spd" * (20 * time)) + m_angleOffset and places the character info
     * on the line from m_start to m_start + (param0, param1, param2) for it (see the class
     * comment). */
    void MoveNormalSync(float time);
    /* Adds dt * (20 * "acc") to m_angle. When the angle is then above 180 with FLAG_TO_END set,
     * or above 360 with FLAG_TO_START set, it sets the angle to 180 or 0, swaps the two flags,
     * sets FLAG_PAUSE and sets m_timer to "pause". Then it places the character info on the line
     * from m_start to m_start + (param0, param1, param2) for the angle. */
    void MovePause(float dt);

    static const char *s_name;

    /* 0x30 */ s32 m_type;       /* "type" as ReadOwner read it */
    /* 0x34 */ s32 m_modelIndex; /* the model entry in use */
    /* 0x38 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x40 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x48 */ FootingMovableParams m_params;
    /* 0x70 */ s32 m_state;         /* a State */
    /* 0x74 */ s32 m_resumeState;   /* the state a damage pause returns to */
    /* 0x78 */ float m_angleOffset; /* 180 - 180 * "pos" (degrees) */
    /* The angle in degrees (normal and path types), the distance along the path (loop path) or
     * the fraction of the offset (one way). */
    /* 0x7C */ float m_angle;
    /* 0x80 */ float m_timer; /* the remaining pause, in seconds */
    /* 0x84 */ SonicteamUSA::System::PathCtrl m_path;
    /* 0xDC */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0xE4 */ Fn8040Vector3 m_start; /* the position ReadOwner read */
    /* 0xF0 */ s32 m_comId;           /* the owner's com id (fn_800C9E50) */
    /* 0xF4 */ int m_probeFlags[16];  /* the probe words that OnCommand keeps */
    /* 0x134 */ u16 m_moveSound;      /* the sound id that UpdateSound requests */
    /* 0x138 */ UnkSoundHandle_801D3DC8 m_sound;
    /* 0x154 */ Fn8040Vector3 m_lastPosition; /* the position at the last sound update */
    /* The time that the sync types add the float that the character info's field_0x60 points to
     * at every update; it starts at the float at 0x94 of lbl_8057D6A0. */
    /* 0x160 */ float m_syncTime;
    /* 0x164 */ UnkBitFlags_80014E04 m_flags; /* Flag bits */
};
CHECK_SIZEOF(FootingMovable, 0x168);

} // namespace FootingMovable

#endif

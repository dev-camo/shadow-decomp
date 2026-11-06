#ifndef GAME_UPDOWNREEL_H
#define GAME_UPDOWNREEL_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharCommand;

namespace UpDownReel {

/* The gadget parameters in the order of the descriptor's table (names from the table's strings). */
struct UpDownReelParams {
    /* Stands for the four float zero stores at 0x80128978-0x8012898C in the gadget's constructor
     * (inferred). */
    UpDownReelParams() : length(0.0f), pos(0.0f), angle(0.0f), power(0.0f) {}

    /* 0x00 */ float length; /* the rope length while nobody hangs on the handle */
    /* 0x04 */ float pos;    /* the rope length that a ride moves to */
    /* 0x08 */ float angle;  /* degrees; tilts the launch direction about the x axis */
    /* 0x0C */ float power;  /* the speed of the launch */
};
CHECK_SIZEOF(UpDownReelParams, 0x10);

/* UpDownReel::UpDownReel (class name from RTTI; the gadget's descriptor is "UD_REEL"): a reel with
 * a rope and a handle that a character hangs on. Behaviour inferred from the code:
 * - While neither RIDE_RIDDEN nor RIDE_TOUCHED is set, the collision callback of the probe at the
 *   handle passes its hits to fn_8015AE44; if fn_8015AE44 called its callback at least once, the
 *   last character the callback stored is sent a Player::TouchHangCommand (when it still exists),
 *   RIDE_TOUCHED is set, RIDE_RELEASED is cleared and the rider is forgotten.
 * - A RideCommand (id 11) in STATE_WAITING records its rider and sets RIDE_RIDDEN; when "length"
 *   minus "pos" is above 0 it starts STATE_SHORTENING, below 0 STATE_LENGTHENING, each with sound
 *   0x1000; otherwise the state stays. The two states move the rope length by 0.18 per update
 *   until it reaches "pos", then set it to "pos", switch to STATE_ARRIVED and stop the sound.
 * - While RIDE_RIDDEN is set and RIDE_RELEASED is not, each update sends the rider, when it still
 *   exists, a TouchHangCommand with the hang position and angles.
 * - Input commands (id 1) turn the handle's y angle towards the input's yaw relative to the reel,
 *   by at most 2 degrees each; updates without RIDE_RIDDEN turn it back towards 0 by at most 4
 *   degrees.
 * - Command id 12 sets RIDE_RELEASED and STATE_WAITING and stops the sound. The next update that
 *   finds both RIDE_RIDDEN and RIDE_RELEASED set calls Launch unless RIDE_NO_LAUNCH (command id
 *   15) is set, then clears RIDE_RIDDEN and RIDE_RELEASED, forgets the rider and suspends the
 *   probe for 1.0 (when there is a handle). Launch acts only while the rider still exists, and a
 *   RideCommand accepted before that update clears RIDE_RELEASED again.
 * Method and member names marked "(name inferred)" or described here are not original, and the
 * parameters' pointer, reference and const spellings are inferred. */
class UpDownReel : public Gadget {
  public:
    UpDownReel(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~UpDownReel();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info. Copies the owner's
     * parameters only when the owner's parameter size equals sizeof(UpDownReelParams), 0x10
     * (fn_800C9DBC); otherwise m_params keeps its previous values. Then sets the rope length to
     * "length", the state to STATE_WAITING, clears the ride flags and zeroes the handle's rotation
     * (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places each model that exists; when part 1 exists, also the probe (when there is a handle),
     * the line and the hang position (name inferred). */
    void UpdateModelMatrix();
    /* When the rider still exists, sends it a ClrVelocityCommand, a SetVelocityCommand with the
     * launch velocity ((0, 0, 1) turned by "angle" about x and by the reel's yaw, times "power")
     * and a Player::LaunchCommand (name inferred). */
    void Launch();
    /* The character info's command handler (name inferred). */
    void OnCommand(CharCommand &command);
    /* The probe's collision callback (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    /* The values of m_state (names inferred). STATE_WAITING moves the rope length back to "length"
     * by 0.18 per update (when "length" minus "pos" is > 0 or < 0); STATE_SHORTENING subtracts
     * 0.18 per update until the length is <= "pos", STATE_LENGTHENING adds 0.18 until it is >
     * "pos"; STATE_ARRIVED stays until command id 12 or ReadOwner sets STATE_WAITING. */
    enum State {
        STATE_WAITING = 0,
        STATE_SHORTENING = 1,
        STATE_LENGTHENING = 2,
        STATE_ARRIVED = 3
    };

    /* The bits of m_rideFlags (names inferred from where they are set and cleared). ReadOwner
     * clears all of them. */
    enum RideFlag {
        /* Set by a RideCommand in STATE_WAITING; cleared after a release. */
        RIDE_RIDDEN = 0,
        /* Set by command id 12; cleared by a RideCommand in STATE_WAITING, by the probe callback
         * when fn_8015AE44 called its callback and after a release. */
        RIDE_RELEASED = 1,
        /* Set by command id 15; cleared by a RideCommand in STATE_WAITING. While it is set, a
         * release does not launch the rider. */
        RIDE_NO_LAUNCH = 2,
        /* Set by the probe callback when fn_8015AE44 called its callback; cleared at the start of
         * every update. */
        RIDE_TOUCHED = 3
    };

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* Three models of resource 0xE5 showing its parts 0, 1 and 2: the reel (at the character info,
     * turned by its yaw), the handle (10 * m_ropeLength below the character info, turned by its
     * yaw and by m_handleRotation.y) and the rope (the matrix left by the part 0 step, or the base
     * matrix when there is no part 0 model, scaled along y by m_ropeLength). */
    /* 0x38 */ GadgetModelInfo *m_models[3];
    /* 0x44 */ UpDownReelParams m_params;
    /* 0x54 */ s32 m_state;
    /* 0x58 */ float m_ropeLength;
    /* 0x5C */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* The line across the handle, from (10, -14, 0) to (-10, -14, 0) in the handle's space. */
    /* 0x64 */ boost::shared_ptr<UnkSegment_80020A20> m_line;
    /* The middle of m_line: the position in the TouchHangCommands. */
    /* 0x6C */ Fn8040Vector3 m_hangPosition;
    /* Only the y angle changes: input commands turn it towards the input's direction, every
     * update without RIDE_RIDDEN turns it back towards 0 and ReadOwner zeroes it. */
    /* 0x78 */ UnkRotation3 m_handleRotation;
    /* The angles in the TouchHangCommands, set by each update that sends one: m_handleRotation
     * with pi and the character info's yaw added to y. */
    /* 0x84 */ UnkRotation3 m_hangAngles;
    /* 0x90 */ boost::weak_ptr<CharInfo> m_rider; /* from the last accepted RideCommand */
    /* 0x98 */ UnkBitFlags_80014E04 m_rideFlags;
    /* 0x9C */ UnkSoundHandle_801D3DC8 m_sound;
};
CHECK_SIZEOF(UpDownReel, 0xB8);

} // namespace UpDownReel

#endif

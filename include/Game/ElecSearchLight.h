#ifndef GAME_ELECSEARCHLIGHT_H
#define GAME_ELECSEARCHLIGHT_H

#include "Game/CharCommand.h"
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "Game/fn_8040_vector_types.h"
#include "rw/rwcore.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class ColliDetectArgs;

/* The gadget parameters in the order of the ELEC_LIGHT descriptor's table, whose entry names give
 * the member names. fn_800C9DBC reads them from the owner. */
struct ElecSearchLightParams {
    /* 0x00 */ s32 type;          /* 0 "NORMAL" or 1 "ENEMY_ID" (the table's value labels) */
    /* 0x04 */ float rotateRange; /* degrees */
    /* 0x08 */ float rotateSpeed; /* degrees per unit of the timer */
    /* 0x0C */ float lightLength; /* the beam is 10 * lightLength long */
};
CHECK_SIZEOF(ElecSearchLightParams, 0x10);

/* ElecSearchLight (class name from RTTI; the gadget's descriptor is "ELEC_LIGHT"): a search light
 * of the electric circuit area that shows LIGHT.BIN and draws its beam as a cone of 26 vertices.
 * A "NORMAL" light sweeps its beam from side to side and catches the nearest character info
 * when it lies inside the cone, or within 50 units behind the light or 25 units beyond the
 * beam's end (the test takes the radius from the fraction along the beam, and points outside the
 * beam get the fraction of the other end; see Search). An "ENEMY_ID" light waits until its com
 * id's switch turns on, then picks the character info nearest to its beam and turns towards it
 * until it points at it. A caught
 * character info is taken out of control and pulled towards the light; once it is close the light
 * lets it go with a jump and breaks (an effect and a sound). From three seconds later on it starts
 * to fade back in once the nearest character info is more than 100 units away, and stays broken
 * while there is none (behaviour inferred); see FadeIn for which parts reappear. Method and
 * member names are inferred. */
class ElecSearchLight : public Gadget {
  public:
    ElecSearchLight(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecSearchLight();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation, parameters and com id into the gadget. */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info, turned by its rotation and then by the beam's yaw
     * and pitch, moves the beam's end point and rebuilds the cone. */
    void UpdateModelMatrix();
    /* Sets the cone's positions and normals from the model matrix and its colours from the
     * brightness. */
    void UpdateCone(const Fn8040Matrix34 *matrix);
    /* Advances the timer by the global step at 0x28 of lbl_8057D6A0 and sets the yaw to the sine
     * of the timer times the rotation speed, scaled by the rotation range. */
    void Sweep();
    /* Catches the character info nearest to the beam when its distance from the beam is within a
     * radius that grows from 25 at the light to 50 at the beam's end. Outside the segment the
     * fractions that GetSegmentDistance gives make the radius 50 for a point behind the light and
     * 25 for a point beyond the beam's end. */
    void Search();
    /* Turns the beam towards m_target by at most s_turnSpeed * dt per angle and catches the
     * target once the beam points at it. */
    void TurnToTarget(float dt);
    /* Takes m_target out of control and places it at position, facing the beam (state 2). */
    void Catch(const Fn8040Vector3 *position);
    /* Stores the angles of the beam's direction (the model rotation turned half a turn about y). */
    void GetBeamAngles(Fn8040Vector3 *angles);
    /* Pulls m_target 20 units towards the light; within 50 units it lets the target go with a
     * jump and enters state 3. */
    void PullTarget();
    /* Starts the break effect of model entry 4, when there is one, and plays sound 0x7231. */
    void StartBreakEffect();
    /* Fades the beam's materials back in over 1.5 seconds and returns to state 0. The parts are
     * shown when the timer crosses 0.666667 (part 0) or 1.0 (parts 1 and 2, and part 3 hidden), but
     * the two crossings are tested as if / else if: when one update crosses both, only part 0 is
     * shown, and parts 1 and 2 stay hidden from the break. */
    void FadeIn(float dt);
    /* Returns the squared distance from point to the segment, storing the closest point and its
     * fraction along the segment; a point before the segment's start gets the fraction 1 and a
     * point beyond its end the fraction 0. Either output may be null. */
    float GetSegmentDistance(const Fn8040Vector3 *point, const UnkSegment_80020A20 *segment,
                             Fn8040Vector3 *closest, float *fraction);
    /* The character info's command handler; it ignores every command. */
    void OnCommand(CharCommand &command);
    /* The model's collision callback: passes the hits to a callback that ignores them. */
    void OnCollision(ColliDetectArgs &args);
    /* The draw layer's callback: draws the cone as a triangle strip with vertex alpha, additive
     * blending and without culling, depth writes or fog. */
    void OnDraw(RednerDrawArgs &args);

    static const char *s_name;

    /* 0x030 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x038 */ GadgetModelInfo *m_model;
    /* 0x03C */ ElecSearchLightParams m_params;
    /* 0x04C */ u8 m_comId; /* the owner's com id (fn_800C9E60) */
    /* The com id's switch state from fn_8016904C at the previous update; an "ENEMY_ID" light
     * searches when it changes to 1. */
    /* 0x050 */ s32 m_switchState;
    /* 0 searching, 1 turning to the target, 2 pulling it, 3 letting it go, 4 broken and 5 fading
     * in (inferred). */
    /* 0x054 */ s32 m_state;
    /* 0x058 */ float m_pitch; /* the beam's angles relative to the gadget (radians) */
    /* 0x05C */ float m_yaw;
    /* 0x060 */ float m_timer;
    /* 0x064 */ Fn8040Vector3 m_beamEnd;
    /* 0x070 */ boost::shared_ptr<CharInfo> m_target;
    /* 0x078 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* Twelve pairs of a vertex near the light and one at the beam's end, around the beam, and a
     * copy of the first pair that closes the strip. */
    /* 0x080 */ RwIm3DVertex m_cone[26];
    /* 0x428 */ float m_brightness; /* scales the cone's colours; 0 while broken */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(ElecSearchLight, 0x430);

#endif

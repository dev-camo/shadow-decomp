#ifndef GAME_SCATTERINGRING_H
#define GAME_SCATTERINGRING_H

#include "Game/CharaColliBase.h"
#include "Game/GadgetCharInfo.h"
#include "Game/Target.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "rw/rpworld.h"
#include "layout_check.h"

class CharCommand;
class ColliDetectArgs;

/* The five constants at the start of ScatteringRing.cpp's .sdata2 (placeholder names). Code
 * elsewhere reads them by address, for example: the constructor at 0x80357B24 stores
 * lbl_805F3CA8 (10) in place of a record count that is not positive; 0x80357BF4 stores
 * lbl_805F3CB0 (5) to its stack frame before the same draw calls as ScatteringRing::OnDraw, where
 * OnDraw stores its sphere radius; 0x80357D04 sets bit 1 of its flags when its float at 0x68 is
 * <= lbl_805F3CB8 (1.5), and adds lbl_805F3CAC (1.5) times lbl_805F3CB4 (60) times its float
 * argument, converted from degrees to radians, to its float at 0x5C. This file uses lbl_805F3CA8,
 * lbl_805F3CB0 and lbl_805F3CB8 in the same positions (Create's count, OnDraw's radius, Update's
 * bit 1 test). */
extern const s32 lbl_805F3CA8;
extern const float lbl_805F3CAC;
extern const float lbl_805F3CB0;
extern const float lbl_805F3CB4;
extern const float lbl_805F3CB8;

namespace ScatteringRing {

/* What ScatteringRing::Create scatters the rings from (placeholder name; the record has an empty
 * out-of-line constructor at 0x800B7E40, which the callers at 0x800B7E1C and 0x8033E8F0 call
 * before they store the position and the count). Only the members Create reads are declared; the
 * full size of the record is not known. */
struct UnkScatterRecord_800B7E40 {
    /* 0x00 */ Fn8040Vector3 position;
    /* 0x0C */ s32 num; /* the number of rings; 10 when it is not positive */
};
/* The size of the mapped view only; the full record may be larger. */
CHECK_SIZEOF(UnkScatterRecord_800B7E40, 0x10);

/* The state of one scattered ring, which Create fills in and the constructor copies (placeholder
 * name; constructor 0x8011B170, assignment 0x8011B0F0). */
struct UnkRingParams_8011B170 {
    UnkRingParams_8011B170() : rotation(0.0f, 0.0f, 0.0f) {}

    /* 0x00 */ Fn8040Vector3 position;
    /* Added to the position in each update that gets past the hit and time checks (see Update). */
    /* 0x0C */ Fn8040Vector3 velocity;
    /* y is the angle the ring is drawn at (Update adds to it); Create zeroes all three components
     * and OnCommand passes a copy of all three to fn_80238A78. */
    /* 0x18 */ UnkRotation3 rotation;
    /* 0x24 */ float field_0x24; /* Create stores 0; not read here */
    /* Once bit 0 of the flags is set, counted down by dt in each update that gets past the hit
     * check; the ring finishes when it is <= 0 (Create stores 5). */
    /* 0x28 */ float time;
    /* 5 plus GetGroundHeight's result under the ring, so 5 when the downward test finds no
     * qualifying hit and GetGroundHeight returns 0 (Create stores 6). */
    /* 0x2C */ float floor;
    /* 0x30 */ RpClump *clump; /* the clump of resource 0x7C, drawn at the ring's position */
    /* Passed to fn_80238A78 as its last argument by OnCommand, and the hit callback passes the
     * character info to fn_8020642C only when it is true. */
    /* 0x34 */ bool field_0x34;
};
CHECK_SIZEOF(UnkRingParams_8011B170, 0x38);

/* ScatteringRing::ScatteringRing (class name from RTTI): a ring that Create throws out with
 * others in a circle, which falls, bounces on the ground under it, can be collected and finishes
 * after a while (behaviour inferred). Method names marked "(name inferred)" are not original.
 *
 * Bit 0 of m_flags is set when the ring first bounces; from then on Update tests the probe's hits
 * and counts m_param.time down (OnCommand does not test it). Update tests m_param.time against
 * 1.5 before it subtracts dt. After the hit check, an update that finds bit 1 clear sets it when
 * m_param.time <= 1.5 (a NaN time never is); one that finds bit 1 already set toggles bit 2
 * instead. OnDraw draws nothing while bit 2 is set. The constructor clears all bits. */
class ScatteringRing : public Task {
  public:
    /* Builds the Task under parent, sets the task name to s_name, copies param, builds the
     * character info (fn_8015CD90(0x26, 4)) and gives it a new command unit whose handler is
     * OnCommand (when that allocation fails, the handler is stored through the null pointer),
     * creates the draw layer "ScatteringRing" with OnDraw as its callback, a probe sphere of radius
     * 5 at the ring's position (fn_800C7EA8 with 0x40000, 0xA, 0, an empty owner and
     * lbl_805E07A0) and a Target for the character info (fn_801E7128 with 0x180), clears the flags
     * and sets m_time to 10. The reference spelling of param is inferred. */
    ScatteringRing(Task *parent, const UnkRingParams_8011B170 &param);
    /* Releases the Target (fn_801E7108) and, when there is one, the probe (fn_800C78B0). */
    virtual ~ScatteringRing();
    /* In order, within one update:
     * - Once bit 0 of m_flags is set: when the probe exists and has hits (a list whose count at 0x4
     *   is > 0) and IsTouched returns true, tries to play sound 0xE00B (fn_801D5300), passes the
     *   position and 0x8A to fn_8015D794 (an effect, inferred; the returned weak pointer is
     *   discarded), sets Task flag 0x01 and returns.
     * - Toggles bit 2 when bit 1 is set; otherwise sets bit 1 when m_param.time, before this
     *   update's subtraction, is <= 1.5 (a NaN time is not).
     * - Once bit 0 is set, subtracts dt from m_param.time; when it is then <= 0 (a NaN time is
     *   not), sets Task flag 0x01 and returns.
     * - Subtracts dt from m_time; when it is then <= 0 (a NaN time is not), sets Task flag 0x01 and
     *   returns.
     * - Sets m_param.floor to 5 plus GetGroundHeight of the position (0 without a qualifying
     *   hit), damps the velocity, pulls its y down and adds it to the position (fn_8040CFF8).
     *   When the position's y is then < the floor and the velocity's y is < 0 (both false for
     *   NaN), sets the position's y to the floor, slows x and z, reverses y (bouncing back
     *   slightly faster) and sets bit 0.
     * - Adds 1.5 degrees (in radians, not scaled by dt) to the y rotation, and moves the probe
     *   (SetPosition) and the Target (fn_800CCE38) to the position when they exist. */
    virtual void Update(float dt);

    /* Creates record.num rings (10 when it is not positive) under task layer 11, at the record's
     * position. They share the clump of resource 0x7C; ring i moves off with the velocity
     * (0, 5.109375, 1.15625) turned about the y axis by a random angle plus (i + 1) times 360
     * degrees divided by the count. Each gets time 5, floor 6, field_0x24 0 and arg1 as
     * field_0x34; a ring is constructed only when its allocation succeeds. Then requests sound
     * 0xE00C (fn_801D560C with 0 and 0), whose result is not used (behaviour inferred). The name,
     * the reference spelling of record, the bool type of arg1 (stored as a byte) and the void
     * return (neither caller uses r3) are inferred: the callers pass the address of a local record
     * and 0 or 1. */
    static void Create(const UnkScatterRecord_800B7E40 &record, bool arg1);

    /* Tests a segment from the position 10000 units straight down (fn_80021FE4 with 1) and
     * returns the y of the hit point (fn_8001E9BC) of the first node in the returned list that
     * has an object at 0x0C and a hit at 0x14; 0 when there is no list or no such node. Deletes
     * the list either way (name inferred; the function does not use the object it is called on;
     * the reference spelling is inferred). */
    float GetGroundHeight(const Fn8040Vector3 &position);
    /* Passes args to fn_8015B004 with the hit callback; true when the callback ran for a
     * character info whose fn_80078B2C (tested by fn_8015B004) and fn_80078B34 both returned
     * true (name inferred). */
    bool IsTouched(ColliDetectArgs &args);
    /* The character info's command handler: for a command with the id 0x14 it passes the
     * command, the position, a copy of the rotation and field_0x34 to fn_80238A78, sets Task flag
     * 0x01 and sets bit 1 of the command's flags; other ids are ignored (name inferred). */
    void OnCommand(CharCommand &command);
    /* The draw layer's callback: unless bit 2 of m_flags is set, draws the clump at the position,
     * turned by the y rotation, when there is a clump and a sphere of radius 5 there is in the
     * camera's view (name inferred). */
    void OnDraw(RednerDrawArgs &args);

    /* The class name the constructor stores in m_name (name inferred). */
    static const char *s_name;

    /* 0x28 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0x30 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
    /* 0x38 */ boost::shared_ptr<Target> m_target;
    /* 0x40 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x48 */ UnkRingParams_8011B170 m_param;
    /* 0x80 */ float m_time; /* the time left before the ring finishes in any case */
    /* 0x84 */ UnkBitFlags_80014E04 m_flags; /* hides Task::m_flags */
};
CHECK_SIZEOF(ScatteringRing, 0x88);

} // namespace ScatteringRing

#endif

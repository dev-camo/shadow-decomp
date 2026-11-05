#ifndef GAME_ELECBAR_H
#define GAME_ELECBAR_H

#include <set>
#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class ElecBar;

/* The gadget parameters in the order of the descriptor's table (names from the table's strings;
 * the first entry has an empty name and labels its values "type< Vertical   >" and
 * "type< Horizontal >"). */
struct ElecBarParams {
    /* 0x00 */ s32 type; /* 0 vertical: the bars move up by "offset"; 1 horizontal: along -z */
    /* 0x04 */ float scale;
    /* 0x08 */ float offset;
    /* 0x0C */ float movingSecond;
    /* 0x10 */ float hidingSecond;
    /* 0x14 */ float appearingRatio;
    /* 0x18 */ float synchronize;
    /* 0x1C */ s32 numberOfBars;
};
CHECK_SIZEOF(ElecBarParams, 0x20);

/* ElecBarSub (class name from the RTTI of its boost::bind callbacks): one electric bar of an
 * ElecBar gadget: a BAR.BIN model, a character info whose command handler is OnCommand, a probe
 * whose collision callback is OnCollision, and a line between the bar's ends. Each update the bar
 * takes its place in the gadget's cycle of m_period seconds, shifted by its phase and the
 * "synchronize" parameter: it fades in its materials while it moves out during the first part of
 * "moving second", stays lit while it moves on, fades out near the end of the move (when the
 * distance still to go is within "offset" times the appearing ratio, so only for a positive
 * offset) and hides for "hiding second". Hiding sets a flag in its handle that suspends hit
 * processing until the bar is lit again; the probe itself keeps being placed every update
 * (behaviour inferred). Method and member names are inferred. */
class ElecBarSub {
  public:
    ElecBarSub(ElecBar *parent);
    ~ElecBarSub();

    /* Advances the bar's cycle and updates its materials, probe and placement (name inferred). */
    void Update(float dt);
    /* Copies the gadget's position and rotation into the character info (name inferred). */
    void CopyFromParent();
    /* Places the model, the line and the probe at the character info, moved by m_offset up or
     * back by the gadget's type and scaled by "scale" along x (name inferred). */
    void UpdateModelMatrix();
    /* The character info's command handler: a RideCommand (id 11) stores its rider in m_rider,
     * command id 12 releases it (name inferred). */
    void OnCommand(CharCommand &command);
    /* The probe's collision callback: passes each character info it hits to the hit callback with
     * the bar's character info and line (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    /* 0x00 */ ElecBar *m_parent;
    /* 0x04 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x0C */ GadgetModelInfo *m_model;
    /* 0x10 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* The line between the bar's ends. The shared pointer's RTTI names its object
     * Sonicteam::System::Geometry::Line; this code uses the segment type it constructs. */
    /* 0x18 */ boost::shared_ptr<UnkSegment_80020A20> m_line;
    /* 0x20 */ boost::weak_ptr<CharInfo> m_rider; /* from the last RideCommand */
    /* 0 fading in, 1 lit, 2 hidden, 3 after construction (inferred). */
    /* 0x28 */ s32 m_state;
    /* 0x2C */ float m_offset; /* the distance the bar has moved */
    /* 0x30 */ float m_time;   /* the time in the gadget's cycle */
    /* 0x34 */ float m_phase;  /* the bar's share of the cycle: its index / "number of bars" */

  private:
    ElecBarSub(const ElecBarSub &);
    ElecBarSub &operator=(const ElecBarSub &);
};
CHECK_SIZEOF(ElecBarSub, 0x38);

/* ElecBar (class name from RTTI; the gadget's descriptor is "ELEC_BAR"): a row of "number of bars"
 * electric bars (ElecBarSub) that move out by "offset" in "moving second" and hide for "hiding
 * second", in turn (behaviour inferred). Method and member names are inferred. */
class ElecBar : public Gadget {
  public:
    ElecBar(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecBar();
    virtual void UpdateBody(float dt);
    virtual void fn_800CBC1C(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameters and derives the bar timing from the
     * parameters (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
        fn_800C9F14(owner, &m_position);
        fn_800C9ED4(owner, &m_rotation);
        fn_800C9DBC(owner, &m_params, sizeof(m_params));
        if (m_params.movingSecond != 0.0f) {
            m_speed = m_params.offset / m_params.movingSecond;
        } else {
            m_speed = 0.0f;
        }
        m_fadeTime = m_params.movingSecond * m_params.appearingRatio;
        m_period = m_params.movingSecond + m_params.hidingSecond;
    }
    /* Creates m_barCount bars with the phases 0, 1 / "number of bars", ... (name inferred). */
    void CreateBars();
    /* Deletes the bars and empties the set (name inferred). */
    void DestroyBars();

    static const char *s_name;

    /* 0x30 */ ElecBarParams m_params;
    /* 0x50 */ std::set<ElecBarSub *> m_bars;
    /* 0x5C */ float m_fadeTime; /* "moving second" times "appearing ratio" */
    /* 0x60 */ float m_speed;    /* "offset" / "moving second", or 0 */
    /* 0x64 */ float m_period;   /* "moving second" plus "hiding second" */
    /* 0x68 */ s32 m_barCount;   /* the number of bars created */
    /* 0x6C */ Fn8040Vector3 m_position;
    /* 0x78 */ Fn8040Vector3 m_rotation;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(ElecBar, 0x88);

#endif

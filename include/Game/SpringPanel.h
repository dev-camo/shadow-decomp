#ifndef GAME_SPRINGPANEL_H
#define GAME_SPRINGPANEL_H

#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table (names from the table's strings).
 * fn_800C9DBC copies them from the owner only when the owner's parameter size is 8 (compare at
 * 0x800C9DD4, USA (GUPE8P)). The constructor stores no defaults, so they stay uninitialised when
 * that first copy is skipped. */
struct SpringPanelParams {
    /* 0x0 */ float level;         /* selects the launch speed; parameter range 0 to 15 */
    /* 0x4 */ float noControlTime; /* seconds the character cannot be controlled */
};
CHECK_SIZEOF(SpringPanelParams, 0x8);

/* SpringPanel (class name from RTTI; the gadget's descriptor is "ELEC_SPRING"): a spring of the
 * SPRINGPANEL.BIN model. For each character info that fn_8015AA84 takes from the hits passed to the
 * model's collision callback, and only when the panel's character info has not expired (checked
 * for each character), the hit callback sends the character commands that take its control for a
 * time, place it at m_position, give it the panel's y rotation and launch it along the panel's y
 * axis turned by its x and z rotations, with a speed that the parameter "level" selects. The
 * colour of its "mat" part cycles with the fractional part of a global float (behaviour inferred).
 * Command ids 0x1A and 0x1B sent to its character info clear and set bit 0 of the Target's flag
 * word (meaning unknown). Member names, the names of the non-virtual methods and the parameters'
 * pointer and reference spellings are inferred. */
class SpringPanel : public Gadget {
  public:
    SpringPanel(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SpringPanel();
    virtual void UpdateBody(float dt);
    /* Updates the colour of the "mat" part; the argument is not used (behaviour inferred). */
    virtual void fn_800CBC1C(float);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and its rotation (converted to radians) into the character
     * info, and the owner's parameters into m_params only when the owner's parameter size is 8
     * (fn_800C9DBC); otherwise m_params keeps its values (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* When the model pointer is non-null: places the model at the character info's position,
     * turned by its x and z rotations, sets m_position to (0, 20, 0) in that frame and, when the
     * Target pointer is non-null, places the Target there. Without a model nothing is updated
     * (name inferred). */
    void UpdateModelMatrix();
    /* Sets the colour of the model's "mat" part from t, the float at 0x94 of lbl_8057D6A0 minus
     * its integer part (truncated toward zero): below 0.5 it fades from s_cycleColor0 to
     * s_cycleColor1, otherwise below 0.8333333f from s_cycleColor1 to s_cycleColor2, otherwise
     * below 1 from s_cycleColor2 back to s_cycleColor0. Any t below 0.5 takes the first branch: a
     * negative non-integer float gives a negative t, a negative integer gives 0, and a float below
     * the int range converts to the lowest int (0x80159380), which leaves t negative. A t of 1 or
     * more (a float above the int range) or a NaN takes none and leaves the colour unchanged.
     * Neither the model pointer nor the part's state is tested for null (name inferred). */
    void UpdateColor();
    /* The command handler of the character info: command id 0x1A clears bit 0 of the Target's
     * flag word and 0x1B sets it (name inferred; the meaning of the ids is unknown). */
    void OnCommand(CharCommand &command);
    /* The model's collision callback: passes the hits to fn_8015AA84 with the hit callback, then
     * calls the model's fn_80161308 with 1.0 when the hit callback launched a character and the
     * model pointer is non-null (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ Fn8040Vector3 m_position; /* (0, 20, 0) in the model's frame */
    /* 0x3C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x44 */ GadgetModelInfo *m_model;
    /* 0x48 */ SpringPanelParams m_params;
    /* 0x50 */ boost::shared_ptr<Target> m_object;
};
CHECK_SIZEOF(SpringPanel, 0x58);

#endif

#ifndef GAME_RESEARCHER_H
#define GAME_RESEARCHER_H

#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* ResearcherCharInfo (class name from RTTI, with CharInfo and GadgetCharInfo as its bases): the
 * character info that the Researcher gadget creates for itself. It adds no members; it only
 * overrides fn_80073634. */
class ResearcherCharInfo : public GadgetCharInfo {
  public:
    ResearcherCharInfo(int arg0) : GadgetCharInfo(arg0) {}

    /* Returns true, where CharInfo's implementation returns false (the meaning is unknown). */
    virtual bool fn_80073634() {
        return true;
    }
};
CHECK_SIZEOF(ResearcherCharInfo, 0x6C);

/* The word that the gadget parameter "Type 0:FaceUp 1:FaceDown" fills. */
struct ResearcherParams {
    /* 0x0 */ u32 type; /* 0 face up, 1 face down (from the parameter's label) */
};
CHECK_SIZEOF(ResearcherParams, 0x4);

/* Researcher (class name from RTTI; the gadget's descriptor is "Researcher"): the RESEARCHER.BIN
 * model, set up for the type, with a second object (fn_802B4254) above it. When its character
 * info receives command id 16 while the gadget waits, it calls UnkSlot7(1, 0) on the actor that
 * fn_800740E4 returns, releases the second object, requests motion 2 of the type from the model
 * and plays sound 0x7634; the gadget then waits half a unit of dt, fades the model out over two
 * units and finishes half a unit later (behaviour inferred). Method names marked "(name
 * inferred)" are not original; all member names are inferred. */
class Researcher : public Gadget {
  public:
    Researcher(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Researcher();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameters and clears m_timer (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Sets up the model's entries for the type, passes the character info to the model together
     * with the type, places the model, creates m_target and passes it to the model's part callback
     * fn_80169D00 (name inferred). */
    void SetupModel();
    /* Places the model at the character info's position, turned by its y rotation, and the object
     * at 0x48 at that position (name inferred). */
    void UpdateModelMatrix();
    /* The character info's command handler (name inferred; the meaning of the ids is unknown). */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x48 */ boost::shared_ptr<Target> m_object;
    /* 0x50 */ boost::shared_ptr<UnkObject_802B4254> m_object2;
    /* 0x58 */ ResearcherParams m_params;
    /* 0 waiting for the command, 1 after it, 2 fading out, 3 faded out (inferred). */
    /* 0x5C */ s32 m_state;
    /* 0x60 */ float m_timer; /* counts down by dt in states 1 to 3 */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(Researcher, 0x68);

#endif

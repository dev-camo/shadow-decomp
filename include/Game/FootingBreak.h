#ifndef GAME_FOOTINGBREAK_H
#define GAME_FOOTINGBREAK_H

#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table (names from the table's strings). */
struct FootingBreakParams {
    /* 0x00 */ u32 type;          /* "Type 0:Burst 1:Hover" */
    /* 0x04 */ float waitSec;     /* "WaitSec" */
    /* 0x08 */ float hoverLength; /* "HoverLength point" */
    /* 0x0C */ float hoverSec;    /* "HoverSec" */
    /* 0x10 */ float moveLengthX; /* "MoveLengthX point" */
    /* 0x14 */ float moveLengthY; /* "MoveLengthY point" */
    /* 0x18 */ float moveLengthZ; /* "MoveLengthZ point" */
    /* 0x1C */ float moveSec;     /* "MoveSec" */
};
CHECK_SIZEOF(FootingBreakParams, 0x20);

/* FootingBreak (class name from RTTI; the gadget's descriptor has the same name). Member names and
 * the names of the non-virtual methods are inferred from the code. The gadget keeps two character
 * infos and two instances of FOOTINGBREAK.BIN: the first pair moves by the parameter "MoveLength"
 * and the second pair hovers where the gadget starts (behaviour inferred). */
class FootingBreak : public Gadget {
  public:
    FootingBreak(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~FootingBreak();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation, com id and parameters from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Place each model at its character info's position and rotation (names inferred). */
    void UpdateModelMatrix();
    void UpdateModel2Matrix();
    /* Leaves the current state and enters state (name inferred). */
    void SetState(int state);
    /* Takes down the model parts of the previous state and sets up those of state (name
     * inferred). */
    void SetupModels(int previousState, int state);
    /* The command handlers of the two character infos (names inferred): a SetTransparentCommand
     * sets the transparency of the matching model. */
    void OnCommand(CharCommand &command);
    void OnCommand2(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ FootingBreakParams m_params;
    /* 0x50 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x58 */ boost::shared_ptr<GadgetCharInfo> m_charInfo2;
    /* 0x60 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x68 */ boost::shared_ptr<GadgetModelInfo> m_model2;
    /* 0x70 */ float m_alpha;  /* the last value of a SetTransparentCommand for m_model */
    /* 0x74 */ float m_alpha2; /* the same for m_model2 */
    /* 0x78 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* State 1 hovers until the com id's state becomes 1, 2 waits "WaitSec", 3 waits two seconds,
     * 4 moves the first part, then 5 (type 0) waits a second before 6, or 7 (type 1) hovers both
     * parts (inferred). */
    /* 0x80 */ s32 m_state;
    /* 0x84 */ float m_timer;
    /* 0x88 */ Fn8040Vector3 m_moveStart;
    /* 0x94 */ Fn8040Vector3 m_moveDelta;
    /* 0xA0 */ float m_hoverTime;
    /* 0xA4 */ Fn8040Vector3 m_hoverBase;
    /* 0xB0 */ float m_hoverTime2;
    /* 0xB4 */ Fn8040Vector3 m_hoverBase2;
    /* 0xC0 */ u8 m_comId;
    /* 0xC4 */ UnkSoundHandle_801D3DC8 m_sound;
};
CHECK_SIZEOF(FootingBreak, 0xE0);

#endif

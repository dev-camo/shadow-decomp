#ifndef GAME_FOOTINGROLL_H
#define GAME_FOOTINGROLL_H

#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table (names from the table's strings).
 * ReadOwner converts the two angles from degrees to radians. */
struct FootingRollParams {
    /* 0x00 */ u32 model;          /* "Model 0:Small 1:Large" */
    /* 0x04 */ float moveLengthX;  /* "MoveLengthX point" */
    /* 0x08 */ float moveLengthY;  /* "MoveLengthY point" */
    /* 0x0C */ float moveLengthZ;  /* "MoveLengthZ point" */
    /* 0x10 */ float moveSec;      /* "MoveSec" */
    /* 0x14 */ float movePauseSec; /* "MovePauseSec" */
    /* 0x18 */ s32 rotType;        /* "RotType 0:OneWay 1:RT" */
    /* 0x1C */ s32 rotAxis;        /* "RotAxis 0:X 1:Y 2:Z" */
    /* 0x20 */ float rotSpd;       /* "RotSpd deg/sec" */
    /* 0x24 */ float rotMax;       /* "RotMax(RT Only) deg" */
};
CHECK_SIZEOF(FootingRollParams, 0x28);

/* FootingRoll (class name from RTTI; the gadget's descriptor has the same name). Member names and
 * the names of the non-virtual methods are inferred from the code. Once the state of its com id
 * becomes 1 (or at once without a com id), the footing moves back and forth by "MoveLength",
 * pausing at each end, while it turns about one of its axes, either all the way round (RotType 0)
 * or to and fro within "RotMax" (RotType 1) (behaviour inferred). */
class FootingRoll : public Gadget {
  public:
    FootingRoll(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~FootingRoll();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation, parameters and com id from the owner and restarts the
     * movement and the turn (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Sets the character info's rotation to the owner's rotation turned by m_angle about the axis
     * "RotAxis", then places the model at the character info's position with that rotation (name
     * inferred). */
    void UpdateModelMatrix();
    /* The command handler of the character info (name inferred): for a SetTransparentCommand it
     * passes the alpha to fn_8015FDD8 when it is below 1 and calls fn_8015FD10 otherwise. */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ FootingRollParams m_params;
    /* 0x58 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x60 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0 waits for the com id's state to become 1, 1 moves and turns (inferred). */
    /* 0x68 */ s32 m_state;
    /* 0 moves towards the other end, 1 pauses there for "MovePauseSec" (inferred). */
    /* 0x6C */ s32 m_movePhase;
    /* 0x70 */ float m_moveTimer;
    /* 0x74 */ s32 m_moveBack; /* nonzero while moving back to the owner's position (inferred) */
    /* 0x78 */ s32 m_turnBack; /* nonzero while turning back (RotType 1; inferred) */
    /* 0x7C */ float m_angle;  /* radians about the axis "RotAxis" */
    /* 0x80 */ u8 m_comId;
};
CHECK_SIZEOF(FootingRoll, 0x88);

#endif

#ifndef GAME_CATCHSTICK_H
#define GAME_CATCHSTICK_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class CharCommand;

/* CatchStick (class name from RTTI; the gadget's descriptor is "CATCHSTICK"): a character info
 * whose command handler does nothing, a probe made by fn_800C78D4 with half of the "length"
 * parameter and 2 as its two floats, and a line of length "length" through the character info's
 * position along its own z axis. The probe's collision callback sends the players that its hits
 * pass on a Player::TouchLineCommand with the gadget's character info and line unless they are
 * more than 12 from the line (see the hit callback in CatchStick.cpp; behaviour inferred).
 * Method and member names are inferred. */
class CatchStick : public Gadget {
  public:
    CatchStick(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~CatchStick();
    /* Runs the handle target's update (fn_8015BE9C) with dt; there is no null test. */
    virtual void UpdateBody(float dt);
    /* Re-reads the owner and places the line; when there is a handle target, also passes the
     * probe's SetSize 2, -2, 0 and half of m_length, passes the target to fn_8015BE64 and draws
     * the line with fn_802E0B58 and the colour lbl_805F06D8. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and its rotation, converted from degrees to radians
     * (fn_800C9ED4), into the character info, and the owner's parameters into m_length when the
     * owner's parameter size is 4 (fn_800C9DBC; otherwise m_length keeps its value). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Sets the line's ends to the points (0, 0, 1) and (0, 0, -1) transformed by the matrix of
     * the character info's position, its y, x and z rotations and the scales 0, 0 and half of
     * m_length, so the line runs half of m_length to either side of the position. */
    void UpdateLine();
    /* The character info's command handler; it does nothing. */
    void OnCommand(CharCommand &command);
    /* The probe's collision callback: passes the hits to fn_8015AE44 with the hit callback of
     * CatchStick.cpp and a context holding weak pointers to the character info and the line.
     * fn_8015AE44 calls the hit callback only for hits whose collision object and character info
     * still lock and whose character info's fn_80078B44 returns true. */
    void OnCollision(GadgetColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ float m_length; /* the "length" parameter */
    /* 0x3C */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0x44 */ boost::shared_ptr<UnkSegment_80020A20> m_line;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(CatchStick, 0x50);

#endif

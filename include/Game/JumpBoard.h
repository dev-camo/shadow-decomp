#ifndef GAME_JUMPBOARD_H
#define GAME_JUMPBOARD_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_801695B8.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table. */
struct JumpBoardParams {
    JumpBoardParams() : level(0.0f), angle(0.0f), noControlTime(0.0f) {}

    /* 0x0 */ float level;         /* selects the launch speed */
    /* 0x4 */ float angle;         /* degrees */
    /* 0x8 */ float noControlTime; /* seconds the character cannot be controlled */
};
CHECK_SIZEOF(JumpBoardParams, 0xC);

namespace JumpBoard {

/* JumpBoard::JumpBoard (class name from RTTI; the gadget's descriptor is "JUMP_BOARD"): a board
 * that launches a character that touches its "col" node along a direction turned by the gadget's
 * rotation and angle, with a speed that the gadget's level selects (behaviour inferred). Member
 * and method names are inferred. */
class JumpBoard : public Gadget {
  public:
    JumpBoard(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~JumpBoard();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads position, rotation and parameters from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position with its rotation (name inferred). */
    void UpdateModelMatrix();
    /* The model's collision callback (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x48 */ JumpBoardParams m_params;
    /* Unclassified storage in the original 0x58-byte allocation; no accesses are identified. */
    /* 0x54 */ unsigned char field_0x54[4];
};
CHECK_SIZEOF(JumpBoard, 0x58);

} // namespace JumpBoard

#endif

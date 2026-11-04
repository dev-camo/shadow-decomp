#ifndef GAME_CLOSEWALL_H
#define GAME_CLOSEWALL_H

#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace CloseWall {

/* The gadget parameters in the order of the descriptor's table (names from the table's strings).
 * The constructor stores the table's default values. */
struct SetCloseWallParams {
    SetCloseWallParams() : width(40.0f), speed(4.0f) {}

    /* 0x0 */ float width; /* twice the distance of each wall from the position at the start */
    /* 0x4 */ float speed; /* m_offset shrinks by speed times dt at each update while closing */
};
CHECK_SIZEOF(SetCloseWallParams, 0x8);

/* CloseWall::SetCloseWall (class name from RTTI; the gadget's descriptor is "CLOSE WALL"). Member
 * names and the names of the non-virtual methods are inferred from the code. */
class SetCloseWall : public Gadget {
  public:
    SetCloseWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetCloseWall();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and parameters from the owner, returns to state 0 with the
     * walls half the width away from the position, and places both character infos there with the
     * gadget's rotation (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places each model at its character info's position, turned by its y rotation (name
     * inferred). */
    void UpdateModelMatrices();
    /* The command handler of both character infos: command id 11 starts the closing once the
     * gadget is waiting (name inferred; the meaning of the id is unknown). */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo[2];
    /* 0x40 */ boost::shared_ptr<GadgetModelInfo> m_model[2];
    /* 0x50 */ SetCloseWallParams m_params;
    /* 0 on creation, 1 waiting for the command, 2 closing, 3 closed (inferred). */
    /* 0x58 */ s32 m_state;
    /* 0x5C */ Fn8040Vector3 m_position;
    /* 0x68 */ Fn8040Vector3 m_rotation;
    /* 0x74 */ float m_offset; /* the distance of each wall from m_position */
};
CHECK_SIZEOF(SetCloseWall, 0x78);

} // namespace CloseWall

#endif

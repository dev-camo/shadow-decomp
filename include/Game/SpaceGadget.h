#ifndef GAME_SPACEGADGET_H
#define GAME_SPACEGADGET_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/CharaColliAttack.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table. ReadOwner converts the three
 * rotation speeds from degrees to radians after reading them. */
struct SpaceGadgetParams {
    /* 0x0 */ u32 model;     /* Model: a part id of the model */
    /* 0x4 */ float rotSpdX; /* RotDegSpdX */
    /* 0x8 */ float rotSpdY; /* RotDegSpdY */
    /* 0xC */ float rotSpdZ; /* RotDegSpdZ */
};
CHECK_SIZEOF(SpaceGadgetParams, 0x10);

/* Class name from RTTI; member names and method names marked "(name inferred)" are not original. */
class SpaceGadget : public Gadget {
  public:
    SpaceGadget(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SpaceGadget();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads position, rotation and parameters from the owner and resets the health and the state
     * (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Passes the part id m_params.model to fn_80162A68, attaches the character info to that part
     * and, when the model has the entry m_params.model + 1000, registers OnAttack with the part's
     * attack object (name inferred). */
    void SetupModel();
    /* Places the model at the character info's position and rotation (name inferred). */
    void UpdateModelMatrix();
    /* Lowers the health by the attack's value; once it is used up, passes the attacker to
     * fn_8020642C and sets m_state to 1 (name inferred). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ SpaceGadgetParams m_params;
    /* 0x40 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x48 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* The names of the members below are inferred. */
    /* 0x50 */ s32 m_state; /* 1 once the health is used up, otherwise 0 */
    /* 0x54 */ float m_health;
};
CHECK_SIZEOF(SpaceGadget, 0x58);

#endif

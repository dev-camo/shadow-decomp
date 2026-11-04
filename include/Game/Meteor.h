#ifndef GAME_METEOR_H
#define GAME_METEOR_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table (names from the table's labels). */
struct MeteorParams {
    /* 0x00 */ float range;        /* RANGE m; the code scales it by 10 */
    /* 0x04 */ float moveSpeed;    /* MOVE_SPD m/s; scaled by 10 */
    /* 0x08 */ float directionX;   /* DIRECTION_X deg; converted to radians */
    /* 0x0C */ float directionY;   /* DIRECTION_Y deg; converted to radians */
    /* 0x10 */ float rotateSpeedX; /* ROT_SPD_X deg/s; converted to radians */
    /* 0x14 */ float rotateSpeedZ; /* ROT_SPD_Z deg/s; converted to radians */
    /* 0x18 */ float rotateSpeedY; /* ROT_SPD_Y deg/s; converted to radians */
    /* 0x1C */ float lifetime;     /* LIFETIME s */
};
CHECK_SIZEOF(MeteorParams, 0x20);

/* Meteor (class name from RTTI; the gadget's descriptor is "METEOR_L"). Method names marked
 * "(name inferred)" are not original. */
class Meteor : public Gadget {
  public:
    Meteor(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Meteor();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and parameters from the owner and starts over (name
     * inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model and the object at the character info's position and rotation (name
     * inferred). */
    void UpdateModelMatrix();
    /* The callback of the meteor's attack object for an attack on it (name inferred). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The two callbacks the meteor gives its attack object with fn_80054DD0; the first passes the
     * meteor's direction, speed and a strength of 5 to the attacked object (behaviour inferred). */
    void fn_8021CA5C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void fn_8021CA58(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ MeteorParams m_params;
    /* 0x50 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x58 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x60 */ boost::shared_ptr<Target> m_object;
    /* 0x68 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x70 */ s32 m_state;               /* 0 waiting, 1 moving, 2 breaking up (inferred) */
    /* 0x74 */ Fn8040Vector3 m_direction; /* the unit direction of the movement (inferred) */
    /* 0x80 */ float m_timer;             /* counts the lifetime down while moving (inferred) */
    /* 0x84 */ float m_health;            /* 1 at start; attacks subtract strength (inferred) */
};
CHECK_SIZEOF(Meteor, 0x88);

#endif

#ifndef GAME_ROLLCIRCLE_H
#define GAME_ROLLCIRCLE_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <list>
#include <vector>
#include "layout_check.h"

class RollCircle;

/* The gadget parameters in the order of the descriptor's table; the field names follow the
 * table's labels. ReadOwner multiplies the radius by 10 (the label gives it in metres) and converts
 * the speeds from degrees to radians per second. */
struct RollCircleParams {
    /* 0x00 */ u32 model;         /* "MODEL 0:Big 1:Sml 2:Head" */
    /* 0x04 */ u32 objNum;        /* "OBJ_NUM": the number of objects on the circle */
    /* 0x08 */ float radius;      /* "RADIUS m" */
    /* 0x0C */ float circleSpeed; /* "CIRCLE_SPD deg/sec" */
    /* 0x10 */ float objAngSpdX;  /* "OBJ_ANGSPD_X deg/sec" */
    /* 0x14 */ float objAngSpdY;  /* "OBJ_ANGSPD_Y deg/sec" */
    /* 0x18 */ float objAngSpdZ;  /* "OBJ_ANGSPD_Z deg/sec" */
};
CHECK_SIZEOF(RollCircleParams, 0x1C);

/* RollCircleObj (named after the boost::checked_deleter<RollCircleObj> RTTI string and the
 * RollCircleObj bound callbacks): one of the objects that a RollCircle moves around its circle. It
 * shows a model of the ROLLCIRCLE.BIN file, carries an attack object and loses the attack's
 * strength from its health in the owner's RollCircleSetState; below 0.0001 its holder breaks it
 * (behaviour inferred). Method and member names are inferred unless they are placeholders. */
class RollCircleObj {
  public:
    RollCircleObj(UnkGadgetOwner_800C9CD0 *owner, RollCircle *holder, u32 index);
    ~RollCircleObj();

    /* Moves the object to its place on the holder's circle and gives its character info the
     * holder's object rotation (name inferred). */
    void UpdatePosition();
    /* Places the model at the character info's position and rotation and the target at the
     * position (name inferred). */
    void UpdateModelMatrix();
    /* Places the object again after the holder has re-read its owner (name inferred). */
    void Reset();
    void Update(float dt);
    /* Shows the break effect of the model's file and plays the sound 0x7330 (name inferred). */
    void Break();
    /* The attack object's callback (behaviour inferred): the attack's strength at 0x50 comes off
     * the object's health. Once the health is below 0.0001, an object not yet queued passes the
     * attacker's character info (if any) to fn_8020642C with 0x3F and is queued for breaking, and
     * the attack is answered with 3; otherwise it is answered with 1. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The two callbacks the object gives its attack object with fn_80054DD0. The first passes the
     * character info's vector at 0x18, divided by the float at 0x18 of lbl_8057D6A0, to the
     * attacked object as a direction at 0x34 and a length at 0x4C, with 1 at 0x50. The direction is
     * normalised with fn_8040CE74, which gives the zero vector for a vector shorter than 1e-6, so a
     * stationary character passes a zero direction. The second queues the object for breaking when
     * bit 0 of the word at 0x54 is set (behaviour inferred). */
    void fn_800D61C4(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void fn_800D6170(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    /* 0x00 */ u32 m_index; /* the object's place in the circle and in the health array */
    /* 0x04 */ RollCircle *m_holder;
    /* 0x08 */ float m_angle; /* m_index times a full turn divided by the number of objects */
    /* 0x0C */ UnkGadgetOwner_800C9CD0 *m_owner;
    /* 0x10 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x18 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x20 */ boost::shared_ptr<Target> m_target;
};
CHECK_SIZEOF(RollCircleObj, 0x28);

/* RollCircle (class name from RTTI; the descriptor's name is ROLL_CIRCLE): a gadget that moves
 * OBJ_NUM RollCircleObj objects around a circle of RADIUS about its position, turning the circle at
 * CIRCLE_SPD and the objects about their own axes at OBJ_ANGSPD_X/Y/Z. The objects' healths are
 * kept in a RollCircleSetState that the owner remembers. The objects are created only on
 * construction and when ResetFromOwner rebuilds them (a changed model or object count also
 * replaces the state first); with the saved state reused, an object whose saved health is below
 * 0.0001 is not created. The second attack callback (fn_800D6170) can queue an object for breaking
 * without changing its health, so such an object can return when the objects are created again.
 * When the last object breaks, the gadget passes its owner to fn_800C9B34 and finishes (behaviour
 * inferred).
 * Method and member names are inferred unless they are placeholders. */
class RollCircle : public Gadget {
  public:
    typedef std::list<boost::shared_ptr<RollCircleObj> > ObjList;

    RollCircle(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~RollCircle();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, the rotation and the parameters from the owner, converts the parameters,
     * reserves room for OBJ_NUM queued indices, sets the offset of the first object from the centre
     * and zeroes the object rotation (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Gives the owner a new RollCircleSetState with every object's health set to the model's
     * starting value (name inferred). */
    void CreateState();
    /* Creates the objects whose health in the owner's state is at least 0.0001 (name inferred). */
    void CreateObjs();
    /* True when the object with the index is queued for breaking (name inferred). */
    bool IsQueued(u32 index);
    /* Queues the object with the index for breaking (name inferred). */
    void Queue(u32 index);
    /* Breaks the queued objects; when none is left, passes the owner to fn_800C9B34 and finishes
     * the gadget (name inferred). */
    void BreakQueued();
    /* Breaks the object with the index and removes it from the list (name inferred). */
    void BreakObj(u32 index);

    static const char *s_name;

    /* 0x30 */ Fn8040Vector3 m_position;
    /* 0x3C */ Fn8040Vector3 m_rotation;
    /* The offset of the object with angle 0 from the position; the update turns it about the y axis
     * at the circle speed. */
    /* 0x48 */ Fn8040Vector3 m_offset;
    /* The rotation that every object's character info takes; the update turns it at the objects'
     * angular speeds. */
    /* 0x54 */ Fn8040Vector3 m_objRotation;
    /* 0x60 */ RollCircleParams m_params;
    /* 0x7C */ ObjList m_objs;
    /* The indices of the objects queued for breaking. */
    /* 0x88 */ std::vector<u32> m_queued;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(RollCircle, 0x98);
CHECK_SIZEOF(RollCircle::ObjList, 0xC);
CHECK_SIZEOF(std::vector<u32>, 0xC);

#endif

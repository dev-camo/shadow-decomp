#ifndef GAME_ELECCRISTALWALL_H
#define GAME_ELECCRISTALWALL_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the ELEC_WALL descriptor's table, whose entry names give
 * the member names. fn_800C9DBC reads them from the owner. */
struct ElecCristalWallParams {
    /* The entry without a name; its labels are "type< Horizontal >" for 0 and
     * "type< Vertical   >" for 1. */
    /* 0x00 */ s32 type;
    /* 0x04 */ float appearAngleX; /* degrees */
    /* 0x08 */ float appearAngleY; /* degrees */
};
CHECK_SIZEOF(ElecCristalWallParams, 0xC);

/* ElecCristalWall (class name from RTTI; "Cristal" is the original spelling; the gadget's
 * descriptor is "ELEC_WALL"): a wall of the WALL.BIN model. A vertical wall (type 1) is turned a
 * quarter turn about the x axis. Once the state of its com id becomes 1, it appears over a second:
 * it slides in from 75 units away along the direction that the "AppearAngle" parameters turn,
 * turning back from a random half turn about each axis. Each update of the appearance looks up its
 * "mat" part and, when the part is found, sets the part's colour along a fade from the first to the
 * second appearance colour; then, as after the appearance, every update in which the flash timer is
 * not running looks the part up again and, when it is found, overwrites the last three bytes of
 * that colour with the health colour. Then six accepted attacks, each followed by a short flash,
 * break it up with an effect and it finishes (behaviour inferred). Method and member names are
 * inferred. */
class ElecCristalWall : public Gadget {
  public:
    ElecCristalWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecCristalWall();
    virtual void UpdateBody(float dt);
    /* Puts m_position back at the owner's position and clears m_rotation, draws the appear
     * direction (30 units long) from the character info's position as a debug line and calls the
     * model's fn_8016108C (behaviour inferred). */
    virtual void fn_800CBC1C(float);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the wall under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation, parameters and com id into the wall; the rotation
     * of a vertical wall is turned a quarter turn about the x axis. The test reads the type
     * before the parameters are copied. */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at m_position, turned by m_rotation in the axis order m_rotationOrder
     * and then by the character info's rotation in the axis order 2. */
    void UpdateModelMatrix();
    /* The model attack object's callback. An attack on the appeared wall (STATE_APPEARED)
     * outside a flash flashes the "mat" part and takes one health; the callback reports 3 and
     * plays sound 0x7241 when that uses up the health (the wall breaks at the next update),
     * otherwise 1 and sound 0x7240. It reports 1 for every other attack. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Shows the break effect of model entry 1. */
    void ShowBreakEffect();

    /* The values of m_state (names inferred). There is no state 3. */
    enum State {
        STATE_RESET = 0,
        STATE_WAITING = 1,
        STATE_APPEARING = 2,
        STATE_APPEARED = 4,
        STATE_HEALTH_USED_UP = 5
    };

    static const char *s_name;

    /* 0x30 */ ElecCristalWallParams m_params;
    /* 0x3C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x44 */ GadgetModelInfo *m_model;
    /* 0x48 */ u8 m_comId;
    /* STATE_RESET after construction or a reset, STATE_WAITING while waiting for the com id,
     * STATE_APPEARING during the appearance, STATE_APPEARED afterwards and STATE_HEALTH_USED_UP
     * once OnAttack has taken the last health (the wall breaks at the next update). */
    /* 0x4C */ State m_state;
    /* 0x50 */ float m_appearTime; /* 0 to 1 over the appearance, in seconds */
    /* 0x54 */ s32 m_health;       /* 6 after construction */
    /* (0, 0, -75) turned about the x and y axes by the "AppearAngle" parameters */
    /* 0x58 */ Fn8040Vector3 m_appearOffset;
    /* 0x64 */ Fn8040Vector3 m_appearRotation; /* pi or -pi about each axis, at random */
    /* The constructor sets 0, 1 or 2 for those results of fn_8040EA90(3) and leaves it unchanged
     * when the helper returns 3, which it can when its float rounds to 1. */
    /* 0x70 */ s32 m_rotationOrder;
    /* 0x74 */ Fn8040Vector3 m_position;
    /* 0x80 */ Fn8040Vector3 m_rotation;
    /* Counts down after a flash; the health colour is applied while it is 0. */
    /* 0x8C */ float m_flashTime;
};
CHECK_SIZEOF(ElecCristalWall, 0x90);

#endif

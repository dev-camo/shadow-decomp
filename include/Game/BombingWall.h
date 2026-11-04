#ifndef GAME_BOMBINGWALL_H
#define GAME_BOMBINGWALL_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The two words that the gadget parameters "Type 0:Out 1:In" and "Range (point)" fill, in this
 * order. */
struct BombingWallParams {
    /* 0x0 */ u32 type; /* 0 out, 1 in (from the parameter's label) */
    /* 0x4 */ float range;
};
CHECK_SIZEOF(BombingWallParams, 0x8);

/* BombingWall (class name from RTTI): a wall that breaks when its stage switch is set, when its
 * attack callback reports an attack object with bit 0x200 of its word at 0x40 set, or, for type 0
 * only, when the nearest character comes within its range. It then shows the break effect of its
 * type, plays a sound, sets its switch and finishes (behaviour inferred). Method names marked
 * "(name inferred)" are not original; all member names are inferred. */
class BombingWall : public Gadget {
  public:
    BombingWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BombingWall();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation, switch id and parameters (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Calls fn_80162A68(0, 0) (the entry with id 0 of the model's second table), attaches the
     * character info to part 0 and registers OnAttack with that part's attack object (name
     * inferred). */
    void SetupModel();
    /* Places the model at the character info's position and rotation, and the object at its
     * position (name inferred). */
    void UpdateModelMatrix();
    /* The attack object's callback (name inferred). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0 standing; 1 (set by an accepted attack) and 2 (set by the switch or a character in range)
     * both break the wall in the next update (inferred from UpdateBody and OnAttack). */
    /* 0x30 */ s32 m_state;
    /* 0x34 */ BombingWallParams m_params;
    /* 0x3C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x44 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x4C */ boost::shared_ptr<Target> m_object;
    /* 0x54 */ s32 m_switchId; /* the id of the stage switch that breaks the wall (inferred) */
};
CHECK_SIZEOF(BombingWall, 0x58);

#endif

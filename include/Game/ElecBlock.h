#ifndef GAME_ELECBLOCK_H
#define GAME_ELECBLOCK_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/CharInfo.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the ELEC_BIGBLOCK descriptor's table, whose entry names
 * give the member names. fn_800C9DBC reads them from the owner. */
struct ElecBlockParams {
    /* 0x00 */ s32 blockType; /* 0 to 9; the model entry id passed to fn_80162A68 */
    /* 0x04 */ float scaleX;
    /* 0x08 */ float scaleY;
    /* 0x0C */ float scaleZ;
    /* 0x10 */ float searchRange;
    /* 0x14 */ float initialY;
    /* 0x18 */ s32 collisionFlag; /* 0 FALSE, 1 TRUE */
};
CHECK_SIZEOF(ElecBlockParams, 0x1C);

/* ElecBlock (class name from RTTI; the gadget's descriptor is "ELEC_BIGBLOCK"): a block of the
 * BIGBLOCK.BIN model that starts "Initial Y" above or below its position and, once the nearest
 * character info comes within its search range, accelerates back to its position and stops there
 * (behaviour inferred). Method names marked "(name inferred)" are not original. */
class ElecBlock : public Gadget {
  public:
    ElecBlock(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecBlock();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the block under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameters into the block (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position raised by m_height, turned by its yaw
     * and scaled by the parameters (name inferred). */
    void UpdateModelMatrix();
    /* Advances a position and a velocity by duration under a constant acceleration and a drag
     * coefficient, as fn_804103E8 does for vectors (name inferred). */
    void Integrate(float *position, float *velocity, float acceleration, float drag,
                   float duration);
    /* True when the nearest character info is within the search range (name inferred). */
    bool IsCharInfoInRange();
    /* Passes the "Scale" parameters, multiplied by three fixed factors, to the model, and the
     * negated y product as a vertical offset (name inferred). */
    void ApplyScale();

    static const char *s_name;

    /* 0x30 */ ElecBlockParams m_params;
    /* 0x4C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x54 */ GadgetModelInfo *m_model;
    /* 0x58 */ s32 m_loadedBlockType; /* the block type the model was set up with */
    /* The state (inferred from the update): 0 waiting for a character info in range, 1 moving to
     * the position, 2 easing m_height to 0 with Integrate (no code in this unit sets it), 3 at
     * rest. */
    /* 0x5C */ s32 m_state;
    /* 0x60 */ float m_height; /* offset above the position; starts at m_params.initialY */
    /* 0x64 */ float m_speed;  /* rate of change of m_height (inferred) */
    /* 0x68 */ boost::shared_ptr<CharInfo> field_0x68; /* only constructed and destroyed */
};
CHECK_SIZEOF(ElecBlock, 0x70);

#endif

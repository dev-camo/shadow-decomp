#ifndef GAME_DAMAGEBLOCK_H
#define GAME_DAMAGEBLOCK_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* DamageBlock (class name from RTTI; the gadget's descriptor is "DAMAGE BLOCK"): shows the
 * DAMAGEBLOCK.BIN model entry that the parameter "kind" selects and attaches its character info
 * to that entry (behaviour inferred). Member names and the names of the non-virtual methods are
 * inferred. */
class DamageBlock : public Gadget {
  public:
    DamageBlock(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~DamageBlock();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and the parameter "kind" from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info, turned about the y axis by the character info's
     * rotation (name inferred). */
    void UpdateModelMatrix();

    static const char *s_name;

    /* 0x30 */ s32 m_modelIndex; /* the entry identifier passed to the model lookup */
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ GadgetModelInfo *m_model; /* owned: the destructor deletes it */
    /* 0x40 */ s32 m_kind;               /* the parameter "kind" */
};
CHECK_SIZEOF(DamageBlock, 0x48);

#endif

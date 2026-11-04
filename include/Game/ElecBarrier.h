#ifndef GAME_ELECBARRIER_H
#define GAME_ELECBARRIER_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "layout_check.h"

/* ElecBarrier (class name from RTTI; the gadget's descriptor is "ELEC_BARRIER"). Member names and
 * the names of the non-virtual methods are inferred from the code. */
class ElecBarrier : public Gadget {
  public:
    ElecBarrier(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecBarrier();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation, com id and barrier type from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position and rotation (name inferred). */
    void UpdateModelMatrix();

    static const char *s_name;

    /* 0x30 */ s32 m_barrierType; /* the parameter "BarrierType" (0-2) */
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ GadgetModelInfo *m_model;
    /* 0x40 */ u8 m_comId;      /* the id whose state opens the barrier (inferred) */
    /* 0x44 */ s32 m_modelType; /* the barrier type the model was set up for (inferred) */
    /* State 0 waits for the com id's state to become 1, state 1 fades the barrier out
     * (inferred). */
    /* 0x48 */ s32 m_state;
    /* 0x4C */ float m_timer;
};
CHECK_SIZEOF(ElecBarrier, 0x50);

#endif

#ifndef GAME_FOOTING_H
#define GAME_FOOTING_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* Footing (class name from RTTI; the gadget's descriptor is "FOOTING"): asks the FOOTING.BIN
 * model to show the entry whose ID is the parameter "model" and to attach its character info to
 * that entry (behaviour inferred), as DamageBlock does with DAMAGEBLOCK.BIN. Both lookups can
 * fail (no entry or part with that ID) and the results are ignored. Member names and the names
 * of the non-virtual methods are inferred. */
class Footing : public Gadget {
  public:
    Footing(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Footing();
    /* Advances the model by dt when there is one. */
    virtual void UpdateBody(float dt);
    /* Rereads the owner (ReadOwner) and places the model. When there is a model and the
     * parameter "model" differs from the ID last passed to the lookups, it clamps the parameter
     * to the model's entry count (first to at least 0, then to at most fn_8015FF44 - 1, so -1
     * when there are none), tries to show the entry with that ID and to attach the character
     * info to it (the results are ignored), and writes the clamped value back to the owner
     * (fn_800C9D68: only when the owner's parameter block is 4 bytes); with a model it then
     * calls the model's fn_8016108C. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and its rotation (converted to radians) into the character
     * info, and the parameter "model" into m_modelParam only when the owner's parameter block is
     * 4 bytes (fn_800C9DBC; otherwise m_modelParam keeps its value) (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info, turned about the y axis by the character info's
     * rotation (name inferred). */
    void UpdateModelMatrix();

    static const char *s_name;

    /* 0x30 */ s32 m_modelIndex; /* the ID passed to the entry lookups */
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ GadgetModelInfo *m_model; /* owned: the destructor deletes it */
    /* 0x40 */ s32 m_modelParam;         /* the parameter "model" */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(Footing, 0x48);

#endif

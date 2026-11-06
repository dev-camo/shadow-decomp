#ifndef GAME_CAGE_H
#define GAME_CAGE_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the CAGE descriptor's table, whose entry names give the
 * member names. The constructor stores the defaults the gadget starts with. */
struct CageParams {
    CageParams() : model(0), scaleX(1.0f), scaleY(1.0f), scaleZ(1.0f) {}

    /* 0x00 */ s32 model; /* the model entry id (the table's limit is 1) */
    /* 0x04 */ float scaleX;
    /* 0x08 */ float scaleY;
    /* 0x0C */ float scaleZ;
};
CHECK_SIZEOF(CageParams, 0x10);

namespace Cage {

/* Cage::Cage (class name from RTTI; the gadget's descriptor is "CAGE"): a gadget of the CASE.BIN
 * model, scaled by the "scale" parameters (behaviour inferred). Member and method names are
 * inferred.
 *
 * The model's collision callback queries fn_8016904C for entry m_comId of fn_800CB69C's table at
 * each call (the query returns 0 when there is no table): while the result is not 1, every
 * character info that fn_8015B70C passes on (the hits whose collision object and character info
 * still lock and whose character info's word at 0x5C is 4) is sent a new BindCommand holding the
 * cage's character info; once it is 1, a ReleaseCommand instead. The commands are requests (a
 * null command when the allocation fails).
 *
 * Each update first advances the model when there is one (fn_80162FB0, which can run the collision
 * callback), then, in state 0 only, once the same query returns 1: sets state 2; when there is a
 * model, requests the motion of the loaded entry (fn_80160B18), calls fn_80161880, attempts to
 * attach the character info to the entry two above the loaded one (fn_80161FC8, whose result is
 * ignored), and passes that entry's id with s_switchedSizes' factors for the loaded entry times
 * the scale parameters to fn_801617D0 and with the y product as an offset to fn_80161740 (both act
 * only on the model's collision parts whose id matches, so none changes when no part matches);
 * then requests sound 0x1002 for "model" 0 or 0x1001 for "model" 1 (any other value requests none;
 * fn_801D54F4 plays nothing when its sound setup returns null), whether or not there is a model.
 * Every update ends by placing the model when there is one.
 * CopyFromOwner sets state 0; nothing else stores a state, so state 2 lasts until the next
 * ResetFromOwner. */
class Cage : public Gadget {
  public:
    Cage(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Cage();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation, parameters (fn_800C9DBC, only when the owner's
     * parameter block has their size) and com id, and sets the state to 0 (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model, when there is one, at the character info's position, turned by its y, x
     * and z rotations in that order and scaled by the scale parameters (name inferred). */
    void UpdateModelMatrix();
    /* The model's collision callback (name inferred; behaviour in the class comment). */
    void OnCollision(ColliDetectArgs &args);
    /* When there is a model, multiplies the bounding sphere radius of every atomic of its parts'
     * clumps by scale X, replaced by scale Y and then by scale Z when scale < that parameter (the
     * greatest of the three unless one is NaN) (name inferred). */
    void ScaleBoundingSpheres();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ CageParams m_params;
    /* Set to 0 by CopyFromOwner and to 2 by the update (see the class comment); no other value
     * is stored. */
    /* 0x50 */ s32 m_state;
    /* 0x54 */ s32 m_loadedModel; /* the model entry the model was set up with */
    /* 0x58 */ u8 m_comId;        /* the owner's com id (fn_800C9E60) */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(Cage, 0x60);

} // namespace Cage

#endif

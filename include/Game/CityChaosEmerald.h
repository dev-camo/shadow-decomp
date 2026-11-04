#ifndef GAME_CITYCHAOSEMERALD_H
#define GAME_CITYCHAOSEMERALD_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* CityChaosEmerald (class name from RTTI): a gadget that shows the model CITYCHAOSEMERALD.BIN and
 * plays a sound at its position. When one of the model's collision hits passes fn_8015D2EC's
 * test, it starts effect 0xC at its position, plays sound 0xE017, sets the byte at 0xC1 of
 * lbl_8057D6A0 to 1 and the state of its com id to 1, passes its owner to fn_800C9B04 and marks
 * itself finished (behaviour inferred). Method and member names are inferred. */
class CityChaosEmerald : public Gadget {
  public:
    CityChaosEmerald(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~CityChaosEmerald();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and com id into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position, turned by its y rotation (name
     * inferred). */
    void UpdateModelMatrix();
    /* The model's collision callback (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model; /* CITYCHAOSEMERALD.BIN */
    /* 0x40 */ s32 m_comId;                                /* the owner's com id (fn_800C9E50) */
    /* 0x44 */ UnkSoundHandle_801D3DC8 m_sound;
};
CHECK_SIZEOF(CityChaosEmerald, 0x60);

#endif

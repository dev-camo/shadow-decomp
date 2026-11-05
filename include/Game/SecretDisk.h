#ifndef GAME_SECRETDISK_H
#define GAME_SECRETDISK_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace SecretDisk {

/* SecretDisk::SecretDisk (class name from RTTI): a gadget, in the namespace SecretDisk, that shows
 * the SECRETDISK.BIN model at the owner's position with a second object (fn_802B4254) above it.
 * Once the model's collision callback has reported a hit for which fn_8015AC84 is true, the
 * gadget calls UnkSlot7(1, 0) on the actor that fn_800740E4 returns for 2, starts an effect 6
 * units below its position, plays a sound, passes its owner to fn_800C9B04 (through fn_8015A768)
 * and marks itself finished (behaviour inferred). Method and member names are inferred. */
class SecretDisk : public Gadget {
  public:
    SecretDisk(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SecretDisk();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the gadget and clears m_state (name
     * inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info and, while there is one, the second object
     * s_objectHeight above the character info's position (name inferred). */
    void UpdateModelMatrix();
    /* The model's collision callback: in state 1, a hit for which fn_8015AC84 is true moves the
     * gadget to state 2 (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkObject_802B4254> m_object;
    /* 0x48 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0 after construction or a reset, 1 while waiting for a hit, 2 once hit (inferred). */
    /* 0x50 */ s32 m_state;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(SecretDisk, 0x58);

} // namespace SecretDisk

#endif

#ifndef GAME_ELECROLLHEXA_H
#define GAME_ELECROLLHEXA_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkAttack_8005A840.h"

/* Class name from RTTI; member names and virtual entry-point names are inferred. */
class ElecRollHexa : public Gadget {
  public:
    ElecRollHexa(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecRollHexa();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Allocates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);
    void fn_802673E8(UnkGadgetOwner_800C9CD0 *owner);
    void fn_80267334();
    void fn_80267278();
    void fn_80267264(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void fn_80267164();
    static const char *s_name;

    /* 0x30 */ float m_rotateSpeed;
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ GadgetModelInfo *m_model;
    /* 0x40 */ float m_timer;
    /* 0x44 */ float m_heightOffset;
    /* 0x48 */ int field_0x48;
    /* Unclassified storage in the original 0x50-byte allocation; no accesses are identified. */
    /* 0x4C */ unsigned char field_0x4C[4];
};
CHECK_SIZEOF(ElecRollHexa, 0x50);

#endif

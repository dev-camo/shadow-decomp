#ifndef GAME_ELECSMALLCUBE_H
#define GAME_ELECSMALLCUBE_H
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkAttack_8005A840.h"

/* Class name from RTTI; member names and virtual entry-point names are inferred. */
class ElecSmallCube : public Gadget {
  public:
    ElecSmallCube(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecSmallCube();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Allocates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);
    void fn_8021B1B0(UnkGadgetOwner_800C9CD0 *owner);
    void fn_8021B140();
    void fn_8021AEC0(float dt);
    void fn_8021AEAC(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void fn_8021ADC0();
    static const char *s_name;
    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ GadgetModelInfo *m_model;
    /* 0x3C */ int field_0x3C;
    /* 0x40 */ float m_timer;
    /* 0x44 */ Fn8040Vector3 m_rotationFrom;
    /* 0x50 */ Fn8040Vector3 m_rotationTo;
    /* Unclassified storage in the original 0x60-byte allocation; no accesses are identified. */
    /* 0x5C */ unsigned char field_0x5C[4];

  private:
    void PickNextRotation();
};
CHECK_SIZEOF(ElecSmallCube, 0x60);
#endif

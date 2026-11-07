#ifndef GAME_GIANTLANTERN_H
#define GAME_GIANTLANTERN_H

#include "Game/CharaColliAttack.h"
#include "Game/CharaColliBase.h"
#include "Game/EffectElement.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class GiantLanternSetState;

/* GiantLantern (class name from RTTI; the gadget's descriptor has the same name): a gadget without
 * a model of its own. A probe sphere around its character info carries an attack object, and an
 * attack whose flag word has bit 0x400 lights the lantern. Once lit it starts an effect at its
 * position, plays the sounds 0x7431 and 0x7430, sets the state of its com id, calls fn_8016B224(1,
 * 0) on the mission that fn_800740E4 returns for 0 when it locks to a non-null pointer, passes its
 * owner to fn_800C985C and drops the object (fn_802B4254) that it shows above itself while unlit
 * (behaviour inferred). Method and member names are inferred unless they are placeholders. */
class GiantLantern : public Gadget {
  public:
    GiantLantern(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~GiantLantern();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and com id into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Passes the character info's position to the effect's base transform (fn_80425E4C) when the
     * weak effect reference has not expired and locks to a non-null effect, and, while there is
     * an object, places it s_objectHeight above that position (name inferred). */
    void UpdatePositions();
    /* The attack object's callback that fn_80096B64 installs (behaviour inferred): an attack for
     * which fn_8015D4E4 is true and whose flag word has bit 0x400 lights the lantern while it is
     * unlit and is answered with 1; every other attack is answered with 5. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The first callback passed to fn_80054DD0: stores the normalised direction from the
     * character info's position to param's field_0x18 in field_0x34, 10 in field_0x4C and 1 in
     * field_0x50. */
    void fn_8023684C(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The second callback passed to fn_80054DD0; it does nothing. */
    void fn_80236848(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
    /* Present while the lantern is unlit. */
    /* 0x40 */ boost::shared_ptr<UnkObject_802B4254> m_object;
    /* 0x48 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* Bit 0 is set by OnAttack when the lantern is lit and consumed by UpdateBody. */
    /* 0x50 */ UnkBitFlags_80014E04 m_flags;
    /* 0x54 */ GiantLanternSetState *m_data;
    /* 0x58 */ UnkSoundHandle_801D3DC8 m_sound;
    /* 0x74 */ u8 m_comId; /* the owner's com id (fn_800C9E60) */
};
CHECK_SIZEOF(GiantLantern, 0x78);

#endif

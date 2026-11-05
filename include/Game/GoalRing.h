#ifndef GAME_GOALRING_H
#define GAME_GOALRING_H

#include "Game/EffectElement.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace GoalRing {

/* GoalRing::GoalRing (class name from RTTI; the gadget's descriptor is "GOALRING"): a gadget, in
 * the namespace GoalRing, that shows the GOALRING.BIN model with a sound and two effects at the
 * owner's position. Its parameter "color" selects the model entry (fn_80162A68; inferred) and,
 * through a table of effect ids, a colour effect; colour 7 has no colour effect. When the model's
 * collision callback reports a character, it passes the manager of fn_8006D6A8 to fn_8016DA34,
 * calls the model's fn_801612E0, stops the sound and, unless the colour is 7, sets the byte at 0xC1
 * of lbl_8057D6A0 to 1 (behaviour inferred). Method and member names are inferred. */
class GoalRing : public Gadget {
  public:
    GoalRing(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~GoalRing();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameter "color" into the gadget (name
     * inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position and passes that position to both
     * effects and, while the sound handle is set, to the sound (name inferred). */
    void UpdatePositions();
    /* The model's collision callback (name inferred). */
    void OnCollision(ColliDetectArgs &args);
    /* While there is a model: clears the active flag of the current colour effect and, when the
     * table has an effect for m_shownColor, starts the effect of color (name inferred; every
     * caller passes m_shownColor). */
    void SetColorEffect(int color);

    static const char *s_name;

    /* The colour of the colour effect; the constructor also passes it to fn_80162A68 to select
     * the model entry (inferred). */
    /* 0x30 */ s32 m_shownColor;
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ boost::shared_ptr<GadgetModelInfo> m_model;              /* GOALRING.BIN */
    /* 0x44 */ s32 m_color;                                             /* the parameter "color" */
    /* 0x48 */ boost::weak_ptr<Effect::System::EffectElement> m_effect; /* fn_8015D794 id 0x8E */
    /* 0x50 */ boost::weak_ptr<Effect::System::EffectElement> m_colorEffect;
    /* 0x58 */ UnkSoundHandle_801D3DC8 m_sound; /* sound 0xE00D */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(GoalRing, 0x78);

} // namespace GoalRing

#endif

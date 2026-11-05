#ifndef GAME_SHOOTINGGAMESCORE_H
#define GAME_SHOOTINGGAMESCORE_H

#include "Game/CharaColliAttack.h"
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkModelAtomicState.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace ShootingGame {

/* ShootingGame::Score (class name from RTTI; the gadget's descriptor is "SHOOTING_PANEL"): a panel
 * that takes part in the game of the shooting game table entry that its "Id" parameter names
 * (see ShootingGameManagerTask.h). It shows model 0 until the entry is in state 1. An attack in its
 * state 1 records the attacker and, unless the entry is already in state 1, puts the entry into
 * state 1. Once the entry is in state 1 the panel plays the "hit" motion with model 1 and then the
 * "act" motion with model 2, whose 16 score parts show the entry's count in colour. After the
 * "act" motion, or once the entry reaches state 2 (an attack whose attack object has bit 0x10 set
 * in its word at 0x40 does this), the panel waits (two seconds from the motion states) and, if its
 * attack put the entry into state 1, passes the attacker to the table's Finish before it finishes
 * (behaviour inferred). Method names marked "(name inferred)" are not original; all member names
 * are inferred. */
class Score : public Gadget {
  public:
    Score(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Score();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    virtual bool IsFinished();

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and "Id" parameter and resets the state, the timer,
     * the shown count and the score parts (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position and rotation (name inferred). */
    void UpdateModelMatrix();
    /* Calls fn_8016232C, fn_80161880, fn_801604FC and fn_80162804 on the model before another
     * model is selected (name inferred). */
    void ClearModel();
    /* Selects model index, attaches the character info and registers OnAttack with the attack
     * object of part 0; model 2 also looks up its score parts (name inferred). */
    void SetupModel(int index);
    /* The attack object's callback (name inferred). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* The number of score parts of model 2 (name inferred). */
    static const int kScorePartCount = 16;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* The owner of the attack object of the last attack in state 1. */
    /* 0x40 */ boost::weak_ptr<CharInfo> m_attacker;
    /* 0x48 */ int m_id; /* the "Id" parameter */
    /* 0 after a reset, 1 waiting for the entry, 2 "hit" motion, 3 "act" motion, 5 waiting before
     * finishing, 6 finished (inferred from UpdateBody). */
    /* 0x4C */ int m_state;
    /* 0x50 */ float m_timer;
    /* The entry's word at 0x10 that the score parts show, -1 after a reset. */
    /* 0x54 */ int m_count;
    /* 0x58 */ UnkModelAtomicState *m_scoreParts[kScorePartCount];
    /* Bit 0: this panel's attack put the entry into state 1 (inferred). */
    /* 0x98 */ UnkBitFlags_80014E04 m_hitFlags;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(Score, 0xA0);

} // namespace ShootingGame

#endif

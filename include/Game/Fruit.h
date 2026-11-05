#ifndef GAME_FRUIT_H
#define GAME_FRUIT_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace Fruit {

/* Fruit::FruitTask (class name from RTTI; the gadget's descriptor is "Fruit"): shows the model of
 * resource 0xD3 and attaches the character info to its part 0; when that succeeds and the part's
 * attack object still exists, the constructor installs two callbacks on that attack object. While
 * the gadget waits (STATE_WAITING), OnHit, or OnAttack for an accepted attack, switches it to
 * STATE_HIT. At the next update UpdateBody tries the break effect of the model's entry 1000 with
 * sound 0x1022 (only when the model and that entry exist). When the model has an entry 2000, it
 * then sets up part 2000 (SetupPart2000) and stays in STATE_PART_2000; otherwise it passes the
 * owner, when there is one, to fn_800C9B34 and sets Task flag 0x01. Behaviour inferred; method
 * names marked "(name inferred)" are not original, and the parameters' pointer, reference and
 * const spellings are inferred. */
class FruitTask : public Gadget {
  public:
    FruitTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~FruitTask();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* When there is a character info, copies the owner's position and its rotation (converted to
     * radians) into it; then sets the state to STATE_RESET (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model, when there is one, at position, turned by the y, x and z angles of
     * rotation (name inferred). */
    void UpdateModelMatrix(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* The callback in the first callback member (0x10) of part 0's attack object (name
     * inferred; FallingStoneHard's OnHit sits in the third member, 0x28): in STATE_WAITING,
     * switches the state to STATE_HIT; in any other state it does nothing. */
    void OnHit(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The callback in the second callback member of the attack objects of parts 0 and 2000 (name
     * inferred). In STATE_WAITING, an attack whose attack object has bit 1 of the word at 0x40
     * gets the result 5; any other attack passes its attack object's owner, when it still
     * exists, to fn_8020642C with 0x16, gets the result 3 and switches the state to STATE_HIT.
     * Without an attack object, or outside that state, the result is 5. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Passes fn_8015D93C a parameter block with the clump of the file that the model's entry 1000
     * names, then passes sound 0x1022 and the character info's position to fn_801D54F4 to play it;
     * nothing happens without a model or that entry (name inferred). */
    void StartBreakEffect();
    /* When the table that fn_80162A68 searches has an entry 2000 (fn_80162EA8), calls
     * fn_80161880 and fn_80162804, passes 2000 to fn_80162A68 and attaches the character info to
     * part 2000; when that succeeds and the part's attack object still exists, registers OnAttack
     * with that attack object. Then places the model and returns true. Otherwise returns false
     * without any of these calls (name inferred). The model pointer is not checked here. */
    bool SetupPart2000();

    /* The values of m_state (names inferred). */
    enum State { STATE_RESET = 0, STATE_WAITING = 1, STATE_HIT = 2, STATE_PART_2000 = 3 };

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* STATE_RESET after construction or a reset, STATE_WAITING until an attack, STATE_HIT
     * after it and STATE_PART_2000 once part 2000 has been set up. */
    /* 0x48 */ s32 m_state;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(FruitTask, 0x50);

} // namespace Fruit

#endif

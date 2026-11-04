#ifndef GAME_BOMBSERVER_H
#define GAME_BOMBSERVER_H

#include "Game/CharCommand.h"
#include "Game/CharaColliBase.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameter in the descriptor's table (the field name follows its label). */
struct BombServerParams {
    /* 0x0 */ u32 type; /* "Type 0:Bomb 1:HealUnit" */
};
CHECK_SIZEOF(BombServerParams, 0x4);

/* BombServer (class name from RTTI; the gadget's descriptor is "BombServer"): a dispenser that
 * hands out a Bomb (type 0) or a HealingUnit (type 1), the classes that the two spawn functions
 * it calls construct (names from RTTI). A command with the id 0xD starts a round while it is idle
 * or finishing one; its timer then runs down only while no Bomb or HealingUnit, and no character
 * info whose fn_80078B2C returns true, is in its probe sphere, after which it spawns the item and
 * returns to idle (behaviour inferred). Member names and the names of the non-virtual methods are
 * inferred from the code. */
class BombServer : public Gadget {
  public:
    BombServer(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BombServer();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameter, and clears the timer (name
     * inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Calls fn_80162A68 with the type (the entry of the model's second table whose id is the
     * type), places the model, creates the target object that it hands to the model's parts and
     * the probe sphere, and calls the probe's UnkSlot2 unless the state is 1 (the method name and
     * the reading of UnkSlot2 as switching the probe off are inferred). */
    void SetupModel();
    /* Places the model at the character info's position, turned about the y axis by its rotation
     * (name inferred). */
    void UpdateModelMatrix();
    /* The character info's command handler (name inferred). */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x48 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
    /* 0x50 */ BombServerParams m_params;
    /* 0 idle, 1 waits for the probe sphere to be clear, 2 waits to spawn the item, 3 waits before
     * going idle (inferred from UpdateBody and OnCommand). */
    /* 0x54 */ s32 m_state;
    /* 0x58 */ float m_timer; /* counts down to the next state change */
};
CHECK_SIZEOF(BombServer, 0x60);

#endif

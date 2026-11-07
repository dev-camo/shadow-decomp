#ifndef GAME_CITYBOMBBIG_H
#define GAME_CITYBOMBBIG_H

#include "Game/CharCommand.h"
#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* CityBombBig (class name from RTTI; the gadget's descriptor is "CITY_BOMB_BIG"): the
 * CITYBOMBBIG.BIN model, with sound 0x714A started by the constructor (fn_801D3E58). Reading the
 * owner puts it in state 1 with a health of 10. Command 0x1A puts it in state 0 unless it is in
 * state 2, and command 0x1B puts it back in state 1 from state 0. Only in state 1 do attacks
 * subtract their field_0x50 from the health; the attack after which the health is below 0, or 0
 * as fn_8000A950 judges it, sets bit 0 of m_stateFlags. An update in state 1 that finds the bit
 * set enters state 2, and from then on each update subtracts its time step from m_countdown (1
 * after ReadOwner). When m_countdown is below 0, or 0 as fn_8000A950 judges it, the gadget
 * requests a CommonBomb at its position (fn_801DD0BC) and finishes (see UpdateBody). Behaviour,
 * method, member and constant names are inferred. */
class CityBombBig : public Gadget {
  public:
    /* m_state values (names inferred: attacks only count in STATE_ARMED). */
    enum State { STATE_IDLE, STATE_ARMED, STATE_COUNTDOWN };

    CityBombBig(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~CityBombBig();
    /* State 1, when bit 0 of m_stateFlags is set: passes 1 to the part target's fn_80166E84,
     * stops the sound, passes 2 to the part target's fn_80167180, plays sounds 0x7131 and 0x7132
     * at the character info's position, enters state 2 and clears the bit.
     * State 2: subtracts dt from m_countdown. When the result is below 0, or 0 as fn_8000A950
     * judges it, it calls the model's fn_80162790 and fn_80161880 and the part target's
     * fn_80166F88, passes an effect parameter block for the character info (with the resource that
     * fn_80012700 looks up for key 1 under the name of the model's entry 1000, which fn_8015F7BC
     * finds and the code reads without a test; the lookup result is null when the resource is
     * absent and is not tested; and the y coordinate of the position) to fn_8015D93C, passes the
     * position, 2, 7 and 25 to fn_801DD0BC (a CommonBomb request), plays sound 0x714B there, calls
     * fn_8016B224(1, 0) on the mission that fn_800740E4 returns for 1 when lbl_8057D6A0's byte at
     * 0x125 is set and for 0 otherwise, only when it locks to a non-null pointer, sets the state of
     * its com id to 1 (fn_8016907C), passes 2.2, 3 and 15 to fn_8000D664, releases m_object2,
     * passes its owner to fn_800C9B04 and sets Task flag 0x01. In every state it then advances the
     * model by dt and, when the part target is not null, the part target. */
    virtual void UpdateBody(float dt);
    /* Rereads the owner (ReadOwner), places the model (UpdateModelMatrix) and calls the model's
     * fn_80162EF4 and fn_8016108C. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and com id, restores the health (10) and the
     * countdown (1), enters state 1 and clears m_stateFlags. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position, turned about the y axis by the
     * character info's rotation, and passes that position to m_object (fn_800CCE38). When
     * m_object2 is not null, passes it the position s_objectHeight above the character info's
     * position (fn_802B44C8). */
    void UpdateModelMatrix();
    /* The character info's command handler: command 0x1A enters state 0 unless the state is 2,
     * command 0x1B enters state 1 only from state 0. Every command gets bit 1 of its byte at 0x7
     * set. */
    void OnCommand(CharCommand &command);
    /* The callback that the constructor passes to the attack object's fn_80096B64. Stores 5 in
     * the parameter's field_0x54; in state 1 it stores 1 there instead, subtracts the parameter's
     * field_0x50 from the health and copies its field_0x24 to its field_0x40. When the health is
     * then below 0, or 0 as fn_8000A950 judges it, it stores 3 and, unless bit 0 of m_stateFlags
     * is already set, passes the character info that fn_8015D570 returns for the parameter to
     * fn_8020642C with 0x36 when it is not null, and sets the bit. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target; /* the model's part target */
    /* 0x48 */ boost::shared_ptr<Target> m_object;
    /* 0x50 */ boost::shared_ptr<UnkObject_802B4254> m_object2;
    /* 0x58 */ s32 m_state; /* a State */
    /* 0x5C */ UnkBitFlags_80014E04 m_stateFlags;
    /* 0x60 */ float m_health;
    /* 0x64 */ float m_countdown; /* reduced by each update's time step in state 2 */
    /* 0x68 */ s32 m_comId;       /* the owner's com id (fn_800C9E50) */
    /* 0x6C */ UnkSoundHandle_801D3DC8 m_sound;
};
CHECK_SIZEOF(CityBombBig, 0x88);

#endif

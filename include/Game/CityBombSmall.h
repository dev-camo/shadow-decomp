#ifndef GAME_CITYBOMBSMALL_H
#define GAME_CITYBOMBSMALL_H

#include "Game/CharCommand.h"
#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/VacuumObject.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* CityBombSmall (class name from RTTI; the gadget's descriptor is "CITY_BOMB_SMALL"): the
 * CITYBOMBSMALL.BIN model with one gadget parameter, "Range (point)" (m_range). Reading the owner
 * puts it in STATE_WAITING with a health of 5 and a countdown of 1. In STATE_WAITING,
 * FLAG_TRIGGERED gets set by a character within m_range (see UpdateBody) or by the attack after
 * which the health is below 0, or 0 as fn_8000A950 judges it (see OnAttack); the update in
 * STATE_WAITING that finds the flag set (the same one, when a character test sets it) starts the
 * countdown (STATE_COUNTDOWN). When the countdown runs out, the gadget requests a CommonBomb at
 * its position (fn_801DD0BC) and finishes. A VacuumCommand in STATE_WAITING with the flag clear
 * creates a VacuumCityBombSmall, which keeps its own references to the model, m_object2 and the
 * part target when its allocation succeeds; the gadget then releases its references and finishes
 * (see OnCommand). Behaviour, method, member and constant names are inferred. */
class CityBombSmall : public Gadget {
  public:
    /* m_state values (names inferred). */
    enum State { STATE_WAITING, STATE_COUNTDOWN, STATE_VACUUMED };
    /* m_stateFlags bits (name inferred). */
    enum Flag { FLAG_TRIGGERED = 0 };

    CityBombSmall(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~CityBombSmall();
    /* STATE_WAITING: when lbl_8057D6A0's word at 0xAC is 8 and fn_801DCB14 returns an entry for
     * 0x91, and that entry's character info locks to a non-null pointer, sets FLAG_TRIGGERED when
     * the length of the character info's position minus that one's (fn_8040CF50) is below
     * m_range. Then it makes the same test with the character info nearest to the position
     * (fn_800A8A7C), when that one locks to a non-null pointer. When FLAG_TRIGGERED is set (by
     * these tests or by OnAttack), it passes 2 to the part target's fn_80167180, plays sound
     * 0x7131 at the character info's position, enters STATE_COUNTDOWN and clears the flag.
     * STATE_COUNTDOWN: subtracts dt from m_countdown. When the result is below 0, or 0 as
     * fn_8000A950 judges it, it calls the model's fn_80162790 and fn_80161880 and the part
     * target's fn_80166F88, passes an effect parameter block for the character info (with the
     * resource that fn_80012700 looks up for key 1 under the name of the model's entry 1000, which
     * fn_8015F7BC finds and the code reads without a test; the lookup result is null when the
     * resource is absent and is not tested; and the y coordinate of the position) to fn_8015D93C,
     * passes the position, 1, 7 and 8 to fn_801DD0BC (a CommonBomb request), plays sound 0x714C
     * there, sets the state of its com id to 1 (fn_8016907C), releases m_object2, passes its owner
     * to fn_800C9B04 and sets Task flag 0x01.
     * In every state it then advances the model by dt when the model is not null, and the part
     * target when the part target is not null. */
    virtual void UpdateBody(float dt);
    /* Rereads the owner (ReadOwner) and places the model (UpdateModelMatrix); when the model is
     * not null, it calls the model's fn_80162EF4 and fn_8016108C. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation, com id and parameter block (m_range, when the
     * block's size is 4), restores the health (5) and the countdown (1), enters STATE_WAITING and
     * clears m_stateFlags. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model, when it is not null, at the character info's position, turned about the
     * y axis by the character info's rotation; passes that position to m_object (fn_800CCE38)
     * when m_object is not null; and when m_object2 is not null, passes it the position
     * s_object2Height above the character info's position (fn_802B44C8). */
    void UpdateModelMatrix();
    /* The character info's command handler. Command 0x14 (a VacuumCommand), only in
     * STATE_WAITING with FLAG_TRIGGERED clear: creates a VacuumCityBombSmall with the command, the
     * model, m_object2 and the part target at the character info's position and rotation
     * (fn_802F46E4; when the task's allocation fails, no task retains the vacuum object, and the
     * creator releases it, with its references, before it returns), releases the gadget's own
     * references to the model, the part target and m_object2 and enters STATE_VACUUMED;
     * calls fn_8016B224(1, 0) on the mission that fn_800740E4 returns for 2, only when it locks to
     * a non-null pointer; sets the state of its com id to 1 (fn_8016907C); passes the character
     * info that fn_800A8C54 returns for 0, when it locks to a non-null pointer, to fn_8020642C
     * with 0x38; passes its owner to fn_800C9B04 and sets Task flag 0x01. Every command gets bit 1
     * of its byte at 0x7 set. */
    void OnCommand(CharCommand &command);
    /* The callback that the constructor passes to the attack object's fn_80096B64. Stores 5 in
     * the parameter's field_0x54; in STATE_WAITING it stores 1 there instead, subtracts the
     * parameter's field_0x50 from the health and copies its field_0x24 to its field_0x40. When
     * the health is then below 0, or 0 as fn_8000A950 judges it, it stores 3 and, unless
     * FLAG_TRIGGERED is already set, passes the character info that fn_8015D570 returns for the
     * parameter to fn_8020642C with 0x37 when it is not null, and sets the flag. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target; /* the model's part target */
    /* 0x48 */ float m_range;                                  /* the "Range (point)" parameter */
    /* 0x4C */ boost::shared_ptr<Target> m_object;
    /* Created by the constructor only when lbl_8057D6A0's word at 0xAC is 12. */
    /* 0x54 */ boost::shared_ptr<UnkObject_802B4254> m_object2;
    /* 0x5C */ s32 m_state; /* a State */
    /* 0x60 */ UnkBitFlags_80014E04 m_stateFlags;
    /* 0x64 */ float m_health;
    /* 0x68 */ float m_countdown; /* reduced by each update's time step in STATE_COUNTDOWN */
    /* 0x6C */ s32 m_comId;       /* the owner's com id (fn_800C9E50) */
};
CHECK_SIZEOF(CityBombSmall, 0x70);

/* VacuumCityBombSmall (class name from RTTI, with VacuumObject as its only base): the
 * CITY_BOMB_SMALL gadget's model, second object and part target while a character pulls it in
 * (inferred from the names). Behaviour and member names are inferred. */
class VacuumCityBombSmall : public VacuumObject {
  public:
    VacuumCityBombSmall(VacuumCommand *command, const boost::shared_ptr<GadgetModelInfo> &model,
                        const boost::shared_ptr<UnkObject_802B4254> &object2,
                        const boost::shared_ptr<UnkTarget_80166AF4> &target);
    virtual ~VacuumCityBombSmall();
    /* Advances the base object by dt. When the model is not null, places it at the object's
     * position (fn_802382B0) turned by its rotation (fn_80238248: y, then x, then z) and advances
     * it by dt; when m_object2 is not null, passes it the object's position plus (0,
     * s_object2Height, 0) (fn_802B44C8); when the part target is not null, advances it by dt. */
    virtual void Update(float dt);
    /* Empty, as VacuumObject's own implementation is; the vtable still points at this class's
     * copy (0x802F4924). */
    virtual void fn_80238180();

    /* 0x08 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x10 */ boost::shared_ptr<UnkObject_802B4254> m_object2;
    /* 0x18 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
};
CHECK_SIZEOF(VacuumCityBombSmall, 0x20);

#endif

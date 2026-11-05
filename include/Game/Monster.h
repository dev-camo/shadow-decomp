#ifndef GAME_MONSTER_H
#define GAME_MONSTER_H

#include "Game/CharaColliAttack.h"
#include "Game/CharaColliBase.h"
#include "Game/Gadget.h"
#include "Game/GadgetModelInfo.h"
#include "Game/MonsterCharInfo.h"
#include "Game/PathCtrl.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table. ReadOwner scales all four by 10
 * (fn_8005AE20) after reading them. */
struct MonsterParams {
    /* 0x0 */ float minSpeed; /* "MIN_SPPED m/sec" */
    /* 0x4 */ float maxSpeed; /* "MAX_SPPED m/sec" */
    /* 0x8 */ float accel;    /* "ACCEL m/sec^2" */
    /* 0xC */ float decel;    /* "DECEL m/sec^2" */
};
CHECK_SIZEOF(MonsterParams, 0x10);

/* Monster (class name from RTTI; the gadget's descriptor has the same name): the MONSTER.BIN model
 * on path 0x22. The MonsterControl gadgets pass it their kind ("0:START 1:MISS 2:END") through its
 * MonsterCharInfo. After START and its start motion it moves along the path. Its target speed is
 * the minimum speed while the distance of the character that fn_800A8B74 returns along the
 * monster's negative z axis, less the probe's depth, is below m_leadDistance, and the maximum speed
 * otherwise. Each update that finds that character moves it along the path by a step made from the
 * length of its previous displacement: the step moves towards the target speed multiplied by the
 * current dt, by at most the acceleration (the deceleration when the step shrinks) multiplied by dt
 * squared. It passes its motion's events 0x5000 and 0x5001 to its target object and the sound
 * player. An attack with bit 0x10 of the attack object's word at 0x40, from an attacker that
 * fn_8015D4E4 accepts, finishes it (behaviour inferred). Method names marked "(name inferred)" are
 * not original; all member names are inferred. */
class Monster : public Gadget {
  public:
    Monster(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Monster();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    virtual bool IsFinished();

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the parameters from the owner and scales them (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position and rotation (name inferred). */
    void UpdateModelMatrix();
    /* Acts on the kind of a MonsterControl gadget (name inferred): 0 (START) starts a waiting
     * monster, 1 (MISS) takes 75 units off m_leadDistance of a moving one and 2 (END) finishes it;
     * each plays sound 0x744F when it acts. */
    void OnControl(int kind);
    /* The callback of the probe's attack object (name inferred). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<MonsterCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ MonsterParams m_params;
    /* 0x50 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
    /* 0x58 */ boost::shared_ptr<SonicteamUSA::System::PathCtrl> m_path;
    /* 0 waits for START, 1 plays the start motion, 2 moves along the path (inferred). */
    /* 0x60 */ s32 m_state;
    /* UpdateBody compares the character's distance along the monster's negative z axis, less the
     * probe's depth, with this: below it the target speed is the minimum speed, otherwise the
     * maximum speed. The length of the previous displacement then moves towards the target speed
     * multiplied by dt, by at most the acceleration or deceleration multiplied by dt squared, and
     * gives the step along the path; 150 units at first. */
    /* 0x64 */ float m_leadDistance;
    /* 0x68 */ u8 m_started; /* cleared by START; set by the first update in state 1 */
    /* 0x6C */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(Monster, 0x78);

#endif

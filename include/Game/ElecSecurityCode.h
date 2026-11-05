#ifndef GAME_ELECSECURITYCODE_H
#define GAME_ELECSECURITYCODE_H

#include "Game/CharaColliAttack.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rwcore.h"
#include "layout_check.h"

class ElecSecurityCodeGenerator;

/* ElecSecurityCode (class name from RTTI): the objects that an ElecSecurityCodeGenerator keeps in
 * its list and updates. A code shows the SECURITY.BIN model entry of its type (0 to 2) and grows
 * in (state 0), then chases the nearest character info (state 1) until the callback installed with
 * fn_80096B64 runs (state 2). A code of type 0 or 1 then creates two codes of the next type on
 * either side of it, which first fly apart before they chase; a code of type 2 shows a break
 * effect instead. Either way the generator deletes the code (behaviour inferred). Method names
 * marked "(name inferred)" are not original. It has no virtual functions. */
class ElecSecurityCode {
  public:
    /* The generator passes 0 as splitSide; a code that splits passes 1 and 2. rotation is a
     * reference: a splitting code passes fn_8040F760's result temporary directly, evaluated after
     * the allocation. */
    ElecSecurityCode(s32 type, const Fn8040Vector3 &position, ElecSecurityCodeGenerator *generator,
                     const Fn8040Vector2 &rotation, s32 splitSide);
    ~ElecSecurityCode();
    /* Advances the code by dt; false once it is done (behaviour inferred from the generator, which
     * then deletes it). */
    bool fn_80156C78(float dt);

    /* The attack object's callback that fn_80096B64 installs (name inferred, as GiantLantern's):
     * while the code chases, it answers the attack with 3 and moves to state 2; otherwise it
     * answers 5. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The first callback passed to fn_80054DD0: stores the character info's field_0x18 (the
     * velocity that Move passes to fn_804103E8), normalised, in param's field_0x34 and its length
     * in field_0x4C; for a zero length, (0, 1, 0) and 1. */
    void fn_80156344(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* The second callback passed to fn_80054DD0; it does nothing. */
    void fn_80156340(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    /* Starts the effect of model entry 9 at the character info (name inferred, as ElecFireWall's).
     */
    void ShowBreakEffect();
    /* Finds the frames of the type's three model parts and turns them about their y axes at
     * different speeds (name inferred). */
    void RotateParts(float dt);
    /* Places the model at the character info with its yaw and the scale, and moves the target
     * above it (name inferred). */
    void UpdateModelMatrix();
    /* Advances the character info's position and velocity (fn_804103E8), then moves a sphere of
     * the type's radius from the old position to the new one through fn_8026EE9C, with
     * fn_80156BA8 as the predicate (name inferred). */
    void Move(float dt);
    /* State 0 of a code created by a split: sets the heading 0.85 pi to one side of the direction
     * of the nearest character info, copies that character info's acceleration (field_0x24, role
     * inferred) to the code's own and, unless the two positions coincide, adds a component of 75
     * along the heading (name inferred). */
    void MoveApart(float dt);
    /* State 1: turns the heading towards the nearest character info by at most a fixed step,
     * copies that character info's acceleration (field_0x24, role inferred) to the code's own
     * and, unless the two positions coincide, adds a component of 50 along the heading (name
     * inferred). */
    void Chase(float dt);

    /* 0x000 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x008 */ GadgetModelInfo *m_model;
    /* 0x00C */ boost::shared_ptr<Target> m_target;
    /* 0x014 */ s32 m_type; /* 0 to 2: the model entry and the radius */
    /* 0x018 */ s32 m_state;
    /* State 0's time: dt is added each update; the scale follows it up to 1, where the code moves
     * to state 1. */
    /* 0x01C */ float m_time;
    /* Grows from (0, 3, 0) to (1, 1, 1) in state 0. */
    /* 0x020 */ Fn8040Vector3 m_scale;
    /* The frames of the type's model parts and copies of their matrices, taken when they are
     * found; RotateParts turns each by its angle and applies the copy after it. */
    /* 0x02C */ RwFrame *m_partFrames[3];
    /* 0x038 */ RwMatrix m_partMatrices[3];
    /* 0x0F8 */ float m_partAngles[3];
    /* The generator passed to the constructor; a code that splits adds the new codes to this
     * generator's list and assigns the generator's field_0x3C. */
    /* 0x104 */ ElecSecurityCodeGenerator *m_generator;
    /* The pitch and the yaw that the code moves along (inferred). */
    /* 0x108 */ Fn8040Vector2 m_rotation;
    /* 0x110 */ boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> m_attack;
    /* 0x118 */ s32 m_splitSide; /* 1 or 2 for the side MoveApart heads to, 0 for none */
};
CHECK_SIZEOF(ElecSecurityCode, 0x11C);

#endif

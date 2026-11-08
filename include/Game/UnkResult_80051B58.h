#ifndef GAME_UNKRESULT_80051B58_H
#define GAME_UNKRESULT_80051B58_H

#include "Game/CharaColliAttack.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;
class UnkParams_80051D10;

/* The object that fn_8005AFE4 returns by value (placeholder name; the original class is unknown).
 * Its constructor (0x80051B58) copies the vector at 0x805E07A0 into the three vectors, constructs
 * the weak pointer, clears the words at 0x2C and 0x30 and the byte at 0x35 and stores its argument
 * in the byte at 0x34; its copy constructor (0x80052844) also copies the word at 0x38, which gives
 * the size. Only the copy constructor and the destructor (0x800528C4), which destroys the weak
 * pointer, are declared here. fn_8005AFE4 sets field_0x34 (and field_0x35) for a hit when attack
 * is empty or fn_800C3ED4 returns false for the hit object, and stops at that hit; the callbacks
 * it runs for the other hits receive the result's address (AttackCallbackParam::field_0x58), and
 * Weapon::Bullet::ChaosSpear::ChaosSpear's second callback sets field_0x34 to 1 for the attack
 * kinds 1, 3 and 5 (behaviour from the bodies). */
class UnkResult_80051B58 {
  public:
    UnkResult_80051B58(const UnkResult_80051B58 &other);
    ~UnkResult_80051B58();

    /* 0x00 */ Fn8040Vector3 field_0x0;
    /* 0x0C */ Fn8040Vector3 field_0xC;
    /* 0x18 */ Fn8040Vector3 field_0x18;
    /* 0x24 */ boost::weak_ptr<CharInfo> field_0x24;
    /* 0x2C */ u32 field_0x2C;
    /* 0x30 */ u32 field_0x30;
    /* 0x34 */ u8 field_0x34;
    /* 0x35 */ u8 field_0x35;
    /* 0x38 */ u32 field_0x38;
};
CHECK_SIZEOF(UnkResult_80051B58, 0x3C);

extern "C" {
/* Tests attack against the objects that fn_80021FE4 finds along the segment from start to end
 * (flags 3, with arg0 as its next argument) and returns a result object (behaviour inferred; the
 * meanings of arg0 and of the result are unknown). */
UnkResult_80051B58
fn_8005AFE4(int arg0, const Fn8040Vector3 *start, const Fn8040Vector3 *end,
            const boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> &attack);
/* Returns the result of the function at 0xC of the record that fn_80051C34 uses for kind, called
 * with params and arg2, when the pointer and that function are set, and otherwise a result built
 * by the constructor at 0x80051B58 with 0 (behaviour from its body). The GunLift vehicle passes
 * 0x1C and 5.0f. */
UnkResult_80051B58 fn_80051AF0(int kind, const UnkParams_80051D10 &params, float arg2);
}

#endif

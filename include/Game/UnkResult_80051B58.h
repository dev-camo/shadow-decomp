#ifndef GAME_UNKRESULT_80051B58_H
#define GAME_UNKRESULT_80051B58_H

#include "Game/CharaColliAttack.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;

/* The object that fn_8005AFE4 returns by value (placeholder name; the original class is unknown).
 * Its constructor (0x80051B58) copies the vector at 0x805E07A0 into the three vectors, constructs
 * the weak pointer, clears the words at 0x2C and 0x30 and the byte at 0x35 and stores its argument
 * in the byte at 0x34; its copy constructor (0x80052844) also copies the word at 0x38, which gives
 * the size. Only the destructor (0x800528C4), which destroys the weak pointer, is declared here. */
class UnkResult_80051B58 {
  public:
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
}

#endif

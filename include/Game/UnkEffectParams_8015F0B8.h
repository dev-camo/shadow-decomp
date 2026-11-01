#ifndef GAME_UNKEFFECTPARAMS_8015F0B8_H
#define GAME_UNKEFFECTPARAMS_8015F0B8_H

#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;
struct RpClump;

/* A 0x5C-byte parameter block, constructed in many places in the game (placeholder name; the
 * original class is unknown). For a kind below 0x20 the constructor first copies a 0x5C-byte entry
 * of a table; then it stores fn_801EB304(fn_8001C4F8(), 11) at 0x00 and copies the position, the
 * rotation and the word at 0x30 of the character info (behaviour inferred). Only the fields that
 * ThreatObj sets are mapped. */
struct UnkEffectParams_8015F0B8 {
    UnkEffectParams_8015F0B8(const boost::shared_ptr<CharInfo> &charInfo, int kind);

    /* 0x00 */ u8 field_0x0[0x34];
    /* 0x34 */ float field_0x34;
    /* 0x38 */ u8 field_0x38[0x40 - 0x38];
    /* 0x40 */ float field_0x40;
    /* 0x44 */ RpClump *field_0x44;
    /* 0x48 */ u8 field_0x48[0x4A - 0x48];
    /* 0x4A */ u8 field_0x4A;
    /* 0x4B */ u8 field_0x4B[0x54 - 0x4B];
    /* 0x54 */ u8 field_0x54;
    /* 0x55 */ u8 field_0x55[0x5C - 0x55];
};
CHECK_SIZEOF(UnkEffectParams_8015F0B8, 0x5C);

extern "C" {
/* Takes a filled parameter block (purpose unknown). */
void fn_8015D93C(UnkEffectParams_8015F0B8 *params);
}

#endif

#ifndef GAME_UNKCHARINFO_800999C0_H
#define GAME_UNKCHARINFO_800999C0_H

#include "Game/CharInfo.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>

/* A CharInfo subclass with a placeholder name, taken from the address of the copy constructor of
 * its shared pointer. Gadgets and player commands cast the shared CharInfo of the character they
 * look at to it, and the TornadoCollision gadget reads a bit set at 0xA8. Only the prefix up to
 * that bit set is mapped, and the size is unknown. */
class UnkCharInfo_800999C0 : public CharInfo {
  public:
    /* 0x64 */ u8 field_0x64[0xA8 - 0x64];
    /* 0xA8 */ u32 field_0xA8; /* the first word of a bit set; its other words are not mapped */
};

extern "C" {
/* True when bit number bit of the bit set starting at bits is set (behaviour inferred). */
bool fn_8006CD78(const u32 *bits, int bit);
/* Return fn_8006CD78 for bit 10 (fn_800B78B0), bit 15 (fn_800B7938) or bit 2 (fn_800B7AD0) of
 * the bit set at 0xA8 of the object that fn_80074630 and fn_8006D714 derive from charInfo, or
 * false when there is none (behaviour inferred). */
bool fn_800B78B0(const boost::shared_ptr<CharInfo> &charInfo);
bool fn_800B7938(const boost::shared_ptr<CharInfo> &charInfo);
bool fn_800B7AD0(const boost::shared_ptr<CharInfo> &charInfo);
}

#endif

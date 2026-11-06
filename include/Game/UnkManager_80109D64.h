#ifndef GAME_UNKMANAGER_80109D64_H
#define GAME_UNKMANAGER_80109D64_H

#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include <vector>
#include "layout_check.h"

class UnkMessageData_80049E1C;

/* A 12-byte record of the manager's vector (placeholder name). fn_801FAF00 passes records holding
 * its id argument, its second argument and a running index to fn_801FAD90; fn_801FAAC8 compares
 * 1000 times the first word plus 100 times the second with the id it is given (behaviour inferred
 * from the bodies). */
struct UnkMessageRequest_801FAF00 {
    /* 0x0 */ s32 field_0x0;
    /* 0x4 */ s32 field_0x4;
    /* 0x8 */ s32 field_0x8;
};
CHECK_SIZEOF(UnkMessageRequest_801FAF00, 0xC);

/* The object that fn_80109D64 returns (placeholder name; the original class is unknown): a static
 * object that fn_80109D64 constructs on first use and registers for destruction. Its constructor
 * (0x801FB180, USA (GUPE8P)) constructs the vector and the weak pointer, then calls 0x801FBDA8 with
 * 0x40, whose error message is "vector::reserve length error". Its destructor (0x80109DD0) only
 * destroys the weak pointer and the vector. The HINTRING gadget passes it to fn_801FAF00 and
 * fn_801FAAC8. */
class UnkManager_80109D64 {
  public:
    UnkManager_80109D64();

    /* 0x00 */ std::vector<UnkMessageRequest_801FAF00> field_0x0;
    /* fn_801FAF00 locks it and passes the object to fn_80049E1C (type inferred). */
    /* 0x0C */ boost::weak_ptr<UnkMessageData_80049E1C> field_0xC;
    /* fn_801FAAC8 compares ids with 1000 times this word plus 100 times the next one. */
    /* 0x14 */ s32 field_0x14;
    /* 0x18 */ s32 field_0x18;
    /* 0x1C */ u8 field_0x1C[0x24 - 0x1C];
    /* fn_801FAF00 returns -1 when this word is non-zero. */
    /* 0x24 */ s32 field_0x24;
    /* fn_801FAF00 returns -1 when the found entry's word at 0x8 is less than this word. */
    /* 0x28 */ s32 field_0x28;
    /* The size is the extent of the static object in .bss (0x8057FA58-0x8057FA90 in the USA build);
     * whether its last bytes are padding is not established. */
    /* 0x2C */ u8 field_0x2C[0x38 - 0x2C];

  private:
    /* Copying has not been recovered. */
    UnkManager_80109D64(const UnkManager_80109D64 &);
    UnkManager_80109D64 &operator=(const UnkManager_80109D64 &);
};
CHECK_SIZEOF(UnkManager_80109D64, 0x38);
CHECK_OFFSETOF(UnkManager_80109D64, field_0x14, 0x14);
CHECK_OFFSETOF(UnkManager_80109D64, field_0x18, 0x18);
CHECK_OFFSETOF(UnkManager_80109D64, field_0x24, 0x24);
CHECK_OFFSETOF(UnkManager_80109D64, field_0x28, 0x28);

#endif

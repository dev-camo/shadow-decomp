#ifndef GAME_CRASHINFOCTRL_H
#define GAME_CRASHINFOCTRL_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class UnkResource_80012700;
struct UnkCrashRecord_802087F4;
struct UnkResourceRecord_80208848;

/* One 0x40-byte entry of the table that CrashInfoCtrl::fn_802086C0 searches (placeholder name;
 * the table is the pair of words at 0x0 and 0x4 of the resource). Only the members that
 * SuperComputer reads are mapped. */
struct UnkCrashEntry_802086C0 {
    /* 0x00 */ const char *name;     /* compared with the record's 0x18 string (name inferred) */
    /* 0x04 */ const char *fileName; /* SuperComputer loads it with fn_80012700 (name inferred) */
    /* 0x08 */ u8 field_0x8[0x18 - 0x8];
    /* 0x18 */ Fn8040Vector3 field_0x18; /* SuperComputer adds it to a position */
    /* 0x24 */ u8 field_0x24[0x40 - 0x24];
};
CHECK_SIZEOF(UnkCrashEntry_802086C0, 0x40);

/* The qualified class name comes from its shared-owner RTTI. Namespace placement is inferred. */
namespace GadgetBuilder {
class CrashInfoCtrl {
  public:
    CrashInfoCtrl(UnkResource_80012700 *resource, int id);
    /* Nullable strings at +0xC and +0x8 of the selected record; constness is inferred. */
    const char *fn_802087BC() const;
    const char *fn_802087D8() const;
    /* The table entry whose name equals the string at 0x18 of the selected record, or null
     * (behaviour inferred from its body; constness inferred). */
    const UnkCrashEntry_802086C0 *fn_802086C0() const;
    /* The nullable string at +0x10 of the selected record; constness is inferred. */
    const char *fn_802087A0() const;
    /* The entry of the resource's table at 0x30 whose name equals the string at 0x1C of the
     * selected record, or null (behaviour inferred from its body, which searches as
     * fn_802086C0 does; constness inferred). */
    const UnkResourceRecord_80208848 *fn_80208618() const;
    /* The nullable string at +0x14 of the selected record; constness is inferred. */
    const char *fn_80208784() const;
    /* The float at +0x20 of the selected record, or 1.0 when there is none (behaviour inferred
     * from its body; constness inferred). */
    float fn_802085FC() const;
    /* The nullable string at +0x1C of the selected record (the one fn_80208618 searches for);
     * constness is inferred. */
    const char *fn_80208768() const;

    /* 0x00 */ UnkResource_80012700 *field_0x0;
    /* 0x04 */ const UnkCrashRecord_802087F4 *field_0x4;
};
CHECK_SIZEOF(CrashInfoCtrl, 0x8);

/* Original free/static spelling is unknown; namespace placement is inferred. */
boost::shared_ptr<CrashInfoCtrl> fn_80208978(UnkResource_80012700 *resource, int id);
bool fn_80208BB8(UnkResource_80012700 *resource, int id);
} // namespace GadgetBuilder

#endif

#ifndef GAME_CRASHINFOCTRL_H
#define GAME_CRASHINFOCTRL_H

#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class UnkResource_80012700;
struct UnkCrashRecord_802087F4;

/* The qualified class name comes from its shared-owner RTTI. Namespace placement is inferred. */
namespace GadgetBuilder {
class CrashInfoCtrl {
  public:
    CrashInfoCtrl(UnkResource_80012700 *resource, int id);
    /* Nullable strings at +0xC and +0x8 of the selected record; constness is inferred. */
    const char *fn_802087BC() const;
    const char *fn_802087D8() const;

    /* 0x00 */ UnkResource_80012700 *field_0x0;
    /* 0x04 */ const UnkCrashRecord_802087F4 *field_0x4;
};
CHECK_SIZEOF(CrashInfoCtrl, 0x8);

/* Original free/static spelling is unknown; namespace placement is inferred. */
boost::shared_ptr<CrashInfoCtrl> fn_80208978(UnkResource_80012700 *resource, int id);
bool fn_80208BB8(UnkResource_80012700 *resource, int id);
} // namespace GadgetBuilder

#endif

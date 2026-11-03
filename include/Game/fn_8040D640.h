#ifndef GAME_FN_8040D640_H
#define GAME_FN_8040D640_H

#include <memory>

#include "layout_check.h"

namespace Sonicteam {
namespace System {
namespace Peripheral {

/* Name from RTTI. This partial declaration exposes only virtual destruction.
 * The checked size is this interface's compiler footprint; the original complete
 * object size, additional members and remaining virtual slots are unrecovered. */
class IMouseManagerImp {
  public:
    virtual ~IMouseManagerImp();
};
CHECK_SIZEOF(IMouseManagerImp, 0x4);

} // namespace Peripheral
} // namespace System
} // namespace Sonicteam

CHECK_SIZEOF(std::auto_ptr<Sonicteam::System::Peripheral::IMouseManagerImp>, 0x4);

/* A 56-byte element. The native constructor and float setter establish the field at
 * 0x24; the surrounding storage intervals do not describe recovered member boundaries. */
struct Fn8040D708Entry {
    Fn8040D708Entry();
    ~Fn8040D708Entry();

    void fn_8040D850(float value);

    /* 0x00 */ char field_0x0[0x24];
    /* 0x24 */ float field_0x24;
    /* 0x28 */ char field_0x28[0x10];
};
CHECK_SIZEOF(Fn8040D708Entry, 0x38);
CHECK_OFFSETOF(Fn8040D708Entry, field_0x24, 0x24);

/* Original owner name unknown. Construction and destruction establish both array counts. */
class Fn8040D640Owner {
  public:
    Fn8040D640Owner();
    ~Fn8040D640Owner();

    Fn8040D708Entry *fn_8040D708(unsigned int index);
    void fn_8040D640(std::auto_ptr<Sonicteam::System::Peripheral::IMouseManagerImp> source,
                     unsigned int index);

    /* 0x00 */ Fn8040D708Entry entries[1];
    /* 0x38 */ std::auto_ptr<Sonicteam::System::Peripheral::IMouseManagerImp> slots[1];
};
CHECK_SIZEOF(Fn8040D640Owner, 0x3C);
CHECK_OFFSETOF(Fn8040D640Owner, slots, 0x38);

#endif /* GAME_FN_8040D640_H */

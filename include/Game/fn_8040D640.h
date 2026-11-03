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

/* A 56-byte element. Integer fields preserve the observed word stores; their
 * signedness and meaning are unrecovered. The byte intervals describe unrecovered
 * storage, not original member boundaries. */
struct Fn8040D708Entry {
    Fn8040D708Entry();
    ~Fn8040D708Entry();

    void fn_8040D850(float value);

    /* 0x00 */ unsigned int field_0x0;
    /* 0x04 */ unsigned int field_0x4;
    /* 0x08 */ unsigned int field_0x8;
    /* 0x0C */ char field_0xC[0x8];
    /* 0x14 */ unsigned int field_0x14;
    /* 0x18 */ unsigned int field_0x18;
    /* 0x1C */ unsigned int field_0x1C;
    /* 0x20 */ float field_0x20;
    /* 0x24 */ float field_0x24;
    /* 0x28 */ char field_0x28[0x8];
    /* 0x30 */ char field_0x30[0x8];
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

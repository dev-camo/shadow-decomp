#ifndef GAME_UNKTARGET_80166AF4_H
#define GAME_UNKTARGET_80166AF4_H

#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

struct UnkModelPart_8015FA78;

/* The 0x10-byte object that fn_80166AF4 creates for gadgets such as BDTrue and CityLaser, with a
 * placeholder name: the gadget keeps it alive through a shared pointer, passes a weak pointer to
 * it to the model's part callback and gives it the update's time step. Its members are not mapped
 * yet. */
class UnkTarget_80166AF4 {
  public:
    /* Called with -1 once the model is set up. Applies to every entry when arg0 is -1, otherwise
     * only to entries whose word at 0xC equals arg0 (behaviour inferred). */
    void fn_80167010(int arg0);
    void fn_8016761C(float dt);
    /* Called before the object is released (behaviour inferred from the CityLaser gadget). */
    void fn_80166F88();

    /* 0x00 */ u8 field_0x0[0x10];
};
CHECK_SIZEOF(UnkTarget_80166AF4, 0x10);

/* A weak pointer to UnkTarget_80166AF4 wrapped in a class of its own, with a placeholder name. The
 * constructor at 0x800CC6C8 and the destructor at 0x800CC62C call the weak_ptr constructor
 * (fn_800CC6F8) and destructor (fn_800CC678) on the same address, which a bare weak_ptr local does
 * not do: it compiles to inline stores and a weak_count destructor at offset 4. */
struct UnkWeakHolder_800CC6C8 {
    /* 0x0 */ boost::weak_ptr<UnkTarget_80166AF4> ptr;
};
CHECK_SIZEOF(UnkWeakHolder_800CC6C8, 0x8);

extern "C" boost::shared_ptr<UnkTarget_80166AF4> fn_80166AF4(void);
/* The part callback that gadgets pass to GadgetModelInfo::fn_8015FA78 together with an
 * UnkWeakHolder_800CC6C8 as the user data (behaviour inferred). */
extern "C" int fn_80169D00(UnkModelPart_8015FA78 part, const void *userData);

#endif

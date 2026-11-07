#ifndef GAME_UNKTARGET_80166AF4_H
#define GAME_UNKTARGET_80166AF4_H

#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

struct RpClump;
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
    /* Acts on the entries whose word at 0x8 has bit 0 clear and, unless arg0 is -1, whose word at
     * 0xC equals arg0; fn_80167010 uses the same selector on the entries with that bit set
     * (behaviour inferred). FallBuildingHolder calls it with -1. */
    void fn_80167448(int arg0);
    /* Called before fn_80167010 and before the object is released (behaviour inferred; purpose
     * unknown). */
    void fn_80166F88();
    /* Does what fn_80167010 does to the same entries, except that it does not set bit 2 of an
     * entry's word at 0x8 after fn_80425E68 (purpose unknown). GravityChangeZone calls it with -1
     * once the model is set up. */
    void fn_80167180(int arg0);
    /* Calls fn_80166D90 on the entries whose word at 0xC equals arg0 (behaviour inferred from
     * its body; purpose unknown). */
    void fn_80166E20(int arg0);
    /* Calls fn_80166D90 on every entry, then destroys the entries and clears the collection
     * (fn_8016847C; behaviour inferred from the bodies; purpose unknown). The DOOR gadget calls it
     * when a key door unlocks. */
    void fn_80166D14();
    /* For each 0x4C-byte entry whose weak pointer at 0x44 to an Effect::System::EffectElement
     * can be locked, clear (fn_80166B70) or set (fn_80166C70) bit 3 of the element's word at 0x18
     * (behaviour inferred from their bodies). The BASwitch gadget calls fn_80166B70 for the part
     * it shows and fn_80166C70 for the part it hides. */
    void fn_80166B70();
    void fn_80166C70();
    /* For every entry when arg0 is -1, otherwise for the entries whose word at 0xC equals arg0, and
     * only when the entry's effect element at 0x44 has expired: creates a new element when the
     * entry's word at 0x0 is 0 (fn_80168C48) or 1 (fn_8015D7DC), then passes the element and the
     * entry's data at 0x14 to fn_80425E68 and sets bit 2 of the entry's word at 0x8, or calls
     * fn_800A31F0 when there is no element (behaviour inferred from its body). */
    void fn_801672E4(int arg0);

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
/* Allocates an UnkTarget_80166AF4, constructs it with clump (fn_8016780C) and returns it in a
 * shared pointer (behaviour inferred from its body). */
extern "C" boost::shared_ptr<UnkTarget_80166AF4> fn_8016690C(RpClump *clump);
/* The part callback that gadgets pass to GadgetModelInfo::fn_8015FA78 together with an
 * UnkWeakHolder_800CC6C8 as the user data (behaviour inferred). */
extern "C" UnkModelPart_8015FA78 *fn_80169D00(UnkModelPart_8015FA78 *part, void *userData);
/* Returns whether the word at 0x8 of the object that fn_80074470 returns is 4 (behaviour from its
 * body; what the object and the value 4 are is unknown). The EGG_BALLOON gadget counts its fire
 * time and shot timer down only while it returns false. */
extern "C" bool fn_80169CD0(void);

#endif

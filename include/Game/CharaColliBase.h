#ifndef GAME_CHARACOLLIBASE_H
#define GAME_CHARACOLLIBASE_H

#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>
#include "layout_check.h"

namespace SonicteamUSA {
namespace System {
namespace Range {
/* The base class of the ranges RangeSphere, RangeBox, RangeCapsule, RangeCylinder and RangePlane
 * (class names from RTTI) that the factory at 0x80037668 creates. */
class RangeBase;
} // namespace Range

namespace CharaColli {

/* CharaColliBase (class name from RTTI, with no bases): the collision object that the probe
 * factories at 0x800C7A84-0x800C7EA8 return and that the touched-object lists refer to. Its
 * constructor (0x8003797C) stores the vtable at 0x0 and its two halfword arguments at 0x4 and 0x6,
 * constructs the hit list at 0xC, sets the bytes at 0x28, 0x29 and 0x2A to 1 and clears the byte
 * at 0x2C; the derived classes' own members start at 0x30.
 *
 * The 32 virtual functions are declared in the order of the 0x88-byte vtable at 0x8051CDF0. The
 * slots that the vtable leaves null are pure. Slot names are inferred where a comment says so;
 * the UnkSlot placeholders are not understood. The parameter and return types of the non-pure
 * slots and of slots 8 and 9 follow their base or derived implementations and callers; the other
 * pure slots' signatures are not known unless a comment gives them. */
class CharaColliBase {
  public:
    CharaColliBase(s16 arg0, s16 arg1);

    /* The number of ranges the probe holds (type inferred): CharaColliGeneral returns its halfword
     * at 0x5C (0x800C18BC) and CharaColliBody returns 1 (0x800C07C8); the caller at 0x800C2434 uses
     * the low 16 bits as the bound of a loop over slot 8. */
    virtual u16 UnkSlot0() = 0;
    /* When the byte at 0x2A is clear, sets it and the byte at 0x28 and clears the byte at 0x2B
     * (0x800378FC, which ignores selector). CharaColliGeneral's implementation (0x800C334C) does
     * the same for a selector of -1. For another selector it takes the range that slot 9 returns;
     * when that range's word at 0x20 equals selector and its byte at 0x19 is clear, it clears the
     * byte at 0x2B, sets the range's byte at 0x19 and calls slot 31. */
    virtual void UnkSlot1(int selector);
    /* Copies the byte at 0x2A to 0x2B and clears 0x2A (0x800378E8, which ignores selector).
     * CharaColliGeneral's implementation (0x800C32B0) does the same for a selector of -1. For
     * another selector it takes the range that slot 9 returns; when that range's word at 0x20
     * equals selector and its byte at 0x19 is set, it clears the byte at 0x2B and the range's byte
     * at 0x19 and calls slot 31. */
    virtual void UnkSlot2(int selector);
    /* A pointer to the probe's position (name and behaviour inferred from the CityLaser gadget,
     * which subtracts the nearest point of a segment from the vector it points to). */
    virtual const Fn8040Vector3 *GetPosition() = 0;
    virtual void UnkSlot4() = 0;
    /* Passes out0, out1 and out2 unchanged to the range that slot 8 (for a selector of -2) or
     * slot 9 returns (its vtable offset 0x18) and returns the range's word at 0x14, or -1 when
     * there is no such range (0x800375AC). The ranges store a float through each non-null pointer
     * (RangeBox at 0x8002B394; RangeSphere at 0x800352D8 through the first). */
    virtual int UnkSlot5(float *out0, float *out1, float *out2, int selector);
    /* Returns the vector that the selected range's function at vtable offset 0xC returns, or the
     * vector at 0x805E07A0 when there is no such range (0x80037418). */
    virtual Fn8040Vector3 UnkSlot6(int selector);
    /* Returns 0 in this class and in CharaColliBaseCharInfo (0x80028F5C). */
    virtual int UnkSlot7();
    /* The range with the given index (the loop at 0x800C2188 passes indices below slot 0's count;
     * 0x800C20B8 selects that element of its list). The base implementation passes 0 for the
     * selector -2. Derived implementations (for example 0x800C07E8) return a range they hold. */
    virtual Range::RangeBase *UnkSlot8(int index) = 0;
    /* The range for selector, or 0 when there is none. */
    virtual Range::RangeBase *UnkSlot9(int selector) = 0;
    /* Returns the word that UnkSlot20 stores (0x800C0808 returns the word at 0x1C of the object
     * at 0x5C, where 0x800C0890 stores UnkSlot20's arg0). The Door gadgets pass 0 and hand the
     * result back to UnkSlot20 with bit 0x100 set or cleared; FootingMovable passes each range
     * index below slot 0's count (at most 16) and hands the result back with bit 0x100 set. The
     * signature is inferred from those calls. */
    virtual int UnkSlot10(int arg0) = 0;
    virtual void UnkSlot11() = 0;
    /* Returns the float at 0x8 (0x80037358). */
    virtual float UnkSlot12();
    /* Moves the probe to position (behaviour inferred). */
    virtual void SetPosition(const Fn8040Vector3 *position) = 0;
    /* Sets the probe's rotation (name inferred from RainEffect, which passes its rotation vector).
     * FallRoad passes the address of a vector built in the call; the bytes do not distinguish a
     * pointer from a reference, and the reference spelling is inferred because it reproduces both
     * callers (named-local pointer forms differ by 8-9 words in ResetFromOwner). */
    virtual void SetRotation(const UnkRotation3 &rotation) = 0;
    virtual void UnkSlot15() = 0;
    /* Sets the probe's size (name inferred): TornadoCollision passes its radius as x and 0.0f as
     * y and z, GravityChangeCollision the three half extents of its box. Both pass -2 as arg1,
     * which 0x800374D0 uses as the range selector of slots 8 and 9. */
    virtual void SetSize(float x, int arg1, float y, float z);
    /* The base implementation (0x80037360) passes vector to the object that slot 8 (for a
     * selector of -2) or slot 9 (otherwise, given the selector) returns, then calls slot 31 and
     * sets the byte at 0x29 to 1 (behaviour inferred; the meaning of vector and selector is not
     * understood). */
    virtual void UnkSlot17(const Fn8040Vector3 *vector, int selector);
    virtual void UnkSlot18() = 0;
    virtual void UnkSlot19() = 0;
    /* The ElecBarrier gadget calls it with 0x30302 or 0x30301 (by barrier type) and -1,
     * GravityChangeZone with 0x140100 and -1 on its zone model's probe, ElecCircuitColli with 0x202
     * or 0x40000 and -1, and BombServer with 0x10302 or 0x50100 and -1. ElecCircuitColli and
     * BombServer create their probes with 0x40000 and 0x50100 as the factories' flags word, so it
     * may re-apply a flags word (inferred; purpose unknown). The Door gadgets pass the word that
     * UnkSlot10(0) returned, with bit 0x100 set or cleared, and -1; FootingMovable passes a range
     * index as arg1 and, as arg0, the word that UnkSlot10 returned for that index, first with bit
     * 0x100 set and later unchanged. */
    virtual void UnkSlot20(int arg0, int arg1) = 0;
    virtual void UnkSlot21() = 0;
    virtual void UnkSlot22() = 0;
    virtual void UnkSlot23() = 0;
    virtual void UnkSlot24() = 0;
    virtual void UnkSlot25() = 0;
    /* Called without arguments right after slot 16 by the GravityChangeCollision gadget (purpose
     * unknown). */
    virtual void UnkSlot26() = 0;
    virtual void UnkSlot27() = 0;
    /* At 0x80037920 in the base class. */
    virtual ~CharaColliBase();
    virtual void UnkSlot29() = 0;
    virtual void UnkSlot30() = 0;
    virtual void UnkSlot31() = 0;

    /* The list of the objects the probe touched (name inferred). */
    Collision::CollisionResultList *GetHits() {
        return &hits;
    }

    /* 0x04 */ s16 field_0x4;
    /* 0x06 */ s16 field_0x6;
    /* 0x08 */ float field_0x8; /* not initialised by the constructor; slot 12 returns it */
    /* 0x0C */ Collision::CollisionResultList hits;
    /* 0x28 */ u8 field_0x28;
    /* 0x29 */ u8 field_0x29;
    /* 0x2A */ u8 field_0x2A;
    /* 0x2B */ u8 field_0x2B; /* not initialised by the constructor */
    /* 0x2C */ u8 field_0x2C;
};
CHECK_OFFSETOF(CharaColliBase, field_0x4, 0x4);
CHECK_OFFSETOF(CharaColliBase, hits, 0xC);
CHECK_OFFSETOF(CharaColliBase, field_0x28, 0x28);
CHECK_OFFSETOF(CharaColliBase, field_0x2C, 0x2C);
CHECK_SIZEOF(CharaColliBase, 0x30);

} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA

/* The bit of a probe's flags word (CharaColliBase::UnkSlot10, UnkSlot20) that the Door gadgets
 * and the FootingMovable gadget set while they are transparent; Door clears it again (meaning
 * unknown). A const object: their set branches load 0x100 from their own .sdata2 and Door's clear
 * branch folds it into an rlwinm, while a literal is folded into an ori. */
static const int s_transparentFlag = 0x100;

#endif

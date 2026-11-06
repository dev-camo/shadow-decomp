#ifndef GAME_CAMERAMODEUSER_H
#define GAME_CAMERAMODEUSER_H

#include "Game/fn_8040_vector_types.h"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The record that the camera manager passes to the camera mode's virtual functions (placeholder
 * name after 0x801CCBF4, where the manager copies the shared pointer that holds it; the original
 * class is not established). Its first two words hold a shared pointer to the camera mode that the
 * manager calls with it: the manager loads the first word as the object it calls (for example at
 * 0x80270298), or copies the pointer with 0x80009804 (at 0x802706BC) and calls the copy. Its other
 * members are not mapped. */
struct UnkCameraModeEntry_801CCBF4;

namespace SonicteamUSA {
namespace System {
namespace Camera {

/* SonicteamUSA::System::Camera::CameraMode (class name from RTTI): the base class of
 * CameraModeUser, whose type info (0x805E3470) lists this class's type info (0x805E3468) as its
 * base. The class is polymorphic: fn_80009DC8 converts a CameraMode pointer to a CameraModeUser
 * pointer with __dynamic_cast, passing 0xA8 as the offset of the vtable pointer, and
 * CameraModeUser's constructor (0x80009CC0) stores its vtable at 0xA8 after it has called this
 * class's constructor (0x8000E9BC) and constructs its own first member at 0xAC, so this class ends
 * at 0xAC. None of the members before the vtable pointer are mapped.
 *
 * The virtual functions are declared in the order of this class's vtable (0x8051C598, stored by
 * the constructor and by the destructor at 0x8000E954): its first four entries are null (pure),
 * the fifth is the empty function at 0x80008D48 and the sixth is the destructor. CameraModeUser
 * (0x8051C484), CameraModeMtnViewer (0x8051C40C) and CameraModeStop (0x8051C43C), all with type
 * info names from RTTI, fill the first four and keep the fifth. The UnkSlot names are placeholders.
 * The parameter kinds (a word, a float) follow the implementations and callers given in the
 * comments, but the exact prototypes are inferred: the return types (the callers at 0x801CA5C8
 * and 0x801CA5E0 compare two UnkSlot0 results for equality at 0x801CA5F0, which does not show their
 * signedness, and the callers found for the other slots, for example 0x802706D4 for UnkSlot1, do
 * not use r3), the pointer spelling of the entry parameters and the parameter names. */
class CameraMode {
  public:
    /* 0x00 */ u8 field_0x0[0xA8];
    /* 0xA8: vtable */

    /* Returns a constant per class: -0x3FF in CameraModeMtnViewer (0x80008D40), -0x400 in
     * CameraModeStop (0x800092CC) and -0x3FE in CameraModeUser (0x80009610). The caller at
     * 0x801CA5C8 compares the results of two modes for equality. */
    virtual int UnkSlot0() = 0;
    /* Called through fn_8000E928 (from 0x802706D4) with an entry and a float.
     * CameraModeUser's implementation passes both to its update callback. The two parameters are
     * passed in different register files, so their order is not established. */
    virtual void UnkSlot1(UnkCameraModeEntry_801CCBF4 *entry, float arg1) = 0;
    /* fn_8000E8D8 calls this when the word at 0xA4 is 0, then increments that word. */
    virtual void UnkSlot2(UnkCameraModeEntry_801CCBF4 *entry) = 0;
    /* fn_8000E88C decrements the word at 0xA4 when it is positive and calls this when the word
     * becomes 0. */
    virtual void UnkSlot3(UnkCameraModeEntry_801CCBF4 *entry) = 0;
    /* Does nothing (0x80008D48). The callers at 0x801CAECC and 0x801CAEF4 pass a mask whose bits
     * 0 and 1 come from two flag tests. */
    virtual void UnkSlot4(int flags);
    virtual ~CameraMode();
};
CHECK_SIZEOF(CameraMode, 0xAC);

/* SonicteamUSA::System::Camera::CameraModeUser (class name from RTTI): the camera mode that
 * fn_80009704 creates. Recovered view: the members after the callback and the full extent are not
 * known. */
class CameraModeUser : public CameraMode {
  public:
    /* The arguments of the camera's update event (class name from RTTI). Its layout is not
     * mapped: the code reconstructed so far only passes it by reference. */
    class UpdateEventArgs;

    /* Returns -0x3FE (0x80009610). */
    virtual int UnkSlot0();
    /* When the callback at 0xC4 is not empty, calls it with a 2-word record that holds arg1 at 0x0
     * and entry at 0x4 (0x800099B8). */
    virtual void UnkSlot1(UnkCameraModeEntry_801CCBF4 *entry, float arg1);
    /* When the member at 0xAC (UnkSlot2, 0x8000996C) or 0xB8 (UnkSlot3, 0x800098E4) is not empty,
     * calls it with a 1-word record that holds entry. */
    virtual void UnkSlot2(UnkCameraModeEntry_801CCBF4 *entry);
    virtual void UnkSlot3(UnkCameraModeEntry_801CCBF4 *entry);
    /* The deleting destructor (0x80009A50); 0x8026F928 calls it through the vtable with 1 to
     * delete a mode. */
    virtual ~CameraModeUser();

    /* Two members of another type, constructed by 0x80009D58 (not mapped). */
    /* 0xAC */ u8 field_0xAC[0x18];
    /* Called with the update event arguments. */
    /* 0xC4 */ boost::function1<void, UpdateEventArgs &> m_callback;
};
CHECK_OFFSETOF(CameraModeUser, m_callback, 0xC4);
/* Checks only the recovered view; the complete native extent remains unknown. */
CHECK_SIZEOF(CameraModeUser, 0xD0);

} // namespace Camera
} // namespace System
} // namespace SonicteamUSA

typedef SonicteamUSA::System::Camera::CameraModeUser::UpdateEventArgs CameraUpdateEventArgs;

/* The type of CameraModeUser's callback (name inferred). */
typedef boost::function1<void, CameraUpdateEventArgs &> CameraUpdateCallback;

extern "C" {
/* Creates a camera mode user (behaviour inferred: fn_8026F6A4 creates a camera mode, which is
 * converted with a dynamic cast; the numbers are not understood). */
boost::shared_ptr<SonicteamUSA::System::Camera::CameraModeUser> fn_80009704(int arg0, int arg1);
}

#endif

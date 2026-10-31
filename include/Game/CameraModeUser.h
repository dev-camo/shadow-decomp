#ifndef GAME_CAMERAMODEUSER_H
#define GAME_CAMERAMODEUSER_H

#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>

namespace SonicteamUSA {
namespace System {
namespace Camera {

/* SonicteamUSA::System::Camera::CameraModeUser (class name from RTTI; the class is polymorphic).
 * Its own members and vtable are not mapped, so this body is a placeholder that only hosts the
 * nested event argument classes, whose mangled names need the enclosing class. */
class CameraModeUser {
  public:
    /* The arguments of the camera's update event (class name from RTTI). Only the half word at
     * 0x4, the byte at 0x7 and the vector at 0x14 are used so far; the layout is not mapped
     * further. The member names are inferred: the Coaster gadget's event handler switches on the
     * half word and sets bit 1 of the byte. */
    class UpdateEventArgs {
      public:
        /* 0x00 */ u8 field_0x0[0x4];
        /* 0x04 */ u16 m_type;
        /* 0x06 */ u8 field_0x6;
        /* 0x07 */ u8 m_flags;
        /* 0x08 */ u8 field_0x8[0x14 - 0x8];
        /* 0x14 */ Fn8040Vector3 m_vector;
    };
};

} // namespace Camera
} // namespace System
} // namespace SonicteamUSA

typedef SonicteamUSA::System::Camera::CameraModeUser::UpdateEventArgs CameraUpdateEventArgs;

#endif

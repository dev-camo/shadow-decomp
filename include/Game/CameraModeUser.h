#ifndef GAME_CAMERAMODEUSER_H
#define GAME_CAMERAMODEUSER_H

#include <dolphin/types.h>

namespace SonicteamUSA {
namespace System {
namespace Camera {

/* SonicteamUSA::System::Camera::CameraModeUser (class name from RTTI; the class is polymorphic).
 * Its own members and vtable are not mapped, so this body is a placeholder that only hosts the
 * nested event argument classes, whose mangled names need the enclosing class. */
class CameraModeUser {
  public:
    /* The arguments of the camera's update event (class name from RTTI). Its layout is not
     * mapped: the code reconstructed so far only passes it by reference. */
    class UpdateEventArgs;
};

} // namespace Camera
} // namespace System
} // namespace SonicteamUSA

typedef SonicteamUSA::System::Camera::CameraModeUser::UpdateEventArgs CameraUpdateEventArgs;

#endif

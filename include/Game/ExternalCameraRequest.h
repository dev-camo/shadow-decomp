#ifndef GAME_EXTERNALCAMERAREQUEST_H
#define GAME_EXTERNALCAMERAREQUEST_H

#include "Game/CameraModeUser.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The 0x2C-byte block that the constructors at 0x8000B31C, 0x8000B2C0 and 0x8000B264 build
 * (placeholder name; the original class is unknown). ExternalCameraRequest holds two of them, at
 * 0x10 and 0x40, and copies the blocks fn_801CED24 is given into them; the Coaster gadget passes
 * one to fn_801CEA54. The constructors store the floats at 0x0 and 0x4 (from fn_8000B31C's
 * constant, from the one argument twice, or from the two arguments), clear the word at 0x8 and the
 * 0x10 bytes at 0xC and at 0x1C; the destructor (0x8000B228) does nothing. The size is the
 * distance between two such blocks on the stack of Game/PlayerEffect.cpp's camera demo (USA
 * (GUPE8P) 0x800A5C74, at r1+0x30 and r1+0x5C). Only the two floats are mapped (meanings
 * unknown); the rest is not. */
class UnkCameraParams_8000B31C {
  public:
    UnkCameraParams_8000B31C(float value);
    UnkCameraParams_8000B31C(float value0, float value4);
    ~UnkCameraParams_8000B31C();

    /* 0x00 */ float field_0x0;
    /* 0x04 */ float field_0x4;
    /* 0x08 */ u8 field_0x8[0x2C - 0x8];
};
CHECK_SIZEOF(UnkCameraParams_8000B31C, 0x2C);

namespace PJSCamera {

/* PJSCamera::ExternalCameraRequest (class name from the type name of its shared pointer's counter,
 * boost::detail::sp_counted_base_impl<PJSCamera::ExternalCameraRequest *, ...> at 0x804CF218): the
 * 0x70-byte object that fn_801CED24 creates. Its constructor (0x801C8B9C) keeps a copy of the
 * camera mode pointer at 0x0. Its members are not mapped. */
class ExternalCameraRequest;

} // namespace PJSCamera

extern "C" {
/* Creates an ExternalCameraRequest for the camera mode in a shared pointer. When fn_801CE644 finds
 * an object for the mode's word at 0xA0 in the manager that fn_80099954 returns, the request is
 * passed to that object's fn_801C948C (inferred: the request is registered) and the returned weak
 * pointer refers to it. Otherwise the request is passed to fn_801CEE4C, which swaps it with an
 * empty shared pointer and releases that reference, and the weak pointer is built from the emptied
 * pointer (0x801CEE28), so no request is registered and the result is empty. arg1, arg2, arg4 and
 * arg6 are stored in the request; arg3 and arg5 are copied into it when they are not null. Their
 * meanings are not understood, and the const qualifiers of arg3 and arg5 are inferred. */
boost::weak_ptr<PJSCamera::ExternalCameraRequest>
fn_801CED24(boost::shared_ptr<SonicteamUSA::System::Camera::CameraMode> mode, int arg1, int arg2,
            const UnkCameraParams_8000B31C *arg3, int arg4, const UnkCameraParams_8000B31C *arg5,
            int arg6);
/* When the request still exists and fn_801CE644 finds an object for the word at 0xA0 of the
 * request's camera mode in the manager that fn_80099954 returns, passes the request to that
 * object's fn_801C93B4; then resets the weak pointer (from its body; inferred: the request is
 * withdrawn). The caller's weak pointer is reset in place; the reference spelling is inferred. */
void fn_801CEAD4(boost::weak_ptr<PJSCamera::ExternalCameraRequest> &request);
}

#endif

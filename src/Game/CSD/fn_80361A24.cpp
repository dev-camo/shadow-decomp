/* Two screen descriptions of the Chao::CSD library and the coordinate conversions between them.
 * The original file name is unknown and every name in this file is inferred from behaviour.
 * Definitions are in reverse address order (deferred inlining emits the last definition first).
 * In the E3 2005 prototype (G9SJ8P) SetScreenSize (0x80293BCC) has both size setters inlined,
 * GetScaledScreenOffsetA (0x80293A50) has GetScreenOffsetA and ScaleByScreenA inlined, and none of
 * these three helpers has a standalone copy. */
#include "Game/CSD/Screen.h"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Chao {
namespace CSD {

/* A screen size, its reciprocals, an offset and the width-to-height ratio. Nothing in the
 * library writes the offsets, so they keep the zero the constructor stores. */
struct ScreenInfo {
    /* 0x00 */ f32 width;
    /* 0x04 */ f32 height;
    /* 0x08 */ f32 invWidth;
    /* 0x0C */ f32 invHeight;
    /* 0x10 */ f32 offsetX;
    /* 0x14 */ f32 offsetY;
    /* 0x18 */ f32 aspect;

    ScreenInfo()
        : width(0.0f), height(0.0f), invWidth(0.0f), invHeight(0.0f), offsetX(0.0f), offsetY(0.0f) {
    }
};
CHECK_SIZEOF(ScreenInfo, 0x1C);

/* The library uses the two descriptions in different places; their roles are unknown. */
static ScreenInfo s_screenA;
static ScreenInfo s_screenB;

void SetScreenSize(f32 width, f32 height) {
    SetScreenSizeA(width, height);
    SetScreenSizeB(width, height);
}

void SetScreenSizeA(f32 width, f32 height) {
    s_screenA.width = width;
    s_screenA.height = height;
    s_screenA.invWidth = 1.0f / width;
    s_screenA.invHeight = 1.0f / height;
    s_screenA.aspect = s_screenA.width / s_screenA.height;
}

void SetScreenSizeB(f32 width, f32 height) {
    s_screenB.width = width;
    s_screenB.height = height;
    s_screenB.invWidth = 1.0f / width;
    s_screenB.invHeight = 1.0f / height;
    s_screenB.aspect = s_screenB.width / s_screenB.height;
}

void ToScreenA(f32 *x, f32 *y) {
    *x = s_screenA.width * (*x + s_screenA.offsetX);
    *y = s_screenA.height * (*y + s_screenA.offsetY);
}

void ToScreenB(f32 *x, f32 *y) {
    *x = s_screenB.width * (*x + s_screenB.offsetX);
    *y = s_screenB.height * (*y + s_screenB.offsetY);
}

void FromScreenA(f32 *x, f32 *y) {
    *x *= s_screenA.invWidth;
    *y *= s_screenA.invHeight;
    *x -= s_screenA.offsetX;
    *y -= s_screenA.offsetY;
}

void FromScreenB(f32 *x, f32 *y) {
    *x *= s_screenB.invWidth;
    *y *= s_screenB.invHeight;
    *x -= s_screenB.offsetX;
    *y -= s_screenB.offsetY;
}

void GetScreenSizeA(f32 *width, f32 *height) {
    *width = s_screenA.width;
    *height = s_screenA.height;
}

void GetScreenSizeB(f32 *width, f32 *height) {
    *width = s_screenB.width;
    *height = s_screenB.height;
}

void GetScaledScreenOffsetA(f32 *x, f32 *y) {
    GetScreenOffsetA(x, y);
    ScaleByScreenA(x, y);
}

void GetScreenOffsetA(f32 *x, f32 *y) {
    *x = s_screenA.offsetX;
    *y = s_screenA.offsetY;
}

void ScaleByScreenA(f32 *x, f32 *y) {
    *x *= s_screenA.width;
    *y *= s_screenA.height;
}

f32 GetScreenAspectB() {
    return s_screenB.aspect;
}

} // namespace CSD
} // namespace Chao

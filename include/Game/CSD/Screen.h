#ifndef GAME_CSD_SCREEN_H
#define GAME_CSD_SCREEN_H

#include <dolphin/types.h>

/* The library's two screen descriptions and the coordinate conversions between them. All names
 * are inferred from behaviour; the roles of the two descriptions (A and B) are unknown. */
namespace Chao {
namespace CSD {

/* Sets both descriptions to the same size (the RenderWare initialisation passes its size). */
void SetScreenSize(f32 width, f32 height);
void SetScreenSizeA(f32 width, f32 height);
void SetScreenSizeB(f32 width, f32 height);

/* x = width * (x + offsetX), and likewise for y. */
void ToScreenA(f32 *x, f32 *y);
void ToScreenB(f32 *x, f32 *y);

/* x = x * invWidth - offsetX (x / width - offsetX), and likewise for y. */
void FromScreenA(f32 *x, f32 *y);
void FromScreenB(f32 *x, f32 *y);

void GetScreenSizeA(f32 *width, f32 *height);
void GetScreenSizeB(f32 *width, f32 *height);
void GetScaledScreenOffsetA(f32 *x, f32 *y);
void GetScreenOffsetA(f32 *x, f32 *y);
void ScaleByScreenA(f32 *x, f32 *y);

/* Width divided by height. */
f32 GetScreenAspectB();

} // namespace CSD
} // namespace Chao

#endif

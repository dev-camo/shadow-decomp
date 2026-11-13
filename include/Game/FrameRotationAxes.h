#ifndef GAME_FRAMEROTATIONAXES_H
#define GAME_FRAMEROTATIONAXES_H

#include <cmath>
#include "rw/rwcore.h"

/* Rotations of a frame about the x, y and z axes, and the unit axes they pass to the matrix
 * rotation (file and names inferred).
 *
 * Each file that includes this header gets its own copies of the axes. Of the files that call the
 * functions, the executable has all three axes, next to each other, in HintRing, StretchGrass,
 * CircusPole and ElecCoreWall, whose code addresses .rodata by offsets from the start of the
 * section, and in the others only the ones a kept function uses: the file with the kept
 * fn_800E9C64 and fn_800E9CEC has the x and y axes alone (0x804B8CE8 and 0x804B8CF4, USA
 * (GUPE8P)). */
static const RwV3d s_xAxis = {1.0f, 0.0f, 0.0f};
static const RwV3d s_yAxis = {0.0f, 1.0f, 0.0f};
static const RwV3d s_zAxis = {0.0f, 0.0f, 1.0f};

/* The three functions below are inline (inferred): the executable has one copy of each, right
 * after the first function in link order that calls it (fn_800E9C64 and fn_800E9CEC after
 * 0x800E9A44, fn_80109AF8 after HintRing's OnDraw, USA (GUPE8P)), and each copy uses the axis of
 * the file it is in. They call the inline std::sin and std::cos, and under the game's
 * -inline noauto,level=1,deferred setting an inline function that calls another inline function
 * is not expanded at its call sites, so every use is a call. */

/* Rotates the frame about the x axis by angle radians, with the combine operation combine
 * (behaviour inferred from the sine and cosine it computes and the matrix rotation it calls). */
extern "C" inline void fn_800E9C64(RwFrame *frame, int combine, float angle) {
    fn_8047ECD4(&frame->modelling, &s_xAxis, combine, 1.0f - std::cos(angle), std::sin(angle));
    fn_8048722C(frame);
}

/* As fn_800E9C64, about the y axis. */
extern "C" inline void fn_800E9CEC(RwFrame *frame, int combine, float angle) {
    fn_8047ECD4(&frame->modelling, &s_yAxis, combine, 1.0f - std::cos(angle), std::sin(angle));
    fn_8048722C(frame);
}

/* As fn_800E9C64, about the z axis. */
extern "C" inline void fn_80109AF8(RwFrame *frame, int combine, float angle) {
    fn_8047ECD4(&frame->modelling, &s_zAxis, combine, 1.0f - std::cos(angle), std::sin(angle));
    fn_8048722C(frame);
}

#endif

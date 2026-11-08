#ifndef GAME_PATHCTRL_H
#define GAME_PATHCTRL_H

#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* Two vectors copied by the original constructor at 0x8001C4B8; type name unknown. */
class UnkBss_80570B50 {
  public:
    UnkBss_80570B50(const Fn8040Vector3 &first, const Fn8040Vector3 &second);

    /* 0x00 */ Fn8040Vector3 field_0x0;
    /* 0x0C */ Fn8040Vector3 field_0xC;
};
CHECK_SIZEOF(UnkBss_80570B50, 0x18);
/* Path-file payload consumed by fn_8001DB08; original type name and layout are unknown. */
struct UnkPathData_8001DB08;

namespace SonicteamUSA {
namespace System {

/* SonicteamUSA::System::PathCtrl (class name from the RTTI strings of its shared pointer's counter
 * in the Coaster unit's .rodata, whose deleter calls the destructor at 0x80018770). The members are
 * not mapped: the constructor initialises the words at 0x8 to 0x54, and the callback of
 * ElecCircuitEntrance that holds one on its stack reserves 0x58 bytes for it. */
class PathCtrl {
  public:
    PathCtrl();
    ~PathCtrl();

    u8 field_0x0[0x58];
};
CHECK_SIZEOF(PathCtrl, 0x58);

} // namespace System
} // namespace SonicteamUSA

/* The collision manager that fn_8001B494 returns (placeholder name; the original class is
 * unknown). */
class UnkCollision_8001B494;

/* The record that fn_8001CDA0 passes to its filter (placeholder name): the first word of a path
 * candidate and fn_8001CDA0's last argument. */
struct UnkPathFilterRecord_8001CDA0 {
    /* 0x0 */ u32 field_0x0;
    /* 0x4 */ int field_0x4;
};
CHECK_SIZEOF(UnkPathFilterRecord_8001CDA0, 0x8);

extern "C" {
UnkCollision_8001B494 *fn_8001B494(void);

/* Collision manager functions that set up or use a PathCtrl (behaviour inferred). */
/* Sets up path for the path number pathNo and returns whether that path exists. pathNo is
 * unsigned: the function compares it with cmplw, and FootingMovable converts its float "path id"
 * for it with __cvt_fp2unsigned. */
bool fn_8001C7B4(UnkCollision_8001B494 *collision, SonicteamUSA::System::PathCtrl *path,
                 u32 pathNo);
/* Moves position along the path by distance and returns whether the end was reached. */
bool fn_8001C91C(UnkCollision_8001B494 *collision, SonicteamUSA::System::PathCtrl *path,
                 s32 reversed, Fn8040Vector3 *position, float distance, UnkRotation3 *rotation,
                 int flag);
/* Sets up path from resource at position and rotation (behaviour inferred). Argument 4 is passed
 * to fn_8001E070 as its third argument, like CharInfo::field_0x30 (rotation order, inferred),
 * which is what the GrindGadget passes; argument 6 is read as two groups of three floats (at
 * 0x0-0x8 and 0xC-0x14) and also passed on (the GrindGadget passes lbl_80570B50). */
void fn_8001DB08(UnkCollision_8001B494 *collision, UnkPathData_8001DB08 *resource,
                 const Fn8040Vector3 *position, UnkRotation3 rotation, int arg4,
                 SonicteamUSA::System::PathCtrl *path, const UnkBss_80570B50 &arg6);
/* Releases path (behaviour inferred from its body: it passes objects derived from the path to
 * release functions). The collision argument is unused. */
void fn_8001E2DC(UnkCollision_8001B494 *collision, SonicteamUSA::System::PathCtrl *path);
/* Reads the three floats of position as a point. When arg2 * arg2 is not zero, a group of
 * candidates is searched only when position lies within the group's bounds widened by arg2; when it
 * is zero, the bounds tests are skipped. When filter is not null, it is called for each candidate
 * of a searched group with a record of the candidate's first word and arg7, and a candidate for
 * which it returns false is skipped. */
bool fn_8001CDA0(UnkCollision_8001B494 *collision, SonicteamUSA::System::PathCtrl *path, float arg2,
                 const Fn8040Vector3 *position, const Fn8040Vector3 *direction, int *result,
                 bool (*filter)(const UnkPathFilterRecord_8001CDA0 *record), int arg7);

/* True when the weak counter at 0x4 of path still counts a reference (behaviour inferred: it is
 * the negation of fn_80016980, which tests the counter for none). */
bool fn_800ABDCC(SonicteamUSA::System::PathCtrl *path);

/* These callers establish the PathCtrl argument but do not use either procedure's result. */
void fn_800181BC(SonicteamUSA::System::PathCtrl *path);
void fn_80018304(SonicteamUSA::System::PathCtrl *path);
float fn_800183D8(SonicteamUSA::System::PathCtrl *path);
/* The position at the path's current state: blends the positions of the current point and the
 * next one by the current fraction (behaviour inferred). */
Fn8040Vector3 fn_80018634(SonicteamUSA::System::PathCtrl *path);
/* The rotation angles of the path at its current state, from the current point and fraction
 * (behaviour inferred; the meaning of the two int arguments is not known). */
UnkRotation3 fn_80018434(SonicteamUSA::System::PathCtrl *path, int arg1, int arg2);

/* External two-vector record read by fn_8001DB08. */
extern UnkBss_80570B50 lbl_80570B50;
}

#endif

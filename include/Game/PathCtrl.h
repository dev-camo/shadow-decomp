#ifndef GAME_PATHCTRL_H
#define GAME_PATHCTRL_H

#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;
class GadgetCharInfo;
class UnkBss_80570B50;
class UnkResource_80012700;
struct Fn8040Vector3;

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

extern "C" {
UnkCollision_8001B494 *fn_8001B494(void);

/* Collision manager functions that set up or use a PathCtrl (behaviour inferred). */
/* Sets up path for the path number pathNo and returns whether that path exists. */
bool fn_8001C7B4(UnkCollision_8001B494 *collision, SonicteamUSA::System::PathCtrl *path,
                 s32 pathNo);
/* Moves the character info along the path by distance and returns whether the end was reached. */
bool fn_8001C91C(UnkCollision_8001B494 *collision, SonicteamUSA::System::PathCtrl *path,
                 s32 reversed, CharInfo *info, float distance, Fn8040Vector3 *rotation, int flag);
/* Sets up path for charInfo and resource (behaviour inferred). Argument 4 is passed on to
 * fn_8001E070 as its third argument, like CharInfo::field_0x30 (the rotation order, inferred),
 * which is what the GrindGadget passes; argument 6 is read as two groups of three floats (at
 * 0x0-0x8 and 0xC-0x14) and also passed on (the GrindGadget passes lbl_80570B50). */
void fn_8001DB08(UnkCollision_8001B494 *collision, UnkResource_80012700 *resource,
                 GadgetCharInfo *charInfo, Fn8040Vector3 rotation, int arg4,
                 SonicteamUSA::System::PathCtrl *path, const UnkBss_80570B50 *arg6);
/* Releases path (behaviour inferred from its body: it passes objects derived from the path to
 * release functions). The collision argument is unused. */
void fn_8001E2DC(UnkCollision_8001B494 *collision, SonicteamUSA::System::PathCtrl *path);
bool fn_8001CDA0(UnkCollision_8001B494 *collision, SonicteamUSA::System::PathCtrl *path, float arg2,
                 GadgetCharInfo *charInfo, const Fn8040Vector3 *direction, int *result, int arg6,
                 int arg7);

/* True when the weak counter at 0x4 of path still counts a reference (behaviour inferred: it is
 * the negation of fn_80016980, which tests the counter for none). */
bool fn_800ABDCC(SonicteamUSA::System::PathCtrl *path);

/* A 0x18-byte object in .bss that fn_8001DB08 receives (placeholder name; its type is unknown). */
extern UnkBss_80570B50 lbl_80570B50;
}

#endif

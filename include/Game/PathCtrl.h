#ifndef GAME_PATHCTRL_H
#define GAME_PATHCTRL_H

#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;
class GadgetCharInfo;
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
bool fn_8001CDA0(UnkCollision_8001B494 *collision, SonicteamUSA::System::PathCtrl *path, float arg2,
                 GadgetCharInfo *charInfo, const Fn8040Vector3 *direction, int *result, int arg6,
                 int arg7);
}

#endif

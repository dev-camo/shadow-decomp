/* The bullet record that entries 0x18, 0x3D and 0x3E of the table lbl_8051D720 address: it has no
 * functions, only the value 600 at 0x10. The original file's name is unknown; the record has its
 * own start-up function in .ctors, right after the one of the BulletSatelliteLaser file. */
#include "Game/UnkParams_80051D10.h"
#include "Game/UnkStartupFloat_800E2570.h"

/* Matching workaround: the original stores the 600 at 0x10 into the record with this file's
 * start-up function in .ctors; a constant initialiser gives no store. UnkStartupFloat_800E2570
 * stands in for the original initialiser expression, which is unknown. It is written in the
 * record because a named static float gets a store of its own, which the original does not have.
 * Revisit when the original initialiser is found. */
UnkFactory_8051DDC8 lbl_8054DB70 = {
    0, 0, 0, 0, UnkStartupFloat_800E2570(600.0f).value,
};

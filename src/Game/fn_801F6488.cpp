/* The weapon type descriptions "GunBattery" and "BKBattery" and their callbacks (the description
 * layout is in WeaponItem.h). Original file name unknown. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first). */
#include "Game/WeaponItem.h"
#include "Game/CharInfo.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8005E418.h"
#include "boost/weak_ptr.hpp"

/* The description callbacks: each plays a sound and creates an effect at the matrix it is passed
 * (behaviour inferred). */
extern "C" void fn_801F653C(const Fn8040Matrix34 *matrix) {
    fn_801D560C(fn_80053164(), 0x200B, 0, 0);
    fn_8005E418(matrix, 0x18C, boost::weak_ptr<CharInfo>(lbl_805EF980));
}

extern "C" void fn_801F6488(const Fn8040Matrix34 *matrix) {
    fn_801D560C(fn_80053164(), 0x201B, 0, 0);
    fn_8005E418(matrix, 0x192, boost::weak_ptr<CharInfo>(lbl_805EF980));
}

/* Matching workaround: the original's start-up function (0x801F65B0, USA (GUPE8P)) loads 1.0
 * and 5.0 from .sdata2 once each and stores them at 0x10 and 0x14 of both descriptions, where its
 * .data has zeros; the other members are initialised data. A constant initialiser gives no store.
 * UnkStartupFloat_800E2570 stands in for the original initialiser expression, which is unknown;
 * it is written in each record, as in WeaponSpecialKatana.cpp. Revisit when the original
 * initialiser is found. */
UnkWeaponDescription_8005B9D4 lbl_80542080 = {
    "GunBattery",
    0,
    0,
    fn_801F653C,
    UnkStartupFloat_800E2570(1.0f).value,
    UnkStartupFloat_800E2570(5.0f).value,
    0,
    1,
    0,
    0.0f,
    0.2f,
    1.0f,
};

UnkWeaponDescription_8005B9D4 lbl_805420B0 = {
    "BKBattery",
    0,
    0,
    fn_801F6488,
    UnkStartupFloat_800E2570(1.0f).value,
    UnkStartupFloat_800E2570(5.0f).value,
    0,
    1,
    0,
    0.0f,
    0.2f,
    1.0f,
};

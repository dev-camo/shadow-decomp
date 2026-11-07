#ifndef GAME_BULLETCHAOSSPEAR_H
#define GAME_BULLETCHAOSSPEAR_H

#include "Game/PJSLauncher.h"
#include "Game/UnkParams_80051D10.h"
#include "boost/shared_ptr.hpp"

/* Functions of the file of Weapon::Bullet::ChaosSpear::ChaosSpear (class name from RTTI) that
 * Weapon::ChaosSpear::ChaosSpear calls. The file is not decompiled yet; the names are placeholders
 * and the parameter names and const reference spellings are inferred. */
extern "C" {
/* Allocates 0x64 bytes and, when that succeeds, constructs a Weapon::Bullet::ChaosSpear::ChaosSpear
 * there with charge, params and launcher (its constructor is at 0x802F2430). The void return is
 * inferred: the body leaves the allocation or constructor result in r3, and the only caller
 * discards it. */
void fn_802F1BB0(const UnkParams_80051D10 &params,
                 const boost::shared_ptr<Effect::PJS::Launcher> &launcher, float charge);
/* Returns the .sbss float at 0x805F07C0 (USA), which the file's start-up code sets. */
float fn_802F1C08(void);
}

#endif

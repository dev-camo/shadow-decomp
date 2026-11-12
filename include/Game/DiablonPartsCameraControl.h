#ifndef GAME_DIABLONPARTSCAMERACONTROL_H
#define GAME_DIABLONPARTSCAMERACONTROL_H

#include "Game/DiablonPartsBase.h"

extern "C" {
/* Creates the CameraControl part beneath parent. The caller at USA (GUPE8P) 0x802DA0B0
 * ignores the result; the base-pointer return type and reference spelling are inferred. */
Boss::Diablon::Parts::PartsBase *
fn_80322590(Boss::Diablon::Parts::PartsBase *parent,
            const boost::shared_ptr<Boss::Diablon::Diablon> &diablon);
}

#endif

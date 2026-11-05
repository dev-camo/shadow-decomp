#ifndef GAME_FN_8005E418_H
#define GAME_FN_8005E418_H

#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

/* Creates effect id at matrix through fn_801D2B74 and returns it. The new effect copies the matrix,
 * but keeps the float pointer that fn_8005E4B8 derives from owner as its time-scale source
 * (fn_803055F8 stores it at 0x803056BC, USA), and the effect manager copies that float into the
 * effect's time scale when it updates the effect (0x80425360-0x80425370); the manager skips its
 * effect updates while its byte at 0x25 is set (0x80425328-0x8042534C). The pointer is 4 bytes into
 * the object that the word at 0x60 of the owner's character info points to, or 0x1C bytes into the
 * global lbl_8057D6A0 when the owner has expired. fn_8005E4B8 releases its lock on the character
 * info before it returns, so nothing here keeps that object alive: its float must stay valid for as
 * long as the effect is updated (behaviour inferred from the bodies; the GunBattery and BKBattery
 * descriptions call it). */
extern "C" boost::weak_ptr<Effect::System::EffectElement>
fn_8005E418(const Fn8040Matrix34 *matrix, int id, const boost::weak_ptr<CharInfo> &owner);

#endif

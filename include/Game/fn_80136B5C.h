#ifndef GAME_FN_80136B5C_H
#define GAME_FN_80136B5C_H

extern "C" {
/* Unless arg0 is 5, creates a Gong::CSD::GongCSD task (Gong.h) under task layer 11, passing it
 * arg0 as the index of its scene's motion (behaviour inferred from its body; the original name is
 * unknown; defined in Gong.cpp). Its callers pass 1 (the FireRing gadget, when a character has
 * passed through the ring), 4 (Gong) and a rank from 0 to 5 (UnkShootingGameTable_8013D818::Finish
 * in ShootingGameManagerTask.cpp). */
void fn_80136B5C(int arg0);
}

#endif

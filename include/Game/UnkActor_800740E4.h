#ifndef GAME_UNKACTOR_800740E4_H
#define GAME_UNKACTOR_800740E4_H

#include "Game/Mission.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>

class UnkManager_8006D6A8;
class UnkManager_80109D64;

/* The functions that gadgets use to reach the missions (Mission::Mission, Game/Mission.h) and the
 * hint manager. */

extern "C" {
/* Singleton managers (constructed on first use, as fn_8001C4F8 does) and calls on them whose
 * names and meaning are unknown (behaviour inferred from the code around the calls). */
UnkManager_8006D6A8 *fn_8006D6A8(void);
boost::weak_ptr<Mission::Mission> fn_800740E4(UnkManager_8006D6A8 *manager, int id);
UnkManager_80109D64 *fn_80109D64(void);
/* Returns its argument. */
int fn_801FAAC4(int arg0);
/* Returns -1 at once when the manager's word at 0x24 is non-zero. Otherwise it locks the manager's
 * weak pointer; when that gives an object, it passes the object, id, arg2 and 0 to fn_80049E1C and
 * returns -1 without queueing when that returns null or the word at 0x8 of the entry it returns
 * is less than the manager's word at 0x28. When it does not return -1 (also when the weak pointer
 * gives no object), it calls fn_801FABE4, queues one record {id, arg2, index} for each index from
 * arg4 up to 35 and returns 1000 times id plus 100 times arg2. arg3 is not read (behaviour
 * inferred from its body; the HINTRING gadget keeps the result and passes it to fn_801FAAC8, the
 * other callers ignore it). */
int fn_801FAF00(UnkManager_80109D64 *manager, int id, int arg2, int arg3, int arg4);
/* For an id of 0 or more: false when id equals 1000 times the manager's word at 0x14 plus 100
 * times its word at 0x18, or the same sum of one of its queued records, and true otherwise. For a
 * negative id: true when fn_801FABD4 returns false (behaviour inferred from its body). */
bool fn_801FAAC8(UnkManager_80109D64 *manager, int id);
/* Stores 1 in the mission's word at 0xC and calls fn_8016B058 with the mission, which it takes in
 * r3 (behaviour inferred from its body; whether it is a Mission::Mission member is not known). */
void fn_8016B2A4(Mission::Mission *mission);
/* Passes a mission of the manager to fn_8016B2A4 and returns true, or returns false when none
 * qualifies (behaviour inferred from its body; the selection by the missions' state is not
 * understood). The GoalRing gadget calls it when a character touches the ring. */
bool fn_8016DA34(UnkManager_8006D6A8 *manager);
}

#endif

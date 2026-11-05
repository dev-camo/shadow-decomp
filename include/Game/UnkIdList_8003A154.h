#ifndef GAME_UNKIDLIST_8003A154_H
#define GAME_UNKIDLIST_8003A154_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"

/* What fn_8003A154 builds for a position (placeholder name; the original class and function names
 * are unknown): a 0x34-byte object holding a count and the 16-bit ids that a manager returns for
 * the position (behaviour inferred from the bodies; the meaning of the ids is unknown). */
class UnkIdList_8003A154;

extern "C" {
/* Returns a new list of the ids for position, or an empty pointer when the manager does not exist
 * or returns no ids. */
boost::shared_ptr<UnkIdList_8003A154> fn_8003A154(const Fn8040Vector3 *position);
/* Returns true when the manager's test (fn_8003A4C0) is true for one of the list's ids, and false
 * when the manager does not exist or the list has expired. Whether the original takes the weak
 * pointer by value or by const reference is not known; the code is the same. */
bool fn_80039EDC(boost::weak_ptr<UnkIdList_8003A154> list);
}

#endif

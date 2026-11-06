#ifndef GAME_UNKMANAGER_8006F754_H
#define GAME_UNKMANAGER_8006F754_H

#include "boost/weak_ptr.hpp"

class CharInfo;
class UnkManager_8006F754;

/* Character lookup interface; class and method names are unknown. */
extern "C" {
UnkManager_8006F754 *fn_8006F754();
int fn_800747E0(UnkManager_8006F754 *manager);
boost::weak_ptr<CharInfo> fn_8007488C(UnkManager_8006F754 *manager, int id);
/* Passes the object of each entry of the manager's list to fn_800739E0 (behaviour inferred from
 * its body). */
void fn_80074AEC(UnkManager_8006F754 *manager);
}

#endif

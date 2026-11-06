#ifndef GAME_FN_80076218_H
#define GAME_FN_80076218_H

#include "boost/weak_ptr.hpp"

class CharInfo;

/* For a character from 1 to 12, returns the result of that character's function, one of 12
 * (fn_8020FAE4, fn_8021262C, ...) that a jump table selects; for 0 and any other value it returns
 * an empty pointer (behaviour inferred from its body; the original name is unknown). The
 * SET_CHARACTOR gadget calls it when the weak pointer that fn_8007488C returns for the character
 * has expired; that the functions create the character info is inferred from that use. */
extern "C" boost::weak_ptr<CharInfo> fn_80076218(int character);

#endif

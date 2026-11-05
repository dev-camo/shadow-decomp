#ifndef GAME_FN_8033F0F8_H
#define GAME_FN_8033F0F8_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"

class CharCommand;
class GadgetModelInfo;

/* Creates a 0x3C-byte object from command, model and arg2 and wraps it in a shared pointer. The
 * object's constructor (0x8033FA30) passes command to fn_80237E34 and then, with that function's
 * result, to fn_80238424 (purposes unknown), keeps a copy of model and stores arg2 at 0x34.
 *
 * The function then calls the object's virtual functions at vtable offsets 0xC and 0x10 with
 * position and rotation and, among other calls, passes the shared pointer to the constructor of a
 * new 0x30-byte object (0x80237C90).
 *
 * Callers: the BAMine gadget's command handler and the functions at 0x800EEAF8 and 0x801080F8 pass
 * arg2 0, 0 and 1 and release their own model pointer right after the call.
 *
 * Behaviour inferred from the body; the original name is unknown. The signature is inferred too:
 * the function forwards command, the model's address and arg2 to the constructor unchanged
 * (0x8033F12C-0x8033F138) and position and rotation to the virtual calls, so the bytes do not
 * determine the parameter types, pointer or reference spellings or const qualifiers. */
extern "C" void fn_8033F0F8(CharCommand *command, const boost::shared_ptr<GadgetModelInfo> &model,
                            int arg2, const Fn8040Vector3 *position, const UnkRotation3 *rotation);

#endif

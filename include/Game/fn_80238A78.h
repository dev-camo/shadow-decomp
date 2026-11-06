#ifndef GAME_FN_80238A78_H
#define GAME_FN_80238A78_H

#include "Game/fn_8040_vector_types.h"

class CharCommand;

/* Calls fn_80239220 with its arguments unchanged. That function creates a 0x18-byte object from
 * command and arg3 (constructor 0x80238F94, which also takes the clump of resource 0x7C and
 * creates a draw layer), wraps it in a shared pointer, calls the object's virtual functions at
 * vtable offsets 0xC and 0x10 with position and rotation, and passes the shared pointer to the
 * constructor of a new 0x30-byte object (0x80237C90). The RING gadget calls it when a command
 * with the id 0x14 collects a ring, with the ring's position and rotation and 1.
 *
 * Behaviour inferred from the bodies; the original name is unknown. The signature is inferred too:
 * the arguments are forwarded unchanged, so the bytes do not determine the parameter types,
 * pointer or reference spellings or const qualifiers. */
extern "C" void fn_80238A78(CharCommand *command, const Fn8040Vector3 *position,
                            const UnkRotation3 *rotation, int arg3);

#endif

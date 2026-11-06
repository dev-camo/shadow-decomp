#ifndef GAME_FN_80254384_H
#define GAME_FN_80254384_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"

class VacuumCommand;
class GadgetModelInfo;

/* Builds a VacuumContainer::Container (0x48 bytes, constructor at USA (GUPE8P) 0x80254CF0; class
 * name from the RTTI of the vtable that constructor installs, 0x80549AC4, which lists VacuumObject
 * as its base) from command, model and the next three arguments and the event id, keeps it in a
 * shared_ptr<VacuumObject>, passes position and rotation to its virtual functions at vtable
 * offsets 0xC and 0x10, then calls fn_802380C4 with it and 0x252 and fn_80254724 with it, and
 * creates a VacuumObjectTask for it (behaviour inferred from its body; the original name is
 * unknown). The CONTAINER gadget's container calls it for a VacuumCommand, with its "model",
 * "select" and "value" parameters, its own position and rotation and an id.
 *
 * The signature is inferred: the bytes give the eight integer register arguments, but not the
 * parameter types, the pointer or reference spellings or the const qualifiers. */
extern "C" void fn_80254384(VacuumCommand *command, const boost::shared_ptr<GadgetModelInfo> &model,
                            int modelIndex, int select, int value, const Fn8040Vector3 *position,
                            const UnkRotation3 *rotation, int eventId);

#endif

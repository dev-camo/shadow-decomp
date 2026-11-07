#ifndef GAME_SETADAPTERSTATE_H
#define GAME_SETADAPTERSTATE_H

#include "layout_check.h"

/* SetAdapterState (class name from RTTI, with no bases): a polymorphic base class whose only
 * virtual function is the destructor, which the code that derives from it inlines. Gadgets that
 * keep state in their owner derive from it (ChaoData, CityGadgetState); the owner's pointer is a
 * SetAdapterState, which the Chao gadget reads back with dynamic_cast and CityGadget without a
 * check. */
class SetAdapterState {
  public:
    virtual ~SetAdapterState() {}
};
CHECK_SIZEOF(SetAdapterState, 0x4);

#endif

#ifndef GAME_SETADAPTERSTATE_H
#define GAME_SETADAPTERSTATE_H

#include "layout_check.h"

/* SetAdapterState (class name from RTTI, with no bases): a polymorphic base class whose only
 * virtual function is the destructor, which the code that derives from it inlines. Gadgets that
 * keep state in their owner derive from it (ChaoData); the owner's pointer is a SetAdapterState
 * and the gadget reads it back with dynamic_cast. */
class SetAdapterState {
  public:
    virtual ~SetAdapterState() {}
};
CHECK_SIZEOF(SetAdapterState, 0x4);

#endif

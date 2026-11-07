#ifndef GAME_MOTIONCALLBACKCONTAINER_H
#define GAME_MOTIONCALLBACKCONTAINER_H

#include "boost/function.hpp"
#include "layout_check.h"

struct _stMtnCallbackParam;

/* MotionCallbackContainer (class name from RTTI, with no bases): a base class whose only member is
 * its vtable pointer. The two virtual functions are declared in the order of its vtable
 * (0x8055D38C); the class has no virtual destructor, and the names are placeholders. The first
 * implementation is empty (0x803365D4). The second (0x8033637C) binds the first to this (the
 * pointer to member at 0x8055D318 selects vtable offset 8) and returns the binding as a 0xC-byte
 * object, whose constructor (0x80336418) clears three words and then stores the binding; the
 * function1 return type is inferred from that. The RTTI name of the binding's type in the DOL
 * contains boost::_mfi::mf1<void, MotionCallbackContainer, _stMtnCallbackParam &>, which gives the
 * first function's signature. */
class MotionCallbackContainer {
  public:
    virtual void fn_803365D4(_stMtnCallbackParam &param);
    virtual boost::function1<void, _stMtnCallbackParam &> fn_8033637C();
};
CHECK_SIZEOF(MotionCallbackContainer, 0x4);

#endif

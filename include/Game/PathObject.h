#ifndef GAME_PATHOBJECT_H
#define GAME_PATHOBJECT_H

#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharCommand;
class CharInfo;

/* PathObject (class name from RTTI, with no bases): by the RTTI strings, the base class of
 * EggBalloon::SetEggBalloon, EscapePlane::SetEscapePlane and BKTank::SetBKTank, which have a copy
 * of its RTTI name each. Its vtable (0x80529B78) has six null (pure virtual) slots after the
 * destructor; its destructor and its three other virtual functions are inline, and the copies the
 * link keeps are those in EggBalloon.cpp (0x80147844, 0x80147900, 0x80147904, 0x80147908):
 * SetEscapePlane's vtable (0x80549B58) uses the last of them. EggBalloonController::Controller
 * keeps one in a shared pointer: it calls UnkSlot8 and passes the commands 0x303 and 0x304 that its
 * command handler receives to OnCommand (0x80273D9C, 0x80273DBC). Names and signatures are inferred
 * from SetEggBalloon's implementations; the empty bodies say nothing about the parameters of
 * UnkSlot7 and UnkSlot8. */
class PathObject {
  public:
    virtual ~PathObject() {}
    /* Advances the object by dt. */
    virtual void Update(float dt) = 0;
    /* Builds the character info and the model and resets the state. */
    virtual void Initialize() = 0;
    virtual void UnkSlot3() = 0;
    /* Places the model. */
    virtual void UpdateModelMatrix() = 0;
    virtual bool IsFinished() = 0;
    virtual boost::weak_ptr<CharInfo> GetCharInfo() = 0;
    virtual void UnkSlot7() {}
    virtual void UnkSlot8() {}
    virtual void OnCommand(CharCommand &command) {}
};
CHECK_SIZEOF(PathObject, 0x4);

#endif

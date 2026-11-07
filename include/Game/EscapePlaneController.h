#ifndef GAME_ESCAPEPLANECONTROLLER_H
#define GAME_ESCAPEPLANECONTROLLER_H

#include "Game/PathObject.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class GadgetCharInfo;
/* The 0x74-byte object that EscapePlaneController::Controller owns at 0x38 (placeholder name; the
 * original class is unknown). Its constructor (0x8025FE00) first runs the
 * SonicteamUSA::System::PathCtrl constructor on it; the controller's destructor runs the PathCtrl
 * destructor on it and frees it. */
class UnkPathCtrl_8025FE00;

namespace EscapePlaneController {

/* EscapePlaneController::Controller (class name from RTTI, with Task as its only base): the task
 * that the ESCAPE_PLANE file creates under task layer 6 for its EscapePlane::SetEscapePlane. Its
 * file (vtable 0x8054BA00) is not recovered: only what the ESCAPE_PLANE file uses is declared. The
 * constructor (0x8025F528) builds a character info, installs a command handler on it (0x8025F648),
 * keeps the object at 0x30, clears the words at 0x38 and 0x3C and sets the float at 0x40
 * (behaviour inferred). */
class Controller : public Task {
  public:
    /* Passing object by value is inferred from the caller's copy, which the caller destroys after
     * the call. */
    Controller(Task *parent, boost::shared_ptr<PathObject> object);
    virtual ~Controller();

    /* Calls the object's UnkSlot8 when the object pointer is not null, then sets the word at 0x3C
     * to 1 (behaviour from its body). */
    void fn_8025F054();

    /* 0x28 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x30 */ boost::shared_ptr<PathObject> m_object;
    /* One of the controller's functions allocates it (0x74 bytes, 0x8025EC8C), constructs it
     * when the allocation succeeds and stores the result, null on failure (0x8025ECA8); the
     * destructor destroys and frees it when the pointer is not null and then clears the pointer
     * (0x8025F0EC-0x8025F114). */
    /* 0x38 */ UnkPathCtrl_8025FE00 *field_0x38;
    /* used by the controller's own functions, which are not recovered */
    /* 0x3C */ u8 field_0x3C[0x44 - 0x3C];
    /* 0x44 */ UnkBitFlags_80014E04 m_flags;

  private:
    /* Copying is unrecovered: an implicit copy would duplicate the owned pointer at 0x38 and the
     * task links (declared only, as in C++03). */
    Controller(const Controller &);
    Controller &operator=(const Controller &);
};
CHECK_SIZEOF(Controller, 0x48);

} // namespace EscapePlaneController

#endif

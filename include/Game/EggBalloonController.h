#ifndef GAME_EGGBALLOONCONTROLLER_H
#define GAME_EGGBALLOONCONTROLLER_H

#include "Game/PathObject.h"
#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class GadgetCharInfo;
/* The 0x6C-byte object that EggBalloonController::Controller owns at 0x38 (placeholder name; the
 * original class is unknown). Its constructor (0x80274C84) first runs the
 * SonicteamUSA::System::PathCtrl constructor on it, then sets the words at 0x58 to 0x68 and calls
 * fn_80274BF0; the controller's destructor runs the PathCtrl destructor (0x80018770) on it and
 * frees it. */
class UnkPathCtrl_80274C84;

namespace EggBalloonController {

/* EggBalloonController::Controller (class name from RTTI, with Task as its only base): the task
 * that EggBalloon.cpp creates under task layer 6 for its SetEggBalloon. Its file (from 0x80273298)
 * is not recovered: only what EggBalloon.cpp uses is declared. The constructor (0x80274718) builds
 * a character info and installs a command handler on it; the handler (0x80273B98) passes the
 * commands 0x303 and 0x304 on to the object's OnCommand when the object pointer is not null.
 * Behaviour inferred. */
class Controller : public Task {
  public:
    /* Keeps object at 0x30. Passing it by value is inferred from the caller's copy, which the
     * caller destroys after the call. */
    Controller(Task *parent, boost::shared_ptr<PathObject> object);
    virtual ~Controller();

    /* Calls the object's UnkSlot8 when the object pointer is not null (behaviour from its
     * body). */
    void fn_802742D4();

    /* 0x28 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x30 */ boost::shared_ptr<PathObject> m_object;
    /* One of the controller's functions allocates it (0x6C bytes, 0x80273ECC), constructs it
     * when the allocation succeeds and stores the result, null on failure (0x80273EE0); the
     * destructor destroys and frees it when the pointer is not null and then clears the pointer
     * (from 0x80274358). */
    /* 0x38 */ UnkPathCtrl_80274C84 *field_0x38;
    /* used by the controller's own functions, which are not recovered */
    /* 0x3C */ u8 field_0x3C[0x44 - 0x3C];
    /* 0x44 */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(Controller, 0x48);

} // namespace EggBalloonController

#endif

#ifndef GAME_UNK802CD448_H
#define GAME_UNK802CD448_H

#include "Game/EnemyMotionEventArg.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class Unk801A35AC;
class Unk801A4430;
struct _stMtnPlayWork;
struct Unk802CD448Data;

/* The model wrapper that fn_802CDB08 creates (see EnemyDispFunc.h; placeholder name). The factory
 * allocates 8 bytes for it (USA (GUPE8P) 0x802CDB2C). Its constructor (0x802CDC64) allocates a
 * 0x3C-byte object, which starts with an Unk801A35AC and has an Unk801A4430 at 0x8, and makes the
 * wrapper's only member a shared pointer that owns it (0x802CDCC8); its destructor (0x802CDBC8)
 * destroys that shared pointer (0x802CDC14, which destroys the shared count at 0x4). Both belong
 * to the wrapper's own file, which is not recovered, and so does any copying: the constructors,
 * the assignment and the destructor are declared private and are not defined, so no other file
 * can construct, copy or destroy a wrapper. Owners hold it through the boost::shared_ptr that the
 * factory returns.
 *
 * The member functions declared are the ones the EggLastMech boss calls (placeholder names). The
 * descriptions are inferred from the boss's use unless they cite the function's body; the
 * reference spellings are inferred. */
class Unk802CD448 {
  private:
    Unk802CD448();
    Unk802CD448(const Unk802CD448 &);
    Unk802CD448 &operator=(const Unk802CD448 &);
    ~Unk802CD448();

  public:
    /* Installs the callback of the model's motion events. */
    void fn_802CD270(const boost::function1<void, EnemyMotionEventArg &> &callback);
    /* The motion control that the boss passes to fn_800C7F7C for its probe. */
    _stMtnPlayWork *fn_802CD420();
    /* The position that the boss copies to its own after the model update. */
    Fn8040Vector3 fn_802CD450();
    /* The position of the frame with the given id, taken from the matrix that fn_802CD560 returns
     * for it; when that returns null, a copy of the zero vector lbl_805E07A0 (USA (GUPE8P)
     * 0x802CD528-0x802CD548). */
    Fn8040Vector3 fn_802CD508(int frameId);
    /* The matrix of the frame with the given id. */
    Fn8040Matrix34 *fn_802CD560(int frameId);
    Unk801A4430 *fn_802CD588();
    Unk801A35AC *fn_802CD5AC();
    void fn_802CD638(float arg0);
    /* Places the model at position, turned by rotation. */
    void fn_802CD728(const Fn8040Vector3 &position, const UnkRotation3 &rotation);
    void fn_802CD7E8();
    /* The model update, with the update time. */
    void fn_802CD824(float dt);

    /* The object the constructor allocates (the pointee's layout is not recovered; fn_802CD448
     * returns this pointer's get()). */
    /* 0x00 */ boost::shared_ptr<Unk802CD448Data> field_0x0;
};
CHECK_SIZEOF(Unk802CD448, 0x8);

#endif

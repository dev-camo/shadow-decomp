#ifndef GAME_ENEMY_DISP_FUNC_H
#define GAME_ENEMY_DISP_FUNC_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class Unk801A4430;
/* Opaque model wrapper and its field0 pointee; original names not recovered.
 * These declarations imply no layout and must be unified with later recovery. */
class Unk802CD448;
struct Unk802CD448Data;

/* RTTI-proven class. Recovered virtual prefix only; additional slots follow.
 * The original constructor stores the vptr at 0xC, after the vector at 0.
 */
class EnemyDispFunc {
  public:
    /* 0x00 */ Fn8040Vector3 field_0x0;

    virtual void fn_8019B3A0();                                             // 0x08
    virtual void fn_8019B3A4(float);                                        // 0x0C
    virtual void fn_8019B3A8(float);                                        // 0x10
    virtual int fn_8019B3AC();                                              // 0x14
    virtual Unk802CD448Data *fn_8019B3B4();                                 // 0x18
    virtual Unk801A4430 *fn_8017CC74();                                     // 0x1C
    virtual boost::shared_ptr<Unk802CD448> fn_80181038();                   // 0x20
    virtual Fn8040Matrix34 *fn_8019448C(unsigned int);                      // 0x24
    virtual Fn8040Vector3 fn_8019B3BC();                                    // 0x28
    virtual Fn8040Vector3 fn_8019B3DC();                                    // 0x2C
    virtual void fn_80188E0C(float);                                        // 0x30
    virtual float fn_8019B3FC();                                            // 0x34
    virtual void fn_8019B404(const Fn8040Vector3 &, const Fn8040Vector3 &); // 0x38
    virtual void fn_8017F234(bool);                                         // 0x3C
};
CHECK_SIZEOF(EnemyDispFunc, 0x10);

#endif

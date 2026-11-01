#ifndef GAME_ENEMY_H
#define GAME_ENEMY_H

#include "Game/Unk8018A374.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;
class EnemyCharInfo;
class EnemyParaFunc;
class EnemyStatFunc;
class EnemyDispFunc;
class EnemyMoveFunc;
class EnemyCtrlFunc;
class EnemySetFunc;

/* Original RTTI names. These nonpolymorphic bases each hold the corresponding
 * borrowed interface pointer; Enemy's own vptr follows the six bases. */
struct EnemyParamObjBase {
    /* 0x00 */ EnemyParaFunc *m_parameter;
};
CHECK_SIZEOF(EnemyParamObjBase, 0x4);
struct EnemyStatusObjBase {
    /* 0x00 */ EnemyStatFunc *m_status;
};
CHECK_SIZEOF(EnemyStatusObjBase, 0x4);
struct EnemyDispObjBase {
    /* 0x00 */ EnemyDispFunc *m_display;
};
CHECK_SIZEOF(EnemyDispObjBase, 0x4);
struct EnemyMoveObjBase {
    /* 0x00 */ EnemyMoveFunc *m_movement;
};
CHECK_SIZEOF(EnemyMoveObjBase, 0x4);
struct EnemyCtrlObjBase {
    /* 0x00 */ EnemyCtrlFunc *m_control;
};
CHECK_SIZEOF(EnemyCtrlObjBase, 0x4);
struct EnemySetObjBase {
    /* 0x00 */ EnemySetFunc *m_set;
};
CHECK_SIZEOF(EnemySetObjBase, 0x4);

/* Enemy is the original RTTI name. The 0x100 base extent is bounded by the base
 * constructor's final 0xFC store and derived components beginning at 0x100. */
class Enemy : public EnemyParamObjBase,
              public EnemyStatusObjBase,
              public EnemyDispObjBase,
              public EnemyMoveObjBase,
              public EnemyCtrlObjBase,
              public EnemySetObjBase {
  public:
    Enemy(int kind);
    virtual ~Enemy();
    virtual bool fn_8019AD0C();
    virtual bool fn_8019ABF4();

    /* 0x1C */ int field_0x1C;
    /* 0x20 */ int field_0x20;
    /* 0x24 */ int field_0x24;
    /* 0x28 */ int field_0x28; // Registration id returned by fn_801A29B0.
    /* 0x2C */ unsigned int field_0x2C;
    /* 0x30 */ boost::shared_ptr<EnemyCharInfo> m_charInfo;
    /* 0x38 */ boost::weak_ptr<EnemyCharInfo> field_0x38;
    /* 0x40 */ boost::weak_ptr<CharInfo> field_0x40;
    /* 0x48 */ Fn8040Vector3 field_0x48;
    /* 0x54 */ Fn8040Vector3 field_0x54;
    /* 0x60 */ Fn8040Vector3 field_0x60;
    /* 0x6C */ Fn8040Vector3 field_0x6C;
    /* 0x78 */ Fn8040Vector3 field_0x78;
    /* 0x84 */ Fn8040Vector3 field_0x84;
    // Actual uninitialized region; field types remain unmapped, not alignment padding.
    /* 0x90 */ unsigned char field_0x90[0xC];
    /* 0x9C */ Fn8040Vector3 field_0x9C;
    /* 0xA8 */ Unk8018A374 m_boundary;
    /* 0xEC */ float field_0xEC;
    /* 0xF0 */ float field_0xF0;
    /* 0xF4 */ unsigned int field_0xF4;
    /* 0xF8 */ unsigned int field_0xF8;
    /* 0xFC */ unsigned int field_0xFC;
};
CHECK_SIZEOF(Enemy, 0x100);

/* Original address-named accessors.
 * Pointer/reference and const qualifiers are inferred at this declaration boundary. */
extern "C" {
int fn_801A642C(const Enemy *enemy);
Fn8040Vector3 *fn_8017C684(Enemy *enemy);
Fn8040Vector3 *fn_8017C690(Enemy *enemy);
Fn8040Vector3 *fn_80181F0C(Enemy *enemy);
float fn_8018346C(const Enemy *enemy);
}

#endif

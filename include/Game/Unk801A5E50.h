#ifndef GAME_UNK801A5E50_H
#define GAME_UNK801A5E50_H

#include "Game/UnkProbeTarget_800587A0.h"
#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

class Enemy;

/* Original class name unknown. The Enemy pointer is borrowed. */
class Unk801A5E50 {
  public:
    Unk801A5E50();
    void fn_801A5DF0(Enemy *enemy);
    void fn_801A5EA8(const Fn8040Vector3 &position);
    bool fn_801A5758() const;
    const Fn8040Vector3 &fn_801A5CB4() const;

    /* 0x00 */ Enemy *m_enemy;
    /* 0x04 */ UnkSphere_80021C04 m_sphere;
    /* 0x14 */ Fn8040Vector3 field_0x14;
    /* 0x20 */ bool field_0x20;
    /* 0x21 */ bool field_0x21;
};
CHECK_SIZEOF(Unk801A5E50, 0x24);

#endif

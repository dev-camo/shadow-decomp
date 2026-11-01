#ifndef GAME_UNK801A6844_H
#define GAME_UNK801A6844_H

#include "Game/PathCtrl.h"

/* Original class name unknown. Recovered fields cover 0x6C bytes; the next observed
 * enclosing field is at wrapper offset 0x70. The complete original extent is unresolved. */
class Unk801A6844 {
  public:
    Unk801A6844();
    ~Unk801A6844();

    /* Const qualification inferred from the read-only query; the this pointer is not read. */
    UnkPathData_8001DB08 *fn_801A6790(int index) const;
    void fn_801A6688(const Fn8040Vector3 *position, const Fn8040Vector3 &rotation,
                     int resourceIndex, bool selectLocalRecord, int reversed);
    void fn_801A6648();
    bool fn_801A6588(float distance, Fn8040Vector3 *position, Fn8040Vector3 *rotation);

    /* 0x00 */ SonicteamUSA::System::PathCtrl m_path;
    /* 0x58 */ float field_0x58;
    /* 0x5C */ int field_0x5C;
    /* 0x60 */ Fn8040Vector3 field_0x60;
};
/* Measured size of the recovered prefix. */
CHECK_SIZEOF(Unk801A6844, 0x6C);

#endif

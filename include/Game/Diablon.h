#ifndef GAME_DIABLON_H
#define GAME_DIABLON_H

#include "Game/CharInfo.h"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Boss {
namespace Diablon {

class Effect;

/* One of the three parameter tables at USA (GUPE8P) 0x80558710, 0x805587F4 and 0x805588D8
 * (placeholder name). The code at 0x802D8F54 passes one of them to the Diablon factory (0x802D9B18)
 * for Diablon::m_params: the first when the word at 0xA8 of lbl_8057D6A0 is 0x263, the second when
 * it is 0x265 and the third for any other value. The tables are 0xE4 bytes apart; only the floats
 * at 0x70 and 0x74 are mapped (Parts::Bomb grows its radius towards the first over the time the
 * second gives; roles inferred). */
struct UnkDiablonParams_80558710 {
    /* 0x00 */ u8 field_0x0[0x70];
    /* 0x70 */ float field_0x70;
    /* 0x74 */ float field_0x74;
};
/* Checks only the recovered view; the complete extent is not established. */
CHECK_SIZEOF(Boss::Diablon::UnkDiablonParams_80558710, 0x78);

/* Boss::Diablon::Diablon (class name from RTTI, with CharInfo as its only base): the boss's
 * character info. Its factory (0x802D9B18) allocates 0xD4 bytes; only the members that the parts
 * use are mapped. The constructor (0x802D9878) clears m_effect and m_params. */
class Diablon : public CharInfo {
  public:
    /* Its vtable (0x80558C54) replaces the destructor (0x802D97EC) and fn_801DC980
     * (0x802D976C) and keeps CharInfo's other slots. */
    virtual ~Diablon();
    virtual void fn_801DC980(CharCommand *command);

    /* 0x64 */ u8 field_0x64[0x6C - 0x64];
    /* The Boss::Diablon::Effect task that stores itself here in its constructor and clears the
     * pointer in its destructor (name inferred). */
    /* 0x6C */ Effect *m_effect;
    /* 0x70 */ u8 field_0x70[0x8C - 0x70];
    /* The parameter table that the factory stores (name inferred). */
    /* 0x8C */ const UnkDiablonParams_80558710 *m_params;

  private:
    /* Copying is disabled in this reconstructed interface because the copy contract is
     * unrecovered: the view ends at 0x90 of the 0xD4 bytes that the factory allocates, so a
     * memberwise copy would leave out the rest of the object. Declared and not defined. */
    Diablon(const Diablon &);
    Diablon &operator=(const Diablon &);
};
/* Checks only the recovered view; the factory's allocation gives a full size of 0xD4. */
CHECK_SIZEOF(Boss::Diablon::Diablon, 0x90);

} // namespace Diablon
} // namespace Boss

#endif

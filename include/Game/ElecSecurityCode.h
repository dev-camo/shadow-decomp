#ifndef GAME_ELECSECURITYCODE_H
#define GAME_ELECSECURITYCODE_H

#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class ElecSecurityCodeGenerator;

/* ElecSecurityCode (class name from RTTI): the 0x11C-byte objects that an
 * ElecSecurityCodeGenerator keeps in its list. Their file (0x80156314-0x80157B90 in the USA release)
 * is not recovered yet: only the members whose type its constructor, destructor and update show are
 * mapped (the constructor also initialises storage at 0x18-0x28 and 0xF8-0x100, left in
 * field_0x18), and the methods are declared under their addresses' placeholder names. */
class ElecSecurityCode {
  public:
    /* The constructor stores type at 0x14 and rotation at 0x108, and copies position into its
     * character info (behaviour inferred). The last argument is stored at 0x118: the generator
     * passes 0, and when a code's update creates codes it creates two, each with the code's own
     * type + 1 as the type and with 1 and 2 as the last argument. */
    ElecSecurityCode(s32 type, const Fn8040Vector3 *position, ElecSecurityCodeGenerator *generator,
                     const Fn8040Vector2 *rotation, s32 arg4);
    ~ElecSecurityCode();
    /* Advances the code by dt; false once it is done (behaviour inferred from the generator, which
     * then deletes it). */
    bool fn_80156C78(float dt);

    /* 0x000 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x008 */ GadgetModelInfo *m_model;
    /* 0x00C */ boost::shared_ptr<Target> m_target;
    /* 0x014 */ s32 m_type;
    /* Unclassified storage; not mapped. */
    /* 0x018 */ u8 field_0x18[0x104 - 0x18];
    /* The generator passed to the constructor; the update adds the codes it creates to this
     * generator's list and assigns the generator's field_0x3C. */
    /* 0x104 */ ElecSecurityCodeGenerator *m_generator;
    /* 0x108 */ Fn8040Vector2 m_rotation;
    /* 0x110 */ boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> m_attack;
    /* 0x118 */ s32 field_0x118;
};
CHECK_SIZEOF(ElecSecurityCode, 0x11C);

#endif

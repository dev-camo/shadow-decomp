#ifndef GAME_CATAPULT_H
#define GAME_CATAPULT_H

#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace Catapult {

/* The gadget parameters in the order of the descriptor's table (names from the table's strings).
 * The constructor clears them before reading them from the owner. */
struct CatapultParams {
    CatapultParams() : model(0), level(0.0f), noControlTime(0.0f), x(0.0f), y(0.0f), z(0.0f) {}

    /* 0x00 */ s32 model;           /* the model entry to show */
    /* 0x04 */ float level;         /* selects the launch speed */
    /* 0x08 */ float noControlTime; /* seconds the launched character cannot be controlled */
    /* 0x0C */ float x;             /* degrees */
    /* 0x10 */ float y;             /* degrees; no code here reads it */
    /* 0x14 */ float z;             /* degrees */
};
CHECK_SIZEOF(CatapultParams, 0x18);

/* Catapult::Catapult (class name from RTTI; the gadget's descriptor is "CATAPULT"): shows the
 * CATAPULT.BIN model entry that the parameter "model" selects. Command id 11 sets bit 1 of m_flags
 * and plays sound 0x7548 at the character info; command id 12 then clears that bit and enters
 * state 3 for 1.5 seconds (see OnCommand), and a UseCommand (id 13), accepted only in state 2 with
 * bit 1 set, records the character that uses the catapult and sets bit 0; the next update then
 * launches that character, if its weak pointer still locks, along the y axis
 * turned by the character info's rotations and the parameters X and Z, with a speed that the
 * parameter "level" selects (behaviour inferred). Member names and the names of the non-virtual
 * methods are inferred. */
class Catapult : public Gadget {
  public:
    Catapult(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Catapult();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and parameters from the owner and returns to state 0 (name
     * inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info, turned about the y axis by the character info's
     * rotation (name inferred). */
    void UpdateModelMatrix();
    /* The character info's command handler (name inferred; the meaning of the ids 11 and 12 is
     * unknown). Command id 11 in state 1 or 3 sets bit 1 of m_flags, enters state 2, passes
     * m_modelIndex and 1 to the model's fn_80160A84, sets the character info's word at 0x50 to 0x37
     * and plays sound 0x7548 at the character info. In state 2 with bit 1 set, command id 12
     * enters state 3 for 1.5 seconds, calls the model's fn_801612B8 and passes m_modelIndex and 2
     * to its fn_80160A84, and a UseCommand records its user and sets bit 0; both clear bit 1 and
     * set the word at 0x50 back to 0x36, and the UseCommand plays sound 0x7549. */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::weak_ptr<CharInfo> m_user; /* the user of the last UseCommand */
    /* 0x48 */ s32 m_modelIndex; /* the entry identifier passed to the model lookup */
    /* 0x4C */ CatapultParams m_params;
    /* 0 after construction or a reset, 1 ready, 2 after command id 11, 3 waiting for m_timer to
     * run out (inferred). */
    /* 0x64 */ s32 m_state;
    /* Seconds left in state 3. */
    /* 0x68 */ float m_timer;
    /* Bit 0: launch at the next update; bit 1: set by command id 11 (inferred). */
    /* 0x6C */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(Catapult, 0x70);

} // namespace Catapult

#endif

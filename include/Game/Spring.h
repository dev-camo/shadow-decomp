#ifndef GAME_SPRING_H
#define GAME_SPRING_H

#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkObject_801695B8.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace Spring {

/* The gadget parameters in the order of the descriptor's table (names from the table's strings).
 * The constructor clears both before reading them from the owner. */
struct SpringParams {
    SpringParams() : level(0.0f), noControlTime(0.0f) {}

    /* 0x0 */ float level;         /* selects the launch speed */
    /* 0x4 */ float noControlTime; /* seconds the character cannot be controlled */
};
CHECK_SIZEOF(SpringParams, 0x8);

/* Spring::Spring (class name from RTTI; the gadget's descriptor is "SPRING"). Member names and
 * the names of the non-virtual methods are inferred from the code. */
class Spring : public Gadget {
  public:
    Spring(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Spring();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and parameters from the owner, clears m_flags and sets its
     * bit 0 (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position, turned by its x and z rotations, and
     * m_position and the target object at (0, 10, 0) in that frame (name inferred). */
    void UpdateModelMatrix();
    /* The command handler of the character info: command id 0x1A clears bit 0 of m_flags and of
     * the target's flag word, 0x1B sets both (name inferred; the meaning of the ids is unknown). */
    void OnCommand(CharCommand &command);
    /* The model's collision callback: while bit 0 of m_flags is set, passes the hits to
     * fn_8015B004 with the hit callback, and calls the model's fn_80160B18 with 0 when it
     * launched a character (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<Target> m_object;
    /* 0x48 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x50 */ SpringParams m_params;
    /* 0x58 */ Fn8040Vector3 m_position; /* (0, 10, 0) in the model's frame */
    /* Bit 0 is set while the spring launches what it touches (inferred). */
    /* 0x64 */ UnkBitFlags_80014E04 m_flags;
};
CHECK_SIZEOF(Spring, 0x68);

} // namespace Spring

#endif

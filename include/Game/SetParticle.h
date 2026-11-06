#ifndef GAME_SETPARTICLE_H
#define GAME_SETPARTICLE_H

#include "Game/EffectElement.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table (names from the table's strings).
 * The constructor stores the defaults the gadget starts with. */
struct SetParticleParams {
    SetParticleParams() : number(0), scaleX(1.0f), scaleY(1.0f), scaleZ(1.0f) {}

    /* 0x00 */ s32 number; /* the effect id passed to fn_80168C48 */
    /* 0x04 */ float scaleX;
    /* 0x08 */ float scaleY;
    /* 0x0C */ float scaleZ;
};
CHECK_SIZEOF(SetParticleParams, 0x10);

/* SetParticle (class name from RTTI; the gadget's descriptor is "PARTICLE"): an effect without a
 * model. The constructor starts effect "number" at the character info's position with the
 * direction (0, 1, 0) (fn_80168C48); when the effect's weak pointer is not expired and the effect
 * still locks, it stores the address of m_matrix as the effect's matrix source (SetMatrixSource).
 * m_matrix holds the owner's position, rotation and scale. A reset with another "number" first
 * clears the old effect's ELEMENT_ACTIVE flag, when its weak pointer is not expired and it still
 * locks, and resets the weak pointer (only when it was not expired), then starts the new effect
 * the same way. The destructor clears the flag and resets the pointer under the same conditions.
 * The update does nothing. Behaviour inferred; method names marked "(name inferred)" are not
 * original. */
class SetParticle : public Gadget {
  public:
    SetParticle(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetParticle();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info and the parameters when
     * the owner's parameter block has their size (fn_800C9DBC), then builds m_matrix from the
     * position, the rotations about y, x and z and the scale (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ SetParticleParams m_params;
    /* 0x48 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x50 */ Fn8040Matrix34 m_matrix; /* stored as the effect's matrix source */
};
CHECK_SIZEOF(SetParticle, 0x80);

#endif

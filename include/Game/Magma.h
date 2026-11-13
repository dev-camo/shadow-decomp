#ifndef GAME_MAGMA_H
#define GAME_MAGMA_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace Magma {

/* The gadget's parameters, in the order of the descriptor's table ("model", "Y", "speed"). */
struct SetMagmaParams {
    SetMagmaParams() : model(0), y(0.0f), speed(0.0f) {}

    /* The id passed to the model's entry selection; the table gives it the limit 9. */
    /* 0x0 */ s32 model;
    /* 0x4 */ float y;     /* the height the gadget moves by; negative moves it down */
    /* 0x8 */ float speed; /* the speed of that movement per unit of the update's time step */
};
CHECK_SIZEOF(SetMagmaParams, 0xC);

/* Magma::SetMagma (class name from RTTI; the gadget's descriptor is "MAGMA"): places the
 * MAGMA.BIN model at the owner's position and asks it for the entry with the id "model"
 * (fn_80162A68, whose result is ignored: it returns 0 when no entry has that id). Once the state
 * of the gadget's com id is 1 (fn_8016904C), each update changes its height by "speed" times the
 * time step, downwards when "Y" is negative and upwards otherwise, and it stops only once it has
 * passed the start height plus "Y"; with a speed of 0, or a negative one, it never stops
 * (behaviour inferred). Method, member and constant names are inferred. */
class SetMagma : public Gadget {
  public:
    /* m_state values (names inferred from what each state does). */
    enum State { STATE_START, STATE_WAIT, STATE_MOVE, STATE_END };

    SetMagma(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetMagma();
    /* Advances the model when the model pointer is non-null, then runs the state:
     * STATE_START enters STATE_WAIT and, when the model pointer is non-null, creates the part
     * target and calls fn_80169D00 for the model's parts (fn_8015FA78) with a weak pointer to it
     * as the callback data: for each part, when the pointer locks, the callback passes the target
     * and the part's clump to fn_80166FDC. Then passes -1 to the target's fn_80167010 when there
     * is one. STATE_WAIT enters STATE_MOVE once fn_8016904C returns 1 for the com id.
     * STATE_MOVE moves the gadget (UpdateMove), entering STATE_END once that reports the end
     * height passed, and places the model. STATE_END does nothing. */
    virtual void UpdateBody(float dt);
    /* Rereads the owner (ReadOwner) and places the model (UpdateModelMatrix). When there is a
     * model and the parameter "model" has changed, it releases the part target, calls the model's
     * fn_80161880 and fn_80162804 and requests the new entry as the constructor does. Then draws a
     * debug line from the character info's position to the end of the movement. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info, the owner's parameters
     * into m_params only when the owner's parameter size equals sizeof(SetMagmaParams), 0xC
     * (fn_800C9DBC; otherwise m_params keeps its previous values), and the owner's com id into
     * m_comId; keeps the position as the start of the movement and enters STATE_START. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's matrix when the model pointer is non-null. */
    void UpdateModelMatrix();
    /* Moves the character info's height by "speed" times dt, down when "Y" is negative and up
     * otherwise, and returns whether it passed the start height plus "Y", where it then stays. */
    bool UpdateMove(float dt);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x48 */ SetMagmaParams m_params;
    /* 0x54 */ s32 m_comId;                   /* the owner's com id (fn_800C9E50) */
    /* 0x58 */ s32 m_modelEntry;              /* the value of "model" last passed to fn_80162A68 */
    /* 0x5C */ s32 m_state;                   /* a State */
    /* 0x60 */ Fn8040Vector3 m_startPosition; /* the owner's position */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(SetMagma, 0x70);

} // namespace Magma

#endif

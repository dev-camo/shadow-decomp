#ifndef GAME_CITYGADGET_H
#define GAME_CITYGADGET_H

#include "Game/CharaColliAttack.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/SetAdapterState.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* CityGadgetState (class name from RTTI, derived from SetAdapterState): what the owner keeps of a
 * CityGadget between its creations. The constructor enters STATE_INTACT; CityGadget's attack
 * callback enters STATE_BREAKING when the health runs out, its update enters STATE_BROKEN when it
 * switches to the broken model, and its owner reset enters STATE_INTACT when the type changes
 * (behaviour inferred). */
class CityGadgetState : public SetAdapterState {
  public:
    /* m_state values (names inferred). */
    enum State { STATE_INTACT, STATE_BREAKING, STATE_BROKEN };

    CityGadgetState() : m_state(STATE_INTACT) {}

    virtual ~CityGadgetState() {}

    /* 0x4 */ s32 m_state; /* a State (the s32 spelling is inferred) */
};
CHECK_SIZEOF(CityGadgetState, 0x8);

/* One entry of CityGadget's table of types, selected by the "Type" parameter (name and member
 * names inferred from their uses; the signedness of the integer members is inferred too). */
struct CityGadgetType {
    /* 0x00 */ s32 modelId;       /* the model id in STATE_INTACT */
    /* 0x04 */ s32 effectEntry;   /* the model entry the break effect looks up */
    /* 0x08 */ s32 brokenModelId; /* the model id in STATE_BROKEN; -1 when there is none */
    /* 0x0C */ u32 sound;         /* the sound played when the gadget breaks */
    /* 0x10 */ u32 attackMask;    /* tested against the attack object's word at 0x40 */
    /* 0x14 */ float health;
};
CHECK_SIZEOF(CityGadgetType, 0x18);

/* The gadget's parameters, in the order of the descriptor's table ("Type", "ScaleX", "ScaleY",
 * "ScaleZ"). The unsigned spelling of type is inferred: the code multiplies it by the table stride
 * and compares it for equality, which a signed type would compile to as well. */
struct CityGadgetParams {
    /* 0x0 */ u32 type;
    /* 0x4 */ float scaleX;
    /* 0x8 */ float scaleY;
    /* 0xC */ float scaleZ;
};
CHECK_SIZEOF(CityGadgetParams, 0x10);

/* CityGadget (class name from RTTI; the gadget's descriptor is "CITYGADGET"): an object of the
 * CITYGADGET.BIN model of one of five types (CityGadgetType), scaled by its parameters. While the
 * state (kept in the owner's CityGadgetState) is STATE_INTACT, attacks whose attack object has one
 * of the type's attackMask bits set in its word at 0x40 reduce its health; the attack after which
 * the health is below 0, or 0 as fn_8000A950 judges it, enters STATE_BREAKING. An update in
 * STATE_BREAKING starts an effect and plays the type's sound, then either finishes the gadget or,
 * when the type has a broken model, shows that model in STATE_BROKEN (see UpdateBody). Behaviour,
 * method, member, state and type names are inferred, and so are the pointer (rather than reference)
 * spelling of the owner parameters and the const on m_type's pointee: the bytes pass an address and
 * read the table in place, which does not establish either. */
class CityGadget : public Gadget {
  public:
    /* Creates the owner's CityGadgetState when the owner has none (fn_800C9930) and keeps the
     * owner's state object in m_data without a type check, then builds the character info
     * (fn_8015CD90 with 0x52 and 4), reads the owner, loads CITYGADGET.BIN and sets up, places
     * and scales the model. */
    CityGadget(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~CityGadget();
    /* STATE_BREAKING: passes an effect parameter block for the character info (with the resource
     * that fn_80012700 looks up for key 1 under the name of the model's entry effectEntry, which
     * fn_8015F7BC finds and the code reads without a test; the lookup result is null when the
     * resource is absent and is not tested; the y coordinate of the position; and m_ownerFlag) to
     * fn_8015D93C and plays the type's sound at the position. Without a broken model (-1) it
     * then calls the model's fn_80162790 and fn_80161880, passes its owner to fn_800C9B04 and
     * sets Task flag 0x01, staying in STATE_BREAKING. Otherwise it calls the model's fn_80162804
     * and fn_80161880, shows the broken model with the character info attached (as SetupModel
     * does in STATE_BROKEN), enters STATE_BROKEN, places the model and applies the scale. In
     * every state it then advances the model by dt. */
    virtual void UpdateBody(float dt);
    /* Rereads the owner (ReadOwner). When the "Type" parameter changed, calls the model's
     * fn_80162804 and fn_80161880, enters STATE_INTACT and sets the model up again
     * (SetupModel). Then places the model (UpdateModelMatrix) and calls the model's fn_80162EF4
     * and fn_8016108C; it does not apply the scale again. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation and, when the owner's parameter size is 0x10
     * (fn_800C9DBC), its parameters; stores whether 0x400000 is set in the owner's flag word
     * (fn_800C9E70) in m_ownerFlag, selects the type entry for the "Type" parameter (the index
     * is not checked) and restores the health from that entry. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* STATE_INTACT: shows the type's model (fn_80162A68 with 0), attaches the character info to
     * it (fn_80161FC8) and, when the model's attack object for that id (fn_80161428) can be
     * locked, passes OnAttack to its fn_80096B64 with 1. STATE_BROKEN: shows the broken model and
     * attaches the character info. Other states: nothing. */
    void SetupModel();
    /* Places the model at the character info's position, turned about the y, x and z axes by
     * the character info's rotation and scaled by ScaleX, ScaleY and ScaleZ (fn_8040B790). */
    void UpdateModelMatrix();
    /* When the collision object that fn_80161614 returns for the type's model (STATE_INTACT) or
     * its broken model (any other state) can be locked, passes the three scale parameters to its
     * UnkSlot15. */
    void ApplyScale();
    /* The callback that SetupModel passes to the attack object's fn_80096B64. Stores 5 in the
     * parameter's field_0x54. In STATE_INTACT, when the parameter's attack object (field_0x0)
     * can be locked and has one of the type's attackMask bits set in its word at 0x40, it stores
     * 1 there instead, subtracts the parameter's field_0x50 from the health and copies its
     * field_0x24 to its field_0x40. When the health is then below 0, or 0 as fn_8000A950 judges
     * it, it stores 3, passes the character info that fn_8015D570 returns for the parameter to
     * fn_8020642C with 0x23 when it is not null, and enters STATE_BREAKING. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ const CityGadgetType *m_type; /* the entry the "Type" parameter selects */
    /* 0x34 */ CityGadgetState *m_data;      /* the owner's state object */
    /* 0x38 */ CityGadgetParams m_params;
    /* 0x48 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x50 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x58 */ float m_health;
    /* 0x5C */ u8 m_ownerFlag; /* whether 0x400000 is set in the owner's flags (fn_800C9E70) */
};
CHECK_SIZEOF(CityGadget, 0x60);

#endif

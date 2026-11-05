#ifndef GAME_DOOR_H
#define GAME_DOOR_H

#include "Game/CharCommand.h"
#include "Game/CharaColliBase.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class UnkResource_80012700;
struct RpClump;

namespace Door {

/* The gadget parameters in the order of the descriptor's table (names from the table's strings),
 * shared by the DOOR and MORPHDOOR descriptors. The constructor stores the table's default
 * values. */
struct SetDoorParams {
    SetDoorParams()
        : type(0), model(0), width(100.0f), height(100.0f), depth(100.0f), x(0.0f), y(0.0f),
          z(0.0f) {}

    /* 0x00 */ s32 type; /* 0 "type : normal", 1 "type : key" (the table's labels) */
    /* 0x04 */ s32 model;
    /* The box probe's sizes and its offset from the gadget's position: the probe is centred
     * height above (x, y, z). */
    /* 0x08 */ float width;
    /* 0x0C */ float height;
    /* 0x10 */ float depth;
    /* 0x14 */ float x;
    /* 0x18 */ float y;
    /* 0x1C */ float z;
};
CHECK_SIZEOF(SetDoorParams, 0x20);

/* Door::SetDoor (class name from RTTI; the gadget's descriptor is "DOOR"): a door that shows the
 * DOOR.BIN model entry its "model" parameter selects and keeps a box probe. A key door (type 1)
 * waits until the entry of its com id in the manager of fn_800CB69C reports 1 and then opens for
 * good; a normal door opens while the probe has hits and closes again once they are gone. Opening
 * and closing play the model's motions and the sounds of the resource records named after the
 * model with "_open", "_openend", "_close" and "_closestart" appended (behaviour inferred). Member
 * names and the names of the methods are inferred from the code. */
class SetDoor : public Gadget {
  public:
    SetDoor(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetDoor();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Enters state (name inferred). */
    virtual void SetState(int state);
    /* Plays the model's motion number motion on the selected model entry: 1 opens, 2 closes (name
     * inferred). */
    virtual void PlayMotion(int motion);
    /* True once the motion that PlayMotion started has ended (name inferred). */
    virtual bool IsMotionFinished();
    /* Switches the model to the entry index and attaches the character info to it (name
     * inferred). */
    virtual void ChangeModel(int index);
    /* Plays the sound of the resource record named after the model entry with the suffix selected
     * by sound: "_open" (1), "_close" (2), "_openend" (3) or "_closestart" (4), at the character
     * info's position plus the record's offset (name inferred). */
    virtual void PlayStateSound(int sound);

    /* Creates the gadget under task layer 11 and initialises it (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Binds OnCommand to the character info's command unit, loads the model and creates the box
     * probe (name inferred). */
    void Init();
    /* Copies the owner's position, rotation, parameters and com id into the gadget and returns to
     * state 0 (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info, turned about the y axis by the character info's
     * rotation (name inferred). */
    void UpdateModelMatrix();
    /* True when the box probe has hits for which fn_8015AC84 is true (name inferred). */
    bool IsTouched();
    /* The character info's command handler: a SetTransparentCommand (id 0x17) fades the model with
     * fn_8015FDD8 below an alpha of 1 and ends the fade with fn_8015FD10 otherwise, and sets or
     * clears bit 0x100 of the word of the selected model entry's probe that UnkSlot10 returns
     * (name inferred). */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
    /* 0x48 */ SetDoorParams m_params;
    /* The "model" parameter, stored as read; a reset with another value clamps it to the entry
     * count and passes it to the model lookup. */
    /* 0x68 */ s32 m_modelIndex;
    /* 0 after reading the owner, 1 locked, 2 closed, 5 opening, 3 open, 6 closing, 4 unlocked
     * (the key door has opened and stays open; inferred). */
    /* 0x6C */ s32 m_state;
    /* 0x70 */ UnkResource_80012700 *m_resource; /* the model file's resource */
    /* 0x74 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x7C */ u8 m_comId; /* the owner's byte that fn_800C9E60 copies */
};
CHECK_SIZEOF(SetDoor, 0x80);

/* Door::SetMorphDoor (class name from RTTI; the gadget's descriptor is "MORPHDOOR"): a SetDoor
 * with the MORPHDOOR.BIN model that also draws the clump of a .DFF file through a draw layer.
 * Opening and closing morph that clump over m_morphTime instead of playing model motions, and it
 * plays no sounds and keeps its model entry. Member names and the names of the methods are
 * inferred from the code. */
class SetMorphDoor : public SetDoor {
  public:
    SetMorphDoor(Task *parent, UnkGadgetOwner_800C9CD0 *owner, const char *clumpFile,
                 const char *dmaFile, float morphTime, float drawRadius);
    virtual ~SetMorphDoor();
    virtual void UpdateBody(float dt);
    virtual void SetState(int state);
    virtual void PlayMotion(int motion);
    virtual bool IsMotionFinished();
    virtual void ChangeModel(int index) {}
    virtual void PlayStateSound(int sound) {}

    /* Creates the gadget under task layer 11 with the drape clump and initialises it (name
     * inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Loads the clump, the draw layer, the model and the box probe (name inferred). */
    void Init();
    /* The draw layer's callback: draws the clump at the character info, turned by its rotation,
     * when a sphere of m_drawRadius around it is in view (name inferred). */
    void OnDraw(RednerDrawArgs &args);

    /* 0x80 */ const char *m_clumpFile;
    /* 0x84 */ const char *m_dmaFile;
    /* 0x88 */ float m_morphTime;
    /* 0x8C */ float m_drawRadius;
    /* 0x90 */ RpClump *m_clump;
    /* 0x94 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0x9C */ s32 m_motion;         /* the motion that PlayMotion started: 1 opens, 2 closes */
    /* 0xA0 */ s32 m_motionFinished; /* 1 once the morph has run out */
    /* 0xA4 */ float m_morphTimer;
};
CHECK_SIZEOF(SetMorphDoor, 0xA8);

} // namespace Door

#endif

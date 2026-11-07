#ifndef GAME_ELECCOREPROGRAM_H
#define GAME_ELECCOREPROGRAM_H

#include "Game/CharaColliAttack.h"
#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkObject_802B4254.h"
#include "rw/rwcore.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* ElecCoreProgram (class name from RTTI; the gadget's descriptor is "ELEC_CORE"): the core of the
 * electric circuit area, shown with the CORE.BIN model, a second object (fn_802B4254) and a Target.
 * Five of its parts (ids 3 to 7) keep spinning; while it waits, its "mat1" material pulses. An
 * attack while it waits and no hit is being shown takes one of its 16 hits and flashes its "mat2"
 * material; otherwise the colour of "mat2" shows how many hits are left. When the last hit is
 * taken, its materials fade over one second while two of its parts change scale: part 0 first
 * shrinks from 1.0 to 0.1 times its size over 0.2 seconds and then grows, and part 8 grows from 1
 * times its size, along curves that reach 30 and 200 times the size at one second. The time is
 * tested before it is advanced, so the last scaling can evaluate the curves past one second and
 * overshoot those values. After that second, if the mission that fn_800740E4 returns for 0 exists
 * and its word at 0xC is not 1, it sets that mission's byte at 0x14 and calls fn_8016B2A4 on it;
 * then, whether or not it did so, it creates the object of kind 9 at its position (fn_8034DE08) and
 * finishes (behaviour inferred). Method and member names are inferred. */
class ElecCoreProgram : public Gadget {
  public:
    ElecCoreProgram(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecCoreProgram();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info. */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position, turned about the y axis by its rotation,
     * the Target at that position and the second object 34 units above it. */
    void UpdateModelMatrix();
    /* Turns each spinning part (ids 3 to 7) about the z and x axes by its fixed angles and about
     * the y axis by its angle in m_partAngles, which advances by dt times its spin speed, on top
     * of the matrix its frame has when it is found. */
    void RotateParts(float dt);
    /* State 0: advances m_time, pulses the "mat1" colour with a period of a third of a second
     * and, unless a hit is being shown, sets the "mat2" colour from the hits left (16 to 9, 8 to
     * 2, 1 or none); while a hit is shown it counts m_hitTime down to 0. */
    void UpdateIdle(float dt);
    /* State 1: advances m_time and fades the colours of the materials "mat1" to "mat5" over the
     * second after the last hit. */
    void UpdateBreakColors(float dt);
    /* State 1: scales the frames of parts 0 and 8 by a factor that m_time takes through three
     * linear stages, on top of the matrices the frames have when they are found. */
    void UpdateBreakScale();
    /* The character info's command handler; it ignores every command. */
    void OnCommand(CharCommand &command);
    /* The model attack object's callback. In state 0 with no hit being shown, it sets the "mat2"
     * colour, shows the hit for 0.05 seconds and takes one hit; the last hit enters state 1,
     * resets m_time, stores 3 in param.field_0x54 and plays sound 0x7241, the others store 1 and
     * play sound 0x7240. In every other case the callback stores 8 in param.field_0x54. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x030 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x038 */ GadgetModelInfo *m_model;
    /* 0x03C */ boost::shared_ptr<Target> m_target;
    /* 0x044 */ s32 m_state; /* 0 waiting for hits, 1 breaking (inferred) */
    /* 0x048 */ s32 m_hitsLeft;
    /* 0x04C */ u8 field_0x4C[0x4]; /* no code here touches it */
    /* The frames of the model parts by id, and copies of their matrices taken when they are found
     * (only ids 0 and 3 to 8 are used). */
    /* 0x050 */ RwFrame *m_partFrames[9];
    /* 0x074 */ RwMatrix m_partMatrices[9];
    /* 0x2B4 */ float m_partAngles[9]; /* the y angles of the spinning parts (ids 3 to 7) */
    /* 0x2D8 */ float m_time;          /* the pulse time in state 0, the break time in state 1 */
    /* 0x2DC */ float m_hitTime;       /* how long the current hit is still shown */
    /* 0x2E0 */ boost::shared_ptr<UnkObject_802B4254> m_object2;
};
CHECK_SIZEOF(ElecCoreProgram, 0x2E8);

#endif

#ifndef GAME_FAN_H
#define GAME_FAN_H

#include "Game/EffectElement.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

struct _stMtnPlayWork;

namespace Fan {

/* The gadget parameters in the order of the descriptor's table (names from the table's strings and
 * value labels). The constructor stores the defaults the gadget starts with. */
struct FanParams {
    FanParams()
        : type(0), form(0), x(10.0f), y(10.0f), z(10.0f), power(10.0f), secActive(-1.0f),
          secNegative(-1.0f), disp(1), id(-1) {}

    /* 0x00 */ s32 type; /* 0: "UpperWay", 1: "SideWay" */
    /* 0x04 */ s32 form; /* 0: "Cylinder", 1: "Box" */
    /* 0x08 */ float x;  /* labelled "radius" for a cylinder and "width" for a box */
    /* 0x0C */ float y;  /* labelled "Not in use" for a cylinder and "height" for a box */
    /* 0x10 */ float z;  /* labelled "height" for a cylinder and "depth" for a box */
    /* 0x14 */ float power;
    /* 0x18 */ float secActive;   /* "sec_active" */
    /* 0x1C */ float secNegative; /* "sec_negative" */
    /* 0x20 */ s32 disp;          /* 0: the model's fn_80162790 is called, otherwise fn_8016271C */
    /* 0x24 */ s32 id;            /* "Id": the switch id, or a negative value for none */
};
CHECK_SIZEOF(FanParams, 0x28);

/* Fan::Fan (class name from RTTI; the gadget's descriptors are "FAN" and "FAN STAGE"). The fan
 * runs while its switch is on, or always when it has none; when "sec_active" and "sec_negative"
 * are both positive it also alternates between running and stopped on its timer. While it runs it
 * starts an effect, speeds its motion up and requests a sound, and blows the character infos its
 * probe touches; while it is stopped it ends the effect and slows the motion down (behaviour
 * inferred). Member and method names are inferred. */
class Fan : public Gadget {
  public:
    /* modelArg is passed to fn_8015F800 with the model's file name: 1 for "FAN STAGE", 0 for
     * "FAN". */
    Fan(Task *parent, UnkGadgetOwner_800C9CD0 *owner, int modelArg);
    virtual ~Fan();
    virtual void UpdateBody(float dt);
    /* Calls fn_8015BE64 on the handle target when there is one (purpose unknown). */
    virtual void fn_800CBC1C(float);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Create the gadget under task layer 11 for the "FAN" and "FAN STAGE" descriptors (names
     * inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);
    static void CreateStage(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameters, restarts the timer and the motion
     * speed, and sets the state bits from the parameters (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model, and the effect when there is one, at the character info (name inferred). */
    void UpdateModelMatrix();
    /* Creates the probe of the parameter "form" and the handle target, and binds OnCollision
     * (name inferred). */
    void CreateProbe();
    /* Follows the switch, starts or ends the effect, spins the motion up or down, and switches
     * between running and stopped when the timer runs out (name inferred). */
    void UpdateRunning(float dt);
    /* Whether position lies within the range of the parameter "form" around center, with a margin
     * of 7.5, after both lose their y components: for a cylinder the distance is tested against
     * "X", for a box the x and z of the offset in the fan's frame against "X" and "Z" (name
     * inferred). The vectors are not const: with const pointers the compiler orders their loads
     * differently from the original. */
    bool IsInRange(Fn8040Vector3 *position, Fn8040Vector3 *center);
    /* The handle target's callback while its probe has hits (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model; /* FAN.BIN */
    /* 0x40 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0x48 */ boost::weak_ptr<CharInfo> m_target; /* the character info the fan put in a stream */
    /* 0x50 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* 0x58 */ boost::weak_ptr<_stMtnPlayWork> m_motion;
    /* 0x60 */ FanParams m_params;
    /* 0x88 */ s32 m_step;    /* 0 after CopyFromOwner, 1 from the first update on */
    /* 0x8C */ float m_timer; /* the time left of the current running or stopped phase */
    /* Grows by 0.01 at each update while the fan has a motion and is then added to m_speed while
     * the fan runs or taken from it while it is stopped; zeroed by CopyFromOwner and when the fan
     * switches between running and stopped. */
    /* 0x90 */ float m_speedStep;
    /* 0x94 */ float m_speed; /* the motion's frame speed, from 0 to 1 */
    /* Bit 0 is set in the frames in which the fan blows a character info, bit 2 while the fan
     * runs, bit 3 when it alternates on its timer and bit 4 for a sideways fan in the stage whose
     * word at 0xA8 of lbl_8057D6A0 is 0x1F5 (inferred). */
    /* 0x98 */ UnkBitFlags_80014E04 m_state;
    /* Unclassified storage in the original 0xA0-byte allocation; no accesses are identified. */
    /* 0x9C */ unsigned char field_0x9C[4];
};
CHECK_SIZEOF(Fan, 0xA0);

} // namespace Fan

#endif

#ifndef GAME_GADGETMODELINFO_H
#define GAME_GADGETMODELINFO_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;
class UnkAttack_8005A840;
class UnkMotionControl_801602B4;

/* Tests or sets a work flag of the motion control (behaviour inferred: CityLaser and ThreatObj
 * pass 0, 3, 1 and test the result, GadgetModelInfo::fn_8015FF60 passes 0, 11, 0). */
extern "C" int stMtnControlWorkFlag(UnkMotionControl_801602B4 *control, int id, int arg2, int arg3);

/* Motion control calls with behaviour inferred from the names: prepare a blend towards the target
 * motion, set the blend length and set the frame speed. The Chao gadget passes 0, -1 and -1, then a
 * blend length of 15, and speeds of 1 and of its movement; the meaning of the other arguments is
 * unknown, and the return types are unknown (the Chao ignores them). */
extern "C" void stMtnPrepareBlendWithTarget(UnkMotionControl_801602B4 *control, int arg1, int arg2,
                                            int arg3);
/* Also declared, with MotionControl for the control, in Game/fn_804157E8_motion_control.h, whose
 * unit defines it (the two control types are not unified yet). */
extern "C" void stMtnRequestBlendFrame(UnkMotionControl_801602B4 *control, int id, float frames);
extern "C" void stMtnRequestFrameSpeed(UnkMotionControl_801602B4 *control, int id, float speed);

/* The three words that fn_8015FA78 passes to its callback for each part (names and meaning
 * unknown). */
struct UnkModelPart_8015FA78 {
    /* 0x0 */ u32 field_0x0;
    /* 0x4 */ u32 field_0x4;
    /* 0x8 */ u32 field_0x8;
};
CHECK_SIZEOF(UnkModelPart_8015FA78, 0xC);

/* GadgetModelInfo (class name from RTTI): the 0x74-byte model of a gadget, loaded from a named
 * model file. Its members are not mapped yet; the methods keep placeholder names and argument types
 * taken from their callers. */
class GadgetModelInfo {
  public:
    GadgetModelInfo(int arg0, const char *fileName);
    virtual ~GadgetModelInfo();

    void fn_8016108C();
    void fn_801604FC();
    /* Looks for the entry of the list at 0x58 whose info word at 0x18 equals arg0; when there is
     * one, passes the entry's word at 0xC to fn_80417568 with the low 16 bits of its word at 0x1C
     * and returns true, otherwise false (behaviour inferred). */
    bool fn_80160B18(int arg0);
    /* Finds the first entry whose id word equals arg0 in a table of the object at 0x30 and
     * returns fn_80160C90's result for its index, or false when there is none (behaviour
     * inferred). */
    bool fn_80160C28(int arg0);
    void fn_80161880();
    /* Passes the three floats to a virtual method of each part whose id equals arg0 (behaviour
     * inferred; meanings of the arguments unknown). */
    void fn_801617D0(int arg0, int arg1, float arg2, float arg3, float arg4);
    /* Attaches a character info to the model node named nodeName (behaviour inferred). */
    bool fn_80161F08(const char *nodeName, const boost::weak_ptr<CharInfo> &info);
    void fn_80162804();
    void fn_80160670(int arg0);
    /* Finds the first entry whose id word equals arg0 in another table of the object at 0x30 and
     * returns fn_80162AD0's result for its index and arg1, or false when there is none (behaviour
     * inferred). */
    bool fn_80162A68(int arg0, int arg1);
    void fn_80162EF4();
    /* Advances the model by dt (decrements its timers; behaviour inferred). */
    void fn_80162FB0(float dt);
    /* Sets the model's world matrix (behaviour inferred). */
    void fn_801633DC(const Fn8040Matrix34 *matrix);

    /* Looks for the entry of the list at 0x58 whose info word at 0x18 equals arg0; when there is
     * one, passes the entry's word at 0xC to fn_80417568 with (u16)(arg1 + the entry's word at
     * 0x1C - 1) and returns true, otherwise false (behaviour inferred; the meaning of the
     * arguments is unknown). */
    bool fn_80160A84(int arg0, int arg1);
    /* Sets a motion control work flag of each entry of the list at 0x58 (behaviour inferred from
     * its call of stMtnControlWorkFlag with 0, 11, 0). */
    void fn_8015FF60();
    /* Calls callback(part, userData) for each entry of the list at 0x4C until it returns zero
     * (behaviour inferred). */
    void fn_8015FA78(int (*callback)(UnkModelPart_8015FA78 part, const void *userData),
                     const void *userData);
    /* Methods that ThreatObj calls (purposes unknown). */
    void fn_8016271C();
    void fn_80162790();
    void fn_80160480(int arg0);
    /* Attaches a character info to the part with id arg0 (behaviour inferred). */
    void fn_80161FC8(int arg0, const boost::weak_ptr<CharInfo> &info);
    /* Returns the attack object of the part with id arg0 (behaviour inferred). */
    boost::weak_ptr<UnkAttack_8005A840> fn_80161428(int arg0);
    /* Returns the motion control of the part with id arg0 (placeholder type; behaviour
     * inferred). */
    boost::weak_ptr<UnkMotionControl_801602B4> fn_801602B4(int arg0);

    /* 0x04 */ u8 field_0x4[0x70];
};
CHECK_SIZEOF(GadgetModelInfo, 0x74);

/* Loads the model file fileName into a new GadgetModelInfo owned by the returned pointer (behaviour
 * inferred; arg0 is the first argument of the GadgetModelInfo constructor, always 1 in callers). */
extern "C" boost::shared_ptr<GadgetModelInfo> fn_8015F800(int arg0, const char *fileName);

/* Returns the word at 0x0 of the object at 0x34 of the model, or null when there is none: a name
 * (behaviour inferred from the ball, which looks it up with fn_80208888). */
extern "C" const char *fn_801626D8(GadgetModelInfo *model);

#endif

#ifndef GAME_GADGETMODELINFO_H
#define GAME_GADGETMODELINFO_H

#include "Game/fn_8040_vector_types.h"
#include "Game/GadgetModelInfoStorage.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/fn_804157E8_motion_range.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;
namespace SonicteamUSA {
namespace System {
namespace CharaColli {
class CharaColliAttack;
class CharaColliBase;
} // namespace CharaColli
} // namespace System
} // namespace SonicteamUSA
struct _stMtnPlayWork;
struct RpClump;
struct RpAtomic;

/* An entry of the array at 0x0 of the object at 0x30 of a GadgetModelInfo, found by the word at
 * 0x3C (placeholder name; only the name at 0x4 is mapped). */
struct UnkModelEntry_8015F7BC {
    /* 0x00 */ u8 field_0x0[4];
    /* 0x04 */ const char *name; /* FallBuildingHolder passes it to fn_80012700 as a file name */
    /* 0x08 */ u8 field_0x8[0x3C - 0x8];
    /* 0x3C */ s32 id;
};
CHECK_SIZEOF(UnkModelEntry_8015F7BC, 0x40);

/* The three words that fn_8015FA78 passes to its callback for each part (clump name inferred;
 * the other words' meanings are unknown). */
struct UnkModelPart_8015FA78 {
    /* 0x0 */ RpClump *clump;
    /* 0x4 */ u32 field_0x4;
    /* 0x8 */ s32 field_0x8;
};
CHECK_SIZEOF(UnkModelPart_8015FA78, 0xC);

/* GadgetModelInfo (class name from RTTI): the 0x74-byte model of a gadget, loaded from a named
 * model file. Only the flag and the two accessed list members are mapped here. The other intervals remain
 * unrecovered storage with subobject lifetimes managed by the native constructor and destructor. */
class GadgetModelInfo {
  public:
    GadgetModelInfo(int arg0, const char *fileName);
    virtual ~GadgetModelInfo();

    /* Finds a model entry by name and forwards its index and arg1 to fn_80162AD0
     * (behaviour inferred; the original integral spelling of arg1 is unknown). */
    bool fn_801629D8(const char *name, int arg1);
    /* Finds a motion resource by name and forwards its index to fn_80160C90 (inferred). */
    bool fn_80160BA0(const char *name);
    /* Requests the motion identified by arg1 plus the matching payload's word at +0x14 minus
     * one, truncated to 16 bits (behaviour inferred). */
    bool fn_80160960(const char *name, int arg1);
    /* Attaches info through the named resource and named motion control (behaviour inferred). */
    bool fn_80161AB8(const char *name, const boost::weak_ptr<CharInfo> &info,
                     const char *motionName);
    /* Returns a weak reference to the first motion control whose payload name matches (inferred). */
    boost::weak_ptr<_stMtnPlayWork> fn_801601A0(const char *name);

    void fn_8016108C();
    void fn_801604FC();
    /* Finds the motion entry whose resource record's word at +0x18 equals arg0, then requests
     * its motion using the shared control and low 16 bits of the payload's word at +0x14
     * (behaviour inferred). */
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
    void fn_80163534();
    void fn_801635D8();
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

    /* Finds the motion entry by its resource record's word at +0x18 and requests the motion
     * identified by the low 16 bits of arg1 + payload word at +0x14 - 1 (behaviour inferred). */
    bool fn_80160A84(int arg0, int arg1);
    /* Sets a motion control work flag of each entry of the list at 0x58 (behaviour inferred from
     * its call of stMtnControlWorkFlag with 0, 11, 0). */
    void fn_8015FF60();
    /* Calls callback(part, userData) for each entry of the list at 0x4C until it returns zero
     * (behaviour inferred). */
    void fn_8015FA78(UnkModelPart_8015FA78 *(*callback)(UnkModelPart_8015FA78 *part,
                                                        void *userData),
                     void *userData);
    /* Methods that ThreatObj calls (purposes unknown). */
    void fn_8016271C();
    void fn_80162790();
    void fn_80160480(int arg0);
    /* Attaches a character info to the part with id arg0 and returns whether that succeeded:
     * false when the info has expired or no entry has that id, otherwise the result of
     * fn_80162084 (behaviour and result type inferred). */
    bool fn_80161FC8(int arg0, const boost::weak_ptr<CharInfo> &info);
    /* Returns the attack object of the part with id arg0 (behaviour inferred). */
    boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> fn_80161428(int arg0);
    /* Returns the entry of the array at 0x0 of the object at 0x30 whose id equals arg0, or null
     * (behaviour inferred). */
    UnkModelEntry_8015F7BC *fn_8015F7BC(int arg0);
    /* Finds the entry of the list at 0x64 with the id arg0, passes the member at 0x8 of that entry
     * to fn_8016193C and erases the entry from the list, returning whether there was one
     * (behaviour inferred from its body). */
    bool fn_80161A24(int arg0);
    /* Looks up an entry of the list at 0x58 by arg2 and an entry of the array at 0x18 of the
     * object at 0x30 by arg0, and passes both on with info (behaviour inferred from its body; the
     * result is not used by FallBuildingHolder). */
    int fn_80161BD8(int arg0, const boost::weak_ptr<CharInfo> &info, int arg2);
    /* Returns the motion control of the part with id arg0 (behaviour
     * inferred). */
    boost::weak_ptr<_stMtnPlayWork> fn_801602B4(int arg0);
    /* Returns whether the table that fn_80162A68 searches has an entry whose id word equals arg0,
     * or false when there is no object at 0x30 (behaviour inferred from its body). */
    bool fn_80162EA8(int arg0);
    /* Returns the atomic state (the member at 0x1C of a part) of the first part of the list at
     * 0x4C for which the word at 0x10 of the object that the state's first word points to equals
     * arg0, or null (behaviour inferred from its body). */
    UnkModelAtomicState *fn_80162298(int arg0);
    /* Calls fn_80456C54 with the callback fn_80164014 for the clump of each part of the list at
     * 0x4C (purpose unknown). */
    void fn_8016232C();
    /* Finds the entry with the id arg0 in the table at 0x20 of the object at 0x30 and passes its
     * index to fn_8016248C, returning that result, or false when there is none (behaviour
     * inferred from its body). */
    bool fn_80162424(int arg0);
    /* Returns the probe of the first node of the list at 0x64 for which the word at 0x14 of the
     * object that the node's word at 0x14 points to equals arg0, or an empty pointer (behaviour
     * inferred from its body). */
    boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> fn_80161614(int arg0);
    /* Sets a motion control work flag (stMtnControlWorkFlag with 0, 1, 0) of each entry of the
     * list at 0x58 whose resource record has the word arg0 at 0x18 (behaviour inferred from its
     * body). */
    void fn_8016082C(int arg0);
    /* Returns the attack object that fn_800C4374 gives for the probe of the entry of the list at
     * 0x64 whose name equals name, or an empty pointer (behaviour inferred from its body). */
    boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack>
    fn_80161334(const char *name);
    /* Calls CharaColliBase::UnkSlot17(vector, selector) on the probe of each node of the list at
     * 0x64 whose id equals arg0, as fn_801617D0 passes its arguments to SetSize (behaviour
     * inferred). */
    void fn_80161740(int arg0, const Fn8040Vector3 *vector, int selector);
    /* Clear and set bit 1 of the flags at 0x3C (behaviour inferred from their bodies; the meaning
     * of the bit is unknown). */
    void fn_801612B8();
    void fn_801612E0();
    /* For each entry of the list at 0x64 whose part id (the word at 0x14 of the object at 0x14 of
     * the entry) equals arg0, pass arg1 to the second virtual method of the entry's object at 0xC
     * (behaviour inferred from its body). */
    void fn_8016111C(int arg0, int arg1);
    /* As fn_8016111C, but calls the third virtual method of those objects (behaviour inferred from
     * its body). */
    void fn_80161230(int arg0, int arg1);
    /* Clamps arg0 to the model indices (a negative index to the last, an index above the last
     * to 0), calls fn_80162804 and passes the index and arg1 to fn_80162AD0, then returns the
     * index (behaviour inferred from its body). */
    int fn_801625D8(int arg0, int arg1);
    /* Returns the atomic state of the first part of the list at 0x4C whose state's entry has a
     * string equal to name as its word at 0x0, or null (also when name is null; behaviour
     * inferred from its body). */
    UnkModelAtomicState *fn_801621F0(const char *name);
    /* Finds the entry of the table at 0x20 of the object at 0x30 whose word at 0x0 is a string
     * equal to name and passes its index to fn_8016248C, returning that result, or false when
     * there is none or name is null (behaviour inferred from its body). */
    bool fn_8016239C(const char *name);
    /* Stores arg0 at 0x40 and sets bit 4 of the flags at 0x3C; a negative arg0 stores 0 and sets
     * bit 5 instead (behaviour inferred from its body). */
    void fn_8016268C(float arg0);

    /* The constructor builds three callback objects at 0x4, 0x10 and 0x1C with the same function,
     * and the destructor destroys them with the same function. The gadgets bind their collision
     * handlers to the first (name inferred). */
    /* 0x04 */ ColliDetectCallback m_collisionCallback;
    /* 0x10 */ ColliDetectCallback field_0x10;
    /* 0x1C */ ColliDetectCallback field_0x1C;
    /* 0x28 */ u8 field_0x28[0x14]; /* Unrecovered members. */
    /* 0x3C */ UnkBitFlags_80014E04 field_0x3C;
    /* 0x40 */ u8 field_0x40[0xC];
    /* 0x4C */ UnkModelPartList field_0x4C;
    /* 0x58 */ UnkModelMotionList field_0x58;
    /* 0x64 */ u8 field_0x64[0x10]; /* Includes a third native container. */
};
CHECK_SIZEOF(GadgetModelInfo, 0x74);

/* Loads the model file fileName into a new GadgetModelInfo owned by the returned pointer (behaviour
 * inferred; arg0 is the first argument of the GadgetModelInfo constructor, always 1 in callers). */
extern "C" boost::shared_ptr<GadgetModelInfo> fn_8015F800(int arg0, const char *fileName);

/* Returns the word at 0x0 of the object at 0x34 of the model, or null when there is none: a name
 * (behaviour inferred from the ball, which looks it up with fn_80208888). */
extern "C" const char *fn_801626D8(GadgetModelInfo *model);

/* Native atomic callback; its original free/static-member spelling is unknown. */
extern "C" RpAtomic *fn_80164064(RpAtomic *atomic, void *data);

/* Looks for the frame with the id arg3 in the model's part arg2, stores the frame's world matrix in
 * matrix and returns whether it was found (behaviour inferred from its fn_8015FA78 callback). */
extern "C" bool fn_80169EF0(Fn8040Matrix34 *matrix, boost::shared_ptr<GadgetModelInfo> model,
                            int arg2, int arg3);

extern "C" {
/* Model effect helpers (behaviour inferred): the Coaster gadget fades its model in with
 * fn_8015FDD8 and ends the fade with fn_8015FD10, and the FootingBreak gadget passes the value of
 * a SetTransparentCommand to fn_8015FDD8. */
void fn_8015FD10(GadgetModelInfo *model);
void fn_8015FDD8(GadgetModelInfo *model, float fraction);
}
#endif

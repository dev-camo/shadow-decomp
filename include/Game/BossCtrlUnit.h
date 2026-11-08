#ifndef GAME_BOSSCTRLUNIT_H
#define GAME_BOSSCTRLUNIT_H

#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>
#include "layout_check.h"

class BossVarParam;

/* BossCtrlUnit and the classes below have their names and bases from RTTI. A boss creates its
 * control units and hands them to its BossVarParam (BossVarParam::AddCtrlUnit). Most unit classes
 * have BossCtrlUnit as a virtual base, so their constructors also take the compiler's flag that says
 * whether to construct the virtual base. Besides the constructors and destructors, Update and the
 * pure methods that BossModeCtrlUnit and BossCtrlUnit_CheckMotionAttribute add are declared;
 * BossCtrlUnit_FixLookCharCamera's own further virtual method is not declared yet. Regions marked
 * as unknown are set up by the constructors and not mapped yet. */
class BossCtrlUnit {
  public:
    /* Inline: every derived constructor expands it. */
    BossCtrlUnit(BossVarParam *param) : field_0x4(0), m_param(param) {}
    /* Inline: HeavyDog0410CtrlUnit's destructor (0x8024EA8C) stores BossCtrlUnit's vtable for the
     * virtual base instead of calling this destructor. The copy the linker keeps is 0x80231EF0. */
    virtual ~BossCtrlUnit() {}
    /* Pure (null in BossCtrlUnit's vtable). BossCtrl::Update calls it on each unit through
     * 0x80179BB0, passing the float at 0x20 of the global at 0x8057D6A0 (meaning unknown), and
     * tests the low byte of the result (name inferred). */
    virtual bool Update(float arg) = 0;

    /* 0x04 */ s32 field_0x4; /* several derived constructors store a value of their own here */
    /* 0x08 */ BossVarParam *m_param;
};
CHECK_SIZEOF(BossCtrlUnit, 0xC);

class BossCtrlUnit_CheckEvent : public BossCtrlUnit {
  public:
    BossCtrlUnit_CheckEvent(BossVarParam *param);
    virtual ~BossCtrlUnit_CheckEvent();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(BossCtrlUnit_CheckEvent, 0xC);

class BossCtrlUnit_CheckFirstDamage : public BossCtrlUnit {
  public:
    BossCtrlUnit_CheckFirstDamage(BossVarParam *param, s32 arg1, s32 arg2);
    virtual ~BossCtrlUnit_CheckFirstDamage();
    virtual bool Update(float arg);

    /* 0x0C */ s32 field_0xC;  /* the constructor's arg1 */
    /* 0x10 */ s32 field_0x10; /* the constructor's arg2 */
};
CHECK_SIZEOF(BossCtrlUnit_CheckFirstDamage, 0x14);

class BossCtrlUnit_DispGauge : public virtual BossCtrlUnit {
  public:
    BossCtrlUnit_DispGauge(BossVarParam *param);
    virtual ~BossCtrlUnit_DispGauge();
    virtual bool Update(float arg);
};
CHECK_SIZEOF(BossCtrlUnit_DispGauge, 0x18);

class BossModeCtrlUnit : public virtual BossCtrlUnit {
  public:
    BossModeCtrlUnit(BossVarParam *param);
    virtual ~BossModeCtrlUnit();
    virtual bool Update(float arg);
    /* Pure (null in BossModeCtrlUnit's vtable). Update (0x80292670) passes the word at 0xC of the
     * variables and keeps the returned unit at 0x8; the Egg Mecha override (0x802A7D68) creates a
     * control unit chosen by that value (name inferred). */
    virtual BossCtrlUnit *CreateUnit(int mode) = 0;

    /* The unit from CreateUnit: Update calls its Update and deletes it through its virtual
     * destructor. Cleared by the constructor. */
    /* 0x08 */ BossCtrlUnit *field_0x8;

  private:
    /* Copying is disabled in this reconstructed interface because the copy contract is
     * unrecovered: field_0x8 owns its unit (the destructor deletes it through its virtual
     * destructor and clears the pointer, 0x80292738-0x80292760), so a memberwise copy would share
     * the unit and both destructors would delete it. Declared and not defined. */
    BossModeCtrlUnit(const BossModeCtrlUnit &);
    BossModeCtrlUnit &operator=(const BossModeCtrlUnit &);
};
CHECK_SIZEOF(BossModeCtrlUnit, 0x1C);

class BossCtrlUnit_CheckMotionAttribute : public virtual BossCtrlUnit {
  public:
    BossCtrlUnit_CheckMotionAttribute(BossVarParam *param);
    virtual ~BossCtrlUnit_CheckMotionAttribute();
    virtual bool Update(float arg);
    /* Pure (null in the class's vtable). Update (0x80290CBC) calls it for each event that
     * fn_80415854 reports for the motion at 0x3C of the variables, with the 16-bit event id and the
     * event's payload, and ignores any result (name inferred). */
    virtual void OnMotionEvent(u16 id, void *payload) = 0;
};
CHECK_SIZEOF(BossCtrlUnit_CheckMotionAttribute, 0x18);

class BossCtrlUnit_FixLookCharCamera : public virtual BossCtrlUnit {
  public:
    BossCtrlUnit_FixLookCharCamera(BossVarParam *param);
    virtual ~BossCtrlUnit_FixLookCharCamera();
    virtual bool Update(float arg);

    /* 0x08 */ u8 field_0x8[0x30 - 0x8]; /* unknown */
};
CHECK_SIZEOF(BossCtrlUnit_FixLookCharCamera, 0x40);

/* BossCtrlUnit_StageObject: constructor at 0x80290920. It sets field_0x4 to 3, keeps arg3 at 0xC
 * and a copy of *position at 0x10, and builds a matrix from that position and the three angles at
 * rotation (behaviour inferred; the parameter names are inferred, and the meanings of arg4-arg6
 * are unknown). */
class BossCtrlUnit_StageObject : public BossCtrlUnit {
  public:
    BossCtrlUnit_StageObject(BossVarParam *param, const Fn8040Vector3 *position,
                             const Fn8040Vector3 *rotation, float arg3, int arg4, int arg5,
                             int arg6);
    virtual ~BossCtrlUnit_StageObject();
    virtual bool Update(float arg);

    /* 0x0C */ float field_0xC;
    /* 0x10 */ Fn8040Vector3 field_0x10;
    /* 0x1C */ u8 field_0x1C[0x28 - 0x1C]; /* unknown */
};
CHECK_SIZEOF(BossCtrlUnit_StageObject, 0x28);

/* BossCtrlUnit_Shadow: constructor at 0x802904D8. It sets field_0x4 to 3 and keeps arg at 0xC; the
 * region from 0x10 is constructed by 0x8009368C and not mapped. */
class BossCtrlUnit_Shadow : public BossCtrlUnit {
  public:
    BossCtrlUnit_Shadow(BossVarParam *param, float arg);
    virtual ~BossCtrlUnit_Shadow();
    virtual bool Update(float arg);

    /* 0x0C */ float field_0xC;
    /* 0x10 */ u8 field_0x10[0x38 - 0x10]; /* unknown */
};
CHECK_SIZEOF(BossCtrlUnit_Shadow, 0x38);

/* BossCtrlUnit_PlayCameraMotion: constructors at 0x802922E0 and 0x802923C4. The first builds a
 * matrix from the vector at position and the three angles at rotation; the second takes them from
 * the character info of param. Both then pass arg1, arg4 and arg5 to 0x80291E0C (behaviour
 * inferred; the parameter names are inferred, and the meanings of arg1, arg4 and arg5 are
 * unknown). */
class BossCtrlUnit_PlayCameraMotion : public BossCtrlUnit {
  public:
    BossCtrlUnit_PlayCameraMotion(BossVarParam *param, int arg1, const Fn8040Vector3 *position,
                                  const UnkRotation3 *rotation, float arg4, float arg5);
    BossCtrlUnit_PlayCameraMotion(BossVarParam *param, int arg1, float arg4, float arg5);
    virtual ~BossCtrlUnit_PlayCameraMotion();
    virtual bool Update(float arg);

    /* 0x0C */ u8 field_0xC[0x54 - 0xC]; /* unknown */

  private:
    /* Copying is disabled in this reconstructed interface because the copy contract is
     * unrecovered: the destructor releases a shared count at 0x1C and a weak count at 0x14
     * (0x8029180C, 0x80291818), which the bytes here would copy without taking a reference.
     * Declared and not defined. */
    BossCtrlUnit_PlayCameraMotion(const BossCtrlUnit_PlayCameraMotion &);
    BossCtrlUnit_PlayCameraMotion &operator=(const BossCtrlUnit_PlayCameraMotion &);
};
CHECK_SIZEOF(BossCtrlUnit_PlayCameraMotion, 0x54);

#endif

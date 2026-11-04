#ifndef GAME_BOSSCTRLUNIT_H
#define GAME_BOSSCTRLUNIT_H

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
    virtual ~BossCtrlUnit();
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

#endif

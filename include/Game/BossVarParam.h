#ifndef GAME_BOSSVARPARAM_H
#define GAME_BOSSVARPARAM_H

#include "Game/BossCharInfo.h"
#include "Game/EffectLaunchInfoUSA.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class BossCtrlUnit;
class BossRender;
class UnkBossCharaColliImpl_8017956C;
class UnkBossTargetImpl_8017B218;
class UnkCtrlUnitList_802A451C;
class UnkSphere_80035CEC;
struct RpClump;
struct Unk801A35ACParameters;
struct _stMtnPlayWork;

/* The parameter tables that a boss's Init stores in BossVarParam::field_0x4 (placeholder names;
 * most of the code that reads them is not reconstructed, so the fields are mostly unnamed). */

/* An entry of the shape list that a record points to. The reader at 0x800C3778 walks the list in
 * steps of 0x10 bytes and stops at the entry whose word at 0x4 is null. */
struct UnkBossParamRecordData {
    /* 0x00 */ u32 field_0x0;
    /* 0x04 */ UnkSphere_80035CEC *field_0x4;
    /* 0x08 */ s32 field_0x8;
    /* 0x0C */ s32 field_0xC;
};
CHECK_SIZEOF(UnkBossParamRecordData, 0x10);

struct UnkBossParamRecord {
    /* 0x00 */ const UnkBossParamRecordData *field_0x0;
    /* 0x04 */ s32 field_0x4;
    /* 0x08 */ s32 field_0x8;
    /* 0x0C */ float field_0xC;
    /* 0x10 */ s32 field_0x10;
    /* 0x14 */ s32 field_0x14;
    /* 0x18 */ u8 field_0x18;
    /* 0x1C */ s32 field_0x1C;
    /* 0x20 */ u8 field_0x20;
};
CHECK_SIZEOF(UnkBossParamRecord, 0x24);

/* An entry of the list at UnkBossParamTable::field_0x18. The BossTarget task's object (0x8017B238)
 * walks the list in steps of 0xC bytes and stops at the entry whose first word is 0. */
struct UnkBossParamRecord2 {
    /* 0x00 */ u32 field_0x0;
    /* 0x04 */ s32 field_0x4;
    /* 0x08 */ Fn8040Vector3 *field_0x8;
};
CHECK_SIZEOF(UnkBossParamRecord2, 0xC);

/* The optional record at UnkBossParamTable::field_0x1C (null-tested at 0x80179118): a sphere's
 * centre and radius (behaviour inferred). The reader transforms the vector that the first word
 * points to by a matrix (0x8017917C) and passes the result and the float at 0x4 (0x80179198) to
 * the UnkSphere_80021C04 constructor as centre and radius; the HEAVY_DOG_0410 unit's record is an
 * 8-byte object in .sdata. */
struct UnkBossParamRecord3 {
    /* 0x00 */ Fn8040Vector3 *field_0x0;
    /* 0x04 */ float field_0x4;
};
CHECK_SIZEOF(UnkBossParamRecord3, 0x8);

struct UnkBossParamTable {
    /* 0x00 */ s32 field_0x0;
    /* 0x04 */ s32 field_0x4;
    /* 0x08 */ s32 field_0x8;
    /* 0x0C */ s32 field_0xC;
    /* 0x10 */ s32 field_0x10;
    /* 0x14 */ UnkBossParamRecord *field_0x14;
    /* 0x18 */ UnkBossParamRecord2 *field_0x18;
    /* Optional (null-tested at 0x80179118). */
    /* 0x1C */ UnkBossParamRecord3 *field_0x1C;
};
CHECK_SIZEOF(UnkBossParamTable, 0x20);

/* BossVarParam (class name from RTTI): the state that a boss task (BossRootTask) shares with its
 * control units. The layout comes from its constructor (0x802332D4); the names are placeholders
 * apart from the character info and the control unit list (inferred from their uses). */
class BossVarParam {
  public:
    BossVarParam();
    /* Inline (the copy the linker keeps is 0x80232440), which is why the boss units emit
     * BossVarParam's RTTI name ahead of the derived class's. */
    virtual ~BossVarParam() {}

    /* Wraps unit in a shared pointer and adds it to the control unit list at 0x68 (behaviour and
     * name inferred). */
    void AddCtrlUnit(BossCtrlUnit *unit);

    /* 0x04 */ UnkBossParamTable *field_0x4; /* not set by the constructor */
    /* 0x08 */ float field_0x8;
    /* The next three words are used by BossModeCtrlUnit::Update (0x802925A0) as the current,
     * previous and requested mode (the word "mode" is inferred from the class names). When Update
     * has a unit, it calls the unit's Update unless field_0x14 is already non-zero; if field_0x14
     * is then non-zero, it deletes the unit and copies field_0xC to field_0x10. Only this path
     * copies the previous mode. When Update has no unit (from the start or after that deletion),
     * it moves field_0x14 to field_0xC, even when it is zero, clears field_0x14 and passes the
     * value to CreateUnit. If field_0x14 is non-zero after CreateUnit, it deletes the new unit and
     * takes the no-unit path again, without the copy. */
    /* 0x0C */ s32 field_0xC;
    /* 0x10 */ s32 field_0x10;
    /* 0x14 */ s32 field_0x14;
    /* 0x18 */ u8 field_0x18;
    /* 0x19 */ u8 field_0x19;
    /* 0x1A */ u8 field_0x1A;
    /* 0x1B */ u8 field_0x1B;
    /* 0x1C */ boost::shared_ptr<BossCharInfo> m_charInfo;
    /* 0x24 */ float field_0x24; /* not set by the constructor */
    /* 0x28 */ float field_0x28;
    /* The BossRender task's object stores the next two (0x8017AC38, 0x8017AC44). */
    /* 0x2C */ RpClump *field_0x2C;
    /* 0x30 */ Unk801A35ACParameters *field_0x30;
    /* A task constructed at 0x8017A880 (class BossRender from RTTI, with Task as its base; task
     * name "Render"). */
    /* 0x34 */ BossRender *field_0x34;
    /* 0x38 */ u8 field_0x38[4];           /* not set by the constructor */
    /* 0x3C */ _stMtnPlayWork *field_0x3C; /* fn_8041B908's result (0x8017A704) */
    /* 0x40 */ float field_0x40;
    /* Not set by the constructor. The HEAVY_DOG_0410 and HEAVY_DOG_0510 mode units pass it plus 1
     * to 4, as a 16-bit motion id, to fn_80417568 with the motion at 0x3C. The signedness is
     * inferred. */
    /* 0x44 */ s32 field_0x44;
    /* 0x48 */ Fn8040Vector3 field_0x48;
    /* The object of the BossCharaColli task, which stores itself here (0x801797E4). */
    /* 0x54 */ UnkBossCharaColliImpl_8017956C *field_0x54;
    /* 0x58 */ float field_0x58;
    /* 0x5C */ float field_0x5C;
    /* 0x60 */ float field_0x60;
    /* The object of the BossTarget task, which stores itself here (0x8017B2B8). */
    /* 0x64 */ UnkBossTargetImpl_8017B218 *field_0x64;
    /* 0x68 */ UnkCtrlUnitList_802A451C *m_ctrlUnits;
    /* The pointee type is inferred: boss code converts this pointer to a shared pointer to
     * EffectLaunchInfo for EffectManager::Create, and each boss's Init copies the shared launcher
 * (lbl_805F0240) into it. */
    /* 0x6C */ boost::shared_ptr<SonicteamUSA::System::Effect::EffectLaunchInfoUSA> field_0x6C;
};
CHECK_SIZEOF(BossVarParam, 0x74);

#endif

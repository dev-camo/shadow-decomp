#ifndef GAME_MOTIONSEQUENCEMODULE_H
#define GAME_MOTIONSEQUENCEMODULE_H

#include "Game/UnkEnemyParaData_8017B8E4.h"
#include <dolphin/types.h>
#include "layout_check.h"

struct _stMtnPlayWork;

/* MotionSequenceModule (class name from RTTI, with no bases): plays the motions of a sequence
 * table (an UnkEnemyParaData_8017B8E4) on a motion control. A group of the table lists the motions
 * to play one after the other; fn_800C9554 marks an entry whose motion has ended and fn_800C9178
 * then moves on to the next entry (behaviour inferred). EnemyMotionSequenceModule derives from it
 * (its RTTI base list at 0x8053ACD0), and its destructor at 0x8018EAA4 has this destructor
 * expanded inline.
 *
 * The members declared without a body are defined at 0x800C9178-0x800C9690, apart from the
 * constructor (see Vine.cpp); the kept copies of fn_800C954C and fn_800C9528 are there too. The
 * virtual functions are inline, so the vtable and the type record are emitted by each file that
 * needs them (the kept ones are in the FIRE_VINE file's data). Member names are placeholders
 * unless a comment says otherwise. */
class MotionSequenceModule {
  public:
    MotionSequenceModule();
    virtual ~MotionSequenceModule() {}
    /* Returns false (0x800C954C); fn_800C9178 calls it while the group loops (purpose unknown). */
    virtual bool fn_800C954C() {
        return false;
    }
    /* Selects the group whose index is field_0x24 (0x800C9528). */
    virtual bool fn_800C9528() {
        return fn_800C9484(field_0x24);
    }

    /* Advances the sequence on motion by dt, moving on to the next entry when fn_800C9554 has
     * marked the current one. When it starts an entry, it passes the low 16 bits of motionBase - 1
     * plus the resolved motion number to fn_80417568: the entry's motion number or, when that is
     * negative, that of the first entry of the table's group field_0x24 (behaviour inferred). */
    void fn_800C9178(_stMtnPlayWork *motion, int motionBase, float dt);
    /* When motion is not null, records in field_0x1B whether stMtnControlWorkFlag(motion, 0, flag,
     * 1) reports flag 3 set and flag 5 clear. When it does, a looping entry counts the loop in
     * field_0x30; otherwise m_finished is set when the entry is the group's last, and the entry is
     * marked (field_0x1C) for fn_800C9178 to advance; it does not advance the entry itself
     * (behaviour inferred). */
    void fn_800C9554(_stMtnPlayWork *motion);
    /* When group and its entries pointer are not null, makes group the current group and returns
     * true; a zero count is not rejected. Otherwise returns false. Either way it restarts the
     * entry state (no current motion, entry index 0) (behaviour inferred). */
    bool fn_800C961C(const UnkEnemyParaGroup_800C961C *group);
    /* When the table is not null and index is below its group count (a signed compare; a negative
     * index is not rejected), does what SelectGroup does and returns fn_800C961C's result;
     * otherwise returns false and changes nothing (behaviour inferred). */
    bool fn_800C9484(int index);

    /* Makes the table's group index the current group when the table is not null and index is
     * below its group count, with no negative check (name inferred); fn_800C9484 does the same
     * out of line. */
    void SelectGroup(int index) {
        if (m_data != 0 && index < m_data->count) {
            m_groupIndex = index;
            fn_800C961C(&m_data->groups[index]);
        }
    }

    /* 0x04 */ const UnkEnemyParaData_8017B8E4 *m_data;
    /* 0x08 */ const UnkEnemyParaGroup_800C961C *m_group;
    /* 0x0C */ const UnkEnemyParaGroup_800C961C *m_previousGroup;
    /* The group's byte at 0x5; selects what follows the group's last entry (inferred). */
    /* 0x10 */ s32 field_0x10;
    /* 0x14 */ s32 m_motion; /* the current entry's motion number, or -1 */
    /* 0x18 */ u8 field_0x18;
    /* 0x19 */ u8 m_loop;       /* set while the current entry loops (inferred) */
    /* 0x1A */ bool m_finished; /* set once the last entry of the group has ended (inferred) */
    /* 0x1B */ u8 field_0x1B;
    /* 0x1C */ u8 field_0x1C;
    /* 0x1D */ u8 field_0x1D;
    /* 0x1E */ u8 field_0x1E;
    /* 0x20 */ s32 m_groupIndex;
    /* 0x24 */ s32 field_0x24;
    /* 0x28 */ s32 m_entryCount;
    /* 0x2C */ s32 m_entryIndex;
    /* 0x30 */ s32 field_0x30;
    /* 0x34 */ float field_0x34;
    /* 0x38 */ float field_0x38;
};
CHECK_SIZEOF(MotionSequenceModule, 0x3C);

#endif

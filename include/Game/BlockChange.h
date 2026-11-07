#ifndef GAME_BLOCKCHANGE_H
#define GAME_BLOCKCHANGE_H

#include "Game/CharaColliBase.h"
#include "Game/Gadget.h"
#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the BlockChange descriptor's table, whose entry names give
 * the member names. ReadOwner (called by the constructor, and by Update in a branch that never
 * runs in this build) writes them, and only when the owner's parameter block is 0x1C bytes
 * (fn_800C9DBC compares the size at 0x800C9DD4). */
struct BlockChangeParams {
    /* "Type"; this file does not read it */
    /* 0x00 */ s32 type;
    /* "On/Off" */
    /* 0x04 */ s32 onOff;
    /* "Radius": the probe sphere's radius */
    /* 0x08 */ float radius;
    /* The four "BLOCK" entries; ProcessHits skips negative values. */
    /* 0x0C */ s32 blocks[4];
};
CHECK_SIZEOF(BlockChangeParams, 0x1C);

/* BlockChange (class name from RTTI, with Task as its only base; the descriptor's name is also
 * "BlockChange"): a task, derived from Task directly, that keeps a probe sphere of the "Radius"
 * parameter at the owner's position. For each hit of the probe whose collision object and owning
 * character info (fn_800C4568) both still exist, when that character info's word at 0x5C is 1, it
 * is a Player::Player and its word at 0x54 is 0 or more (no upper bound is checked), and
 * lbl_8051CE88 holds an object at that index, each "BLOCK" value that is not negative is passed
 * to fn_8003AAB0 for that object, with 1 when "On/Off" is not 0 and 2 when it is 0 (behaviour
 * inferred). Method names marked "(name inferred)" and the member names are not original. */
class BlockChange : public Task {
  public:
    BlockChange(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    /* When there is a probe, passes it to fn_800C78B0 with the world object of fn_80021B08; then
     * releases the owner as m_owner describes. */
    virtual ~BlockChange();
    /* With an owner: when fn_800C9D0C and fn_800C9D14 are both true, re-reads the position and
     * the parameters, moves the probe to the position and sets its radius (the probe pointer is
     * not checked) and calls fn_800C9CF8 (fn_800C9D0C is always false in this build, see
     * Gadget.h, so this step never runs); then, when fn_800C9A64 or else fn_800C996C is true for
     * the owner, sets Task flag 0x01 and returns without calling ProcessHits. Otherwise, and
     * always without an owner, it calls ProcessHits in the same update. */
    virtual void Update(float dt);

    /* Creates the task under task layer 11 when its allocation succeeds (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position into m_position and its parameter block into m_params (see
     * BlockChangeParams) (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
        fn_800C9F14(owner, &m_position);
        fn_800C9DBC(owner, &m_params, sizeof(m_params));
    }
    /* Walks the probe's hits (the probe pointer is not checked) and acts on the qualifying
     * character infos as the class comment describes (name inferred). The float is the time step
     * that Update passes; the function does not use it. */
    void ProcessHits(float dt);

    static const char *s_name;

    /* 0x28 */ Fn8040Vector3 m_position;
    /* Set by the constructor only when its owner argument is not null; the constructor passes it
     * to ReadOwner either way. The destructor releases it (fn_800C9BE8) and clears it when it is
     * set. */
    /* 0x34 */ UnkGadgetOwner_800C9CD0 *m_owner;
    /* 0x38 */ BlockChangeParams m_params;
    /* 0x54 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(BlockChange, 0x60);

#endif

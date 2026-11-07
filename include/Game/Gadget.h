#ifndef GAME_GADGET_H
#define GAME_GADGET_H

#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

class SetAdapterState;
class UnkGadgetOwner_800C9CD0;
class UnkOwnerManager_800C9B84;

extern "C" {
/* Owner lists indexed by the byte id; original names are unknown. */
UnkOwnerManager_800C9B84 *fn_800C9B84();
UnkGadgetOwner_800C9CD0 *fn_800CB560(UnkOwnerManager_800C9B84 *manager, int id);
/* Passes the manager's word at 0x0 to fn_800CA098 when it is not null (behaviour from its body;
 * the SAVEPOINT gadget calls it when it saves). */
void fn_800CB508(UnkOwnerManager_800C9B84 *manager);
/* Returns the unsigned 16-bit kind stored in the owner's record. */
u16 fn_800C9D24(UnkGadgetOwner_800C9CD0 *owner);
/* Copy the owner's position, its rotation converted from degrees to radians, and its byte at
 * +0x22 (behaviour inferred). */
void fn_800C9F14(UnkGadgetOwner_800C9CD0 *owner, Fn8040Vector3 *position);
void fn_800C9ED4(UnkGadgetOwner_800C9CD0 *owner, UnkRotation3 *rotation);
/* CandleStick's parameter SAME_COMID_SWITCH_NUM suggests that the byte at +0x22 is the com id
 * (CandleStick.cpp stores it as m_comId). */
void fn_800C9E60(UnkGadgetOwner_800C9CD0 *owner, u8 *value);
/* Stores byte 0x22 of the owner's record (the byte fn_800C9E60 copies), widened to a word
 * (behaviour inferred; CircusBall reads its key id with it). */
void fn_800C9E50(UnkGadgetOwner_800C9CD0 *owner, s32 *value);
/* Copies the owner's parameter block into params when its size equals size, and otherwise flags
 * the owner (behaviour inferred). */
void fn_800C9DBC(UnkGadgetOwner_800C9CD0 *owner, void *params, int size);
/* Registers task with the owner when the task is constructed, and releases it when the task is
 * destroyed (behaviour inferred). */
void fn_800C9CD0(UnkGadgetOwner_800C9CD0 *owner, Task *task);
void fn_800C9BE8(UnkGadgetOwner_800C9CD0 *owner);
/* Owner state queries and the call that follows two of them (behaviour inferred; the original
 * names are unknown). Gadget::IsFinished marks the task finished when fn_800C9A64 or fn_800C996C
 * is true. fn_800C9D0C returns false unconditionally in this build. */
bool fn_800C9D0C(UnkGadgetOwner_800C9CD0 *owner);
bool fn_800C9D14(UnkGadgetOwner_800C9CD0 *owner);
void fn_800C9CF8(UnkGadgetOwner_800C9CD0 *owner);
bool fn_800C9A64(UnkGadgetOwner_800C9CD0 *owner);
bool fn_800C996C(UnkGadgetOwner_800C9CD0 *owner);
/* The writing counterpart of fn_800C9DBC: copies size bytes of params into the owner's parameter
 * block when its size equals size, and otherwise flags the owner (behaviour inferred). */
void fn_800C9D68(UnkGadgetOwner_800C9CD0 *owner, const void *params, int size);
/* Clears bit 0 of the word at 0x18 of the record that owner points to, then calls fn_800C9BE8,
 * fn_800C9B84 and fn_800CB534 (purpose unknown). ThreatObjHolder calls it when its ThreatObj is
 * finished. */
void fn_800C9B34(UnkGadgetOwner_800C9CD0 *owner);
/* Sets bit 16 of the word at 0x18 of the record that owner points to, then calls fn_800C9B34
 * (behaviour inferred from its body). Gadgets call it before marking themselves finished. */
void fn_800C9AD4(UnkGadgetOwner_800C9CD0 *owner);
/* Sets bit 3 of the word at 0x18 of the record that owner points to (behaviour inferred from its
 * body). */
void fn_800C985C(UnkGadgetOwner_800C9CD0 *owner);
/* Stores state in the owner's word at 0x14 (behaviour inferred from its body). */
void fn_800C9930(UnkGadgetOwner_800C9CD0 *owner, SetAdapterState *state);
/* Deletes the object in the owner's word at 0x14 through its virtual destructor and clears the
 * word (behaviour inferred from its body). RollCircle calls it before it stores a new state. */
void fn_800C9894(UnkGadgetOwner_800C9CD0 *owner);
/* Sets bit 3 of the word at 0x18 of the record that owner points to, then calls fn_800C9B34
 * (behaviour inferred from its body). ArkCannon calls it when it is destroyed and BombingWall when
 * it breaks. */
void fn_800C9B04(UnkGadgetOwner_800C9CD0 *owner);
/* Stores the word at 0x18 of the record that owner points to in *flags (behaviour inferred from
 * its body). MagmaGuard tests bit 0x400000 of it. */
void fn_800C9EC4(UnkGadgetOwner_800C9CD0 *owner, u32 *flags);
/* Sets bits in the word at 0x18 of the record that owner points to (behaviour inferred from its
 * body). MagmaGuard sets bit 0x400000 when it is destroyed. */
void fn_800C9EB0(UnkGadgetOwner_800C9CD0 *owner, u32 bits);
/* Returns 100 times one more than the byte at 0x23 of the record that owner points to (behaviour
 * inferred from its body; Coaster compares a distance with it, BombServer passes it to the items
 * it spawns). */
float fn_800C9938(UnkGadgetOwner_800C9CD0 *owner);
/* Stores the word at 0x1C of the record that owner points to in *flags (behaviour inferred from
 * its body). Coaster and BreakableOrnament test bit 0x400000 of it. */
void fn_800C9E70(UnkGadgetOwner_800C9CD0 *owner, u32 *flags);
/* Returns whether every bit of bits is set in the word at 0x18 of the record that owner points to
 * (behaviour and spellings inferred from its body; the SECRETDOOR gadget tests bit 0x400000). */
bool fn_800C9E80(UnkGadgetOwner_800C9CD0 *owner, u32 bits);
/* True when neither bit 16 nor bit 0 of the word at 0x18 of the record that owner points to is
 * set (behaviour from its body; purpose unknown). The RING gadget's destructor leaves the owner's
 * state alone while it is true. */
bool fn_800C9870(UnkGadgetOwner_800C9CD0 *owner);
/* Sets bit 0x20000000 of the word at 0x18 of the record that owner points to (behaviour inferred
 * from its body). */
void fn_800C9690(UnkGadgetOwner_800C9CD0 *owner);
}

/* Gadget (class name from RTTI): a stage object task registered with an owner object. */
class Gadget : public Task {
  public:
    Gadget(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Gadget();
    /* Runs UpdateBody unless IsFinished reports the gadget done (name inferred). */
    virtual void Update(float dt);
    /* Per-frame work of the concrete gadget (name inferred). */
    virtual void UpdateBody(float dt) = 0;
    /* ElecBar's override passes dt on to its bars' updates; the other known overrides ignore it
     * (parameter meaning inferred from that use). */
    virtual void fn_800CBC1C(float dt);
    /* Re-reads the owner's settings (name inferred). */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) = 0;
    /* Sets Task flag 0x01 and returns true when the first owner query is true, or else when the
     * second is; the second is not made when the first is true (name inferred). */
    virtual bool IsFinished();

    /* The class name the constructor stores in m_name (name inferred). */
    static const char *s_name;

    /* 0x28 */ UnkGadgetOwner_800C9CD0 *m_owner;
    /* 0x2C */ u32 field_0x2C;
};
CHECK_SIZEOF(Gadget, 0x30);

/* Passes the gadget's owner (m_owner) to fn_800C9B34 when there is one (behaviour inferred). */
extern "C" void fn_8015A794(Gadget *gadget);
/* Passes the gadget's owner (m_owner) to fn_800C9B04 when there is one (behaviour inferred). */
extern "C" void fn_8015A768(Gadget *gadget);
/* Sets bit 0x20000000 of the word at 0x18 of the owner's record when m_owner is set
 * (behaviour inferred). Original free-function or member spelling is unrecovered. */
extern "C" void fn_8015A7EC(Gadget *gadget);
/* Return whether the gadget has an owner (m_owner) and fn_800C9A64 (fn_8015A858) or fn_800C996C
 * (fn_8015A818) is true for it, the two owner queries that Gadget::IsFinished makes (behaviour
 * inferred from their bodies). */
extern "C" bool fn_8015A858(Gadget *gadget);
extern "C" bool fn_8015A818(Gadget *gadget);
/* Passes the gadget's owner (m_owner) to fn_800C9AD4 when there is one (behaviour inferred). */
extern "C" void fn_8015A7C0(Gadget *gadget);
/* A 16-bit 2 in .sdata2 right after Gadget's file (USA (GUPE8P) 0x805F4A80; name, meaning and
 * defining file unknown). MWCC places a const definition ahead of the RTTI names it pools when
 * it generates code, so in Gadget's file it would precede "Task", which follows "Gadget" there.
 * The ELEVATOR gadget passes it as the last argument of fn_801D53C4.
 * Matching workaround: declared without const. In each of the four fn_801D53C4 calls of
 * Elevator::UpdateBody the original loads m_loopSound and then this value, both after the
 * fn_80053164 call (USA: the call at 0x8014877C, the loads at 0x80148780 and 0x80148788 in the
 * first). Declared const, the calls stay in place and the two loads swap (8 words differ). Its
 * place in .sdata2 suggests a const definition; whether the original declaration had const is
 * inferred from this load order only. Revisit when its definition is recovered. */
extern s16 lbl_805F4A80;

#endif

#ifndef GAME_MESSAGE_H
#define GAME_MESSAGE_H

#include <dolphin/types.h>
#include "layout_check.h"

/* A 0x18-byte record of a mission's message list (placeholder name; the lists are the arrays that
 * fn_802EEBF8 returns). Message::fn_802EEAF0 passes the word at 0x0 to fn_801FAF00 as its id.
 * BasicMessageManage allocates a message for each record whose word at 0x4 is 0, and
 * TimerMessageManage for each one whose word at 0x4 is 2; each constructs the message only when
 * the allocation succeeds. When the word at 0x8 is 0, 1 or 2, TimerMessageManage derives a value
 * from the float at 0x10 in a way chosen by that word; for any other value it stores a constant
 * without reading the float. */
struct UnkMessageRecord_802EEBF8 {
    /* 0x00 */ int field_0x0;
    /* 0x04 */ int field_0x4;
    /* 0x08 */ int field_0x8;
    /* 0x0C */ int field_0xC;
    /* 0x10 */ float field_0x10;
    /* 0x14 */ int field_0x14;
};
CHECK_SIZEOF(UnkMessageRecord_802EEBF8, 0x18);

namespace Mission {

/* Mission::MessageManage (class name from RTTI): the base of the message managers that a mission
 * and its count and timer parts hold, 0x10 bytes. Its three virtual functions are pure; they are
 * named after BasicMessageManage's implementations, and CountMission and TimerMission call them
 * (slots 0, 1 and 2) from their fn_8016AF40, fn_8016AF44 and fn_8016AF48. */
class MessageManage {
  public:
    /* Stores index at 0xC and the message list of mission id (fn_802EEBF8) and its length
     * (fn_802EEB9C) at 0x4 and 0x8. Mission passes its own id and index (parameter types
     * inferred). */
    MessageManage(int id, int index);
    /* Empty and not virtual (the vtable has no destructor slot). BasicMessageManage's destructor
     * stores this class's vtable after destroying its own members; with the implicitly declared
     * destructor that store is left out. */
    ~MessageManage() {}
    virtual void fn_802EBDF0() = 0;
    virtual void fn_802EBCF0() = 0;
    virtual void fn_802EB9E0() = 0;

    /* 0x04 */ const UnkMessageRecord_802EEBF8 *field_0x4;
    /* 0x08 */ u32 field_0x8;
    /* 0x0C */ int field_0xC;
};
CHECK_SIZEOF(MessageManage, 0x10);

/* Mission::Message (class name from RTTI): the base of the messages that the managers create, one
 * per record, 0x8 bytes. */
class Message {
  public:
    Message(const UnkMessageRecord_802EEBF8 *record);
    virtual ~Message();
    /* Passes the record's word at 0x0, fn_801FAAC4(arg), 0 and 0 to fn_801FAF00 on the manager
     * that fn_80109D64 returns (the void return is inferred). */
    virtual void fn_802EEAF0(int arg);

    /* 0x04 */ const UnkMessageRecord_802EEBF8 *field_0x4;
};
CHECK_SIZEOF(Message, 0x8);

} // namespace Mission

/* The message lists (0x802EEB9C-0x802EEC4C). The record tables start a new file at 0x80507EF8;
 * these two functions are taken to belong to it (inferred). For an id from 0 to 40 they return
 * the list of that mission or its length from one of two tables of 41 entries, chosen by the word
 * at 0x38 of lbl_8057D6A0 (the second when it is 5), and null or 0 for any other id (behaviour
 * inferred from their bodies). */
extern "C" {
u32 fn_802EEB9C(int id);
const UnkMessageRecord_802EEBF8 *fn_802EEBF8(int id);
}

#endif

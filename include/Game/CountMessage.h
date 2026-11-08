#ifndef GAME_COUNTMESSAGE_H
#define GAME_COUNTMESSAGE_H

#include "Game/Message.h"
#include "boost/shared_ptr.hpp"
#include <vector>
#include "layout_check.h"

namespace Mission {

/* The count that stands for none (name inferred): CountMessageManage::field_0x14 while
 * field_0x18 is empty. */
const int MESSAGE_COUNT_NONE = -1;

/* Mission::CountMessage (class name from RTTI): the message that CountMessageManage creates for
 * each record whose word at 0x4 is 1. */
class CountMessage : public Message {
  public:
    /* The record's word at 0x8 selects field_0x8: the record's word at 0xC for 0, count times the
     * record's float at 0x10 rounded up for 1, count minus the word at 0xC for 2, and 0 for any
     * other value. The parameter name is inferred. The word at 0xC and the float are read through
     * field_0x4 (the member the base class stored), as the original does; reading them through
     * the parameter changes the code. */
    CountMessage(const UnkMessageRecord_802EEBF8 *record, int count);
    virtual ~CountMessage();

    /* 0x08 */ int field_0x8;
};
CHECK_SIZEOF(CountMessage, 0xC);

/* Mission::CountMessageManage (class name from RTTI): the message manager that a
 * Mission::CountMission holds at 0x28. It keeps the messages it has created in field_0x18, sorted
 * by their field_0x8 in descending order, and a snapshot of that list in field_0x24. The snapshot
 * is empty after construction and after fn_802EB9E0, which clear both lists and fill only
 * field_0x18. fn_802EBDF0 saves field_0x18 into it and fn_802EBCF0 restores field_0x18 from it;
 * a Mission::CountMission calls them from its fn_8016AF40 and fn_8016AF44 (USA 0x8016AB34 and
 * 0x8016AAF0). The removals in fn_802ED4E0 leave the snapshot unchanged. */
class CountMessageManage : public MessageManage {
  public:
    /* Stores count at 0x10 and MESSAGE_COUNT_NONE at 0x14, reserves room in each list for as many
     * messages as the message list has records, then builds the messages (fn_802EB9E0, called
     * through the vtable). A Mission::CountMission passes the argument it stores at 0x8. The
     * parameter types of id and index are inferred, as for MessageManage, and so is the name of
     * count. */
    CountMessageManage(int id, int index, int count);
    ~CountMessageManage();
    /* Saves the snapshot: replaces field_0x24 with the current messages of field_0x18. */
    virtual void fn_802EBDF0();
    /* Restores the snapshot: replaces field_0x18 with the messages of field_0x24, then updates
     * field_0x14 (fn_802ECF8C). */
    virtual void fn_802EBCF0();
    /* Empties both lists. When the message list is not null, adds a new message to field_0x18 for
     * each record whose word at 0x4 is 1, sorts field_0x18 and updates field_0x14
     * (fn_802ECF8C). When a message's own allocation fails, the shared pointer added holds null;
     * a failure to allocate the shared pointer's control block goes to the deleter and
     * boost::throw_exception instead. */
    virtual void fn_802EB9E0();
    /* Repeats while field_0x14 is not MESSAGE_COUNT_NONE and returns when field_0x14 is greater
     * than count (an exact signed comparison: a message is due once count is at or above its
     * field_0x8). A last message whose field_0x8 is -1 also ends the loop, since field_0x14 then
     * equals MESSAGE_COUNT_NONE. Each pass reads the word at 0x20 of the manager that fn_8006D6A8
     * returns and takes the last message of field_0x18, then:
     * - when the message's record has -1 at 0x14 and the index at 0xC equals that word, calls
     *   the message's fn_802EEAF0 with the index;
     * - otherwise, when the record's word at 0x14 equals that word, calls it with the record's
     *   word;
     * and then removes the message and updates field_0x14 (fn_802ECF8C). A message that fails
     * both tests is removed without the call (USA 0x802ED55C branches past the call). A
     * Mission::CountMission passes its word at 0x18 after setting it to, or increasing it by, a
     * value that is not negative (the parameter name is inferred). */
    void fn_802ED4E0(int count);
    /* Sets field_0x14 to the field_0x8 of the last message of field_0x18, or to
     * MESSAGE_COUNT_NONE when that list is empty. */
    void fn_802ECF8C();

    /* 0x10 */ int field_0x10;
    /* 0x14 */ int field_0x14;
    /* 0x18 */ std::vector<boost::shared_ptr<CountMessage> > field_0x18;
    /* 0x24 */ std::vector<boost::shared_ptr<CountMessage> > field_0x24;

  private:
    /* Copying is disabled in this reconstructed interface because the copy contract is
     * unrecovered. Both lists own their storage, so a memberwise copy would have both
     * destructors free it; std::__vector_obj_imp restricts its own copy operations for the same
     * reason. */
    CountMessageManage(const CountMessageManage &);
    CountMessageManage &operator=(const CountMessageManage &);
};
CHECK_SIZEOF(CountMessageManage, 0x30);

} // namespace Mission

#endif

#ifndef GAME_BASICMESSAGE_H
#define GAME_BASICMESSAGE_H

#include "Game/Message.h"
#include "boost/shared_ptr.hpp"
#include <vector>
#include "layout_check.h"

namespace Mission {

/* Mission::BasicMessage (class name from RTTI): the message that BasicMessageManage creates for
 * each record whose word at 0x4 is 0. It adds nothing to Message. */
class BasicMessage : public Message {
  public:
    BasicMessage(const UnkMessageRecord_802EEBF8 *record) : Message(record) {}
    virtual ~BasicMessage();
};
CHECK_SIZEOF(BasicMessage, 0x8);

/* Mission::BasicMessageManage (class name from RTTI): the message manager that a Mission holds at
 * 0x18. It keeps the messages it has created in field_0x10 and a snapshot of that list in
 * field_0x1C. The snapshot is empty after construction and after fn_802EB9E0, which clear both
 * lists and fill only field_0x10. fn_802EBDF0 saves field_0x10 into it and fn_802EBCF0 restores
 * field_0x10 from it. The removals in fn_802EBE68 leave the snapshot unchanged. */
class BasicMessageManage : public MessageManage {
  public:
    /* Reserves room for four messages in each list, then builds the messages (fn_802EB9E0). */
    BasicMessageManage(int id, int index);
    ~BasicMessageManage();
    /* Saves the snapshot: replaces field_0x1C with the current messages of field_0x10. */
    virtual void fn_802EBDF0();
    /* Restores the snapshot: replaces field_0x10 with the messages of field_0x1C. */
    virtual void fn_802EBCF0();
    /* Empties both lists, then adds a new message to field_0x10 for each record of the list
     * whose word at 0x4 is 0 (none when the list is null). When a message's own allocation fails,
     * the shared pointer added holds null; a failure to allocate the shared pointer's control
     * block goes to the deleter and boost::throw_exception instead. */
    virtual void fn_802EB9E0();
    /* Removes the current messages that fn_802EC130 selects for arg. The missions pass 1 or 2
     * and do not read a result (the void return is inferred). The type of arg is inferred too:
     * the game has two copies of the accessor that reads a bound value (0x802EC4BC and
     * 0x802EC4C4 in the USA release), one for arg and one for the index at 0xC, so the two have
     * different types; which of them is not int is not known. */
    void fn_802EBE68(s32 arg);

    /* 0x10 */ std::vector<boost::shared_ptr<BasicMessage> > field_0x10;
    /* 0x1C */ std::vector<boost::shared_ptr<BasicMessage> > field_0x1C;
};
CHECK_SIZEOF(BasicMessageManage, 0x28);

} // namespace Mission

#endif

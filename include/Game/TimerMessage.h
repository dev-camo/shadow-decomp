#ifndef GAME_TIMERMESSAGE_H
#define GAME_TIMERMESSAGE_H

#include "Game/Message.h"
#include "boost/shared_ptr.hpp"
#include <vector>
#include "layout_check.h"

namespace Mission {

/* The time that stands for none (name inferred): TimerMessage::field_0x8 when the record's word at
 * 0x8 is not 0, 1 or 2, and TimerMessageManage::field_0x14 while field_0x18 is empty. */
const float MESSAGE_TIME_NONE = -1.0f;

/* Mission::TimerMessage (class name from RTTI): the message that TimerMessageManage creates for
 * each record whose word at 0x4 is 2. */
class TimerMessage : public Message {
  public:
    /* The record's word at 0x8 selects field_0x8: time (the manager's float at 0x10) minus the
     * record's float at 0x10 for 0, time times that float for 1, the float itself for 2, and
     * MESSAGE_TIME_NONE (-1) for any other value. The parameter name is inferred. The float is
     * read through field_0x4 (the member the base class stored), as the original does; reading it
     * through the parameter changes the code. */
    TimerMessage(const UnkMessageRecord_802EEBF8 *record, float time) : Message(record) {
        switch (record->field_0x8) {
        case 0:
            field_0x8 = time - field_0x4->field_0x10;
            break;
        case 1:
            field_0x8 = time * field_0x4->field_0x10;
            break;
        case 2:
            field_0x8 = field_0x4->field_0x10;
            break;
        default:
            field_0x8 = MESSAGE_TIME_NONE;
            break;
        }
    }
    virtual ~TimerMessage();

    /* 0x08 */ float field_0x8;
};
CHECK_SIZEOF(TimerMessage, 0xC);

/* Mission::TimerMessageManage (class name from RTTI): the message manager that a
 * Mission::TimerMission holds at 0xC. It keeps the messages it has created in field_0x18, sorted
 * by their field_0x8 in ascending order, and a snapshot of that list in field_0x24. The snapshot
 * is empty after construction and after fn_802EB9E0, which clear both lists and fill only
 * field_0x18. fn_802EBDF0 saves field_0x18 into it and fn_802EBCF0 restores field_0x18 from it;
 * a Mission::TimerMission calls them from its fn_8016AF40 and fn_8016AF44 (USA 0x802C8D30 and
 * 0x802C8D04). The removals in fn_802EF224 leave the snapshot unchanged. */
class TimerMessageManage : public MessageManage {
  public:
    /* Stores time at 0x10 and MESSAGE_TIME_NONE at 0x14, reserves room in each list for as many
     * messages as the message list has records, then builds the messages (fn_802EB9E0, called
     * through the vtable). The parameter types of id and index are inferred, as for
     * MessageManage. */
    TimerMessageManage(int id, int index, float time);
    ~TimerMessageManage();
    /* Saves the snapshot: replaces field_0x24 with the current messages of field_0x18. */
    virtual void fn_802EBDF0();
    /* Restores the snapshot: replaces field_0x18 with the messages of field_0x24, then updates
     * field_0x14 (fn_802EEC4C). */
    virtual void fn_802EBCF0();
    /* Empties both lists. When the message list is not null, adds a new message to field_0x18 for
     * each record whose word at 0x4 is 2, sorts field_0x18 and updates field_0x14
     * (fn_802EEC4C). When a message's own allocation fails, the shared pointer added holds null;
     * a failure to allocate the shared pointer's control block goes to the deleter and
     * boost::throw_exception instead. */
    virtual void fn_802EB9E0();
    /* Repeats while field_0x14 >= 0, or field_0x14 is 0 as fn_8000A950 judges it, and returns
     * unless time <= field_0x14 or time equals field_0x14 as fn_801C455C judges it. Each pass reads
     * the word at 0x20 of the manager that fn_8006D6A8 returns and takes the last message of
     * field_0x18, then:
     * - when the message's record has -1 at 0x14 and the index at 0xC equals that word, calls
     *   the message's fn_802EEAF0 with the index;
     * - otherwise, when the record's word at 0x14 equals that word, calls it with the record's
     *   word;
     * and then removes the message and updates field_0x14 (fn_802EEC4C). A message that fails
     * both tests is removed without the call (USA 0x802EF2C8 branches past the call).
     * fn_801C455C tests |time - field_0x14| < 0.0001 (behaviour inferred), so a message is also
     * due when time is above the message's time by less than 0.0001. A Mission::TimerMission
     * passes the float at 0xA0 of lbl_8057D6A0 (the parameter name is inferred). */
    void fn_802EF224(float time);
    /* Sets field_0x14 to the field_0x8 of the last message of field_0x18, or to MESSAGE_TIME_NONE
     * when that list is empty. */
    void fn_802EEC4C();

    /* 0x10 */ float field_0x10;
    /* 0x14 */ float field_0x14;
    /* 0x18 */ std::vector<boost::shared_ptr<TimerMessage> > field_0x18;
    /* 0x24 */ std::vector<boost::shared_ptr<TimerMessage> > field_0x24;

  private:
    /* Copying is disabled in this reconstructed interface because the copy contract is
     * unrecovered: std::vector (through std::__vector_obj) declares no copy constructor or
     * assignment here, so a memberwise copy would share both lists' storage, which both
     * destructors would then free. */
    TimerMessageManage(const TimerMessageManage &);
    TimerMessageManage &operator=(const TimerMessageManage &);
};
CHECK_SIZEOF(TimerMessageManage, 0x30);

} // namespace Mission

#endif

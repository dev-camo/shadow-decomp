/* Mission::TimerMessageManage and Mission::TimerMessage (class names from RTTI): the message
 * manager of a timer mission and its messages. File name inferred from the classes. Definitions
 * are in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/TimerMessage.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkManager_8006D6A8.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8040_vector_types.h"
#include <algorithm>

namespace Mission {

typedef boost::shared_ptr<TimerMessage> TimerMessagePtr;

TimerMessage::~TimerMessage() {}

/* The order in which fn_802EB9E0 sorts the messages: whether a's field_0x8 is less than b's. */
static bool fn_802EF60C(TimerMessagePtr a, TimerMessagePtr b) {
    return a->field_0x8 < b->field_0x8;
}

TimerMessageManage::TimerMessageManage(int id, int index, float time)
    : MessageManage(id, index), field_0x10(time), field_0x14(MESSAGE_TIME_NONE) {
    field_0x18.reserve(field_0x8);
    field_0x24.reserve(field_0x8);
    fn_802EB9E0();
}

TimerMessageManage::~TimerMessageManage() {}

void TimerMessageManage::fn_802EF224(float time) {
    while (field_0x14 >= 0.0f || fn_8000A950(field_0x14)) {
        if (!(time <= field_0x14 || fn_801C455C(time, field_0x14))) {
            return;
        }
        int manager_word = fn_8006D6A8()->field_0x20;
        // The copy of the message is destroyed before the message is removed from the list.
        {
            TimerMessagePtr message = field_0x18.back();
            const UnkMessageRecord_802EEBF8 *record = message->field_0x4;
            if (record->field_0x14 == -1 && field_0xC == manager_word) {
                message->fn_802EEAF0(field_0xC);
            } else if (record->field_0x14 == manager_word) {
                message->fn_802EEAF0(record->field_0x14);
            }
        }
        field_0x18.pop_back();
        fn_802EEC4C();
    }
}

/* This and fn_802EBCF0 copy each element into a local shared_ptr before push_back: the original
 * calls the copy constructor into a stack temporary and destroys it after push_back.
 * push_back(*it) emits no copy and makes each function 24 bytes shorter. */
void TimerMessageManage::fn_802EBDF0() {
    field_0x24.clear();
    for (std::vector<TimerMessagePtr>::iterator it = field_0x18.begin(); it != field_0x18.end();
         ++it) {
        TimerMessagePtr message = *it;
        field_0x24.push_back(message);
    }
}

void TimerMessageManage::fn_802EBCF0() {
    field_0x18.clear();
    for (std::vector<TimerMessagePtr>::iterator it = field_0x24.begin(); it != field_0x24.end();
         ++it) {
        TimerMessagePtr message = *it;
        field_0x18.push_back(message);
    }
    fn_802EEC4C();
}

void TimerMessageManage::fn_802EB9E0() {
    field_0x18.clear();
    field_0x24.clear();
    if (field_0x4 != 0) {
        for (u32 i = 0; i < field_0x8; i++) {
            const UnkMessageRecord_802EEBF8 *record = &field_0x4[i];
            if (record->field_0x4 == 2) {
                field_0x18.push_back(TimerMessagePtr(new TimerMessage(record, field_0x10)));
            }
        }
        std::sort(field_0x18.begin(), field_0x18.end(), fn_802EF60C);
        fn_802EEC4C();
    }
}

void TimerMessageManage::fn_802EEC4C() {
    if (field_0x18.empty()) {
        field_0x14 = MESSAGE_TIME_NONE;
    } else {
        field_0x14 = field_0x18.back()->field_0x8;
    }
}

} // namespace Mission

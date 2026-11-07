/* Mission::CountMessageManage and Mission::CountMessage (class names from RTTI): the message
 * manager of a count mission and its messages. File name inferred from the classes. Definitions
 * are in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/CountMessage.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkManager_8006D6A8.h"
#include <algorithm>
#include <cmath>

namespace Mission {

typedef boost::shared_ptr<CountMessage> CountMessagePtr;

CountMessage::CountMessage(const UnkMessageRecord_802EEBF8 *record, int count) : Message(record) {
    switch (record->field_0x8) {
    case 0:
        field_0x8 = field_0x4->field_0xC;
        break;
    case 1:
        field_0x8 = std::ceil(count * field_0x4->field_0x10);
        break;
    case 2:
        field_0x8 = count - field_0x4->field_0xC;
        break;
    default:
        field_0x8 = 0;
        break;
    }
}

CountMessage::~CountMessage() {}

/* The order in which fn_802EB9E0 sorts the messages: whether a's field_0x8 is greater than b's. */
static bool fn_802ED870(CountMessagePtr a, CountMessagePtr b) {
    return a->field_0x8 > b->field_0x8;
}

CountMessageManage::CountMessageManage(int id, int index, int count)
    : MessageManage(id, index), field_0x10(count), field_0x14(MESSAGE_COUNT_NONE) {
    field_0x18.reserve(field_0x8);
    field_0x24.reserve(field_0x8);
    fn_802EB9E0();
}

CountMessageManage::~CountMessageManage() {}

void CountMessageManage::fn_802ED4E0(int count) {
    while (field_0x14 != MESSAGE_COUNT_NONE) {
        if (field_0x14 > count) {
            return;
        }
        int manager_word = fn_8006D6A8()->field_0x20;
        // The copy of the message is destroyed before the message is removed from the list.
        {
            CountMessagePtr message = field_0x18.back();
            const UnkMessageRecord_802EEBF8 *record = message->field_0x4;
            if (record->field_0x14 == -1 && field_0xC == manager_word) {
                message->fn_802EEAF0(field_0xC);
            } else if (record->field_0x14 == manager_word) {
                message->fn_802EEAF0(record->field_0x14);
            }
        }
        field_0x18.pop_back();
        fn_802ECF8C();
    }
}

/* This and fn_802EBCF0 copy each element into a local shared_ptr before push_back: the original
 * calls the copy constructor into a stack temporary and destroys it after push_back.
 * push_back(*it) emits no copy and makes each function 24 bytes shorter. */
void CountMessageManage::fn_802EBDF0() {
    field_0x24.clear();
    for (std::vector<CountMessagePtr>::iterator it = field_0x18.begin(); it != field_0x18.end();
         ++it) {
        CountMessagePtr message = *it;
        field_0x24.push_back(message);
    }
}

void CountMessageManage::fn_802EBCF0() {
    field_0x18.clear();
    for (std::vector<CountMessagePtr>::iterator it = field_0x24.begin(); it != field_0x24.end();
         ++it) {
        CountMessagePtr message = *it;
        field_0x18.push_back(message);
    }
    fn_802ECF8C();
}

void CountMessageManage::fn_802EB9E0() {
    field_0x18.clear();
    field_0x24.clear();
    if (field_0x4 != 0) {
        for (u32 i = 0; i < field_0x8; i++) {
            const UnkMessageRecord_802EEBF8 *record = &field_0x4[i];
            if (record->field_0x4 == 1) {
                field_0x18.push_back(CountMessagePtr(new CountMessage(record, field_0x10)));
            }
        }
        std::sort(field_0x18.begin(), field_0x18.end(), fn_802ED870);
        fn_802ECF8C();
    }
}

void CountMessageManage::fn_802ECF8C() {
    if (field_0x18.empty()) {
        field_0x14 = MESSAGE_COUNT_NONE;
    } else {
        field_0x14 = field_0x18.back()->field_0x8;
    }
}

} // namespace Mission

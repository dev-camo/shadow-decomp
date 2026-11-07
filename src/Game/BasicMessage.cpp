/* Mission::BasicMessageManage and Mission::BasicMessage (class names from RTTI): the message
 * manager of a plain mission and its messages. File name inferred from the classes. Definitions
 * are in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/BasicMessage.h"
#include "Game/UnkActor_800740E4.h"
#include "Game/UnkManager_8006D6A8.h"
#include "boost/bind.hpp"
#include <algorithm>

namespace Mission {

typedef boost::shared_ptr<BasicMessage> BasicMessagePtr;

BasicMessage::~BasicMessage() {}

/* Whether fn_802EBE68 removes message for arg:
 * - arg 1: when the record's word at 0x8 is 0, after calling fn_802EEAF0(index) on the message;
 * - arg 2: when the record's word at 0x8 is 1; fn_802EEAF0(index) is called on the message first
 *   only when the record's word at 0x14 is -1 or equals the word at 0x20 of the manager that
 *   fn_8006D6A8 returns;
 * - otherwise the message is kept. */
static bool fn_802EC130(BasicMessagePtr message, int index, s32 arg) {
    const UnkMessageRecord_802EEBF8 *record = message->field_0x4;

    if (arg == 1 && record->field_0x8 == 0) {
        message->fn_802EEAF0(index);
        return true;
    }
    if (arg == 2 && record->field_0x8 == 1) {
        if (record->field_0x14 == -1) {
            message->fn_802EEAF0(index);
            return true;
        }
        if (record->field_0x14 == fn_8006D6A8()->field_0x20) {
            message->fn_802EEAF0(index);
        }
        return true;
    }
    return false;
}

BasicMessageManage::BasicMessageManage(int id, int index) : MessageManage(id, index) {
    field_0x10.reserve(4);
    field_0x1C.reserve(4);
    fn_802EB9E0();
}

BasicMessageManage::~BasicMessageManage() {}

void BasicMessageManage::fn_802EBE68(s32 arg) {
    field_0x10.erase(std::remove_if(field_0x10.begin(), field_0x10.end(),
                                    boost::bind(fn_802EC130, _1, field_0xC, arg)),
                     field_0x10.end());
}

/* This and fn_802EBCF0 copy each element into a local shared_ptr before push_back: the original
 * calls the copy constructor into a stack temporary and destroys it after push_back.
 * push_back(*it) emits no copy and makes each function 24 bytes shorter. */
void BasicMessageManage::fn_802EBDF0() {
    field_0x1C.clear();
    for (std::vector<BasicMessagePtr>::iterator it = field_0x10.begin(); it != field_0x10.end();
         ++it) {
        BasicMessagePtr message = *it;
        field_0x1C.push_back(message);
    }
}

void BasicMessageManage::fn_802EBCF0() {
    field_0x10.clear();
    for (std::vector<BasicMessagePtr>::iterator it = field_0x1C.begin(); it != field_0x1C.end();
         ++it) {
        BasicMessagePtr message = *it;
        field_0x10.push_back(message);
    }
}

void BasicMessageManage::fn_802EB9E0() {
    field_0x10.clear();
    field_0x1C.clear();
    if (field_0x4 != 0) {
        for (u32 i = 0; i < field_0x8; i++) {
            const UnkMessageRecord_802EEBF8 *record = &field_0x4[i];
            if (record->field_0x4 == 0) {
                field_0x10.push_back(BasicMessagePtr(new BasicMessage(record)));
            }
        }
    }
}

} // namespace Mission

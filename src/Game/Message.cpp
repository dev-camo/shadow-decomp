/* Mission::Message and Mission::MessageManage (class names from RTTI): the bases of the mission
 * messages and their managers. File name inferred from the class. Definitions are in reverse
 * address order (deferred inlining emits functions last-defined first). */
#include "Game/Message.h"
#include "Game/UnkActor_800740E4.h"

namespace Mission {

Message::Message(const UnkMessageRecord_802EEBF8 *record) : field_0x4(record) {}

Message::~Message() {}

void Message::fn_802EEAF0(int arg) {
    fn_801FAF00(fn_80109D64(), field_0x4->field_0x0, fn_801FAAC4(arg), 0, 0);
}

MessageManage::MessageManage(int id, int index) : field_0xC(index) {
    field_0x4 = fn_802EEBF8(id);
    field_0x8 = fn_802EEB9C(id);
}

} // namespace Mission

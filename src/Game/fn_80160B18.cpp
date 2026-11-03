/* Requests a model motion selected by the resource entry's identifier (behaviour inferred). */
#include "Game/GadgetModelInfo.h"
#include "Game/fn_804157E8_motion_control.h"

bool GadgetModelInfo::fn_80160B18(int arg0) {
    for (UnkModelMotionIterator it = field_0x58.begin(); it != field_0x58.end(); ++it) {
        UnkModelMotionData &entry = *it;
        if (entry.field_0x18->field_0x18 == arg0) {
            fn_80417568(entry.field_0x4.get(), static_cast<u16>(entry.field_0x14));
            return true;
        }
    }
    return false;
}

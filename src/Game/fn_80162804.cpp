/* Releases the model's parts and their registry references (behaviour inferred). */
#include "Game/GadgetModelInfo.h"
#include "Game/UnkRegistry_8010A458.h"
#include "rw/rpworld.h"

void GadgetModelInfo::fn_80162804() {
    fn_80163534();
    fn_801635D8();
    for (UnkModelPartIterator it = field_0x4C.begin(); it != field_0x4C.end(); ++it) {
        UnkModelPartData &part = *it;
        if (part.field_0x8 != 0) {
            fn_8010A458()->fn_801EB550(UnkString_8004F888(part.field_0x8));
        }
        if (part.field_0x0 != 0) {
            fn_80456C54(part.field_0x0, fn_80164064, 0);
            RpWorld *world = fn_8046256C(part.field_0x0);
            if (world != 0) {
                fn_804623EC(world, part.field_0x0);
            }
            fn_80457468(part.field_0x0);
        }
        part.fn_80162944();
    }
    field_0x4C.clear();
    field_0x3C.Reset(2);
}

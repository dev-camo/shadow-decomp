#include "Game/UnkSwitchCounter_801660A0.h"

void UnkSwitchCounter_801660A0::fn_80165F3C() {
    if (field_0x10 == 0) {
        fn_80166424(this);
        field_0x10 = 1;
    }
}

void UnkSwitchCounter_801660A0::fn_80165EFC() {
    if (field_0x10 != 0) {
        fn_80166424(this);
        field_0x10 = 0;
    }
}

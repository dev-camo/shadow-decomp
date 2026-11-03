/* Select the optional transform or offset before updating the clump. */
#include "Game/fn_80015A7C.h"

void Unk80015A7C::fn_800159AC() {
    field_0x14 = 0;
    field_0x18 = 0;
    fn_800158C4();
}

void Unk80015A7C::fn_80015980(const Fn8040Vector3 *offset) {
    field_0x14 = offset;
    field_0x18 = 0;
    fn_800158C4();
}

void Unk80015A7C::fn_80015954(const Fn8040Matrix34 *transform) {
    field_0x14 = 0;
    field_0x18 = transform;
    fn_800158C4();
}

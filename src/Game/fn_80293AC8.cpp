/* Access and lifecycle of the gravity direction index. */
#include <Game/UnkGravity_80293928.h>
#include "layout_check.h"

class UnkGravityState_805F0340 {
  public:
    UnkGravityState_805F0340() : index(0) {}
    /* Nontrivial destruction is inferred from the native class-delete pattern. */
    ~UnkGravityState_805F0340() {}

    /* 0x00 */ int index;
};
CHECK_SIZEOF(UnkGravityState_805F0340, 0x4);

void fn_80293B1C(void) {
    if (lbl_UnkGravityState_805F0340 == 0) {
        lbl_UnkGravityState_805F0340 = new UnkGravityState_805F0340;
    }
}

void fn_80293AE4(void) {
    if (lbl_UnkGravityState_805F0340 != 0) {
        delete lbl_UnkGravityState_805F0340;
        lbl_UnkGravityState_805F0340 = 0;
    }
}

int fn_80293AC8(void) {
    if (lbl_UnkGravityState_805F0340 != 0) {
        return lbl_UnkGravityState_805F0340->index;
    }
    return 0;
}

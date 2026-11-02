/* Pointer-slot transfer and indexed access to the manager's records. */
#include "Game/fn_8040D608.h"

/* Takes the pointer out of *slot, leaving NULL behind (behaviour inferred). */
void *fn_8040D498(void **slot) {
    void *value = *slot;
    *slot = 0;
    return value;
}

Fn8040D608Object *fn_8040D4AC(Fn8040D608Object *base, unsigned int index) {
    return base + index;
}

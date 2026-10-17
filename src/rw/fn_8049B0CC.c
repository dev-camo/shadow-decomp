/* RenderWare pipeline library: the pipeline module, which the engine opens and closes with its
 * other core modules. The original file name is unknown. */
#include "rw/rwcore.h"

RwInt32 lbl_805F1690;

/* Opens the pipeline module with its globals at offset in the engine instance */
void *fn_8049B0CC(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F1690 = offset;
    if (fn_8049B258()) {
        return instance;
    }
    return NULL;
}

/* Closes the pipeline module */
void *fn_8049B110(void *instance, RwInt32 offset, RwInt32 size) {
    fn_8049B1F8();
    return instance;
}

RwBool fn_8049B140(void) {
    return TRUE;
}

void fn_8049B148(RwCamera *camera) {}

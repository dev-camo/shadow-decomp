/* RenderWare delta morph plugin (plugin ID 0x122), engine part: the engine plugin, which sets up the
 * plugin's pipelines while an engine instance is open, and the attachment of the whole plugin. The
 * original file name is unknown. */
#include "rw/rpdmorph.h"
#include "layout_check.h"

/* The plugin's engine globals (inferred) */
typedef struct DMorphEngineGlobals {
    /* 0x0 */ RwInt32 field_0x0;
    /* 0x4 */ RwInt32 numInstances; /* the open engine instances */
} DMorphEngineGlobals;
CHECK_SIZEOF(DMorphEngineGlobals, 0x8);

DMorphEngineGlobals lbl_805EDD68 = {0, 0};

/* engine plugin constructor: creates the pipelines for the first instance */
void *fn_804447B0(void *instance, RwInt32 offset, RwInt32 size) {
    if (lbl_805EDD68.numInstances == 0) {
        fn_80445C38(1);
    }
    lbl_805EDD68.numInstances++;
    return instance;
}

/* engine plugin destructor: destroys the pipelines with the last instance */
void *fn_80444808(void *instance, RwInt32 offset, RwInt32 size) {
    if (--lbl_805EDD68.numInstances == 0) {
        fn_80445C9C();
    }
    return instance;
}

/* Attaches the delta morph plugin to the engine, geometries and atomics */
RwBool fn_8044484C(void) {
    if (fn_80485D78(0, rpID_DMORPHPLUGIN, fn_804447B0, fn_80444808) < 0) {
        return FALSE;
    }
    if (fn_804437B0() == FALSE) {
        return FALSE;
    }
    return fn_8044420C() != 0;
}

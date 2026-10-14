/* RenderWare world pipelines: the default sector and atomic pipelines, and opening and closing the
 * pipeline globals. The original file name is unknown. */
#include "rw/rpworld.h"

#define PIPELINEGLOBAL(var) (RWPLUGINOFFSET(RpPipelineGlobals, lbl_805F15B4, lbl_805F1690)->var)

/* Sets the default world sector pipeline, or the platform's for NULL */
RxPipeline *fn_804626F8(RxPipeline *pipeline) {
    if (pipeline == NULL) {
        /* the platform's pipeline, else none (the original tests it with no other fallback) */
        RxPipeline *platform = PIPELINEGLOBAL(platformWorldSectorPipeline);

        if (platform != NULL) {
            pipeline = platform;
        } else {
            pipeline = NULL;
        }
    }
    PIPELINEGLOBAL(defaultWorldSectorPipeline) = pipeline;
    return pipeline;
}

/* Sets the default atomic pipeline, or the platform's for NULL */
RxPipeline *fn_80462734(RxPipeline *pipeline) {
    if (pipeline == NULL) {
        /* the platform's pipeline, else none (the original tests it with no other fallback) */
        RxPipeline *platform = PIPELINEGLOBAL(platformAtomicPipeline);

        if (platform != NULL) {
            pipeline = platform;
        } else {
            pipeline = NULL;
        }
    }
    PIPELINEGLOBAL(defaultAtomicPipeline) = pipeline;
    return pipeline;
}

/* Closes the pipelines */
void fn_80462770(void) {
    fn_80463244();
    fn_80463308();
    fn_804631D4();
}

/* Opens the pipelines: TRUE on success */
RwBool fn_80462798(void) {
    RwBool status;

    PIPELINEGLOBAL(defaultAtomicPipeline) = NULL;
    PIPELINEGLOBAL(defaultWorldSectorPipeline) = NULL;
    PIPELINEGLOBAL(defaultMaterialPipeline) = NULL;
    PIPELINEGLOBAL(genericAtomicPipeline) = NULL;
    PIPELINEGLOBAL(genericWorldSectorPipeline) = NULL;
    PIPELINEGLOBAL(genericMaterialPipeline) = NULL;
    PIPELINEGLOBAL(platformAtomicPipeline) = NULL;
    PIPELINEGLOBAL(platformWorldSectorPipeline) = NULL;
    PIPELINEGLOBAL(platformMaterialPipeline) = NULL;

    status = fn_804631CC();
    if (status) {
        status = fn_80463298();
    }
    if (status) {
        status = fn_804631D8();
    }
    if (status) {
        return TRUE;
    }
    fn_80462770();
    return FALSE;
}

/* The pipeline plugin attachment: nothing to attach */
RwBool fn_80462884(void) {
    return TRUE;
}

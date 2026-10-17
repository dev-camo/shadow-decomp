/* RenderWare immediate mode 3D, GameCube: creating and destroying the platform's Im3D transform
 * and render pipelines. The original file name is unknown. */
#include "rw/rwcore.h"

RwBool fn_8049F8AC(RxPipeline **transformPipeline) {
    RxPipeline *pipeline = fn_8049B76C();

    if (pipeline != NULL) {
        RxPipeline *lockedPipeline;

        pipeline->pluginId = rwID_COREPLUGIN;
        lockedPipeline = fn_8049CCB8(pipeline);
        if (lockedPipeline != NULL) {
            pipeline = fn_8049C1E0(fn_8049D0D0(lockedPipeline, NULL, fn_8049FB0C(), NULL));
            if (pipeline != NULL) {
                *transformPipeline = pipeline;
                fn_8049AC90(pipeline);
                return TRUE;
            }
        }
        fn_8049B800(pipeline);
    }
    return FALSE;
}

void fn_8049F948(RxPipeline **transformPipeline) {
    fn_8049AC90(NULL);
    if (*transformPipeline != NULL) {
        fn_8049B800(*transformPipeline);
        *transformPipeline = NULL;
    }
}

void fn_8049F990(rwIm3DRenderPipelines *renderPipelines) {
    fn_8049ACE0(NULL, rwPRIMTYPETRILIST);
    fn_8049ACE0(NULL, rwPRIMTYPETRIFAN);
    fn_8049ACE0(NULL, rwPRIMTYPETRISTRIP);
    fn_8049ACE0(NULL, rwPRIMTYPELINELIST);
    fn_8049ACE0(NULL, rwPRIMTYPEPOLYLINE);

    /* one pipeline serves the five render primitive types */
    if (renderPipelines->triList != NULL) {
        fn_8049B800(renderPipelines->triList);
    }
    renderPipelines->triList = NULL;
    renderPipelines->triFan = NULL;
    renderPipelines->triStrip = NULL;
    renderPipelines->lineList = NULL;
    renderPipelines->polyLine = NULL;
}

RwBool fn_8049FA1C(rwIm3DRenderPipelines *renderPipelines) {
    RxPipeline *pipeline = fn_8049B76C();

    if (pipeline != NULL) {
        RxPipeline *lockedPipeline;

        pipeline->pluginId = rwID_COREPLUGIN;
        lockedPipeline = fn_8049CCB8(pipeline);
        if (lockedPipeline != NULL) {
            pipeline = fn_8049C1E0(fn_8049D0D0(lockedPipeline, NULL, fn_804A0760(), NULL));
            if (pipeline != NULL) {
                renderPipelines->triList = pipeline;
                renderPipelines->triFan = pipeline;
                renderPipelines->triStrip = pipeline;
                renderPipelines->lineList = pipeline;
                renderPipelines->polyLine = pipeline;
                fn_8049ACE0(pipeline, rwPRIMTYPETRILIST);
                fn_8049ACE0(pipeline, rwPRIMTYPETRIFAN);
                fn_8049ACE0(pipeline, rwPRIMTYPETRISTRIP);
                fn_8049ACE0(pipeline, rwPRIMTYPELINELIST);
                fn_8049ACE0(pipeline, rwPRIMTYPEPOLYLINE);
                return TRUE;
            }
        }
        fn_8049B800(pipeline);
    }
    return FALSE;
}

/* RenderWare pipeline library: the default render state vector. The original file name is
 * unknown. */
#include "rw/rwcore.h"

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* Sets rsvp to the default render states. Once the engine is started they are copied from the
 * pipeline module's defaults; before that only those defaults themselves can be set, to the
 * built-in values. */
RxRenderStateVector *fn_8049F2A4(RxRenderStateVector *rsvp) {
    if (rsvp != NULL) {
        if (lbl_805F15B4->engineStatus == rwENGINESTATUSSTARTED) {
            *rsvp = RXPIPELINEGLOBAL(defaultRenderState);
        } else if (rsvp != &RXPIPELINEGLOBAL(defaultRenderState)) {
            RWERROR((E_RW_PLUGINNOTINIT));
            return NULL;
        } else {
            RwRGBA opaqueWhite = {255, 255, 255, 255};

            rsvp->Flags = rxRENDERSTATEFLAG_TEXTUREPERSPECTIVE | rxRENDERSTATEFLAG_ZTESTENABLE |
                          rxRENDERSTATEFLAG_ZWRITEENABLE;
            rsvp->ShadeMode = rwSHADEMODEGOURAUD;
            rsvp->SrcBlend = rwBLENDSRCALPHA;
            rsvp->DestBlend = rwBLENDINVSRCALPHA;
            rsvp->TextureRaster = NULL;
            rsvp->AddressModeU = rwTEXTUREADDRESSWRAP;
            rsvp->AddressModeV = rwTEXTUREADDRESSWRAP;
            rsvp->FilterMode = rwFILTERLINEAR;
            rsvp->BorderColor = opaqueWhite;
            rsvp->FogType = rwFOGTYPENAFOGTYPE;
            rsvp->FogColor = opaqueWhite;
        }
        return rsvp;
    }

    RWERROR((E_RW_NULLP));
    return NULL;
}

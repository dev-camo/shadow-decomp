/* RenderWare material effects plugin (plugin ID 0x120), GameCube multi-texture effects: their
 * stream format, creating them, the engine plugin (plugin ID 0x129) with the frames effects can
 * project from, setting up the GX state of an effect and rendering a mesh with it, and attaching
 * the GameCube multi-texturing. The original file name is unknown. */
#include "rw/rpmatfx.h"
#include <dolphin/gx.h>
#include <dolphin/mtx.h>
#include <string.h>
#include "layout_check.h"

#define rwMEMHINT_GCNMTEFFECT (rwMEMHINTDUR_EVENT | rpID_GCNMATPLUGIN)

/* Texture matrix sources: the object's LTM, the camera's view, or (from 16) one of the engine's
 * projection frames (inferred) */
#define rpMTGCNSOURCELTM 2
#define rpMTGCNSOURCECAMERA 3
#define rpMTGCNSOURCEFRAME0 16

/* A TEV stage's texture map, flagged in its stream record's flags (inferred) */
#define rpMTGCNTEXMAPDISABLE 0x100
#define rpMTGCNSTREAMTEXMAPDISABLE 0x80

/* The GameCube effect's header in a stream (names of the stream records inferred) */
typedef struct MTEffectGameCubeStreamHeader {
    /* 0x00 */ RwUInt8 numTexGens;
    /* 0x01 */ RwUInt8 numTevStages;
    /* 0x02 */ RwUInt8 flags;
    /* 0x03 */ RwUInt8 blendModes; /* the source blend mode in the high nibble */
    /* 0x04 */ GXColorS10 tevColors[4];
    /* 0x24 */ GXColor kColors[4];
    /* 0x34 */ RwUInt8 numTexMtx;
    /* 0x35 */ RwUInt8 numIndMtx;
    /* 0x36 */ RwUInt8 numIndStages;
    /* 0x37 */ RwUInt8 pad;
} MTEffectGameCubeStreamHeader;
CHECK_SIZEOF(MTEffectGameCubeStreamHeader, 0x38);

/* A texture coordinate generator in a stream */
typedef struct MTEffectGameCubeStreamTexGen {
    /* 0x0 */ RwUInt8 flags;
    /* 0x1 */ RwUInt8 func;
    /* 0x2 */ RwUInt8 src;
    /* 0x3 */ RwUInt8 mtx;
    /* 0x4 */ RwUInt8 postMtx;
    /* 0x5 */ RwUInt8 pad[3];
    /* 0x8 */ RwUInt16 scale[2];
} MTEffectGameCubeStreamTexGen;
CHECK_SIZEOF(MTEffectGameCubeStreamTexGen, 0xC);

/* A texture matrix's header in a stream; its matrix follows */
typedef struct MTEffectGameCubeStreamTexMtx {
    /* 0x0 */ RwUInt8 id;
    /* 0x1 */ RwUInt8 source;
    /* 0x2 */ RwUInt8 flags;
    /* 0x3 */ RwUInt8 pad;
} MTEffectGameCubeStreamTexMtx;
CHECK_SIZEOF(MTEffectGameCubeStreamTexMtx, 0x4);

/* An indirect texture matrix's header in a stream; its offsets follow */
typedef struct MTEffectGameCubeStreamIndMtx {
    /* 0x0 */ RwUInt8 id;
    /* 0x1 */ RwUInt8 source;
    /* 0x2 */ RwUInt8 flags;
    /* 0x3 */ RwUInt8 scaleExp;
} MTEffectGameCubeStreamIndMtx;
CHECK_SIZEOF(MTEffectGameCubeStreamIndMtx, 0x4);

/* An indirect texture stage in a stream */
typedef struct MTEffectGameCubeStreamIndStage {
    /* 0x0 */ RwUInt8 flags;
    /* 0x1 */ RwUInt8 texCoord;
    /* 0x2 */ RwUInt8 texMap;
    /* 0x3 */ RwUInt8 scaleS;
    /* 0x4 */ RwUInt8 scaleT;
    /* 0x5 */ RwUInt8 pad[3];
} MTEffectGameCubeStreamIndStage;
CHECK_SIZEOF(MTEffectGameCubeStreamIndStage, 0x8);

/* A TEV stage in a stream */
typedef struct MTEffectGameCubeStreamTevStage {
    /* 0x00 */ RwUInt8 colorIn[4];
    /* 0x04 */ RwUInt8 alphaIn[4];
    /* 0x08 */ RwUInt8 colorOp;
    /* 0x09 */ RwUInt8 colorBias;
    /* 0x0A */ RwUInt8 colorScale;
    /* 0x0B */ RwUInt8 colorClamp;
    /* 0x0C */ RwUInt8 colorOut;
    /* 0x0D */ RwUInt8 alphaOp;
    /* 0x0E */ RwUInt8 alphaBias;
    /* 0x0F */ RwUInt8 alphaScale;
    /* 0x10 */ RwUInt8 alphaClamp;
    /* 0x11 */ RwUInt8 alphaOut;
    /* 0x12 */ RwUInt8 kColorSel;
    /* 0x13 */ RwUInt8 kAlphaSel;
    /* 0x14 */ RwUInt8 texCoord;
    /* 0x15 */ RwUInt8 texMap;
    /* 0x16 */ RwUInt8 color;
    /* 0x17 */ RwUInt8 flags;
    /* 0x18 */ RwUInt32 indirect;
} MTEffectGameCubeStreamTevStage;
CHECK_SIZEOF(MTEffectGameCubeStreamTevStage, 0x1C);

/* The fields of a TEV stage's packed indirect setup (GXSetTevIndirect) (name inferred) */
typedef struct MTEffectGameCubeIndirect {
    /* 0x00 */ RwUInt32 indStage;
    /* 0x04 */ RwUInt32 format;
    /* 0x08 */ RwUInt32 biasSel;
    /* 0x0C */ RwUInt32 matrixSel;
    /* 0x10 */ RwUInt32 wrapS;
    /* 0x14 */ RwUInt32 wrapT;
    /* 0x18 */ RwUInt8 addPrev;
    /* 0x19 */ RwUInt8 utcLod;
    /* 0x1C */ RwUInt32 alphaSel;
} MTEffectGameCubeIndirect;
CHECK_SIZEOF(MTEffectGameCubeIndirect, 0x20);

/* The GameCube engine plugin's globals: frames that texture matrices can project from (name
 * inferred) */
typedef struct MTGameCubeGlobals {
    /* 0x0 */ RwUInt32 numFrames;
    /* 0x4 */ RwFrame **frames;
} MTGameCubeGlobals;
CHECK_SIZEOF(MTGameCubeGlobals, 0x8);

/* A GameCube material's platform data: callbacks around rendering it with its effect (name
 * inferred) */
typedef void (*MTGameCubeRenderCallBack)(RpMaterial *material, RpAtomic *atomic,
                                         RpGameCubeRenderMesh *renderMesh, void *data);
typedef struct MTGameCubeMaterialData {
    /* 0x0 */ MTGameCubeRenderCallBack preRender;
    /* 0x4 */ MTGameCubeRenderCallBack postRender;
    /* 0x8 */ void *data;
} MTGameCubeMaterialData;
CHECK_SIZEOF(MTGameCubeMaterialData, 0xC);

RwInt32 lbl_805F16C8; /* engine plugin offset */

/* the engine plugin's globals (name inferred) */
#define MTGAMECUBEGLOBALS() RWPLUGINOFFSET(MTGameCubeGlobals, lbl_805F15B4, lbl_805F16C8)

/* The bytes of a GameCube effect's data in a stream (name inferred) */
static inline RwUInt32 MTEffectGameCubeStreamSize(const MTEffectGameCube *data) {
    RwUInt32 size = sizeof(MTEffectGameCubeStreamHeader);

    size += data->numTexMtx * (sizeof(MTEffectGameCubeStreamTexMtx) + sizeof(Mtx));
    size += data->numTexGens * sizeof(MTEffectGameCubeStreamTexGen);
    size += data->numIndMtx * (sizeof(MTEffectGameCubeStreamIndMtx) + sizeof(RwReal) * 6);
    size += data->numIndStages * sizeof(MTEffectGameCubeStreamIndStage);
    size += data->numTevStages * sizeof(MTEffectGameCubeStreamTevStage);
    return size;
}

RwInt32 fn_804A55CC(const RpMTEffect *effect) {
    return MTEffectGameCubeStreamSize(&((const RpMTEffectGameCube *)effect)->data);
}

const RpMTEffect *fn_804A560C(const RpMTEffect *effect, RwStream *stream) {
    const MTEffectGameCube *data = &((const RpMTEffectGameCube *)effect)->data;
    MTEffectGameCubeStreamHeader header;
    MTEffectGameCubeStreamTevStage tevRecord;
    MTEffectGameCubeStreamTexGen texGenRecord;
    MTEffectGameCubeStreamIndStage indStageRecord;
    MTEffectGameCubeStreamTexMtx texMtxRecord;
    MTEffectGameCubeStreamIndMtx indMtxRecord;
    RwUInt32 i;

    if (fn_8047CEF0(stream, rwID_EXTENSION, MTEffectGameCubeStreamSize(data), RW_STREAM_VERSION,
                    RW_STREAM_BUILD) == NULL) {
        return NULL;
    }

    header.flags = data->flags;
    header.numTevStages = data->numTevStages;
    header.numTexGens = data->numTexGens;
    header.numTexMtx = data->numTexMtx;
    header.numIndMtx = data->numIndMtx;
    header.numIndStages = data->numIndStages;
    header.blendModes = ((data->srcBlend & 0xF) << 4) | (data->dstBlend & 0xF);
    memcpy(header.kColors, data->kColors, sizeof(header.kColors));
    memcpy(header.tevColors, data->tevColors, sizeof(header.tevColors));
    fn_8047D1C8(header.tevColors, sizeof(header.tevColors));
    if (fn_80480ED0(stream, &header, sizeof(header)) == NULL) {
        return NULL;
    }

    for (i = 0; i < data->numTexGens; i++) {
        const MTEffectGameCubeTexGen *texGen = &data->texGens[i];

        texGenRecord.flags = texGen->flags;
        texGenRecord.func = texGen->func;
        texGenRecord.src = texGen->src;
        texGenRecord.mtx = texGen->mtx;
        texGenRecord.postMtx = texGen->postMtx;
        texGenRecord.pad[0] = 0;
        texGenRecord.pad[1] = 0;
        texGenRecord.pad[2] = 0;
        texGenRecord.scale[0] = texGen->scaleS;
        texGenRecord.scale[1] = texGen->scaleT;
        fn_8047D1C8(texGenRecord.scale, sizeof(texGenRecord.scale));
        if (fn_80480ED0(stream, &texGenRecord, sizeof(texGenRecord)) == NULL) {
            return NULL;
        }
    }

    for (i = 0; i < data->numTevStages; i++) {
        const MTEffectGameCubeTevStage *tev = &data->tevStages[i];

        tevRecord.colorIn[0] = tev->colorIn[0];
        tevRecord.colorIn[1] = tev->colorIn[1];
        tevRecord.colorIn[2] = tev->colorIn[2];
        tevRecord.colorIn[3] = tev->colorIn[3];
        tevRecord.alphaIn[0] = tev->alphaIn[0];
        tevRecord.alphaIn[1] = tev->alphaIn[1];
        tevRecord.alphaIn[2] = tev->alphaIn[2];
        tevRecord.alphaIn[3] = tev->alphaIn[3];
        tevRecord.colorOp = tev->colorOp;
        tevRecord.colorBias = tev->colorBias;
        tevRecord.colorScale = tev->colorScale;
        tevRecord.colorClamp = tev->colorClamp;
        tevRecord.colorOut = tev->colorOut;
        tevRecord.alphaOp = tev->alphaOp;
        tevRecord.alphaBias = tev->alphaBias;
        tevRecord.alphaScale = tev->alphaScale;
        tevRecord.alphaClamp = tev->alphaClamp;
        tevRecord.alphaOut = tev->alphaOut;
        tevRecord.kColorSel = tev->kColorSel;
        tevRecord.kAlphaSel = tev->kAlphaSel;
        tevRecord.texCoord = tev->texCoord;
        tevRecord.texMap = tev->texMap;
        tevRecord.color = tev->color;
        tevRecord.flags = tev->flags;
        tevRecord.indirect = tev->indirect;
        fn_8047D158(&tevRecord.indirect, sizeof(tevRecord.indirect));
        if (tev->texMap & rpMTGCNTEXMAPDISABLE) {
            tevRecord.flags |= rpMTGCNSTREAMTEXMAPDISABLE;
        }
        if (fn_80480ED0(stream, &tevRecord, sizeof(tevRecord)) == NULL) {
            return NULL;
        }
    }

    for (i = 0; i < data->numTexMtx; i++) {
        const MTEffectGameCubeTexMtx *texMtx = &data->texMtx[i];

        texMtxRecord.id = texMtx->id;
        texMtxRecord.source = texMtx->source;
        texMtxRecord.flags = texMtx->flags;
        texMtxRecord.pad = 0;
        if (fn_80480ED0(stream, &texMtxRecord, sizeof(texMtxRecord)) == NULL ||
            fn_8047D350(stream, &texMtx->matrix[0][0], sizeof(Mtx)) == NULL) {
            return NULL;
        }
    }

    for (i = 0; i < data->numIndMtx; i++) {
        const MTEffectGameCubeIndMtx *indMtx = &data->indMtx[i];

        indMtxRecord.id = indMtx->id;
        indMtxRecord.source = indMtx->source;
        indMtxRecord.flags = indMtx->flags;
        indMtxRecord.scaleExp = indMtx->scaleExp;
        if (fn_80480ED0(stream, &indMtxRecord, sizeof(indMtxRecord)) == NULL ||
            fn_8047D350(stream, &indMtx->offset[0][0], sizeof(indMtx->offset)) == NULL) {
            return NULL;
        }
    }

    for (i = 0; i < data->numIndStages; i++) {
        const MTEffectGameCubeIndStage *indStage = &data->indStages[i];

        indStageRecord.flags = indStage->flags;
        indStageRecord.texCoord = indStage->texCoord;
        indStageRecord.texMap = indStage->texMap;
        indStageRecord.scaleS = indStage->scaleS;
        indStageRecord.scaleT = indStage->scaleT;
        indStageRecord.pad[0] = 0;
        indStageRecord.pad[1] = 0;
        indStageRecord.pad[2] = 0;
        if (fn_80480ED0(stream, &indStageRecord, sizeof(indStageRecord)) == NULL) {
            return NULL;
        }
    }
    return effect;
}

RpMTEffect *fn_804A5A94(RwStream *stream, RwPlatformID platformID, RwUInt32 version,
                        RwUInt32 length) {
    MTEffectGameCubeStreamHeader header;
    MTEffectGameCubeStreamTevStage tevRecord;
    MTEffectGameCubeStreamTexGen texGenRecord;
    MTEffectGameCubeStreamIndStage indStageRecord;
    MTEffectGameCubeStreamTexMtx texMtxRecord;
    MTEffectGameCubeStreamIndMtx indMtxRecord;
    RpMTEffect *effect;
    MTEffectGameCube *data;
    RwUInt32 i;

    if (fn_80480D4C(stream, &header, sizeof(header)) != sizeof(header)) {
        return NULL;
    }
    effect = fn_804A5F48(header.numTevStages, header.numTexGens, header.numTexMtx,
                         header.numIndStages, header.numIndMtx);
    if (effect == NULL) {
        return NULL;
    }

    data = fn_804A60D0(effect);
    data->flags = header.flags;
    data->srcBlend = (header.blendModes >> 4) & 0xF;
    data->dstBlend = header.blendModes & 0xF;
    memcpy(data->kColors, header.kColors, sizeof(data->kColors));
    fn_8047D254(header.tevColors, sizeof(header.tevColors));
    memcpy(data->tevColors, header.tevColors, sizeof(data->tevColors));

    for (i = 0; i < data->numTexGens; i++) {
        MTEffectGameCubeTexGen *texGen = &data->texGens[i];

        if (fn_80480D4C(stream, &texGenRecord, sizeof(texGenRecord)) != sizeof(texGenRecord)) {
            RwFree(effect);
            return NULL;
        }
        texGen->flags = texGenRecord.flags;
        texGen->func = texGenRecord.func;
        texGen->src = texGenRecord.src;
        texGen->mtx = texGenRecord.mtx;
        texGen->postMtx = texGenRecord.postMtx;
        fn_8047D254(texGenRecord.scale, sizeof(texGenRecord.scale));
        texGen->scaleS = texGenRecord.scale[0];
        texGen->scaleT = texGenRecord.scale[1];
    }

    for (i = 0; i < data->numTevStages; i++) {
        MTEffectGameCubeTevStage *tev = &data->tevStages[i];

        if (fn_80480D4C(stream, &tevRecord, sizeof(tevRecord)) != sizeof(tevRecord)) {
            RwFree(effect);
            return NULL;
        }
        tev->colorIn[0] = tevRecord.colorIn[0];
        tev->colorIn[1] = tevRecord.colorIn[1];
        tev->colorIn[2] = tevRecord.colorIn[2];
        tev->colorIn[3] = tevRecord.colorIn[3];
        tev->alphaIn[0] = tevRecord.alphaIn[0];
        tev->alphaIn[1] = tevRecord.alphaIn[1];
        tev->alphaIn[2] = tevRecord.alphaIn[2];
        tev->alphaIn[3] = tevRecord.alphaIn[3];
        tev->colorOp = tevRecord.colorOp;
        tev->colorBias = tevRecord.colorBias;
        tev->colorScale = tevRecord.colorScale;
        tev->colorClamp = tevRecord.colorClamp;
        tev->colorOut = tevRecord.colorOut;
        tev->alphaOp = tevRecord.alphaOp;
        tev->alphaBias = tevRecord.alphaBias;
        tev->alphaScale = tevRecord.alphaScale;
        tev->alphaClamp = tevRecord.alphaClamp;
        tev->alphaOut = tevRecord.alphaOut;
        tev->kColorSel = tevRecord.kColorSel;
        tev->kAlphaSel = tevRecord.kAlphaSel;
        tev->texCoord = tevRecord.texCoord;
        tev->texMap = tevRecord.texMap;
        tev->color = tevRecord.color;
        tev->flags = tevRecord.flags;
        fn_8047D254(&tevRecord.indirect, sizeof(tevRecord.indirect));
        tev->indirect = tevRecord.indirect;
        if (tev->flags & rpMTGCNSTREAMTEXMAPDISABLE) {
            tev->texMap |= rpMTGCNTEXMAPDISABLE;
            tev->flags &= ~rpMTGCNSTREAMTEXMAPDISABLE;
        }
    }

    for (i = 0; i < data->numTexMtx; i++) {
        MTEffectGameCubeTexMtx *texMtx = &data->texMtx[i];

        if (fn_80480D4C(stream, &texMtxRecord, sizeof(texMtxRecord)) != sizeof(texMtxRecord) ||
            fn_8047D784(stream, &texMtx->matrix[0][0], sizeof(Mtx)) == NULL) {
            RwFree(effect);
            return NULL;
        }
        texMtx->id = texMtxRecord.id;
        texMtx->source = texMtxRecord.source;
        texMtx->flags = texMtxRecord.flags;
    }

    for (i = 0; i < data->numIndMtx; i++) {
        MTEffectGameCubeIndMtx *indMtx = &data->indMtx[i];

        if (fn_80480D4C(stream, &indMtxRecord, sizeof(indMtxRecord)) != sizeof(indMtxRecord) ||
            fn_8047D784(stream, &indMtx->offset[0][0], sizeof(indMtx->offset)) == NULL) {
            RwFree(effect);
            return NULL;
        }
        indMtx->id = indMtxRecord.id;
        indMtx->source = indMtxRecord.source;
        indMtx->flags = indMtxRecord.flags;
        indMtx->scaleExp = (RwInt8)indMtxRecord.scaleExp;
    }

    for (i = 0; i < data->numIndStages; i++) {
        MTEffectGameCubeIndStage *indStage = &data->indStages[i];

        if (fn_80480D4C(stream, &indStageRecord, sizeof(indStageRecord)) !=
            sizeof(indStageRecord)) {
            RwFree(effect);
            return NULL;
        }
        indStage->flags = indStageRecord.flags;
        indStage->texCoord = indStageRecord.texCoord;
        indStage->texMap = indStageRecord.texMap;
        indStage->scaleS = indStageRecord.scaleS;
        indStage->scaleT = indStageRecord.scaleT;
    }
    return effect;
}

RwBool fn_804A5F08(void) {
    return fn_804A912C(rwID_GAMECUBE, fn_804A5A94, fn_804A560C, fn_804A55CC, NULL);
}

#define ALIGN4(x) (((x) + 3) & ~3)

RpMTEffect *fn_804A5F48(RwUInt32 numTevStages, RwUInt32 numTexGens, RwUInt32 numTexMtx,
                        RwUInt32 numIndStages, RwUInt32 numIndMtx) {
    RwInt32 texMtxSize = numTexMtx * sizeof(MTEffectGameCubeTexMtx);
    RwInt32 texGensSize = numTexGens * sizeof(MTEffectGameCubeTexGen);
    RwInt32 indMtxSize = numIndMtx * sizeof(MTEffectGameCubeIndMtx);
    RwInt32 indStagesSize = numIndStages * sizeof(MTEffectGameCubeIndStage);
    RwInt32 size;
    RpMTEffectGameCube *effect;
    RwUInt8 *arrays;

    size = sizeof(RpMTEffectGameCube) + texGensSize;
    size = ALIGN4(size);
    size += numTevStages * sizeof(MTEffectGameCubeTevStage);
    size = ALIGN4(size);
    size += indStagesSize;
    size = ALIGN4(size);
    size += texMtxSize;
    size = ALIGN4(size);
    size += indMtxSize;
    size = ALIGN4(size);
    effect = RwMalloc(size, rwMEMHINT_GCNMTEFFECT);
    if (effect == NULL) {
        RwError error;

        error.pluginID = rpID_MATFXPLUGIN;
        error.errorCode = fn_8047DC28(E_RW_NOMEM, size);
        fn_8047DB84(&error);
        return NULL;
    }
    memset(effect, 0, size);
    fn_804A932C(&effect->effect, rwID_GAMECUBE);

    effect->data.maxTevStages = numTevStages;
    effect->data.maxTexGens = numTexGens;
    effect->data.maxIndStages = numIndStages;
    effect->data.maxTexMtx = numTexMtx;
    effect->data.maxIndMtx = numIndMtx;
    effect->data.numTevStages = effect->data.maxTevStages;
    effect->data.numTexGens = effect->data.maxTexGens;
    effect->data.numIndStages = effect->data.maxIndStages;
    effect->data.numTexMtx = effect->data.maxTexMtx;
    effect->data.numIndMtx = effect->data.maxIndMtx;

    arrays = (RwUInt8 *)(effect + 1);
    if (numTexMtx != 0) {
        effect->data.texMtx = (MTEffectGameCubeTexMtx *)arrays;
        arrays += texMtxSize;
    }
    if (numTexGens != 0) {
        effect->data.texGens = (MTEffectGameCubeTexGen *)arrays;
        arrays += texGensSize;
    }
    if (numIndMtx != 0) {
        effect->data.indMtx = (MTEffectGameCubeIndMtx *)arrays;
        arrays += indMtxSize;
    }
    if (numIndStages != 0) {
        effect->data.indStages = (MTEffectGameCubeIndStage *)arrays;
        arrays += indStagesSize;
    }
    if (numTevStages != 0) {
        effect->data.tevStages = (MTEffectGameCubeTevStage *)arrays;
    }
    return &effect->effect;
}

MTEffectGameCube *fn_804A60D0(RpMTEffect *effect) {
    return &((RpMTEffectGameCube *)effect)->data;
}

/* Unpacks a TEV stage's indirect setup */
void fn_804A60D8(MTEffectGameCubeIndirect *indirect, const RwUInt32 *packed) {
    RwUInt32 bits = *packed;

    indirect->indStage = bits & 0x3;
    indirect->format = (bits >> 2) & 0x3;
    indirect->biasSel = (bits >> 4) & 0x7;
    indirect->matrixSel = (bits >> 7) & 0xF;
    indirect->wrapS = (bits >> 11) & 0x7;
    indirect->wrapT = (bits >> 14) & 0x7;
    indirect->addPrev = (bits >> 17) & 0x1;
    indirect->utcLod = (bits >> 18) & 0x1;
    indirect->alphaSel = (bits >> 19) & 0x3;
}

void *fn_804A6128(void *instance, RwInt32 offset, RwInt32 size) {
    memset(MTGAMECUBEGLOBALS(), 0, sizeof(MTGameCubeGlobals));
    return instance;
}

void *fn_804A616C(void *instance, RwInt32 offset, RwInt32 size) {
    return instance;
}

/* The matrix a texture matrix source projects from, combined with the object's LTM: a projection
 * frame's inverse LTM, the camera's view or the LTM itself; NULL for an unknown source */
RwMatrix *fn_804A6170(RwInt32 source, RwMatrix *ltm, RwMatrix *result) {
    if (source >= rpMTGCNSOURCEFRAME0) {
        MTGameCubeGlobals *globals = MTGAMECUBEGLOBALS();
        RwUInt32 index = source - rpMTGCNSOURCEFRAME0;
        RwUInt32 numFrames = globals->numFrames;
        RwFrame **frames = globals->frames;

        if (index < numFrames && frames != NULL && frames[index] != NULL) {
            if (ltm != NULL) {
                RwMatrix inverse;

                fn_8047F0B0(&inverse, RwFrameGetLTM(frames[index]));
                fn_8047EBB8(result, ltm, &inverse);
            } else {
                fn_8047F0B0(result, RwFrameGetLTM(frames[index]));
            }
            return result;
        }
        if (ltm != NULL) {
            return ltm;
        }
        RwMatrixSetIdentity(result);
        return result;
    } else if (source == rpMTGCNSOURCECAMERA) {
        RwMatrix *matrix;

        if (ltm != NULL) {
            fn_8047EBB8(result, ltm, &lbl_805E31CC);
            matrix = result;
        } else {
            matrix = &lbl_805E31CC;
        }
        return matrix;
    } else if (source == rpMTGCNSOURCELTM) {
        if (ltm != NULL) {
            return ltm;
        }
        return NULL;
    }
    return NULL;
}

/* Sets up the GX state of a GameCube effect for a material's texture set: the TEV colours, the
 * texture and indirect matrices, the indirect and TEV stages, the texture coordinate generators
 * (whose texture coordinates follow the material's coordinate sets), alpha testing and blending */
RpMTEffect *fn_804A62E4(RpMTEffect *effect, RwUInt32 numTextures, const RwUInt8 *coords,
                        RwMatrix *ltm) {
    MTEffectGameCube *data = fn_804A60D0(effect);
    RwInt32 colorMode;
    RwInt8 texMapOfTexCoord[8];
    RwInt32 i;

    if (data->flags & rpMTGCNEFFECTTEVCOLORS) {
        GXSetTevColorS10(GX_TEVREG0, data->tevColors[0]);
        GXSetTevColorS10(GX_TEVREG1, data->tevColors[1]);
        GXSetTevColorS10(GX_TEVREG2, data->tevColors[2]);
        GXSetTevColorS10(GX_TEVPREV, data->tevColors[3]);
    }
    if (data->flags & rpMTGCNEFFECTKCOLORS) {
        GXSetTevKColor(GX_KCOLOR0, data->kColors[0]);
        GXSetTevKColor(GX_KCOLOR1, data->kColors[1]);
        GXSetTevKColor(GX_KCOLOR2, data->kColors[2]);
        GXSetTevKColor(GX_KCOLOR3, data->kColors[3]);
    }
    if (data->flags & rpMTGCNEFFECTENVMATRIX) {
        RwMatrix objectMatrix;
        RwMatrix *matrix;
        Mtx texMatrix;

        if (ltm != NULL) {
            matrix = fn_8047EBB8(&objectMatrix, ltm, &lbl_805E31CC);
        } else {
            matrix = &lbl_805E31CC;
        }
        texMatrix[0][0] = -0.5f * matrix->right.x;
        texMatrix[0][1] = -0.5f * matrix->up.x;
        texMatrix[0][2] = -0.5f * matrix->at.x;
        texMatrix[0][3] = 0.5f;
        texMatrix[1][0] = -0.5f * matrix->right.y;
        texMatrix[1][1] = -0.5f * matrix->up.y;
        texMatrix[1][2] = -0.5f * matrix->at.y;
        texMatrix[1][3] = 0.5f;
        GXLoadTexMtxImm(texMatrix, GX_TEXMTX0, GX_MTX2x4);
    }

    for (i = 0; i < data->numTexMtx; i++) {
        MTEffectGameCubeTexMtx *texMtx = &data->texMtx[i];
        RwMatrix sourceMatrix;
        RwMatrix *source = fn_804A6170(texMtx->source, ltm, &sourceMatrix);

        if (source != NULL) {
            Mtx sourceMtx;
            Mtx result;

            sourceMtx[0][0] = source->right.x;
            sourceMtx[0][1] = source->up.x;
            sourceMtx[0][2] = source->at.x;
            sourceMtx[1][0] = source->right.y;
            sourceMtx[1][1] = source->up.y;
            sourceMtx[1][2] = source->at.y;
            sourceMtx[2][0] = source->right.z;
            sourceMtx[2][1] = source->up.z;
            sourceMtx[2][2] = source->at.z;
            if (texMtx->flags & rpMTGCNTEXMTXTRANSLATION) {
                sourceMtx[0][3] = source->pos.x;
                sourceMtx[1][3] = source->pos.y;
                sourceMtx[2][3] = source->pos.z;
            } else {
                sourceMtx[0][3] = 0.0f;
                sourceMtx[1][3] = 0.0f;
                sourceMtx[2][3] = 0.0f;
            }
            PSMTXConcat(texMtx->matrix, sourceMtx, result);
            GXLoadTexMtxImm(result, texMtx->id, texMtx->flags & rpMTGCNTEXMTX2X4);
        } else {
            GXLoadTexMtxImm(texMtx->matrix, texMtx->id, texMtx->flags & rpMTGCNTEXMTX2X4);
        }
    }

    for (i = 0; i < data->numIndMtx; i++) {
        MTEffectGameCubeIndMtx *indMtx = &data->indMtx[i];
        RwMatrix sourceMatrix;
        RwMatrix *source = fn_804A6170(indMtx->source, ltm, &sourceMatrix);
        RwReal(*offset)[3];
        RwReal combined[2][3];

        if (source != NULL) {
            combined[0][0] = indMtx->offset[0][0] * source->right.x +
                             indMtx->offset[0][1] * source->right.y +
                             indMtx->offset[0][2] * source->right.z;
            combined[0][1] = indMtx->offset[0][0] * source->up.x +
                             indMtx->offset[0][1] * source->up.y +
                             indMtx->offset[0][2] * source->up.z;
            combined[0][2] = indMtx->offset[0][0] * source->at.x +
                             indMtx->offset[0][1] * source->at.y +
                             indMtx->offset[0][2] * source->at.z;
            combined[1][0] = indMtx->offset[1][0] * source->right.x +
                             indMtx->offset[1][1] * source->right.y +
                             indMtx->offset[1][2] * source->right.z;
            combined[1][1] = indMtx->offset[1][0] * source->up.x +
                             indMtx->offset[1][1] * source->up.y +
                             indMtx->offset[1][2] * source->up.z;
            combined[1][2] = indMtx->offset[1][0] * source->at.x +
                             indMtx->offset[1][1] * source->at.y +
                             indMtx->offset[1][2] * source->at.z;
            offset = combined;
        } else {
            offset = indMtx->offset;
        }
        GXSetIndTexMtx(indMtx->id, offset, (RwInt8)indMtx->scaleExp);
    }

    /* 2: the stages' colour channels are all channel 0 */
    colorMode = 1;
    if (data->flags & rpMTGCNEFFECTCOLOR0) {
        colorMode = 2;
    }
    for (i = 0; i < data->numTexGens; i++) {
        texMapOfTexCoord[i] = -1;
    }

    if (data->numIndStages != 0) {
        GXSetNumIndStages(data->numIndStages);
        for (i = 0; i < data->numIndStages; i++) {
            MTEffectGameCubeIndStage *indStage = &data->indStages[i];

            if (!(indStage->flags & rpMTGCNDISABLED)) {
                GXSetIndTexOrder(i, indStage->texCoord, indStage->texMap);
                GXSetIndTexCoordScale(i, indStage->scaleS, indStage->scaleT);
                texMapOfTexCoord[indStage->texCoord] = indStage->texMap;
            }
        }
    }

    if (data->numTevStages != 0) {
        GXSetNumTevStages(data->numTevStages);
        for (i = 0; i < data->numTevStages; i++) {
            MTEffectGameCubeTevStage *tev = &data->tevStages[i];
            GXChannelID color;

            if (tev->flags & rpMTGCNDISABLED) {
                continue;
            }
            if (tev->indirect != 0) {
                MTEffectGameCubeIndirect indirect;

                fn_804A60D8(&indirect, &tev->indirect);
                GXSetTevIndirect(i, indirect.indStage, indirect.format, indirect.biasSel,
                                 indirect.matrixSel, indirect.wrapS, indirect.wrapT,
                                 indirect.addPrev, indirect.utcLod, indirect.alphaSel);
            } else {
                GXSetTevDirect(i);
            }
            GXSetTevColorIn(i, tev->colorIn[0], tev->colorIn[1], tev->colorIn[2], tev->colorIn[3]);
            GXSetTevAlphaIn(i, tev->alphaIn[0], tev->alphaIn[1], tev->alphaIn[2], tev->alphaIn[3]);
            color = tev->color;
            if (color <= GX_COLOR1A1 && colorMode == 2) {
                color = GX_COLOR0A0;
            }
            GXSetTevOrder(i, tev->texCoord, tev->texMap, color);
            GXSetTevKColorSel(i, tev->kColorSel);
            GXSetTevKAlphaSel(i, tev->kAlphaSel);
            GXSetTevColorOp(i, tev->colorOp, tev->colorBias, tev->colorScale, tev->colorClamp,
                            tev->colorOut);
            GXSetTevAlphaOp(i, tev->alphaOp, tev->alphaBias, tev->alphaScale, tev->alphaClamp,
                            tev->alphaOut);
            texMapOfTexCoord[tev->texCoord] = tev->texMap;
        }
    }

    GXSetNumTexGens(data->numTexGens);
    for (i = 0; i < data->numTexGens; i++) {
        MTEffectGameCubeTexGen *texGen = &data->texGens[i];
        GXTexGenSrc src;

        if (texGen->flags & rpMTGCNDISABLED) {
            continue;
        }
        src = texGen->src;
        if (coords != NULL && src >= GX_TG_TEX0 && src <= GX_TG_TEX7 && texMapOfTexCoord[i] >= 0) {
            src = coords[texMapOfTexCoord[i]] + GX_TG_TEX0;
        }
        GXSetTexCoordGen2(i, texGen->func, src, texGen->mtx,
                          (texGen->flags & rpMTGCNTEXGENNORMALIZE) >> 1, texGen->postMtx);
        if (texGen->flags & rpMTGCNTEXGENSCALEMANUALLY) {
            GXSetTexCoordScaleManually(i, GX_TRUE, texGen->scaleS, texGen->scaleT);
        }
    }

    if (data->flags & rpMTGCNEFFECTALPHATEST) {
        fn_80497778(FALSE);
    } else {
        fn_80497778(TRUE);
    }
    if (data->flags & rpMTGCNEFFECTBLEND) {
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)data->srcBlend);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)data->dstBlend);
    }
    return effect;
}

void fn_804A6A08(RpGameCubeDisplayList *displayList, RpMaterial *material, RpAtomic *atomic,
                 RpGameCubeRenderMesh *renderMesh, RwMatrix *ltm) {
    RpMultiTexture *multiTexture = fn_804A90C8(material, rwID_GAMECUBE);
    MTGameCubeMaterialData *materialData = multiTexture->platformData;
    RwInt32 srcBlend;
    RwInt32 dstBlend;
    RwInt32 i;

    RwRenderStateGet(rwRENDERSTATESRCBLEND, &srcBlend);
    RwRenderStateGet(rwRENDERSTATEDESTBLEND, &dstBlend);
    for (i = 0; i < multiTexture->numTextures; i++) {
        fn_80499C38(multiTexture->textures[i], i);
    }
    if (materialData->preRender != NULL) {
        materialData->preRender(material, atomic, renderMesh, materialData->data);
    }

    if (multiTexture->effect != NULL) {
        MTEffectGameCube *data;

        fn_804A62E4(multiTexture->effect, multiTexture->numTextures, multiTexture->coords, ltm);
        GXCallDisplayList(displayList->data, displayList->size);

        /* undo what the effect set that later rendering does not set itself */
        data = fn_804A60D0(multiTexture->effect);
        for (i = 0; i < data->numTexGens; i++) {
            if (data->texGens[i].flags & rpMTGCNTEXGENSCALEMANUALLY) {
                GXSetTexCoordScaleManually(i, GX_FALSE, 0, 0);
            }
        }
        if (data->numIndStages != 0) {
            GXSetNumIndStages(0);
            for (i = 0; i < data->numIndStages; i++) {
                GXSetIndTexCoordScale(i, GX_ITS_1, GX_ITS_1);
            }
            for (i = 0; i < data->numTevStages; i++) {
                if (data->tevStages[i].indirect != 0) {
                    GXSetTevDirect(i);
                }
            }
        }
    } else {
        GXCallDisplayList(displayList->data, displayList->size);
    }

    if (materialData->postRender != NULL) {
        materialData->postRender(material, atomic, renderMesh, materialData->data);
    }
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)srcBlend);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)dstBlend);
}

RwBool fn_804A6C30(void) {
    lbl_805F16C8 =
        fn_80485D78(sizeof(MTGameCubeGlobals), rpID_GCNMATPLUGIN, fn_804A6128, fn_804A616C);
    return lbl_805F16C8 >= 0;
}

RwBool fn_804A6C74(void) {
    if (fn_804A8E90() == FALSE) {
        return FALSE;
    }
    if (fn_804A8EFC(rwID_GAMECUBE, rpID_GCNMATPLUGIN, sizeof(MTGameCubeMaterialData)) == FALSE) {
        return FALSE;
    }
    if (fn_804A5F08() == FALSE) {
        return FALSE;
    }
    return fn_804A6C30();
}

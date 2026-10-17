#ifndef RW_RPMATFX_H
#define RW_RPMATFX_H

#include "rw/rpworld.h"
#include "layout_check.h"

/* RenderWare material effects (plugin ID 0x120): up to two effects on a material (bump mapping,
 * environment mapping, a second texture pass, texture coordinate transforms), rendered for the
 * atomics and world sectors that enable them; and multi-texturing, a per-platform set of textures
 * on a material (plugin ID 0x12C) rendered by a named multi-texture effect, which effect
 * dictionaries keep. Names follow RenderWare's conventions; those that the stream format, plugin
 * IDs and code do not establish are inferred. */

#ifdef __cplusplus
extern "C" {
#endif

#define rpID_MATFXPLUGIN 0x120
#define rpID_MULTITEXPLUGIN 0x12C

/* the platforms a multi-texture effect or texture set can be for */
#define rpMTNUMPLATFORMS 10

/* The effects of a material (fn_804A20D0), and the effect of each of its two slots */
#define rpMATFXEFFECTNULL 0
#define rpMATFXEFFECTBUMPMAP 1
#define rpMATFXEFFECTENVMAP 2
#define rpMATFXEFFECTBUMPENVMAP 3 /* a bump map slot, then an environment map slot */
#define rpMATFXEFFECTDUAL 4
#define rpMATFXEFFECTUVTRANSFORM 5
#define rpMATFXEFFECTDUALUVTRANSFORM 6 /* a texture coordinate transform slot, then a dual slot */

/* A bump map: the texture, the texture with the bumps in its alpha that is rendered, the frame of
 * the light direction and the bumpiness (stored negated) */
typedef struct MatFXBumpMapData {
    /* 0x00 */ RwFrame *frame;
    /* 0x04 */ RwTexture *bumpedTexture;
    /* 0x08 */ RwTexture *texture;
    /* 0x0C */ RwReal coef;
    /* 0x10 */ RwReal invBumpWidth; /* 1 / the bumped texture's width */
} MatFXBumpMapData;
CHECK_SIZEOF(MatFXBumpMapData, 0x14);

/* An environment map: the frame it is projected from, its texture and how strongly it applies */
typedef struct MatFXEnvMapData {
    /* 0x00 */ RwFrame *frame;
    /* 0x04 */ RwTexture *texture;
    /* 0x08 */ RwReal coef;
    /* 0x0C */ RwBool useFrameBufferAlpha;
} MatFXEnvMapData;
CHECK_SIZEOF(MatFXEnvMapData, 0x10);

/* A second texture pass and how it blends with the first */
typedef struct MatFXDualData {
    /* 0x00 */ RwTexture *texture;
    /* 0x04 */ RwInt32 srcBlendMode; /* rwBLEND* */
    /* 0x08 */ RwInt32 dstBlendMode;
} MatFXDualData;
CHECK_SIZEOF(MatFXDualData, 0xC);

/* The texture coordinate transforms of the base and the dual texture pass */
typedef struct MatFXUVAnimData {
    /* 0x00 */ RwMatrix *baseTransform;
    /* 0x04 */ RwMatrix *dualTransform;
} MatFXUVAnimData;
CHECK_SIZEOF(MatFXUVAnimData, 0x8);

typedef union MatFXEffectUnion {
    MatFXBumpMapData bumpMap;
    MatFXEnvMapData envMap;
    MatFXDualData dual;
    MatFXUVAnimData uvAnim;
} MatFXEffectUnion;
CHECK_SIZEOF(MatFXEffectUnion, 0x14);

/* One effect slot of a material */
typedef struct MatFXEffectData {
    /* 0x00 */ MatFXEffectUnion data;
    /* 0x14 */ RwInt32 flag; /* the slot's effect: rpMATFXEFFECT* */
} MatFXEffectData;
CHECK_SIZEOF(MatFXEffectData, 0x18);

#define rpMATFXNUMSLOTS 2

/* The material effects of a material (from a free list) */
typedef struct rpMatFXMaterialData {
    /* 0x00 */ MatFXEffectData data[rpMATFXNUMSLOTS];
    /* 0x30 */ RwInt32 flags; /* the material's effects: rpMATFXEFFECT* */
} rpMatFXMaterialData;
CHECK_SIZEOF(rpMatFXMaterialData, 0x34);

extern RwInt32 lbl_805F16A8; /* material plugin offset */
extern RwInt32 lbl_805F16AC; /* atomic plugin offset */
extern RwInt32 lbl_805F16B0; /* world sector plugin offset */

/* the plugin data: a material's effects (NULL until it has some), and whether an atomic's or a
 * world sector's material effects are rendered */
#define MATFXMATERIALGETDATA(material) RWPLUGINOFFSET(rpMatFXMaterialData *, material, lbl_805F16A8)
#define MATFXATOMICGETDATA(atomic) RWPLUGINOFFSET(RwBool, atomic, lbl_805F16AC)
#define MATFXWORLDSECTORGETDATA(sector) RWPLUGINOFFSET(RwBool, sector, lbl_805F16B0)

/* material effects textures in a stream: a flag, then the texture when it is set (inferred) */
RwStream *fn_804A0C50(RwStream *stream, const RwTexture *texture); /* write */
RwStream *fn_804A0CDC(RwStream *stream, RwTexture **texture);      /* read */
RwUInt32 fn_804A0DB8(const RwTexture *texture);                    /* the stream size */

/* makes the bumped texture of a bump map texture and a base texture (or none) (inferred) */
RwTexture *fn_804A1A08(RwTexture *baseTexture, RwTexture *bumpTexture);

/* attaches the plugin, with the GameCube multi-texturing; TRUE on success (inferred) */
RwBool fn_804A1E74(void);

/* atomics and world sectors (inferred) */
RpAtomic *fn_804A2004(RpAtomic *atomic);                /* enables its material effects */
RwBool fn_804A2064(RpAtomic *atomic);                   /* whether an atomic has material effects */
RpWorldSector *fn_804A2070(RpWorldSector *worldSector); /* enables its material effects */

/* materials (inferred) */
RpMaterial *fn_804A20D0(RpMaterial *material, RwInt32 flags); /* sets the effects */
RwInt32 fn_804A22E4(const RpMaterial *material);              /* the effects */
/* the bump map */
RpMaterial *fn_804A2304(RpMaterial *material, RwTexture *texture); /* sets the texture */
RpMaterial *fn_804A25E0(RpMaterial *material, RwFrame *frame);
RpMaterial *fn_804A2628(RpMaterial *material, RwReal coef);
RwTexture *fn_804A2674(const RpMaterial *material); /* the texture */
RwFrame *fn_804A26BC(const RpMaterial *material);
RwReal fn_804A2704(const RpMaterial *material); /* the coefficient */
/* the environment map */
RpMaterial *fn_804A2750(RpMaterial *material, RwTexture *texture);
RpMaterial *fn_804A27F8(RpMaterial *material, RwFrame *frame);
RpMaterial *fn_804A2840(RpMaterial *material, RwBool useFrameBufferAlpha);
RpMaterial *fn_804A2888(RpMaterial *material, RwReal coef);
RwTexture *fn_804A28D0(const RpMaterial *material);
RwFrame *fn_804A2918(const RpMaterial *material);
RwBool fn_804A2960(const RpMaterial *material); /* whether it uses the frame buffer alpha */
RwReal fn_804A29A8(const RpMaterial *material); /* the coefficient */
/* the dual pass */
RpMaterial *fn_804A29F0(RpMaterial *material, RwTexture *texture);
RpMaterial *fn_804A2AB0(RpMaterial *material, RwInt32 srcBlendMode, RwInt32 dstBlendMode);
RwTexture *fn_804A2B44(const RpMaterial *material);
const RpMaterial *fn_804A2B8C(const RpMaterial *material, RwInt32 *srcBlendMode,
                              RwInt32 *dstBlendMode);
/* the texture coordinate transforms */
RpMaterial *fn_804A2BE0(RpMaterial *material, RwMatrix *baseTransform, RwMatrix *dualTransform);
const RpMaterial *fn_804A2C2C(const RpMaterial *material, RwMatrix **baseTransform,
                              RwMatrix **dualTransform);

/* GameCube rendering (inferred) */
void fn_804A2F08(void); /* resets the material effects render state */
/* renders a mesh with material effects */
void fn_804A457C(RpMesh *mesh, RpGameCubeDisplayList *displayList, RpAtomic *atomic, RwMatrix *ltm,
                 RpGameCubeRenderMesh *renderMesh);
extern RpGameCubeVertexFormat *lbl_805F16B8; /* the vertex format of material effects geometries */
/* builds the binormals and tangents of a skinned geometry's render mesh */
void fn_804A8120(RpGameCubeRenderMesh *renderMesh, RpGameCubeVertexFormat *format,
                 RwInt32 numVertices);
/* whether any of a render mesh's materials has a GameCube multi-texture effect that needs
 * binormals and tangents */
RwBool fn_804A8478(RpGameCubeRenderMesh *renderMesh);

/* platform pipelines and render setup (inferred) */
RwBool fn_804A5494(void);                          /* creates the pipelines */
RwBool fn_804A5524(void);                          /* destroys them */
RpAtomic *fn_804A558C(RpAtomic *atomic);           /* sets an atomic's pipeline */
RpWorldSector *fn_804A5598(RpWorldSector *sector); /* sets a world sector's pipeline */
RwBool fn_804A55A4(MatFXDualData *dualData, RwInt32 renderState);
/* the bumped texture made from a bump map texture and the material's texture */
RwTexture *fn_804A55AC(RwTexture *baseTexture, RwTexture *bumpTexture);
/* renders a mesh with its material's GameCube multi-texture effect (inferred) */
void fn_804A6A08(RpGameCubeDisplayList *displayList, RpMaterial *material, RpAtomic *atomic,
                 RpGameCubeRenderMesh *renderMesh, RwMatrix *ltm);
/* attaches the GameCube multi-texture effects; TRUE on success */
RwBool fn_804A6C74(void);

#define rpMTEFFECTNAMELENGTH 32
#define rpMTEFFECTPATHLENGTH 256

/* A multi-texture effect (platform data follows it): it is in at most one effect dictionary */
typedef struct RpMTEffect {
    /* 0x00 */ RwPlatformID platformID; /* 0 for a dummy effect */
    /* 0x04 */ RwUInt32 refCount;
    /* 0x08 */ RwChar name[rpMTEFFECTNAMELENGTH];
    /* 0x28 */ RwLLLink dictLink;
} RpMTEffect;
CHECK_SIZEOF(RpMTEffect, 0x30);

/* A dictionary of multi-texture effects */
typedef struct RpMTEffectDict {
    /* 0x00 */ RwLinkList effectList;
    /* 0x08 */ RwLLLink dictLink; /* in the list of all dictionaries */
} RpMTEffectDict;
CHECK_SIZEOF(RpMTEffectDict, 0x10);

#define MTEFFECTFROMDICTLINK(link)                                                                 \
    ((RpMTEffect *)((RwUInt8 *)(link) - offsetof(RpMTEffect, dictLink)))
#define MTEFFECTDICTFROMLINK(link)                                                                 \
    ((RpMTEffectDict *)((RwUInt8 *)(link) - offsetof(RpMTEffectDict, dictLink)))

/* a platform's multi-texture effect callbacks */
typedef void (*rpMTEffectDestroyCallBack)(RpMTEffect *effect);
typedef RpMTEffect *(*rpMTEffectStreamReadCallBack)(RwStream *stream, RwPlatformID platformID,
                                                    RwUInt32 version, RwUInt32 length);
typedef const RpMTEffect *(*rpMTEffectStreamWriteCallBack)(const RpMTEffect *effect,
                                                           RwStream *stream);
typedef RwInt32 (*rpMTEffectStreamGetSizeCallBack)(const RpMTEffect *effect);

/* The multi-texture effect callbacks registered for a platform */
typedef struct rpMTEffectPlatform {
    /* 0x00 */ RwPlatformID platformID;
    /* 0x04 */ rpMTEffectDestroyCallBack destroy; /* NULL frees the effect */
    /* 0x08 */ rpMTEffectStreamReadCallBack streamRead;
    /* 0x0C */ rpMTEffectStreamWriteCallBack streamWrite;
    /* 0x10 */ rpMTEffectStreamGetSizeCallBack streamGetSize;
} rpMTEffectPlatform;
CHECK_SIZEOF(rpMTEffectPlatform, 0x14);

/* The multi-texturing globals in the engine */
typedef struct rpMultiTextureGlobals {
    /* 0x00 */ RwLinkList dictList;         /* the effect dictionaries */
    /* 0x08 */ RpMTEffectDict *currentDict; /* effects are created in and found in it */
    /* 0x0C */ RwUInt32 pathSize;
    /* 0x10 */ RwChar *path;     /* where effects that are not in a dictionary are read from */
    /* 0x14 */ RwChar *fileName; /* the path and an effect's name */
} rpMultiTextureGlobals;
CHECK_SIZEOF(rpMultiTextureGlobals, 0x18);

extern RwModuleInfo lbl_805EDF68; /* the multi-texturing module */

#define MULTITEXTUREGLOBALS()                                                                      \
    RWPLUGINOFFSET(rpMultiTextureGlobals, lbl_805F15B4, lbl_805EDF68.globalsOffset)
#define MULTITEXTUREGLOBAL(var) (MULTITEXTUREGLOBALS()->var)

/* multi-texture effects (inferred) */
RwBool fn_804A90F8(void); /* initializes the platforms' callbacks */
RwBool fn_804A912C(RwPlatformID platformID, rpMTEffectStreamReadCallBack streamRead,
                   rpMTEffectStreamWriteCallBack streamWrite,
                   rpMTEffectStreamGetSizeCallBack streamGetSize,
                   rpMTEffectDestroyCallBack destroy); /* registers a platform's callbacks */
RwBool fn_804A9158(void);                              /* opens the effect dictionaries */
RwBool fn_804A9274(void);                              /* closes them */
/* initializes a new effect with one reference, adding it to the current dictionary */
RpMTEffect *fn_804A932C(RpMTEffect *effect, RwPlatformID platformID);
RpMTEffectDict *fn_804A93B0(void);                                     /* creates a dictionary */
void fn_804A9468(RpMTEffectDict *dict);                                /* destroys a dictionary */
RpMTEffectDict *fn_804A9500(RpMTEffectDict *dict, RpMTEffect *effect); /* adds an effect */
RpMTEffect *fn_804A9584(RpMTEffect *effect); /* removes an effect from its dictionary */
RpMTEffect *fn_804A95D4(void);               /* creates an effect for no platform */
void fn_804A9674(RpMTEffect *effect);        /* releases a reference, destroying it with the last */
/* finds an effect by name, reading it from the effect path when no dictionary has it; adds a
 * reference */
RpMTEffect *fn_804A9914(RwChar *name);
RpMTEffect *fn_804A9BA4(RpMTEffect *effect, RwChar *name); /* sets the name */
RpMTEffect *fn_804A9BE8(RpMTEffect *effect);               /* adds a reference */

#define rpMULTITEXTUREMAXTEXTURES 8

/* A platform's multi-texture material plugin */
typedef struct rpMultiTexturePlatform {
    /* 0x00 */ RwPlatformID platformID;
    /* 0x04 */ RwUInt32 pluginID; /* 0 while the platform has none */
    /* 0x08 */ RwInt32 offset;    /* of a material's texture set for the platform */
    /* 0x0C */ RwUInt32 dataSize; /* the bytes of platform data after each texture set */
} rpMultiTexturePlatform;
CHECK_SIZEOF(rpMultiTexturePlatform, 0x10);

/* A material's textures for a platform and the effect that renders them; the platform's data
 * follows it */
typedef struct RpMultiTexture {
    /* 0x00 */ rpMultiTexturePlatform *platform;
    /* 0x04 */ RwUInt32 numTextures;
    /* 0x08 */ RwTexture *textures[rpMULTITEXTUREMAXTEXTURES];
    /* 0x28 */ RwUInt8 coords[rpMULTITEXTUREMAXTEXTURES]; /* each texture's coordinate set */
    /* 0x30 */ RpMTEffect *effect;
    /* 0x34 */ void *platformData; /* NULL for none */
} RpMultiTexture;
CHECK_SIZEOF(RpMultiTexture, 0x38);

/* multi-texturing (inferred) */
RwBool fn_804A8E90(void); /* attaches it */
/* registers a platform's material plugin with platform data of size bytes */
RwBool fn_804A8EFC(RwPlatformID platformID, RwUInt32 pluginID, RwUInt32 size);
RpMultiTexture *fn_804A8FCC(RpMultiTexture *multiTexture, RpMTEffect *effect); /* sets the effect */
RpMTEffect *fn_804A9028(const RpMultiTexture *multiTexture);                   /* the effect */
RpMultiTexture *fn_804A9030(RpMultiTexture *multiTexture, RwUInt32 index, RwTexture *texture);
RwTexture *fn_804A90A0(const RpMultiTexture *multiTexture, RwUInt32 index);
RpMultiTexture *fn_804A90B0(RpMultiTexture *multiTexture, RwUInt32 index, RwUInt32 texCoordIndex);
RwUInt32 fn_804A90BC(const RpMultiTexture *multiTexture, RwUInt32 index); /* the coordinate set */
/* a material's texture set for a platform, or NULL */
RpMultiTexture *fn_804A90C8(const RpMaterial *material, RwPlatformID platformID);

#ifdef __cplusplus
}
#endif

#endif

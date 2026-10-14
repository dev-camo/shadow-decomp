/* RenderWare GameCube lights: loading the world's directional and ambient lights and an object's
 * local lights into the GX hardware lights, and the light plugin that holds a light's GX distance
 * attenuation. The original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>

/* the light plugin ID (inferred) */
#define rwID_GAMECUBELIGHTPLUGIN 0x505

/* the GX hardware lights */
#define rwGAMECUBEMAXLIGHTS 8

/* The light plugin data: GX attenuation coefficients that replace the default attenuation when
 * enabled; nothing in the game enables them (inferred) */
typedef struct LightAttenuation {
    /* 0x00 */ RwUInt32 enabled; /* nonzero: use these coefficients */
    /* 0x04 */ RwReal a0;        /* angle attenuation */
    /* 0x08 */ RwReal a1;
    /* 0x0C */ RwReal a2;
    /* 0x10 */ RwReal k0; /* distance attenuation */
    /* 0x14 */ RwReal k1;
    /* 0x18 */ RwReal k2;
} LightAttenuation;

/* The light plugin module (8 bytes); only the plugin offset is used */
typedef struct LightModule {
    /* 0x0 */ RwInt32 attenuationOffset; /* light plugin offset of the attenuation */
    /* 0x4 */ RwInt32 field_0x4;
} LightModule;

#define LIGHTATTENUATION(light)                                                                    \
    RWPLUGINOFFSET(LightAttenuation, light, lbl_805F1530.attenuationOffset)

/* The light of a link in the world's directional light list */
#define LIGHTFROMWORLDLINK(link) ((RpLight *)((RwUInt8 *)(link) - offsetof(RpLight, inWorld)))

GXLightObj lbl_805E16A0[rwGAMECUBEMAXLIGHTS];

LightModule lbl_805F1530;

/* Sets a hardware light's colour from a light's */
static inline void LightInitColor(GXLightObj *lightObj, const RpLight *light) {
    GXColor color;

    color.r = 255.0f * light->color.red;
    color.g = 255.0f * light->color.green;
    color.b = 255.0f * light->color.blue;
    color.a = 0;
    GXInitLightColor(lightObj, color);
}

/* Loads a directional light into a hardware light */
static inline void LightLoadDirectional(const RpLight *light, RwInt32 index) {
    RwMatrix *ltm = RwFrameGetLTM(light->object.parent);
    LightAttenuation *attenuation;
    GXLightObj *lightObj;
    RwV3d direction;

    fn_80482504(&direction, &ltm->at, &lbl_805E31CC);
    attenuation = LIGHTATTENUATION(light);
    if (!attenuation->enabled) {
        GXInitLightAttn(&lbl_805E16A0[index], 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    } else {
        GXInitLightAttn(&lbl_805E16A0[index], attenuation->a0, attenuation->a1, attenuation->a2,
                        attenuation->k0, attenuation->k1, attenuation->k2);
    }

    /* a directional light is a point light far away against its direction */
    lightObj = &lbl_805E16A0[index];
    GXInitLightPos(lightObj, -1048576.0f * -direction.x, -1048576.0f * direction.y,
                   -1048576.0f * -direction.z);
    LightInitColor(lightObj, light);
    GXLoadLightObjImm(lightObj, 1 << index);
}

/* Loads the world's directional lights with the given flags into the next hardware lights, and
 * adds its ambient lights to the ambient colour. Light directions and positions are transformed by
 * the current camera's view transform. */
void fn_804641CC(RwUInt8 lightFlags, RpGameCubeRenderMesh *renderMesh) {
    RpWorld *world = lbl_805F15B4->curWorld;
    RwLLLink *link = world->directionalLightList.link.next;
    RwLLLink *end = &world->directionalLightList.link;

    while (link != end) {
        RpLight *light = LIGHTFROMWORLDLINK(link);

        if (light != NULL && (light->object.flags & lightFlags)) {
            if (RpLightGetType(light) == rpLIGHTDIRECTIONAL) {
                LightLoadDirectional(light, renderMesh->numLights);
                renderMesh->lightMask |= 1 << renderMesh->numLights;
                renderMesh->numLights++;
            } else {
                renderMesh->ambientColor.red += light->color.red;
                renderMesh->ambientColor.green += light->color.green;
                renderMesh->ambientColor.blue += light->color.blue;
                renderMesh->hasAmbient = TRUE;
            }
        }
        link = link->next;
    }
}

/* Loads a point or spot light into the next hardware light, while one is free */
void fn_804643E8(RpLight *light, RpGameCubeRenderMesh *renderMesh) {
    if (renderMesh->numLights < rwGAMECUBEMAXLIGHTS) {
        GXColor color;
        RwMatrix *ltm;
        RwV3d direction;
        RwV3d position;
        LightAttenuation *attenuation;

        color.r = 255.0f * light->color.red;
        color.g = 255.0f * light->color.green;
        color.b = 255.0f * light->color.blue;
        color.a = 0;
        GXInitLightColor(&lbl_805E16A0[renderMesh->numLights], color);

        ltm = RwFrameGetLTM(light->object.parent);
        fn_80482458(&position, &ltm->pos, &lbl_805E31CC);
        GXInitLightPos(&lbl_805E16A0[renderMesh->numLights], -position.x, position.y, -position.z);

        attenuation = LIGHTATTENUATION(light);
        switch (light->object.subType) {
        case rpLIGHTDIRECTIONAL:
        case rpLIGHTAMBIENT:
            break;
        case rpLIGHTPOINT:
            if (!attenuation->enabled) {
                GXInitLightAttnA(&lbl_805E16A0[renderMesh->numLights], 1.0f, 0.0f, 0.0f);
                GXInitLightDistAttn(&lbl_805E16A0[renderMesh->numLights], 0.5f * light->radius,
                                    0.5f, GX_DA_MEDIUM);
            } else {
                GXInitLightAttn(&lbl_805E16A0[renderMesh->numLights], attenuation->a0,
                                attenuation->a1, attenuation->a2, attenuation->k0, attenuation->k1,
                                attenuation->k2);
            }
            break;
        case rpLIGHTSPOT:
            fn_80482504(&direction, &ltm->at, &lbl_805E31CC);
            GXInitLightDir(&lbl_805E16A0[renderMesh->numLights], -direction.x, direction.y,
                           -direction.z);
            if (!attenuation->enabled) {
                GXInitLightSpot(&lbl_805E16A0[renderMesh->numLights],
                                57.29578f * fn_8045A818(light), GX_SP_FLAT);
                GXInitLightDistAttn(&lbl_805E16A0[renderMesh->numLights], 0.5f * light->radius,
                                    0.5f, GX_DA_MEDIUM);
            } else {
                GXInitLightAttn(&lbl_805E16A0[renderMesh->numLights], attenuation->a0,
                                attenuation->a1, attenuation->a2, attenuation->k0, attenuation->k1,
                                attenuation->k2);
            }
            break;
        case rpLIGHTSPOTSOFT:
            fn_80482504(&direction, &ltm->at, &lbl_805E31CC);
            GXInitLightDir(&lbl_805E16A0[renderMesh->numLights], -direction.x, direction.y,
                           -direction.z);
            if (!attenuation->enabled) {
                GXInitLightSpot(&lbl_805E16A0[renderMesh->numLights],
                                57.29578f * fn_8045A818(light), GX_SP_COS);
                GXInitLightDistAttn(&lbl_805E16A0[renderMesh->numLights], 0.5f * light->radius,
                                    0.5f, GX_DA_MEDIUM);
            } else {
                GXInitLightAttn(&lbl_805E16A0[renderMesh->numLights], attenuation->a0,
                                attenuation->a1, attenuation->a2, attenuation->k0, attenuation->k1,
                                attenuation->k2);
            }
            break;
        }

        GXLoadLightObjImm(&lbl_805E16A0[renderMesh->numLights], 1 << renderMesh->numLights);
        renderMesh->lightMask |= 1 << renderMesh->numLights;
        renderMesh->numLights++;
    }
}

/* light plugin constructor, destructor and copy */
void *fn_804647B0(void *object, RwInt32 offset, RwInt32 size) {
    LIGHTATTENUATION(object)->enabled = FALSE;
    return object;
}

void *fn_804647C0(void *object, RwInt32 offset, RwInt32 size) {
    return object;
}

void *fn_804647C4(void *dstObject, const void *srcObject, RwInt32 offset, RwInt32 size) {
    return dstObject;
}

/* Attaches the light plugin: TRUE on success */
RwBool fn_804647C8(void) {
    lbl_805F1530.attenuationOffset = fn_8045AAA0(sizeof(LightAttenuation), rwID_GAMECUBELIGHTPLUGIN,
                                                 fn_804647B0, fn_804647C0, fn_804647C4);
    return lbl_805F1530.attenuationOffset >= 0;
}

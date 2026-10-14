/* RenderWare GameCube vertex stream writers: writing positions, normals (also with binormals and
 * tangents, and from compressed normals), prelit colours and texture coordinates into instanced
 * data in a GX component type at a stride, returning the size written. A stride of 0 means the
 * component size. The original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>
#include <math.h>

/* the fixed point scales of normals */
#define NORMALSCALES8 64.0f
#define NORMALSCALES16 16384.0f

/* a compressed normal component as a real */
#define VERTEXNORMALCOMPONENT(component) ((1.0f / 128.0f) * (component))

/* Writes positions relative to the origin truncated toward zero to a multiple of the fixed point
 * step, scaled to the type's fixed point; floats are written unchanged */
RwUInt32 fn_80466B58(void *data, RwV3d *positions, RwInt32 type, RwInt32 numVertices,
                     RwUInt32 stride, const RwV3d *origin, RwReal scale) {
    RwUInt32 size = 0;
    void *destination;
    RwReal originX;
    RwReal originY;
    RwReal originZ;
    RwInt32 i;

    if (origin != NULL) {
        RwReal step = 1.0f / scale;

        originX = origin->x - (RwReal)fmod(origin->x, step);
        originY = origin->y - (RwReal)fmod(origin->y, step);
        originZ = origin->z - (RwReal)fmod(origin->z, step);
    } else {
        originX = 0.0f;
        originY = originX;
        originZ = originX;
    }

    switch (type) {
    case GX_U8:
        if (stride == 0) {
            stride = 3 * sizeof(RwUInt8);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwUInt8 *)destination)[0] = scale * (positions->x - originX);
            ((RwUInt8 *)destination)[1] = scale * (positions->y - originY);
            ((RwUInt8 *)destination)[2] = scale * (positions->z - originZ);
            positions++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 3;
        break;
    case GX_S8:
        if (stride == 0) {
            stride = 3 * sizeof(RwInt8);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwInt8 *)destination)[0] = scale * (positions->x - originX);
            ((RwInt8 *)destination)[1] = scale * (positions->y - originY);
            ((RwInt8 *)destination)[2] = scale * (positions->z - originZ);
            positions++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 3;
        break;
    case GX_U16:
        if (stride == 0) {
            stride = 3 * sizeof(RwUInt16);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwUInt16 *)destination)[0] = scale * (positions->x - originX);
            ((RwUInt16 *)destination)[1] = scale * (positions->y - originY);
            ((RwUInt16 *)destination)[2] = scale * (positions->z - originZ);
            positions++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 6;
        break;
    case GX_S16:
        if (stride == 0) {
            stride = 3 * sizeof(RwInt16);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwInt16 *)destination)[0] = scale * (positions->x - originX);
            ((RwInt16 *)destination)[1] = scale * (positions->y - originY);
            ((RwInt16 *)destination)[2] = scale * (positions->z - originZ);
            positions++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 6;
        break;
    case GX_F32:
        if (stride == 0) {
            stride = sizeof(RwV3d);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            *(RwV3d *)destination = *positions;
            positions++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 12;
        break;
    }
    return size;
}

/* Writes normals in a signed fixed point type, or as floats */
RwUInt32 fn_80467BC8(void *data, RwV3d *normals, RwInt32 type, RwInt32 numVertices,
                     RwUInt32 stride) {
    RwUInt32 size = 0;
    void *destination;
    RwInt32 i;

    switch (type) {
    case GX_U8:
    case GX_U16:
        break;
    case GX_S8:
        if (stride == 0) {
            stride = 3 * sizeof(RwInt8);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwInt8 *)destination)[0] = normals->x * NORMALSCALES8;
            ((RwInt8 *)destination)[1] = normals->y * NORMALSCALES8;
            ((RwInt8 *)destination)[2] = normals->z * NORMALSCALES8;
            normals++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 3;
        break;
    case GX_S16:
        if (stride == 0) {
            stride = 3 * sizeof(RwInt16);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwInt16 *)destination)[0] = normals->x * NORMALSCALES16;
            ((RwInt16 *)destination)[1] = normals->y * NORMALSCALES16;
            ((RwInt16 *)destination)[2] = normals->z * NORMALSCALES16;
            normals++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 6;
        break;
    case GX_F32:
        if (stride == 0) {
            stride = sizeof(RwV3d);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            *(RwV3d *)destination = *normals;
            normals++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 12;
        break;
    }
    return size;
}

/* Writes compressed normals in a signed fixed point type, or as floats */
RwUInt32 fn_804683AC(void *data, RpVertexNormal *normals, RwInt32 type, RwInt32 numVertices,
                     RwUInt32 stride) {
    RwUInt32 size = 0;
    void *destination;
    RwInt32 i;

    switch (type) {
    case GX_U8:
    case GX_U16:
        break;
    case GX_S8:
        if (stride == 0) {
            stride = 3 * sizeof(RwInt8);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            RwV3d normal;

            normal.x = VERTEXNORMALCOMPONENT(normals->x);
            normal.y = VERTEXNORMALCOMPONENT(normals->y);
            normal.z = VERTEXNORMALCOMPONENT(normals->z);
            normals++;
            normal.x *= NORMALSCALES8;
            normal.y *= NORMALSCALES8;
            normal.z *= NORMALSCALES8;
            ((RwInt8 *)destination)[0] = normal.x;
            ((RwInt8 *)destination)[1] = normal.y;
            ((RwInt8 *)destination)[2] = normal.z;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 3;
        break;
    case GX_S16:
        if (stride == 0) {
            stride = 3 * sizeof(RwInt16);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            RwV3d normal;

            normal.x = VERTEXNORMALCOMPONENT(normals->x);
            normal.y = VERTEXNORMALCOMPONENT(normals->y);
            normal.z = VERTEXNORMALCOMPONENT(normals->z);
            normals++;
            normal.x *= NORMALSCALES16;
            normal.y *= NORMALSCALES16;
            normal.z *= NORMALSCALES16;
            ((RwInt16 *)destination)[0] = normal.x;
            ((RwInt16 *)destination)[1] = normal.y;
            ((RwInt16 *)destination)[2] = normal.z;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 6;
        break;
    case GX_F32:
        if (stride == 0) {
            stride = sizeof(RwV3d);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            RwV3d normal;

            normal.x = VERTEXNORMALCOMPONENT(normals->x);
            normal.y = VERTEXNORMALCOMPONENT(normals->y);
            normal.z = VERTEXNORMALCOMPONENT(normals->z);
            normals++;
            *(RwV3d *)destination = normal;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 12;
        break;
    }
    return size;
}

/* Writes normals for a format with binormals and tangents; the sizes returned count all three
 * vectors only for floats */
RwUInt32 fn_80468ADC(void *data, RwV3d *normals, RwInt32 type, RwInt32 numVertices,
                     RwUInt32 stride) {
    RwUInt32 size = 0;
    void *destination;
    RwInt32 i;

    switch (type) {
    case GX_U8:
    case GX_U16:
        break;
    case GX_S8:
        if (stride == 0) {
            stride = 3 * sizeof(RwInt8);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwInt8 *)destination)[0] = normals->x * NORMALSCALES8;
            ((RwInt8 *)destination)[1] = normals->y * NORMALSCALES8;
            ((RwInt8 *)destination)[2] = normals->z * NORMALSCALES8;
            normals++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 3;
        break;
    case GX_S16:
        if (stride == 0) {
            stride = 3 * sizeof(RwInt16);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwInt16 *)destination)[0] = normals->x * NORMALSCALES16;
            ((RwInt16 *)destination)[1] = normals->y * NORMALSCALES16;
            ((RwInt16 *)destination)[2] = normals->z * NORMALSCALES16;
            normals++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 6;
        break;
    case GX_F32:
        if (stride == 0) {
            stride = sizeof(RwV3d);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            *(RwV3d *)destination = *normals;
            normals++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 36;
        break;
    }
    return size;
}

/* Writes compressed normals for a format with binormals and tangents */
RwUInt32 fn_804692C0(void *data, RpVertexNormal *normals, RwInt32 type, RwInt32 numVertices,
                     RwUInt32 stride) {
    RwUInt32 size = 0;
    void *destination;
    RwInt32 i;

    switch (type) {
    case GX_U8:
    case GX_U16:
        break;
    case GX_S8:
        if (stride == 0) {
            stride = 3 * sizeof(RwInt8);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            RwV3d normal;

            normal.x = VERTEXNORMALCOMPONENT(normals->x);
            normal.y = VERTEXNORMALCOMPONENT(normals->y);
            normal.z = VERTEXNORMALCOMPONENT(normals->z);
            normals++;
            normal.x *= NORMALSCALES8;
            normal.y *= NORMALSCALES8;
            normal.z *= NORMALSCALES8;
            ((RwInt8 *)destination)[0] = normal.x;
            ((RwInt8 *)destination)[1] = normal.y;
            ((RwInt8 *)destination)[2] = normal.z;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 3;
        break;
    case GX_S16:
        if (stride == 0) {
            stride = 3 * sizeof(RwInt16);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            RwV3d normal;

            normal.x = VERTEXNORMALCOMPONENT(normals->x);
            normal.y = VERTEXNORMALCOMPONENT(normals->y);
            normal.z = VERTEXNORMALCOMPONENT(normals->z);
            normals++;
            normal.x *= NORMALSCALES16;
            normal.y *= NORMALSCALES16;
            normal.z *= NORMALSCALES16;
            ((RwInt16 *)destination)[0] = normal.x;
            ((RwInt16 *)destination)[1] = normal.y;
            ((RwInt16 *)destination)[2] = normal.z;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 6;
        break;
    case GX_F32:
        if (stride == 0) {
            stride = sizeof(RwV3d);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            RwV3d normal;

            normal.x = VERTEXNORMALCOMPONENT(normals->x);
            normal.y = VERTEXNORMALCOMPONENT(normals->y);
            normal.z = VERTEXNORMALCOMPONENT(normals->z);
            normals++;
            *(RwV3d *)destination = normal;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 36;
        break;
    }
    return size;
}

/* Writes prelit colours in a GX colour type */
RwUInt32 fn_804699F0(void *data, RwRGBA *colors, RwInt32 type, RwInt32 numVertices,
                     RwUInt32 stride) {
    RwUInt32 size = 0;
    void *destination;
    RwInt32 i;

    switch (type) {
    case GX_RGB565:
        if (stride == 0) {
            stride = sizeof(RwUInt16);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            *(RwUInt16 *)destination = (((RwUInt32)colors->red >> 3) << 11) |
                                       (((RwUInt32)colors->green & 0xFC) << 3) |
                                       ((RwUInt32)colors->blue >> 3);
            colors++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 2;
        break;
    case GX_RGB8:
        if (stride == 0) {
            stride = 3;
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwUInt8 *)destination)[0] = colors->red;
            ((RwUInt8 *)destination)[1] = colors->green;
            ((RwUInt8 *)destination)[2] = colors->blue;
            colors++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 3;
        break;
    case GX_RGBA4:
        if (stride == 0) {
            stride = sizeof(RwUInt16);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            *(RwUInt16 *)destination =
                (((RwUInt32)colors->red & 0xF0) << 8) | (((RwUInt32)colors->green & 0xF0) << 4) |
                ((RwUInt32)colors->blue & 0xF0) | ((RwUInt32)colors->alpha >> 4);
            colors++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 2;
        break;
    case GX_RGBA6:
        if (stride == 0) {
            stride = 3;
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwUInt8 *)destination)[0] = (colors->green >> 6) | (colors->red & 0xFC);
            ((RwUInt8 *)destination)[1] = (colors->blue >> 4) | ((colors->green << 2) & 0xF0);
            ((RwUInt8 *)destination)[2] = (colors->alpha >> 2) | ((colors->blue << 4) & 0xC0);
            colors++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 3;
        break;
    case GX_RGBX8:
    case GX_RGBA8:
        if (stride == 0) {
            stride = sizeof(RwRGBA);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            *(RwRGBA *)destination = *colors;
            colors++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 4;
        break;
    }
    return size;
}

/* Writes texture coordinates scaled to the type's fixed point; floats are written unchanged */
RwUInt32 fn_8046A270(void *data, RwTexCoords *texCoords, RwInt32 type, RwInt32 numVertices,
                     RwUInt32 stride, RwReal scale) {
    RwUInt32 size = 0;
    void *destination;
    RwInt32 i;

    switch (type) {
    case GX_U8:
        if (stride == 0) {
            stride = 2 * sizeof(RwUInt8);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwUInt8 *)destination)[0] = texCoords->u * scale;
            ((RwUInt8 *)destination)[1] = texCoords->v * scale;
            texCoords++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 2;
        break;
    case GX_S8:
        if (stride == 0) {
            stride = 2 * sizeof(RwInt8);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwInt8 *)destination)[0] = texCoords->u * scale;
            ((RwInt8 *)destination)[1] = texCoords->v * scale;
            texCoords++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 2;
        break;
    case GX_U16:
        if (stride == 0) {
            stride = 2 * sizeof(RwUInt16);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwUInt16 *)destination)[0] = texCoords->u * scale;
            ((RwUInt16 *)destination)[1] = texCoords->v * scale;
            texCoords++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 4;
        break;
    case GX_S16:
        if (stride == 0) {
            stride = 2 * sizeof(RwInt16);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            ((RwInt16 *)destination)[0] = texCoords->u * scale;
            ((RwInt16 *)destination)[1] = texCoords->v * scale;
            texCoords++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 4;
        break;
    case GX_F32:
        if (stride == 0) {
            stride = sizeof(RwTexCoords);
        }
        destination = data;
        for (i = 0; i < numVertices; i++) {
            *(RwTexCoords *)destination = *texCoords;
            texCoords++;
            destination = (RwUInt8 *)destination + stride;
        }
        size = numVertices * 8;
        break;
    }
    return size;
}

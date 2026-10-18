/* RenderWare GameCube vertex arrays: the size of the instanced data header, and the indexed
 * attribute arrays of a geometry: their size, their vertex descriptors and their data. The
 * original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>

/* Whether an attribute is sent as an index into an array */
#define INDEXED(type) ((type) == GX_INDEX8 || (type) == GX_INDEX16)

/* The bytes of an attribute array, rounded up to 32 bytes */
#define ARRAYBYTES(bytes) (((bytes) + 31) & ~31)

/* The size of the instanced data header with a vertex descriptor per builder attribute */
RwUInt32 fn_8046EE8C(RpGameCubeVertexDescBuilder *builder) {
    return sizeof(RpGameCubeInstanceData) +
           (builder->numAttributes - 1) * sizeof(RpGameCubeVertexDescriptor);
}

/* The bytes of texture coordinate set n's array, whose type is at bit 2 * n of the VCD high
 * register */
#define TEXCOORDARRAYBYTES(size, n)                                                                \
    do {                                                                                           \
        RwUInt32 type = (builder->vcd[1] >> (2 * (n))) & 3;                                        \
                                                                                                   \
        if (INDEXED(type)) {                                                                       \
            (size) += ARRAYBYTES(source->numElements[attribute] *                                  \
                                 fn_80466A54(builder, attribute - GX_VA_TEX0));                    \
        }                                                                                          \
    } while (0)

/* The bytes of the arrays of the indexed attributes, each rounded up to 32 bytes */
RwUInt32 fn_8046EEA0(RpGameCubeVertexDescBuilder *builder, RpGameCubeVertexSource *source) {
    RwUInt32 attribute;
    RwUInt32 size = 0;

    for (attribute = GX_VA_POS; attribute < GX_VA_TEX7 + 1; attribute++) {
        switch (attribute) {
        case GX_VA_POS: {
            RwUInt32 type = (builder->vcd[0] >> 9) & 3;

            if (INDEXED(type)) {
                size += ARRAYBYTES(source->numElements[attribute] * fn_8046693C(builder));
            }
            break;
        }
        case GX_VA_NRM: {
            RwUInt32 type = (builder->vcd[0] >> 11) & 3;

            if (INDEXED(type)) {
                if (((builder->vat[0] >> 9) & 1) == GX_NRM_NBT) {
                    /* normals, binormals and tangents */
                    size += ARRAYBYTES(source->numElements[GX_VA_NRM] * fn_804669A0(builder) * 3);
                } else {
                    size += ARRAYBYTES(source->numElements[GX_VA_NRM] * fn_804669A0(builder));
                }
            }
            break;
        }
        case GX_VA_CLR0:
        case GX_VA_CLR1: {
            RwInt32 channel = attribute - GX_VA_CLR0;
            RwUInt32 type = (builder->vcd[0] >> (channel * 2 + 13)) & 3;

            if (INDEXED(type)) {
                size += ARRAYBYTES(source->numElements[attribute] *
                                   fn_804669EC(builder, attribute - GX_VA_CLR0));
            }
            break;
        }
        case GX_VA_TEX0:
            TEXCOORDARRAYBYTES(size, 0);
            break;
        case GX_VA_TEX1:
            TEXCOORDARRAYBYTES(size, 1);
            break;
        case GX_VA_TEX2:
            TEXCOORDARRAYBYTES(size, 2);
            break;
        case GX_VA_TEX3:
            TEXCOORDARRAYBYTES(size, 3);
            break;
        case GX_VA_TEX4:
            TEXCOORDARRAYBYTES(size, 4);
            break;
        case GX_VA_TEX5:
            TEXCOORDARRAYBYTES(size, 5);
            break;
        case GX_VA_TEX6:
            TEXCOORDARRAYBYTES(size, 6);
            break;
        case GX_VA_TEX7:
            TEXCOORDARRAYBYTES(size, 7);
            break;
        }
    }
    return size;
}

/* Sets the next vertex descriptor to an attribute array of elements of the given size */
#define SETDESCRIPTOR(elementSize, type)                                                           \
    do {                                                                                           \
        instance->vertexDescriptors[numDescriptors].data = data;                                   \
        instance->vertexDescriptors[numDescriptors].attribute = attribute;                         \
        instance->vertexDescriptors[numDescriptors].stride = (elementSize);                        \
        instance->vertexDescriptors[numDescriptors].type = (type);                                 \
    } while (0)

/* Sets the vertex descriptor of texture coordinate set n's array */
#define TEXCOORDDESCRIPTOR(n)                                                                      \
    do {                                                                                           \
        RwUInt32 type = (builder->vcd[1] >> (2 * (n))) & 3;                                        \
                                                                                                   \
        if (INDEXED(type)) {                                                                       \
            RwUInt32 elementSize = fn_80466A54(builder, attribute - GX_VA_TEX0);                   \
                                                                                                   \
            SETDESCRIPTOR(elementSize, type);                                                      \
            numDescriptors++;                                                                      \
            offset += ARRAYBYTES(elementSize * source->numElements[attribute]);                    \
        }                                                                                          \
    } while (0)

/* Lays out the arrays of the indexed attributes in the vertex data, each rounded up to 32 bytes,
 * and sets a vertex descriptor for each */
void fn_8046F208(RpGameCubeVertexDescBuilder *builder, RpGameCubeInstanceData *instance,
                 RpGameCubeVertexSource *source, void *vertexData) {
    RwInt32 numDescriptors = 0;
    RwUInt32 offset = 0;
    RwUInt32 attribute;

    for (attribute = GX_VA_POS; attribute < GX_VA_TEX7 + 1; attribute++) {
        void *data = (RwUInt8 *)vertexData + offset;

        switch (attribute) {
        case GX_VA_POS: {
            RwUInt32 type = (builder->vcd[0] >> 9) & 3;

            if (INDEXED(type)) {
                RwUInt32 elementSize = fn_8046693C(builder);

                SETDESCRIPTOR(elementSize, type);
                numDescriptors++;
                offset += ARRAYBYTES(elementSize * source->numElements[attribute]);
            }
            break;
        }
        case GX_VA_NRM: {
            RwUInt32 type = (builder->vcd[0] >> 11) & 3;

            if (INDEXED(type)) {
                if (((builder->vat[0] >> 9) & 1) == GX_NRM_NBT) {
                    /* normals, binormals and tangents */
                    RwUInt32 elementSize = fn_804669A0(builder) * 3;

                    SETDESCRIPTOR(elementSize, type);
                    offset += ARRAYBYTES(elementSize * source->numElements[GX_VA_NRM]);
                } else {
                    RwUInt32 elementSize = fn_804669A0(builder);

                    SETDESCRIPTOR(elementSize, type);
                    offset += ARRAYBYTES(elementSize * source->numElements[GX_VA_NRM]);
                }
                numDescriptors++;
            }
            break;
        }
        case GX_VA_CLR0:
        case GX_VA_CLR1: {
            RwInt32 channel = attribute - GX_VA_CLR0;
            RwUInt32 type = (builder->vcd[0] >> (channel * 2 + 13)) & 3;

            if (INDEXED(type)) {
                RwUInt32 elementSize = fn_804669EC(builder, attribute - GX_VA_CLR0);

                SETDESCRIPTOR(elementSize, type);
                numDescriptors++;
                offset += ARRAYBYTES(elementSize * source->numElements[attribute]);
            }
            break;
        }
        case GX_VA_TEX0:
            TEXCOORDDESCRIPTOR(0);
            break;
        case GX_VA_TEX1:
            TEXCOORDDESCRIPTOR(1);
            break;
        case GX_VA_TEX2:
            TEXCOORDDESCRIPTOR(2);
            break;
        case GX_VA_TEX3:
            TEXCOORDDESCRIPTOR(3);
            break;
        case GX_VA_TEX4:
            TEXCOORDDESCRIPTOR(4);
            break;
        case GX_VA_TEX5:
            TEXCOORDDESCRIPTOR(5);
            break;
        case GX_VA_TEX6:
            TEXCOORDDESCRIPTOR(6);
            break;
        case GX_VA_TEX7:
            TEXCOORDDESCRIPTOR(7);
            break;
        }
    }
    instance->numVertexDescriptors = numDescriptors;
}

/* Writes texture coordinate set n's array with its component type and fixed point scale. The
 * component type is held in a byte, as the vertex format stores the component types
 * (texCoordType); with a plain int the setup of the sets compiles to a different instruction order. */
#define WRITETEXCOORDS(n, compType, scale)                                                         \
    do {                                                                                           \
        RwUInt32 type = (builder->vcd[1] >> (2 * (n))) & 3;                                        \
                                                                                                   \
        if (INDEXED(type)) {                                                                       \
            RwUInt8 componentType = (compType);                                                    \
                                                                                                   \
            fn_8046A270(instance->vertexDescriptors[numDescriptors].data, source->data[attribute], \
                        componentType, source->numElements[attribute],                             \
                        instance->vertexDescriptors[numDescriptors].stride, (scale));              \
            numDescriptors++;                                                                      \
        }                                                                                          \
    } while (0)

/* Writes the arrays of the indexed attributes through their vertex descriptors; positions are
 * made relative to the origin */
void fn_8046F6CC(RpGameCubeVertexDescBuilder *builder, RpGameCubeInstanceData *instance,
                 RpGameCubeVertexSource *source, RwBool compressedNormals, const RwV3d *origin) {
    RwInt32 numDescriptors = 0;
    RwUInt32 attribute;

    for (attribute = GX_VA_POS; attribute < GX_VA_TEX7 + 1; attribute++) {
        switch (attribute) {
        case GX_VA_POS: {
            RwUInt32 type = (builder->vcd[0] >> 9) & 3;

            if (INDEXED(type)) {
                RwUInt32 vat = builder->vat[0];

                if ((vat & 1) == GX_POS_XYZ) {
                    fn_80466B58(instance->vertexDescriptors[numDescriptors].data,
                                source->data[attribute], (vat >> 1) & 7,
                                source->numElements[attribute],
                                instance->vertexDescriptors[numDescriptors].stride, origin,
                                1 << ((vat >> 4) & 0x1F));
                    numDescriptors++;
                }
            }
            break;
        }
        case GX_VA_NRM: {
            RwUInt32 type = (builder->vcd[0] >> 11) & 3;

            if (INDEXED(type)) {
                RwUInt32 vat = builder->vat[0];
                RwInt32 compType = (vat >> 10) & 7;
                if (((vat >> 9) & 1) == GX_NRM_NBT) {
                    if (!compressedNormals) {
                        fn_80468ADC(instance->vertexDescriptors[numDescriptors].data,
                                    source->data[attribute], compType,
                                    source->numElements[attribute],
                                    instance->vertexDescriptors[numDescriptors].stride);
                    } else {
                        fn_804692C0(instance->vertexDescriptors[numDescriptors].data,
                                    source->data[attribute], compType,
                                    source->numElements[attribute],
                                    instance->vertexDescriptors[numDescriptors].stride);
                    }
                } else if (!compressedNormals) {
                    fn_80467BC8(instance->vertexDescriptors[numDescriptors].data,
                                source->data[attribute], compType, source->numElements[attribute],
                                instance->vertexDescriptors[numDescriptors].stride);
                } else {
                    fn_804683AC(instance->vertexDescriptors[numDescriptors].data,
                                source->data[attribute], compType, source->numElements[attribute],
                                instance->vertexDescriptors[numDescriptors].stride);
                }
                numDescriptors++;
            }
            break;
        }
        case GX_VA_CLR0:
        case GX_VA_CLR1: {
            RwInt32 channel = attribute - GX_VA_CLR0;
            RwUInt32 type = (builder->vcd[0] >> (channel * 2 + 13)) & 3;

            if (INDEXED(type)) {
                fn_804699F0(instance->vertexDescriptors[numDescriptors].data,
                            source->data[attribute],
                            (builder->vat[0] >> ((attribute - GX_VA_CLR0) * 4 + 14)) & 7,
                            source->numElements[attribute],
                            instance->vertexDescriptors[numDescriptors].stride);
                numDescriptors++;
            }
            break;
        }
        /* Except for sets 0 and 4 to 7 the original masks the scale instead of the fraction
         * field, (1 << (vat >> shift)) & 0x1F (sic). */
        case GX_VA_TEX0:
            WRITETEXCOORDS(0, (builder->vat[0] >> 22) & 7, 1 << ((builder->vat[0] >> 25) & 0x1F));
            break;
        case GX_VA_TEX1:
            WRITETEXCOORDS(1, (builder->vat[1] >> 1) & 7, (1 << (builder->vat[1] >> 4)) & 0x1F);
            break;
        case GX_VA_TEX2:
            WRITETEXCOORDS(2, (builder->vat[1] >> 10) & 7, (1 << (builder->vat[1] >> 13)) & 0x1F);
            break;
        case GX_VA_TEX3:
            WRITETEXCOORDS(3, (builder->vat[1] >> 19) & 7, (1 << (builder->vat[1] >> 22)) & 0x1F);
            break;
        case GX_VA_TEX4:
            WRITETEXCOORDS(4, (builder->vat[1] >> 28) & 7, 1 << (builder->vat[2] & 0x1F));
            break;
        case GX_VA_TEX5:
            WRITETEXCOORDS(5, (builder->vat[2] >> 6) & 7, 1 << ((builder->vat[2] >> 9) & 0x1F));
            break;
        case GX_VA_TEX6:
            WRITETEXCOORDS(6, (builder->vat[2] >> 15) & 7, 1 << ((builder->vat[2] >> 18) & 0x1F));
            break;
        case GX_VA_TEX7:
            WRITETEXCOORDS(7, (builder->vat[2] >> 24) & 7, 1 << ((builder->vat[2] >> 27) & 0x1F));
            break;
        }
    }
}

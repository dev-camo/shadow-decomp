/* RenderWare GameCube vertex descriptor builder: keeps a vertex attribute format and descriptor
 * in the layouts of the GX VAT and VCD registers, with the attribute counts of the XF vertex
 * specification. The original file name is unknown. */
#include "rw/rpworld.h"
#include <dolphin/gx.h>

/* Replaces the bits of a register word under a mask with a value */
static inline void SetField(RwUInt32 *word, RwUInt32 mask, RwUInt32 value) {
    *word &= ~mask;
    *word |= mask & value;
}

/* Clears a builder: the VAT A and B words start with the GX byte dequantization and vertex cache
 * enhancement bits set */
void fn_8046FC1C(RpGameCubeVertexDescBuilder *builder) {
    builder->vertexFormat = 0;
    builder->vat[0] = 0x40000000;
    builder->vat[1] = 0x80000000;
    builder->vat[2] = 0;
    builder->vcd[0] = 0;
    builder->vcd[1] = 0;
    builder->xfSpecs = 0;
}

/* Sets the GX vertex format the builder is for */
void fn_8046FC48(RpGameCubeVertexDescBuilder *builder, RwInt32 vertexFormat) {
    builder->vertexFormat = vertexFormat;
}

/* Sets an attribute's component count, type and fraction in the VAT words. The original's
 * masks clear texture coordinate set 4's count bit with set 3 and leave it out of set 4 (sic). */
void fn_8046FC50(RpGameCubeVertexDescBuilder *builder, RwInt32 attribute, RwInt32 count,
                 RwInt32 type, RwUInt8 frac) {
    switch (attribute) {
    case GX_VA_POS:
        SetField(&builder->vat[0], 0x1FF, count | (type << 1) | (frac << 4));
        break;
    case GX_VA_NRM:
        SetField(&builder->vat[0], 0x1E00, (count << 9) | (type << 10));
        builder->vat[0] &= ~0x80000000;
        break;
    case GX_VA_NBT:
        SetField(&builder->vat[0], 0x1E00, (count << 9) | (type << 10));
        /* the normal index 3 bit */
        if (count == GX_NRM_NBT3) {
            builder->vat[0] |= 0x80000000;
        } else {
            builder->vat[0] &= ~0x80000000;
        }
        break;
    case GX_VA_CLR0:
        SetField(&builder->vat[0], 0x1E000, (count << 13) | (type << 14));
        break;
    case GX_VA_CLR1:
        SetField(&builder->vat[0], 0x1E0000, (count << 17) | (type << 18));
        break;
    case GX_VA_TEX0:
        SetField(&builder->vat[0], 0x3FE00000, (count << 21) | (type << 22) | (frac << 25));
        break;
    case GX_VA_TEX1:
        SetField(&builder->vat[1], 0x1FF, count | (type << 1) | (frac << 4));
        break;
    case GX_VA_TEX2:
        SetField(&builder->vat[1], 0x3FE00, (count << 9) | (type << 10) | (frac << 13));
        break;
    case GX_VA_TEX3:
        SetField(&builder->vat[1], 0xFFC0000, (count << 18) | (type << 19) | (frac << 22));
        break;
    case GX_VA_TEX4:
        SetField(&builder->vat[1], 0xF0000000, (count << 27) | (type << 28));
        SetField(&builder->vat[2], 0x1F, frac);
        break;
    case GX_VA_TEX5:
        SetField(&builder->vat[2], 0x3FE0, (count << 5) | (type << 6) | (frac << 9));
        break;
    case GX_VA_TEX6:
        SetField(&builder->vat[2], 0x7FC000, (count << 14) | (type << 15) | (frac << 18));
        break;
    case GX_VA_TEX7:
        SetField(&builder->vat[2], 0xFF800000, (count << 23) | (type << 24) | (frac << 27));
        break;
    }
}

/* Sets texture coordinate set n's descriptor mode and counts the sets present */
#define SETTEXCOORDDESC(n)                                                                         \
    do {                                                                                           \
        RwUInt32 i;                                                                                \
        RwUInt32 numTexCoords;                                                                     \
                                                                                                   \
        SetField(&builder->vcd[1], 3 << (2 * (n)), mode << (2 * (n)));                             \
        i = 8;                                                                                     \
        numTexCoords = 0;                                                                          \
        while (i-- != 0) {                                                                         \
            if (builder->vcd[1] & (3 << (i * 2))) {                                                \
                numTexCoords++;                                                                    \
            }                                                                                      \
        }                                                                                          \
        SetField(&builder->xfSpecs, 0xF0, numTexCoords << 4);                                      \
    } while (0)

/* Sets how an attribute is sent (the VCD mode) and updates the XF attribute counts */
void fn_8046FF34(RpGameCubeVertexDescBuilder *builder, RwInt32 attribute, RwInt32 mode) {
    switch (attribute) {
    case GX_VA_PNMTXIDX:
        SetField(&builder->vcd[0], 1, mode);
        break;
    case GX_VA_TEX0MTXIDX:
    case GX_VA_TEX1MTXIDX:
    case GX_VA_TEX2MTXIDX:
    case GX_VA_TEX3MTXIDX:
    case GX_VA_TEX4MTXIDX:
    case GX_VA_TEX5MTXIDX:
    case GX_VA_TEX6MTXIDX:
    case GX_VA_TEX7MTXIDX:
        SetField(&builder->vcd[0], 2 << (attribute - 1), mode << attribute);
        break;
    case GX_VA_POS:
        SetField(&builder->vcd[0], 0x600, mode << 9);
        break;
    case GX_VA_NRM: {
        RwUInt32 numNormals;

        SetField(&builder->vcd[0], 0x1800, mode << 11);
        numNormals = 0;
        if (mode != GX_NONE) {
            numNormals = 1;
        }
        SetField(&builder->xfSpecs, 0xC, numNormals << 2);
        break;
    }
    case GX_VA_NBT: {
        RwUInt32 numNormals;

        SetField(&builder->vcd[0], 0x1800, mode << 11);
        numNormals = 0;
        if (mode != GX_NONE) {
            numNormals = 2;
        }
        SetField(&builder->xfSpecs, 0xC, numNormals << 2);
        break;
    }
    case GX_VA_CLR0:
    case GX_VA_CLR1: {
        RwUInt32 numColors;

        /* (sic) the original's mask and shift for color 1 sit one bit below the bits 15-16 that
         * the count below tests: the mask is 0xC000 and the mode is shifted by 14 */
        SetField(&builder->vcd[0], 0x6000 << (attribute - GX_VA_CLR0), mode << (attribute + 2));
        numColors = 0;
        if (builder->vcd[0] & 0x6000) {
            numColors = 1;
        }
        if (builder->vcd[0] & 0x18000) {
            numColors++;
        }
        SetField(&builder->xfSpecs, 3, numColors);
        break;
    }
    case GX_VA_TEX0: {
        RwUInt32 numTexCoords;
        RwUInt32 i;

        SetField(&builder->vcd[1], 3, mode);
        i = 8;
        numTexCoords = 0;
        while (i-- != 0) {
            if (builder->vcd[1] & (3 << (i * 2))) {
                numTexCoords++;
            }
        }
        SetField(&builder->xfSpecs, 0xF0, numTexCoords << 4);
        break;
    }
    case GX_VA_TEX1:
        SETTEXCOORDDESC(1);
        break;
    case GX_VA_TEX2:
        SETTEXCOORDDESC(2);
        break;
    case GX_VA_TEX3:
        SETTEXCOORDDESC(3);
        break;
    case GX_VA_TEX4:
        SETTEXCOORDDESC(4);
        break;
    case GX_VA_TEX5:
        SETTEXCOORDDESC(5);
        break;
    case GX_VA_TEX6:
        SETTEXCOORDDESC(6);
        break;
    case GX_VA_TEX7:
        SETTEXCOORDDESC(7);
        break;
    }
}

/* Sets the number of attributes, which is the number of vertex descriptors of the instanced
 * data */
void fn_8047046C(RpGameCubeVertexDescBuilder *builder, RwUInt8 numAttributes) {
    builder->numAttributes = numAttributes;
}

/* csdLoader.cpp (the name in the library's allocation calls) reads a project's file data; the rest
 * of the address range is provisionally in this unit. Function, variable and helper names are
 * inferred. Definitions are in reverse address order (deferred inlining emits the last definition
 * first). */
#include "Game/CSD/csdLoader.h"
#include "Game/CSD/CPlatform.h"
#include "Game/CSD/Matrix.h"
#include "Game/CSD/csdBase.h"
#include <cmath>
#include <cstring>

/* Signatures set when the library is initialised: the first word of the project file's header
 * ("NGIF") and the tag of the chunk that ends the list of chunks ("NEND"). */
extern u32 lbl_805859E0;
extern u32 lbl_80585A14;

namespace Chao {
namespace CSD {

/* Functions that are used above their definition. */
static void NormalizeFolder(SceneFolder *folder);
static void NormalizeScene(SceneData *scene);
static void NormalizeLayer(LayerData *layer);
static void SwapBytes(void *data, int size);
static void InterpolateLinear(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame);
static void InterpolateHermite(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame);
static void HoldKeyValue(u32 *out, const Keyframe *first);

static inline u32 SwapU32(u32 value) {
    return (value >> 24) | ((value >> 8) & 0xFF00) | ((value << 8) & 0xFF0000) | (value << 24);
}

void NormalizeProject(ProjectData *project) {
    NormalizeFolder(project->root);
}

static void NormalizeFolder(SceneFolder *folder) {
    for (int i = 0; i < folder->sceneCount; i++) {
        NormalizeScene(folder->scenes[i]);
    }
    for (int i = 0; i < folder->folderCount; i++) {
        NormalizeFolder(&folder->folders[i]);
    }
}

static void NormalizeScene(SceneData *scene) {
    for (int i = 0; i < scene->layerCount; i++) {
        NormalizeLayer(&scene->layers[i]);
    }
}

static void NormalizeLayer(LayerData *layer) {
    ElementData *element;
    int i;

    for (i = 0; i < layer->elementCount; i++) {
        element = layer->elements[i];

        if (!(element->flags & 0xD000)) {
            if (element->flags & 0x1) {
                element->flags |= 0x300;
            }
            if (element->flags & 0x4) {
                element->flags |= 0xC00;
            }
        }
    }
}

void LoadProject(ProjectData **project, void *file) {
    ResolveProject(project, file);
    NormalizeProject(*project);
}

void ResolveProject(ProjectData **project, void *file) {
    const u32 *header;
    int chunkCount;
    const u32 *relocations;
    int relocationCount;
    u32 offset;
    const u32 *entry;
    u8 *bodyStart;
    u8 *body;
    const u32 *chunk;
    u32 size;
    int i;

    if (project == NULL) {
        return;
    }
    /* The second test is redundant in source, but removing it removes one of the original's two
     * branches. */
    if (project != NULL) {
        *project = NULL;
    }

    header = static_cast<const u32 *>(file);
    if (header[0] != lbl_805859E0) {
        return;
    }

    chunkCount = header[2];
    body = (u8 *)file + header[3];
    bodyStart = body;
    relocations = (const u32 *)((u8 *)file + header[5]);

    /* Entries are the offsets, from the body, of words that hold body-relative offsets; adding the
     * body's address makes them pointers. */
    relocationCount = relocations[2];
    entry = relocations + 4;
    for (i = 0; i < relocationCount; i++) {
        offset = *entry++ & ~3;
        *(u32 *)(body + offset) += (u32)body;
    }

    for (i = 0; i < chunkCount; i++) {
        chunk = (const u32 *)body;

        if (chunk[0] == 0x6E43504A /* "nCPJ" */) {
            /* Likewise: dropping this test removes one of the original's two branches. */
            if (project != NULL) {
                *project = (ProjectData *)(bodyStart + chunk[2]);
            }
        } else if (chunk[0] == lbl_80585A14) {
            return;
        }

        /* The chunk sizes are byte-reversed when this platform slot (which the GameCube
         * implementation answers with 1) returns nonzero. */
        if (GetPlatform()->fn_80363BE8()) {
            size = SwapU32(chunk[1]);
        } else {
            size = chunk[1];
        }
        /* Two separate additions: combining them changes the code. */
        body += size;
        body += 8;
    }
}

void ExtractFileParts(void **part1, u32 *size1, void **part2, u32 *size2, const u32 *file,
                      u32 fileSize) {
    if (file[0] == 0x43504146 /* "CPAF" */) {
        const u8 *data = (const u8 *)(file + 1);
        u32 length = *(const u32 *)data;

        data += 4;
        *part1 = Alloc(length, __FILE__, __LINE__);
        *size1 = length;
        memcpy(*part1, data, length);
        data += length;

        length = *(const u32 *)data;
        data += 4;
        *part2 = Alloc(length, __FILE__, __LINE__);
        *size2 = length;
        memcpy(*part2, data, length);
    } else if (file[0] == 0x46415043 /* "FAPC" */) {
        /* The byte-reversed tag: the lengths are byte-reversed too. */
        const u8 *data = (const u8 *)(file + 1);
        u32 length = *(const u32 *)data;

        data += 4;
        SwapBytes(&length, 4);
        *part1 = Alloc(length, __FILE__, __LINE__);
        *size1 = length;
        memcpy(*part1, data, length);
        data += length;

        length = *(const u32 *)data;
        data += 4;
        SwapBytes(&length, 4);
        *part2 = Alloc(length, __FILE__, __LINE__);
        *size2 = length;
        memcpy(*part2, data, length);
    } else {
        *part2 = NULL;
        *part1 = Alloc(fileSize, __FILE__, __LINE__);
        *size1 = fileSize;
        *size2 = 0;
        memcpy(*part1, file, fileSize);
    }
}

/* The point (x, y) transformed by the matrix. */
static void TransformPoint(f32 *out, const Matrix *matrix, const f32 *point) {
    f32 x = point[0];
    f32 y = point[1];
    f32 transformedX = matrix->m[2] + (matrix->m[0] * x + matrix->m[1] * y);
    f32 transformedY = matrix->m[5] + (matrix->m[3] * x + matrix->m[4] * y);

    out[0] = transformedX;
    out[1] = transformedY;
}

static void SwapBytes(void *data, int size) {
    switch (size) {
    case 2:
        __sthbrx(*(u16 *)data, data, 0);
        break;
    case 4:
        __stwbrx(*(u32 *)data, data, 0);
        break;
    }
}

/* Sets the colour of the four corners of every quad of an entry to the entry's corner colours.
 * Entries without textures are read with a 0xC vertex stride (PlainVertex, inferred). */
void SetEntryColors(MotionPaletteEntry *entry) {
    if (entry->textures != NULL) {
        for (int quad = 0; quad < entry->quadCount; quad++) {
            for (int corner = 0; corner < 4; corner++) {
                entry->vertices[quad * 4 + corner].color = entry->colors[corner];
            }
        }
    } else {
        for (int quad = 0; quad < entry->quadCount; quad++) {
            for (int corner = 0; corner < 4; corner++) {
                ((PlainVertex *)entry->vertices)[quad * 4 + corner].color = entry->colors[corner];
            }
        }
    }
}

union ColorBytes {
    u32 packed;
    u8 channel[4];
};
CHECK_SIZEOF(Chao::CSD::ColorBytes, 0x4);

/* Sets the draw colour of every vertex of an entry to the vertex colour with each channel scaled by
 * the same channel of the word at 0x24 of the entry (field_0x24, used as four channel bytes), as a
 * fraction of 255. */
void TintEntryColors(MotionPaletteEntry *entry) {
    ColorBytes tint;

    tint.packed = entry->field_0x24;
    if (entry->textures != NULL) {
        Vertex *vertices = entry->vertices;
        Vertex *drawVertices = entry->drawVertices;
        int vertex = 0;
        for (int quad = 0; quad < entry->quadCount; quad++) {
            for (int corner = 0; corner < 4; corner++) {
                ColorBytes color;
                color.packed = vertices[vertex].color;
                for (int i = 0; i < 4; i++) {
                    color.channel[i] = color.channel[i] * tint.channel[i] / 255;
                }
                drawVertices[vertex].color = color.packed;
                vertex++;
            }
        }
    } else {
        PlainVertex *vertices = (PlainVertex *)entry->vertices;
        PlainVertex *drawVertices = (PlainVertex *)entry->drawVertices;
        int vertex = 0;
        for (int quad = 0; quad < entry->quadCount; quad++) {
            for (int corner = 0; corner < 4; corner++) {
                ColorBytes color;
                color.packed = vertices[vertex].color;
                for (int i = 0; i < 4; i++) {
                    color.channel[i] = color.channel[i] * tint.channel[i] / 255;
                }
                drawVertices[vertex].color = color.packed;
                vertex++;
            }
        }
    }
}

/* The value at frame, interpolated between two keys by the first key's interpolation. */
static void InterpolateKeys(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame) {
    switch (first->interpolation) {
    case KEY_HOLD:
        HoldKeyValue(out, first);
        break;
    case KEY_LINEAR:
        InterpolateLinear(out, first, second, frame);
        break;
    case KEY_HERMITE:
        InterpolateHermite(out, first, second, frame);
        break;
    default:
        HoldKeyValue(out, first);
        break;
    }
}

/* The value of the first key, unchanged. */
static void HoldKeyValue(u32 *out, const Keyframe *first) {
    *out = first->value.bits;
}

/* Interpolates float values linearly. */
static void InterpolateLinear(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame) {
    f32 weight;
    KeyValue result;

    if (second->frame - first->frame <= 0) {
        weight = 0.0f;
    } else {
        weight = (frame - first->frame) / (second->frame - first->frame);
    }
    result.number = first->value.number * (1.0f - weight) + second->value.number * weight;
    *out = result.bits;
}

/* Interpolates float values along a cubic Hermite curve, with the first key's tangents. */
static void InterpolateHermite(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame) {
    f32 weight;
    f32 span;
    f32 delta;
    KeyValue result;

    if (second->frame - first->frame <= 0) {
        weight = 0.0f;
    } else {
        weight = (frame - first->frame) / (second->frame - first->frame);
    }
    span = second->frame - first->frame;
    delta = second->value.number - first->value.number;

    f32 squared = weight * weight;
    f32 cubed = squared * weight;
    f32 quadratic = 3.0f * delta - span * (2.0f * first->startTangent + first->endTangent);
    f32 cubic = -2.0f * delta + span * (first->startTangent + first->endTangent);
    f32 linear = span * first->startTangent;

    result.number = first->value.number + (linear * weight + (cubic * cubed + quadratic * squared));
    *out = result.bits;
}

/* Interpolates colours by channel (four bytes). */
static void InterpolateColor(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame) {
    f32 weight;
    ColorBytes firstColor, secondColor, result;

    firstColor.packed = first->value.bits;
    secondColor.packed = second->value.bits;
    if (second->frame - first->frame <= 0) {
        weight = 0.0f;
    } else {
        weight = (frame - first->frame) / (second->frame - first->frame);
    }
    for (int i = 0; i < 4; i++) {
        result.channel[i] =
            firstColor.channel[i] * (1.0f - weight) + secondColor.channel[i] * weight;
    }
    /* The copy is kept: reading result directly changes the code. */
    ColorBytes blended = result;
    *out = blended.packed;
}

f32 PackPatternBlend(int pattern, int nextPattern, f32 blend) {
    if (pattern > 10000) {
        PrintError("Large PatternIndex Error.\n");
        /* The original hangs here. */
        for (;;) {
        }
    }
    return blend + (pattern + nextPattern * 10000);
}

/* Splits value into integer and fractional parts like modff does (the integral part is stored in
 * *integral, the signed fractional part is returned). The parameter order (integral pointer first)
 * is a guess: the float and the pointer travel in different registers, so the bytes of the callers
 * do not show it. A game function outside the CSD library also calls this (not declared in a
 * header). */
float SplitFloat(float *integral, float value) {
    switch (fpclassify(value)) {
    case FP_NAN:
    case FP_ZERO:
        *integral = value;
        return value;
    case FP_INFINITE:
        *integral = value;
        return 0.0f;
    case FP_SUBNORMAL:
        *integral = 0.0f;
        return value;
    default: {
        float truncated = (int)value;
        float fraction = value - truncated;
        if (*(int *)&fraction == 0 || (*(int *)&value & 0x7F800000) >= 0x4B800000) {
            *integral = value;
            return 0.0f;
        }
        *integral = (int)value;
        return fraction;
    }
    }
}

void UnpackPatternBlend(int *pattern, int *nextPattern, f32 *blend, f32 packed) {
    f32 integral;
    int count;

    *blend = SplitFloat(&integral, packed);
    count = (int)integral;
    *nextPattern = count / 10000;
    *pattern = count % 10000;
}

} // namespace CSD
} // namespace Chao

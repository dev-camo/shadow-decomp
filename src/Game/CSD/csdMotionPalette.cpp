/* csdMotionPalette.cpp (original name from the library's allocation calls): builds the runtime
 * entries of a scene's layers (MotionPalette.h) from the scene's file data. Function names are
 * inferred. Definitions are in reverse address order (deferred inlining emits the last definition
 * first). */
#include "Game/CSD/MotionPalette.h"
#include "Game/CSD/Screen.h"
#include "Game/CSD/csdBase.h"
#include <stdlib.h>
#include <string.h>

/* Undecompiled callees at 0x8035BA28 and 0x8035C294: a name-to-index search of a sorted name
 * table and the reset of an entry to its defaults (behaviour inferred). */
extern "C" int fn_8035BA28(const void *names, int count, const char *name);
extern "C" void fn_8035C294(Chao::CSD::MotionPaletteEntry *entry);

namespace Chao {
namespace CSD {

static MatrixValue *s_pMatrixPool;
static int s_matrixPoolSize;
static int s_matrixPoolUsed;

u32 ElementData::fn_8036172C() const {
    return packed & 0xFF;
}

u32 ElementData::fn_80361720() const {
    return (packed >> 8) & 3;
}

u32 ElementData::fn_80361714() const {
    return (packed >> 12) & 3;
}

u32 ElementData::fn_80361708() const {
    return (packed >> 10) & 1;
}

u32 ElementData::fn_803616FC() const {
    return (packed >> 11) & 1;
}

u32 ElementData::fn_803616F0() const {
    return (packed >> 16) & 3;
}

u32 ElementData::fn_803616E4() const {
    return (packed >> 18) & 3;
}

static int CompareCharacterCode(const void *a, const void *b) {
    s32 codeA = static_cast<const FontCharacter *>(a)->code;
    s32 codeB = static_cast<const FontCharacter *>(b)->code;

    if (codeA < codeB) {
        return -1;
    }
    return codeA > codeB;
}

u32 GetPaletteSize(const SceneData *scene) {
    int entryCount = 0;

    for (int i = 0; i < scene->layerCount; i++) {
        entryCount += scene->layers[i].elementCount;
    }
    return sizeof(MotionPalette) + scene->layerCount * sizeof(MotionPaletteLayer) +
           entryCount * sizeof(MotionPaletteEntry);
}

void InitPalette(MotionPalette *palette, const SceneData *scene, const FontList *fonts) {
    int entryBase = 0;

    palette->layerCount = scene->layerCount;
    palette->layers = reinterpret_cast<MotionPaletteLayer *>(palette + 1);
    for (int i = 0; i < palette->layerCount; i++) {
        palette->layers[i].entryCount = scene->layers[i].elementCount;
        palette->layers[i].entries = reinterpret_cast<MotionPaletteEntry *>(
            reinterpret_cast<u8 *>(palette) + sizeof(MotionPalette) +
            palette->layerCount * sizeof(MotionPaletteLayer) +
            entryBase * sizeof(MotionPaletteEntry));
        for (int j = 0; j < palette->layers[i].entryCount; j++) {
            fn_8035C294(&palette->layers[i].entries[j]);
            palette->layers[i].entries[j].layer = &palette->layers[i];
        }
        palette->layers[i].field_0x8 = 0;
        entryBase += scene->layers[i].elementCount;
        palette->layers[i].palette = palette;
    }

    palette->field_0x8 = 0;
    palette->field_0xC = -1.0f;
    palette->field_0x10 = 1.0f;
    palette->field_0x14 = 0;
    palette->field_0x30 = 0;
    palette->field_0x34 = 0;
    palette->matrixPool = NULL;

    int matrixCount = CountPaletteMatrices(palette, scene);
    if (matrixCount > 0) {
        palette->matrixPool = static_cast<MatrixValue *>(
            Alloc(matrixCount * sizeof(MatrixValue), __FILE__, __LINE__));
    }
    s_pMatrixPool = palette->matrixPool;
    s_matrixPoolSize = matrixCount;
    s_matrixPoolUsed = 0;
    BuildPalette(palette, scene, fonts);
    s_pMatrixPool = NULL;
}
static void BuildLayer(MotionPaletteLayer *layer, const LayerData *src, const SceneData *scene,
                       const FontList *fonts);

void BuildPalette(MotionPalette *palette, const SceneData *scene, const FontList *fonts) {
    f32 width;
    f32 height;
    f32 x;
    f32 y;

    for (int i = 0; i < scene->layerCount; i++) {
        BuildLayer(&palette->layers[i], &scene->layers[i], scene, fonts);
    }
    GetScreenSizeA(&width, &height);
    palette->screenSizeA[0] = width;
    palette->screenSizeA[1] = height;
    GetScreenSizeB(&width, &height);
    palette->screenSizeB[0] = width;
    palette->screenSizeB[1] = height;
    GetScaledScreenOffsetA(&x, &y);
    palette->screenOffsetA[0] = x;
    palette->screenOffsetA[1] = y;
}

static void BuildEntries(MotionPaletteLayer *layer, int index, const LayerData *src,
                         MotionPaletteEntry *parent, const SceneData *scene, const FontList *fonts);

static void BuildLayer(MotionPaletteLayer *layer, const LayerData *src, const SceneData *scene,
                       const FontList *fonts) {
    if (src->rootIndex >= 0) {
        BuildEntries(layer, src->rootIndex, src, NULL, scene, fonts);
    }
}

static void BuildQuadEntry(MotionPaletteEntry *entry, const ElementData *element);
static void BuildTextEntry(MotionPaletteEntry *entry, const ElementData *element,
                           const SceneData *scene, const FontList *fonts);

void BuildEntry(MotionPaletteEntry *entry, const ElementData *element, const SceneData *scene,
                const FontList *fonts) {
    switch (element->type) {
    case 0:
        BuildQuadEntry(entry, element);
        break;
    case 1:
        BuildQuadEntry(entry, element);
        break;
    case 2:
        BuildTextEntry(entry, element, scene, fonts);
        break;
    }
}

static void BuildQuadEntry(MotionPaletteEntry *entry, const ElementData *element) {
    if (entry->textures != NULL) {
        if (entry->ownsVertices) {
            Free(entry->vertices);
        } else {
            entry->vertices = NULL;
            entry->drawVertices = NULL;
            entry->textures = NULL;
        }
    }
    entry->vertices = entry->quadVertices;
    entry->drawVertices = entry->quadDrawVertices;
    entry->textures = &entry->quadTexture;
    entry->ownsVertices = 0;
    entry->quadCount = 1;

    Vertex *vertices = entry->vertices;
    vertices[0].x = element->corners[0][0];
    vertices[0].y = element->corners[0][1];
    vertices[1].x = element->corners[1][0];
    vertices[1].y = element->corners[1][1];
    vertices[2].x = element->corners[2][0];
    vertices[2].y = element->corners[2][1];
    vertices[3].x = element->corners[3][0];
    vertices[3].y = element->corners[3][1];
    for (int i = 0; i < 4; i++) {
        ToScreenB(&vertices[i].x, &vertices[i].y);
    }

    entry->field_0x40 = -100000.0f;
    entry->flags = 0;
    entry->field_0x140 = 0;
}

/* Shift JIS lead bytes: 0x81-0x9F and 0xE0-0xF5. */
static inline int IsShiftJisLeadByte(u8 c) {
    u8 folded = c ^ 0x20;
    return folded >= 0xA1 && folded <= 0xD5;
}

static void BuildTextEntry(MotionPaletteEntry *entry, const ElementData *element,
                           const SceneData *scene, const FontList *fonts) {
    if (fonts == NULL || fonts->fonts == NULL) {
        return;
    }
    if (entry->textures != NULL) {
        if (entry->ownsVertices) {
            if (entry->vertices != NULL) {
                Free(entry->vertices);
                entry->vertices = NULL;
            }
        } else {
            entry->vertices = NULL;
            entry->drawVertices = NULL;
            entry->textures = NULL;
        }
    }

    const char *text = element->text;
    const char *fontName = element->fontName;
    if (entry->text != NULL) {
        text = entry->text;
    }
    if (entry->fontName != NULL) {
        fontName = entry->fontName;
    }
    Font *font = &fonts->fonts[fn_8035BA28(fonts->names, fonts->count, fontName)];
    int length = strlen(text);
    if (length <= 0) {
        return;
    }

    f32 spacing = 0.0f;
    if (element->field_0x0 >= 1) {
        if (entry->charSpacing <= -1000000.0f) {
            spacing = element->charSpacing;
        } else {
            spacing = entry->charSpacing;
        }
    }
    f32 height = element->corners[1][1] - element->corners[0][1];
    f32 cursor = 0.0f;
    f32 glyphWidth = 16.0f;
    f32 glyphHeight = 16.0f;
    FromScreenB(&glyphWidth, &glyphHeight);

    /* Per character: four vertices, four drawing vertices and a texture index. */
    u8 *block = static_cast<u8 *>(
        Alloc(length * sizeof(s32) + length * 8 * sizeof(Vertex), __FILE__, __LINE__));
    entry->vertices = reinterpret_cast<Vertex *>(block);
    entry->drawVertices = reinterpret_cast<Vertex *>(block + length * 4 * sizeof(Vertex));
    entry->textures = reinterpret_cast<s32 *>(block + length * 8 * sizeof(Vertex));
    entry->ownsVertices = 1;

    Vertex *vertices = entry->vertices;
    int quadCount = 0;
    for (int i = 0; i < length; i++) {
        u8 c = *text;
        int code;
        if (IsShiftJisLeadByte(c)) {
            code = (c << 8) | static_cast<u8>(*++text);
            i++;
        } else {
            code = c;
        }
        text++;
        FontCharacter key;
        key.code = code;
        key.glyph = -1;
        FontCharacter *found =
            static_cast<FontCharacter *>(bsearch(&key, font->characters, font->characterCount,
                                                 sizeof(FontCharacter), CompareCharacterCode));
        int glyph = found != NULL ? found->glyph : -1;

        Vertex *quad = &vertices[quadCount * 4];
        if (glyph < 0) {
            /* No glyph: an untextured quad of the default size. */
            entry->textures[quadCount] = -1;
            f32 left = element->corners[0][0] + cursor;
            f32 scale = glyphHeight / height;
            quad[0].x = left;
            quad[0].y = element->corners[0][1] * scale;
            quad[0].color = entry->colors[0];
            quad[0].u = 0.0f;
            quad[0].v = 0.0f;
            quad[1].x = left;
            quad[1].y = element->corners[1][1] * scale;
            quad[1].color = entry->colors[1];
            quad[1].u = 0.0f;
            quad[1].v = 0.0f;
            quad[2].x = glyphWidth + left;
            quad[2].y = element->corners[2][1] * scale;
            quad[2].color = entry->colors[2];
            quad[2].u = 0.0f;
            quad[2].v = 0.0f;
            quad[3].x = glyphWidth + left;
            quad[3].y = element->corners[3][1] * scale;
            quad[3].color = entry->colors[3];
            quad[3].u = 0.0f;
            quad[3].v = 0.0f;
            cursor += glyphWidth + spacing;
        } else {
            GlyphData *data = &scene->glyphs[glyph];
            entry->textures[quadCount] = data->texture;
            TextureSize *size = &scene->textureSizes[data->texture];
            f32 left = element->corners[0][0] + cursor;
            f32 scale = size->height * (data->v1 - data->v0) / height;
            f32 right = size->width * (data->u1 - data->u0) + left;
            quad[0].x = left;
            quad[0].y = element->corners[0][1] * scale;
            quad[0].color = entry->colors[0];
            quad[0].u = data->u0;
            quad[0].v = data->v0;
            quad[1].x = left;
            quad[1].y = element->corners[1][1] * scale;
            quad[1].color = entry->colors[1];
            quad[1].u = data->u0;
            quad[1].v = data->v1;
            quad[2].x = right;
            quad[2].y = element->corners[2][1] * scale;
            quad[2].color = entry->colors[2];
            quad[2].u = data->u1;
            quad[2].v = data->v0;
            quad[3].x = right;
            quad[3].y = element->corners[3][1] * scale;
            quad[3].color = entry->colors[3];
            quad[3].u = data->u1;
            quad[3].v = data->v1;
            cursor += spacing + (quad[2].x - quad[0].x);
        }
        quadCount++;
    }
    entry->quadCount = quadCount;

    /* Move the line to start at the element's left edge scaled by text width / element width. */
    f32 textWidth = vertices[(quadCount - 1) * 4 + 2].x - vertices[0].x;
    f32 offset =
        element->corners[0][0] * (textWidth / (element->corners[2][0] - element->corners[0][0])) -
        vertices[0].x;
    for (int q = 0; q < quadCount; q++) {
        for (int k = 0; k < 4; k++) {
            Vertex *vertex = &vertices[q * 4 + k];
            vertex->x += offset;
            ToScreenB(&vertex->x, &vertex->y);
            entry->drawVertices[q * 4 + k].u = vertex->u;
            entry->drawVertices[q * 4 + k].v = vertex->v;
            entry->drawVertices[q * 4 + k].color = 0xFFFFFFFF;
        }
    }

    entry->textWidth = textWidth;
    entry->field_0x140 = 0;
    entry->flags &= ~0x200000;
    entry->flags &= ~0x80;
    entry->flags &= ~0x80000;
    entry->flags &= ~0xF00;
    entry->field_0x44 = -1;
}

static void BuildEntries(MotionPaletteLayer *layer, int index, const LayerData *src,
                         MotionPaletteEntry *parent, const SceneData *scene,
                         const FontList *fonts) {
    MotionPaletteEntry *entry = &layer->entries[index];
    ElementData *element = src->elements[index];

    BuildEntry(entry, element, scene, fonts);
    if (!(element->flags & 0x400) || !(element->flags & 0x800) || !(element->flags & 0x2)) {
        AllocEntryMatrices(entry);
    }
    if (src->links[index].child >= 0) {
        BuildEntries(layer, src->links[index].child, src, entry, scene, fonts);
    }
    if (parent != NULL && entry->matrix.sub0 != NULL) {
        AllocEntryMatrices(parent);
    }
    if (src->links[index].next >= 0) {
        BuildEntries(layer, src->links[index].next, src, parent, scene, fonts);
    }
}

static inline MatrixValue *TakePoolMatrix() {
    MatrixValue *matrix = s_pMatrixPool;
    s_pMatrixPool++;
    s_matrixPoolUsed++;
    return matrix;
}

void AllocEntryMatrices(MotionPaletteEntry *entry) {
    if (entry->matrix.sub1 == NULL || entry->matrix.sub0 == NULL) {
        entry->matrix.sub0 = TakePoolMatrix();
        entry->matrix.sub1 = TakePoolMatrix();
    }
}

void ReleasePalette(MotionPalette *palette) {
    for (int i = 0; i < palette->layerCount; i++) {
        for (int j = 0; j < palette->layers[i].entryCount; j++) {
            MotionPaletteEntry *entry = &palette->layers[i].entries[j];
            entry->matrix.sub1 = NULL;
            entry->matrix.sub0 = NULL;
            if (entry->field_0x78 != NULL) {
                Free(entry->field_0x78);
                entry->field_0x78 = NULL;
            }
            if (entry->ownsVertices && entry->vertices != NULL) {
                Free(entry->vertices);
                entry->vertices = NULL;
            }
        }
    }
    if (palette->matrixPool != NULL) {
        Free(palette->matrixPool);
        palette->matrixPool = NULL;
    }
}

void ReleaseEntry(MotionPaletteEntry *entry) {
    entry->matrix.sub1 = NULL;
    entry->matrix.sub0 = NULL;
    if (entry->field_0x78 != NULL) {
        Free(entry->field_0x78);
        entry->field_0x78 = NULL;
    }
    if (entry->ownsVertices && entry->vertices != NULL) {
        Free(entry->vertices);
        entry->vertices = NULL;
    }
}

/* Clears the up-to-date flag (0x100000) of an entry and its ancestors and marks the root's layer
 * and palette for an update. The original's out-of-line copy came from a caller removed at link;
 * it is not emitted here although other units call it. */
inline void MarkEntryDirty(MotionPaletteEntry *entry) {
    entry->flags &= ~0x100000;
    if (entry->parent != NULL) {
        MarkEntryDirty(entry->parent);
    } else {
        MotionPaletteLayer *layer = entry->layer;
        layer->field_0x8 = 0;
        layer->palette->field_0x8 = 0;
        layer->palette->field_0x30 = 1;
    }
}

static void MarkLayerMatrixUsers(MotionPaletteLayer *layer, const LayerData *src);

static inline void ClearEntryMatrices(MotionPalette *palette) {
    for (int i = 0; i < palette->layerCount; i++) {
        for (int j = 0; j < palette->layers[i].entryCount; j++) {
            palette->layers[i].entries[j].matrix.sub1 = NULL;
            palette->layers[i].entries[j].matrix.sub0 = NULL;
        }
    }
}

int CountPaletteMatrices(MotionPalette *palette, const SceneData *scene) {
    ClearEntryMatrices(palette);

    for (int i = 0; i < scene->layerCount; i++) {
        MarkLayerMatrixUsers(&palette->layers[i], &scene->layers[i]);
    }

    int matrixCount = 0;
    for (int i = 0; i < palette->layerCount; i++) {
        for (int j = 0; j < palette->layers[i].entryCount; j++) {
            if (palette->layers[i].entries[j].matrix.sub0 != NULL) {
                matrixCount += 2;
            }
        }
    }

    ClearEntryMatrices(palette);
    return matrixCount;
}

static inline void MarkMatrixUsers(MotionPaletteLayer *layer, int index, const LayerData *src,
                                   MotionPaletteEntry *parent);

static void MarkLayerMatrixUsers(MotionPaletteLayer *layer, const LayerData *src) {
    if (src->rootIndex >= 0) {
        MarkMatrixUsers(layer, src->rootIndex, src, NULL);
    }
}

/* Marks the entries that need extra matrices, and their ancestors, with a non-null sub0 so that
 * CountPaletteMatrices can size the pool. */
static inline void MarkMatrixUsers(MotionPaletteLayer *layer, int index, const LayerData *src,
                                   MotionPaletteEntry *parent) {
    MotionPaletteEntry *entry = &layer->entries[index];
    u32 flags = src->elements[index]->flags;

    if (!(flags & 0x400) || !(flags & 0x800) || !(flags & 0x2)) {
        entry->matrix.sub0 = reinterpret_cast<MatrixValue *>(1);
    }
    if (src->links[index].child >= 0) {
        MarkMatrixUsers(layer, src->links[index].child, src, entry);
    }
    if (parent != NULL && entry->matrix.sub0 != NULL) {
        parent->matrix.sub0 = reinterpret_cast<MatrixValue *>(1);
    }
    if (src->links[index].next >= 0) {
        MarkMatrixUsers(layer, src->links[index].next, src, parent);
    }
}

void SetMatrixPool(MatrixValue *pool, int count) {
    s_pMatrixPool = pool;
    s_matrixPoolSize = count;
    s_matrixPoolUsed = 0;
}

void ClearMatrixPool() {
    s_pMatrixPool = NULL;
}

} // namespace CSD
} // namespace Chao

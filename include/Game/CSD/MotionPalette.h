#ifndef GAME_CSD_MOTIONPALETTE_H
#define GAME_CSD_MOTIONPALETTE_H

#include <cstddef>
#include <dolphin/types.h>
#include "Game/CSD/Matrix.h"
#include "layout_check.h"

/* The runtime records the Chao::CSD library builds from a scene's file data, and that file data.
 * csdMotionPalette.cpp builds them, the csdm* files animate and draw them. Every type and member
 * name here is inferred from that code; members without a supported meaning keep their offset. */
namespace Chao {
namespace CSD {

/* An entry of a name table, which is sorted by name. The index selects the item the name stands
 * for in the parallel array of the table's owner. */
struct NameEntry {
    /* 0x00 */ const char *name;
    /* 0x04 */ s32 index;
};
CHECK_SIZEOF(Chao::CSD::NameEntry, 0x8);

/* A quad corner: position, colour and texture coordinates. */
struct Vertex {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ u32 color;
    /* 0x0C */ f32 u;
    /* 0x10 */ f32 v;
};
CHECK_SIZEOF(Chao::CSD::Vertex, 0x14);

/* A quad corner without texture coordinates: the layout of the vertices of an entry whose textures
 * pointer is null (inferred from the 0xC stride the colour functions of csdLoader.cpp use when
 * textures is null). */
struct PlainVertex {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ u32 color;
};
CHECK_SIZEOF(Chao::CSD::PlainVertex, 0xC);

/* File data of one element of a scene layer. Type 0 and 1 elements are single quads, type 2
 * elements draw their text with a font. The aspect (name inferred: ApplyScreenAspect defaults to
 * 1.3333 and divides by the screen's aspect ratio) is used only when field_0x0 is at
 * least 1. */
struct ElementData {
    /* 0x00 */ s32 field_0x0;
    /* 0x04 */ s32 type;
    /* 0x08 */ u32 field_0x8;
    /* 0x0C */ f32 corners[4][2];
    /* 0x2C */ u32 field_0x2C;
    /* 0x30 */ u32 field_0x30;
    /* 0x34 */ u32 flags;
    /* 0x38 */ u32 packed;
    /* 0x3C */ u32 field_0x3C;
    /* 0x40 */ f32 aspect;
    /* 0x44 */ const char *text;
    /* 0x48 */ const char *fontName;
    /* 0x4C */ f32 charSpacing;

    /* Fields of the packed word (placeholder names: their meaning is unknown). */
    u32 fn_803616E4() const;
    u32 fn_803616F0() const;
    u32 fn_803616FC() const;
    u32 fn_80361708() const;
    u32 fn_80361714() const;
    u32 fn_80361720() const;
    u32 fn_8036172C() const;
};
/* Only the members up to 0x50 are known. */
CHECK_SIZEOF(Chao::CSD::ElementData, 0x50);

/* The element tree of a layer: indices of the first child and the next sibling, -1 for none. */
struct ElementLink {
    /* 0x00 */ s32 child;
    /* 0x04 */ s32 next;
};
CHECK_SIZEOF(Chao::CSD::ElementLink, 0x8);

/* File data of one scene layer. */
struct LayerData {
    /* 0x00 */ s32 elementCount;
    /* 0x04 */ ElementData **elements;
    /* 0x08 */ s32 rootIndex;
    /* 0x0C */ ElementLink *links;
};
CHECK_SIZEOF(Chao::CSD::LayerData, 0x10);

/* A texture region of a font glyph: texture index and texture coordinates. */
struct GlyphData {
    /* 0x00 */ s32 texture;
    /* 0x04 */ f32 u0;
    /* 0x08 */ f32 v0;
    /* 0x0C */ f32 u1;
    /* 0x10 */ f32 v1;
};
CHECK_SIZEOF(Chao::CSD::GlyphData, 0x14);

/* The size of a texture used by the scene. */
struct TextureSize {
    /* 0x00 */ f32 width;
    /* 0x04 */ f32 height;
};
CHECK_SIZEOF(Chao::CSD::TextureSize, 0x8);

/* An entry of a scene's node table, which is sorted by name (searched by FindNode). The
 * members after the name are used by the csdm* files. */
struct NodeEntry {
    /* 0x00 */ const char *name;
    /* 0x04 */ s32 field_0x4;
    /* 0x08 */ s32 field_0x8;
};
CHECK_SIZEOF(Chao::CSD::NodeEntry, 0xC);

/* File data of a scene (only the members used by the CSD code so far). */
struct SceneData {
    /* 0x00 */ u8 field_0x0[0x18];
    /* 0x18 */ TextureSize *textureSizes;
    /* 0x1C */ u32 field_0x1C;
    /* 0x20 */ GlyphData *glyphs;
    /* 0x24 */ s32 layerCount;
    /* 0x28 */ LayerData *layers;
    /* 0x2C */ s32 nodeCount;
    /* 0x30 */ NodeEntry *nodes;
    /* 0x34 */ s32 nameCount;
    /* 0x38 */ u32 field_0x38;
    /* 0x3C */ NameEntry *names;
};
/* Only the members up to 0x40 are known. */
CHECK_SIZEOF(Chao::CSD::SceneData, 0x40);

/* A character code and its glyph index, sorted by code. */
struct FontCharacter {
    /* 0x00 */ s32 code;
    /* 0x04 */ s32 glyph;
};
CHECK_SIZEOF(Chao::CSD::FontCharacter, 0x8);

struct Font {
    /* 0x00 */ s32 characterCount;
    /* 0x04 */ FontCharacter *characters;
};
CHECK_SIZEOF(Chao::CSD::Font, 0x8);

/* The fonts of a project and their name table (searched by FindNameIndex). */
struct FontList {
    /* 0x00 */ s32 count;
    /* 0x04 */ Font *fonts;
    /* 0x08 */ const NameEntry *names;
};
CHECK_SIZEOF(Chao::CSD::FontList, 0xC);

struct MotionPalette;
struct MotionPaletteLayer;

/* The runtime state of one element. Quad elements keep their vertices inline; text elements
 * allocate four vertices of each kind and one texture index per character. */
struct MotionPaletteEntry {
    /* 0x00 */ Matrix matrix;
    /* 0x20 */ s32 field_0x20;
    /* 0x24 */ u32 field_0x24;
    /* 0x28 */ u8 field_0x28[0x38 - 0x28];
    /* 0x38 */ f32 field_0x38;
    /* 0x3C */ f32 field_0x3C;
    /* 0x40 */ f32 field_0x40;
    /* 0x44 */ s32 field_0x44;
    /* 0x48 */ u32 colors[4];
    /* 0x58 */ s32 field_0x58[3];
    /* 0x64 */ const char *text;
    /* 0x68 */ const char *fontName;
    /* 0x6C */ f32 textWidth;
    /* 0x70 */ f32 charSpacing;
    /* 0x74 */ u32 field_0x74;
    /* 0x78 */ void *field_0x78;
    /* 0x7C */ u32 flags;
    /* 0x80 */ MotionPaletteEntry *parent;
    /* 0x84 */ MotionPaletteLayer *layer;
    /* 0x88 */ s32 quadCount;
    /* 0x8C */ Vertex *vertices;
    /* 0x90 */ Vertex *drawVertices;
    /* 0x94 */ s32 *textures;
    /* 0x98 */ Vertex quadVertices[4];
    /* 0xE8 */ Vertex quadDrawVertices[4];
    /* 0x138 */ s32 quadTexture;
    /* 0x13C */ s32 ownsVertices;
    /* 0x140 */ s32 field_0x140;
};
CHECK_SIZEOF(Chao::CSD::MotionPaletteEntry, 0x144);

struct MotionPaletteLayer {
    /* 0x00 */ s32 entryCount;
    /* 0x04 */ MotionPaletteEntry *entries;
    /* 0x08 */ s32 field_0x8;
    /* 0x0C */ MotionPalette *palette;
};
CHECK_SIZEOF(Chao::CSD::MotionPaletteLayer, 0x10);

/* A palette is allocated in one block: this header, its layers, then every layer's entries. */
struct MotionPalette {
    /* 0x00 */ s32 layerCount;
    /* 0x04 */ MotionPaletteLayer *layers;
    /* 0x08 */ s32 field_0x8;
    /* 0x0C */ f32 field_0xC;
    /* 0x10 */ f32 field_0x10;
    /* 0x14 */ u32 field_0x14;
    /* 0x18 */ f32 screenSizeA[2];
    /* 0x20 */ f32 screenSizeB[2];
    /* 0x28 */ f32 screenOffsetA[2];
    /* 0x30 */ s32 field_0x30;
    /* 0x34 */ u32 field_0x34;
    /* 0x38 */ u32 field_0x38;
    /* 0x3C */ MatrixValue *matrixPool;
};
CHECK_SIZEOF(Chao::CSD::MotionPalette, 0x40);

void ClearMatrixPool();
void SetMatrixPool(MatrixValue *pool, int count);
int CountPaletteMatrices(MotionPalette *palette, const SceneData *scene);
void MarkEntryDirty(MotionPaletteEntry *entry);
void ReleaseEntry(MotionPaletteEntry *entry);
void ReleasePalette(MotionPalette *palette);
void AllocEntryMatrices(MotionPaletteEntry *entry);
void BuildEntry(MotionPaletteEntry *entry, const ElementData *element, const SceneData *scene,
                const FontList *fonts);
void BuildPalette(MotionPalette *palette, const SceneData *scene, const FontList *fonts);
void InitPalette(MotionPalette *palette, const SceneData *scene, const FontList *fonts);
u32 GetPaletteSize(const SceneData *scene);

} // namespace CSD
} // namespace Chao

#endif

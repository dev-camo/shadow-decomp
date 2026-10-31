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

/* Two floats that travel together: a translation or a scale (name inferred). */
struct Vector2 {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
};
CHECK_SIZEOF(Chao::CSD::Vector2, 0x8);

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
 * elements draw their text with a font. A type 1 element picks the texture region of its quad from
 * a table of patterns (names inferred from SetEntryPattern in csdLoader.cpp): the table has
 * patternCount glyph indices (-1 for none) into the scene's glyphs. The element's stored channel
 * values (names inferred from EvaluateChannels in csdLoader.cpp, which reads them through
 * GetChannelValue) have one word for each set bit of channelMask, in channel order; when the mask
 * is full, EvaluateChannels indexes the 15 words by channel directly. */
struct ElementData {
    /* 0x00 */ s32 field_0x0;
    /* 0x04 */ s32 type;
    /* 0x08 */ u32 field_0x8;
    /* 0x0C */ f32 corners[4][2];
    /* 0x2C */ u32 channelMask;
    /* 0x30 */ const u32 *channelValues;
    /* 0x34 */ u32 flags;
    /* 0x38 */ u32 packed;
    /* 0x3C */ s32 patternCount;
    /* 0x40 */ const s32 *patterns;
    /* 0x44 */ const char *text;
    /* 0x48 */ const char *fontName;
    /* 0x4C */ f32 charSpacing;

    /* Fields of the packed word (placeholder names: their meaning is unknown). SetEntryPattern
     * swaps the u corners of a pattern when fn_80361708 is set and the v corners when
     * fn_803616FC is set (a horizontal and a vertical flip, inferred). */
    int fn_803616E4() const;
    int fn_803616F0() const;
    int fn_803616FC() const;
    int fn_80361708() const;
    int fn_80361714() const;
    int fn_80361720() const;
    int fn_8036172C() const;
};
/* Only the members up to 0x50 are known. */
CHECK_SIZEOF(Chao::CSD::ElementData, 0x50);

/* Bits of ElementData::flags that the update code of csdLoader.cpp tests (names inferred;
 * meanings that are not clear keep the value). The translation bits select which coordinates of
 * the entry's translation the matrix stack transforms, and the scale bits (ELEMENT_FLAG_0x400,
 * ELEMENT_FLAG_0x800) select how the entry's scale combines with its parent's. */
enum ElementFlags {
    ELEMENT_FLAG_0x2 = 0x2,
    ELEMENT_COMBINE_TINT = 0x8,    /* the tint is the parent's scaled by the entry's */
    ELEMENT_COMBINE_HIDDEN = 0x10, /* the entry is hidden when its parent is */
    ELEMENT_TRANSLATE_X = 0x100,
    ELEMENT_TRANSLATE_Y = 0x200,
    ELEMENT_TRANSLATE_MASK = 0x300,
    ELEMENT_FLAG_0x400 = 0x400,
    ELEMENT_FLAG_0x800 = 0x800,
    ELEMENT_SCALE_MASK = 0xC00
};

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

/* A texture coordinate. */
struct TexCoord {
    /* 0x00 */ f32 u;
    /* 0x04 */ f32 v;
};
CHECK_SIZEOF(Chao::CSD::TexCoord, 0x8);

/* A texture region of a font glyph or of a pattern: texture index and the coordinates of two
 * opposite corners. */
struct GlyphData {
    /* 0x00 */ s32 texture;
    /* 0x04 */ TexCoord corner[2];
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

struct SceneMotion;

/* The first and last frame of one motion of a scene (names inferred from CScene's frame range). */
struct FrameRange {
    /* 0x00 */ f32 start;
    /* 0x04 */ f32 end;
};
CHECK_SIZEOF(Chao::CSD::FrameRange, 0x8);

/* File data of a scene (only the members used by the CSD code so far). The aspect (name inferred:
 * the entry transform divides the screen's aspect ratio by it, ApplyScreenAspect divides it by
 * the screen's) is present only when the version (name inferred from that test) is at least 1;
 * otherwise they use 1.3333. The frame ranges are present only when the version is at least 2. */
struct SceneData {
    /* 0x00 */ s32 version;
    /* 0x04 */ u8 field_0x4[0x14];
    /* 0x18 */ TextureSize *textureSizes;
    /* 0x1C */ u32 field_0x1C;
    /* 0x20 */ GlyphData *glyphs;
    /* 0x24 */ s32 layerCount;
    /* 0x28 */ LayerData *layers;
    /* 0x2C */ s32 nodeCount;
    /* 0x30 */ NodeEntry *nodes;
    /* 0x34 */ s32 nameCount; /* also the number of motions */
    /* 0x38 */ const SceneMotion *motions;
    /* 0x3C */ NameEntry *names;
    /* 0x40 */ f32 aspect;
    /* 0x44 */ const FrameRange *frameRanges;
};
/* Only the members up to 0x48 are known. */
CHECK_SIZEOF(Chao::CSD::SceneData, 0x48);

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

/* The number of animated values of an element. Each is a word, selected by a channel number. */
const int kEntryChannelCount = 15;

/* The animated values of an entry, as named values and as words by channel number. The meanings
 * are inferred from how the update code of csdLoader.cpp uses them: the entry's matrix is built
 * from the translation, rotation and scale, field_0x20 and field_0x24 follow hidden and tint,
 * and the pattern selects the texture region of a quad. */
struct EntryChannelValues {
    /* 0x00 */ s32 hidden;        /* channel 0 */
    /* 0x04 */ Vector2 translate; /* channels 1 and 2 */
    /* 0x0C */ f32 rotation;      /* channel 3 */
    /* 0x10 */ Vector2 scale;     /* channels 4 and 5 */
    /* 0x18 */ f32 pattern;       /* channel 6 */
    /* 0x1C */ u32 tint;          /* channel 7 */
    /* 0x20 */ u32 colors[4];     /* channels 8 to 11 */
    /* 0x30 */ s32 field_0x30[3]; /* channels 12 to 14 */
};
CHECK_SIZEOF(Chao::CSD::EntryChannelValues, 0x3C);

/* The channel numbers (the bit of a channel is 1 << number in the masks below). The names are
 * inferred from EntryChannelValues. */
enum EntryChannel {
    CHANNEL_HIDDEN = 0,
    CHANNEL_TRANSLATE_X = 1,
    CHANNEL_TRANSLATE_Y = 2,
    CHANNEL_ROTATION = 3,
    CHANNEL_SCALE_X = 4,
    CHANNEL_SCALE_Y = 5,
    CHANNEL_PATTERN = 6,
    CHANNEL_TINT = 7,
    CHANNEL_COLOR_0 = 8,
    CHANNEL_FIELD_0x30 = 12
};

/* A channel value as the word it is stored in (the type depends on the channel). An entry's
 * override block is an array of these indexed by EntryChannel, with the layout of
 * EntryChannels. */
union ChannelWord {
    s32 integer;
    u32 bits;
    f32 number;
};
CHECK_SIZEOF(Chao::CSD::ChannelWord, 0x4);

union EntryChannels {
    EntryChannelValues named;
    u32 raw[kEntryChannelCount];
};
CHECK_SIZEOF(Chao::CSD::EntryChannels, 0x3C);

/* Bits of MotionPaletteEntry::flags (names inferred from the update code of csdLoader.cpp). The
 * bit of each channel number (the low 15 bits) is set once the channel's value is final. */
enum EntryFlags {
    ENTRY_TRANSFORM_CHANNELS = 0x3E, /* channels 1 to 5 */
    ENTRY_TINT_CHANNEL = 0x80,       /* channel 7 */
    ENTRY_COLOR_CHANNELS = 0xF00,    /* channels 8 to 11 */
    ENTRY_CHANNELS_MASK = 0x7FFF,
    ENTRY_APPLY = 0x8000,             /* a value changed and has to be applied */
    ENTRY_MATRIX_CHANGED = 0x10000,   /* with ENTRY_APPLY: rebuild the matrix */
    ENTRY_HIDDEN_CHANGED = 0x20000,   /* with ENTRY_APPLY: update field_0x20 */
    ENTRY_TINT_CHANGED = 0x40000,     /* with ENTRY_APPLY: update field_0x24 */
    ENTRY_ALL_CHANNELS_SET = 0x80000, /* all 15 channels are final */
    ENTRY_SUBTREE_SET = 0x100000,     /* the entry and its children are final */
    ENTRY_TRANSFORM_SET = 0x200000    /* channels 1 to 5 (the transform) are final */
};
const s32 kEntryChangeMask =
    ENTRY_APPLY | ENTRY_MATRIX_CHANGED | ENTRY_HIDDEN_CHANGED | ENTRY_TINT_CHANGED;

/* The screen description an entry without quads was last brought up to date with (inferred from
 * CMotionPattern::UpdateTransform, csdmMotionPattern.cpp): both sizes and the scaled offset, in
 * the order GetScreenSizeA, GetScreenSizeB and GetScaledScreenOffsetA return them. */
struct ScreenState {
    /* 0x00 */ f32 sizeA[2];
    /* 0x08 */ f32 sizeB[2];
    /* 0x10 */ f32 offsetA[2];
};
CHECK_SIZEOF(Chao::CSD::ScreenState, 0x18);

/* The runtime state of one element. Quad elements keep their vertices inline; text elements
 * allocate four vertices of each kind and one texture index per character. An entry that has no
 * quads keeps a ScreenState in the same slot. */
struct MotionPaletteEntry {
    /* 0x00 */ Matrix matrix;
    /* 0x20 */ s32 field_0x20;
    /* 0x24 */ u32 field_0x24;
    /* 0x28 */ EntryChannels channels;
    /* 0x64 */ const char *text;
    /* 0x68 */ const char *fontName;
    /* 0x6C */ f32 textWidth;
    /* 0x70 */ f32 charSpacing;
    /* 0x74 */ u32 overrideMask;
    /* 0x78 */ ChannelWord *overrides;
    /* 0x7C */ s32 flags;
    /* 0x80 */ MotionPaletteEntry *parent;
    /* 0x84 */ MotionPaletteLayer *layer;
    /* 0x88 */ s32 quadCount;
    union {
        /* 0x8C */ Vertex *vertices;
        /* 0x8C */ ScreenState *screenState;
    };
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

/* A palette is allocated in one block: this header, its layers, then every layer's entries. The
 * values the palette was last updated with (frame, parent, motion and fonts) are kept to tell
 * what changed (names inferred from UpdatePalette in csdLoader.cpp). */
struct MotionPalette {
    /* 0x00 */ s32 layerCount;
    /* 0x04 */ MotionPaletteLayer *layers;
    /* 0x08 */ s32 field_0x8;
    /* 0x0C */ f32 lastFrame;
    /* 0x10 */ f32 lastFrameStep;
    /* 0x14 */ const MotionPaletteEntry *lastParent;
    /* 0x18 */ f32 screenSizeA[2];
    /* 0x20 */ f32 screenSizeB[2];
    /* 0x28 */ f32 screenOffsetA[2];
    /* 0x30 */ s32 field_0x30;
    /* 0x34 */ const SceneMotion *lastMotion;
    /* 0x38 */ const FontList *lastFonts;
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

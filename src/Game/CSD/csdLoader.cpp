/* csdLoader.cpp (the name in the library's allocation calls) reads a project's file data; the rest
 * of the address range is provisionally in this unit. Function, variable and helper names are
 * inferred. Definitions are in reverse address order (deferred inlining emits the last definition
 * first). */
#include "Game/CSD/csdLoader.h"
#include "Game/CSD/CPlatform.h"
#include "Game/CSD/Matrix.h"
#include "Game/CSD/RenderState.h"
#include "Game/CSD/Screen.h"
#include "Game/CSD/csdBase.h"
#include "Game/CSD/csdChunkTags.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace Chao {
namespace CSD {

/* Functions that are used above their definition. */
static void NormalizeFolder(SceneFolder *folder);
static void NormalizeScene(SceneData *scene);
static void NormalizeLayer(LayerData *layer);
static void SwapBytes(void *data, int size);
static void InterpolateKeys(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame);
static void InterpolateLinear(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame);
static void InterpolateColor(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame);
static void InterpolateHermite(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame);
static void HoldKeyValue(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame);
static void UpdateEntries(MotionPaletteLayer *layer, int index, const LayerData *source,
                          const LayerMotion *motion, const FontList *fonts,
                          MotionPaletteEntry *parent, const SceneData *scene, u32 flags, int depth,
                          f32 frame, f32 frameStep);
static void UpdateEntry(MotionPaletteEntry *entry, const ElementData *element,
                        const ChannelAnimation *animation, const MotionPaletteEntry *parent,
                        const SceneData *scene, u32 flags, f32 frame, f32 frameStep);
static void EvaluateChannels(MotionPaletteEntry *entry, const ElementData *element,
                             const ChannelAnimation *animation, const SceneData *scene, f32 frame,
                             f32 frameStep);
static void UpdateEntryTransform(MotionPaletteEntry *entry, const ElementData *element,
                                 const MotionPaletteEntry *parent, const SceneData *scene);

/* A colour as four channel bytes. */
union ColorBytes {
    u32 packed;
    u8 channel[4];
};
CHECK_SIZEOF(Chao::CSD::ColorBytes, 0x4);

/* Passes an element's filtering, addressing, blending and state at 0x24 to the render state. */
static inline void SetElementState(const ElementData *element, MotionPaletteEntry *entry) {
    if (entry->textures != NULL) {
        GetRenderState().SetFilter(element->fn_80361714());
        int addressV = element->fn_803616E4();
        int addressU = element->fn_803616F0();
        GetRenderState().SetAddressing(addressU, addressV);
    }

    switch (element->fn_8036172C()) {
    case 0:
        GetRenderState().SetBlendMode(1);
        break;
    case 1:
        GetRenderState().SetBlendMode(0);
        break;
    }

    /* Cases 1 and 3 are separate: merging them leaves one case block fewer than the original. */
    int value;
    switch (element->fn_80361720()) {
    case 0:
    case 2:
        value = 1;
        break;
    case 1:
        value = 0;
        break;
    case 3:
        value = 0;
        break;
    default:
        value = 1;
        break;
    }
    GetRenderState().SetField0x24(value);
}

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
                element->flags |= ELEMENT_TRANSLATE_MASK;
            }
            if (element->flags & 0x4) {
                element->flags |= ELEMENT_SCALE_MASK;
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

void InitLibrary() {
    InitMatrixStack();
    InitTexLists();
}

void TermLibrary() {
    TermMatrixStack();
    TermTexLists();
}

RenderState::RenderState()
    : m_enterCount(0), m_applied(0), m_changed(0), m_texture(0), m_addressU(-1), m_addressV(-1),
      m_filter(-1), m_blendMode(-1), m_field_0x24(-1), m_textured(0), m_appliedTextured(0) {
    for (int i = 0; i < 3; i++) {
        m_appliedField0x30[i] = 0;
        m_field_0x30[i] = 0;
    }
}

RenderState::~RenderState() {}

RenderState &GetRenderState() {
    static RenderState s_state;
    return s_state;
}

void RenderState::SetTexture(const RCPtr<CTexList> &texList, int texture) {
    if (!(m_texList == texList) || m_texture != texture) {
        m_texList = texList;
        m_texture = texture;
        m_changed = 1;
    }
}

void RenderState::SetAddressing(int addressU, int addressV) {
    if (m_addressU == addressU && m_addressV == addressV) {
        return;
    }
    m_addressU = addressU;
    m_addressV = addressV;
    m_changed = 1;
}

void RenderState::SetFilter(int filter) {
    if (m_filter == filter) {
        return;
    }
    m_filter = filter;
    m_changed = 1;
}

void RenderState::SetBlendMode(int mode) {
    if (m_blendMode == mode) {
        return;
    }
    m_blendMode = mode;
    m_changed = 1;
}

void RenderState::SetField0x24(int value) {
    if (m_field_0x24 == value) {
        return;
    }
    m_field_0x24 = value;
    m_changed = 1;
}

void RenderState::Apply() {
    if (!m_changed) {
        return;
    }

    /* Leave the state the previous quad was drawn in. */
    if (m_applied) {
        if (m_appliedTextured) {
            GetPlatform()->fn_80362E74(m_appliedField0x30, 3);
        } else {
            GetPlatform()->fn_80362E78(m_appliedField0x30, 3);
        }
    }

    if (m_texList.Get() != NULL) {
        m_texList->fn_803637CC(m_texture, m_field_0x30, 3);
        m_texList->fn_80363724(m_addressU, m_addressV, m_field_0x30, 3);
        m_texList->fn_803636C8(m_texture, m_filter, m_field_0x30, 3);
    }
    GetPlatform()->fn_803632E4(m_blendMode, m_field_0x30, 3);
    if (m_textured) {
        GetPlatform()->fn_803632A0(m_field_0x24, m_field_0x30, 3);
    } else {
        GetPlatform()->fn_80363198(m_field_0x24, m_field_0x30, 3);
    }

    for (int i = 0; i < 3; i++) {
        m_appliedField0x30[i] = m_field_0x30[i];
    }
    m_appliedTextured = m_textured;
    m_applied = 1;
    m_changed = 0;
}

void RenderState::SetTextured(int textured) {
    if (m_textured == textured) {
        return;
    }
    m_textured = textured;
    m_changed = 1;
}

void RenderState::Enter() {
    if (m_enterCount == 0) {
        Begin();
    }
    m_enterCount++;
}

void RenderState::Leave() {
    m_enterCount--;
    if (m_enterCount == 0) {
        End();
    }
}

void RenderState::Begin() {
    GetPlatform()->Begin();
    m_applied = 0;
    m_changed = 1;
}

void RenderState::End() {
    GetPlatform()->End();
    m_texList.Reset(NULL);
    if (m_applied) {
        if (m_appliedTextured) {
            GetPlatform()->fn_80362E74(m_field_0x30, 3);
        } else {
            GetPlatform()->fn_80362E78(m_field_0x30, 3);
        }
    }
}

void RenderState::SetField0x30(const s32 *values) {
    for (int i = 0; i < 3; i++) {
        if (m_field_0x30[i] != values[i]) {
            m_field_0x30[i] = values[i];
            m_changed = 1;
        }
    }
}

void DrawScene(const SceneData *scene, const RCPtr<CTexList> &texList, void *unused,
               const MotionPalette *palette) {
    GetRenderState().Enter();
    for (int i = 0; i < scene->layerCount; i++) {
        DrawLayer(scene, &scene->layers[i], texList, unused, &palette->layers[i]);
    }
    GetRenderState().Leave();
}

void DrawLayer(const SceneData *scene, const LayerData *layer, const RCPtr<CTexList> &texList,
               void *unused, const MotionPaletteLayer *paletteLayer) {
    for (int i = 0; i < layer->elementCount; i++) {
        DrawElement(scene, layer->elements[i], texList, unused, &paletteLayer->entries[i]);
    }
}

void DrawElement(const SceneData *scene, const ElementData *element, const RCPtr<CTexList> &texList,
                 void *unused, MotionPaletteEntry *entry) {
    if (element->field_0x8 != 0 && entry->field_0x20 == 0 && entry->field_0x140 != 0) {
        switch (element->type) {
        case 0:
            /* Not drawn. */
            return;
        case 1:
            DrawQuadElement(scene, element, texList, entry);
            break;
        case 2:
            DrawTextElement(scene, element, texList, unused, entry);
            break;
        }
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

void DrawQuadElement(const SceneData *scene, const ElementData *element,
                     const RCPtr<CTexList> &texList, MotionPaletteEntry *entry) {
    s32 *values = entry->channels.named.field_0x30;

    if (entry->textures != NULL) {
        GetRenderState().SetTexture(texList, entry->textures[0]);
    }
    /* SetField0x30 takes the member, not the local: passing the local changes the code. */
    GetRenderState().SetField0x30(entry->channels.named.field_0x30);
    SetElementState(element, entry);
    if (entry->textures != NULL) {
        GetRenderState().SetTextured(1);
        GetRenderState().Apply();
        GetPlatform()->DrawPrimitive2D(entry->drawVertices, 4, values, 3);
    } else {
        GetRenderState().SetTextured(0);
        GetRenderState().Apply();
        GetPlatform()->fn_803630C8(reinterpret_cast<PlainVertex *>(entry->drawVertices), 4, values,
                                   3);
    }
}

void DrawTextElement(const SceneData *scene, const ElementData *element,
                     const RCPtr<CTexList> &texList, void *unused, MotionPaletteEntry *entry) {
    if (entry->textures == NULL) {
        return;
    }
    s32 *values = entry->channels.named.field_0x30;

    GetRenderState().SetField0x30(values);
    SetElementState(element, entry);

    s32 *textures = entry->textures;
    int quadCount = entry->quadCount;
    int vertex = 0;
    for (int quad = 0; quad < quadCount; quad++, vertex += 4) {
        if (textures[quad] >= 0) {
            GetRenderState().SetTextured(1);
            GetRenderState().SetTexture(texList, entry->textures[quad]);
            GetRenderState().Apply();
            GetPlatform()->DrawPrimitive2D(&entry->drawVertices[quad * 4], 4, values, 3);
        } else {
            /* Glyphs without a texture are drawn as plain quads. The corners are copied one by
             * one: a loop over them compiles to a counted loop, where the original has
             * straight-line code. */
            PlainVertex plain[4];
            Vertex *vertices = entry->drawVertices;
            plain[0].x = vertices[vertex].x;
            plain[0].y = vertices[vertex].y;
            plain[0].color = vertices[vertex].color;
            plain[1].x = vertices[vertex + 1].x;
            plain[1].y = vertices[vertex + 1].y;
            plain[1].color = vertices[vertex + 1].color;
            plain[2].x = vertices[vertex + 2].x;
            plain[2].y = vertices[vertex + 2].y;
            plain[2].color = vertices[vertex + 2].color;
            plain[3].x = vertices[vertex + 3].x;
            plain[3].y = vertices[vertex + 3].y;
            plain[3].color = vertices[vertex + 3].color;
            GetRenderState().SetTextured(0);
            GetRenderState().Apply();
            GetPlatform()->fn_803630C8(plain, 4, values, 3);
        }
    }
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

/* The kinds of change UpdatePalette finds, as the flags it passes to UpdateEntries (names
 * inferred). The screen checks clear UPDATE_NONE. */
enum PaletteUpdate {
    UPDATE_NONE = 1,
    UPDATE_VALUES = 2, /* all channels are evaluated again */
    UPDATE_PARENT = 4, /* the parent entry changed or has changes to apply */
    UPDATE_SCREEN = 8  /* the screen changed: the entries are rebuilt */
};

void UpdatePalette(MotionPalette *palette, const SceneData *scene, const SceneMotion *motion,
                   f32 frame, const FontList *fonts, MotionPaletteEntry *parent) {
    if (frame < 0.0f) {
        frame = 0.0f;
    }
    f32 frameStep = frame - palette->lastFrame;

    /* What changed since the last update. */
    s32 changes = 0;
    if (palette->lastMotion != motion || palette->lastFonts != fonts) {
        changes |= UPDATE_VALUES;
    } else if (frameStep == 0.0f && palette->field_0x30 == 0) {
        changes |= UPDATE_NONE;
    }
    palette->field_0x30 = 0;

    f32 width;
    f32 height;
    GetScreenSizeA(&width, &height);
    if (width != palette->screenSizeA[0] || height != palette->screenSizeA[1]) {
        palette->screenSizeA[0] = width;
        palette->screenSizeA[1] = height;
        changes |= UPDATE_SCREEN;
        changes &= ~UPDATE_NONE;
        changes |= UPDATE_VALUES;
    }

    GetScreenSizeB(&width, &height);
    if (width != palette->screenSizeB[0] || height != palette->screenSizeB[1]) {
        palette->screenSizeB[0] = width;
        palette->screenSizeB[1] = height;
        changes |= UPDATE_SCREEN;
        changes &= ~UPDATE_NONE;
        changes |= UPDATE_VALUES;
    }

    f32 offsetX;
    f32 offsetY;
    GetScaledScreenOffsetA(&offsetX, &offsetY);
    if (offsetX != palette->screenOffsetA[0] || offsetY != palette->screenOffsetA[1]) {
        palette->screenOffsetA[0] = offsetX;
        palette->screenOffsetA[1] = offsetY;
        changes |= UPDATE_SCREEN;
        changes &= ~UPDATE_NONE;
        changes |= UPDATE_VALUES;
    }

    if (palette->lastParent != parent || (parent != NULL && (parent->flags & ENTRY_APPLY))) {
        changes |= UPDATE_PARENT;
    }
    /* A step against the direction of the last one. */
    if (frameStep * palette->lastFrameStep < 0.0f) {
        changes |= UPDATE_VALUES;
    }

    palette->lastFrame = frame;
    palette->lastMotion = motion;
    palette->lastFonts = fonts;
    if (frameStep != 0.0f) {
        palette->lastFrameStep = frameStep;
    }
    palette->lastParent = parent;

    if ((changes & UPDATE_PARENT) ||
        (!(changes & UPDATE_NONE) && ((changes & UPDATE_VALUES) || palette->field_0x8 == 0))) {
        s32 parentFlags = 0;
        if (parent != NULL) {
            parentFlags = parent->flags;
            PushMatrix(&parent->matrix);
        }
        palette->field_0x8 = 1;

        /* The two loops differ only in the motion passed on. The original has both: a single loop
         * that selects each layer's motion compiles to 199 words, where the original has 243. */
        if (motion != NULL) {
            for (int i = 0; i < scene->layerCount; i++) {
                const LayerData *source = &scene->layers[i];
                MotionPaletteLayer *layer = &palette->layers[i];
                s32 rootIndex = source->rootIndex;

                if (rootIndex >= 0) {
                    if ((changes & UPDATE_PARENT) || (changes & UPDATE_VALUES) ||
                        (!(changes & UPDATE_NONE) && layer->field_0x8 == 0)) {
                        layer->field_0x8 = 1;
                        UpdateEntries(layer, rootIndex, source, &motion->layers[i], fonts, parent,
                                      scene, changes, 0, frame, frameStep);
                    }
                }
                if (palette->layers[i].field_0x8 == 0) {
                    palette->field_0x8 = 0;
                }
            }
        } else {
            for (int i = 0; i < scene->layerCount; i++) {
                const LayerData *source = &scene->layers[i];
                MotionPaletteLayer *layer = &palette->layers[i];
                s32 rootIndex = source->rootIndex;

                if (rootIndex >= 0) {
                    if ((changes & UPDATE_PARENT) || (changes & UPDATE_VALUES) ||
                        (!(changes & UPDATE_NONE) && layer->field_0x8 == 0)) {
                        layer->field_0x8 = 1;
                        UpdateEntries(layer, rootIndex, source, NULL, fonts, parent, scene, changes,
                                      0, frame, frameStep);
                    }
                }
                if (palette->layers[i].field_0x8 == 0) {
                    palette->field_0x8 = 0;
                }
            }
        }

        if (parent != NULL) {
            parent->flags = parentFlags;
            PopMatrix();
        }
    }
}

int FindSceneNameIndex(const SceneData *scene, const char *name) {
    NameEntry key;
    key.name = name;
    key.index = -1;
    const NameEntry *entry = static_cast<const NameEntry *>(
        bsearch(&key, scene->names, scene->nameCount, sizeof(NameEntry), CompareNames));

    if (entry != NULL) {
        return entry->index;
    }
    return -1;
}

/* Updates the entry of an element, then its children and its next sibling. Flags are those of
 * UpdatePalette; depth is the number of ancestors below the palette's parent. */
static void UpdateEntries(MotionPaletteLayer *layer, int index, const LayerData *source,
                          const LayerMotion *motion, const FontList *fonts,
                          MotionPaletteEntry *parent, const SceneData *scene, u32 flags, int depth,
                          f32 frame, f32 frameStep) {
    MotionPaletteEntry *entry = &layer->entries[index];

    if (depth > 0) {
        entry->parent = parent;
    } else {
        entry->parent = NULL;
    }

    if (flags & UPDATE_SCREEN) {
        BuildEntry(&layer->entries[index], source->elements[index], scene, fonts);
        layer->entries[index].flags = 0;
    }
    if (flags & UPDATE_VALUES) {
        layer->entries[index].flags = 0;
    }

    int settled = 0;
    if (parent != NULL && (parent->flags & ENTRY_APPLY)) {
        /* The changes of the parent apply to its children. */
        entry->flags &= ~kEntryChangeMask;
        entry->flags |= parent->flags & kEntryChangeMask;
    } else if (entry->flags & ENTRY_SUBTREE_SET) {
        settled = 1;
    } else {
        entry->flags &= ~kEntryChangeMask;
    }

    if (!settled) {
        PushMatrix(&layer->entries[index].matrix);
        if (motion != NULL) {
            UpdateEntry(&layer->entries[index], source->elements[index], &motion->entries[index],
                        parent, scene, flags, frame, frameStep);
        } else {
            UpdateEntry(&layer->entries[index], source->elements[index], NULL, parent, scene, flags,
                        frame, frameStep);
        }

        if (entry->flags & ENTRY_ALL_CHANNELS_SET) {
            entry->flags |= ENTRY_SUBTREE_SET;
        } else {
            entry->flags &= ~ENTRY_SUBTREE_SET;
            /* The original has these five words, which clear the same flag of the same entry
             * again; the source form of this repeated clear is not known. */
            if (parent != NULL) {
                entry->flags &= ~ENTRY_SUBTREE_SET;
            }
        }

        int child = source->links[index].child;
        if (child >= 0) {
            UpdateEntries(layer, child, source, motion, fonts, entry, scene, flags, depth + 1,
                          frame, frameStep);
        }

        if (!(entry->flags & ENTRY_SUBTREE_SET)) {
            if (parent != NULL) {
                parent->flags &= ~ENTRY_SUBTREE_SET;
            }
            layer->field_0x8 = 0;
        }
        PopMatrix();
    }

    int next = source->links[index].next;
    if (next >= 0) {
        UpdateEntries(layer, next, source, motion, fonts, parent, scene, flags, depth, frame,
                      frameStep);
    }
    entry->field_0x140 = 1;
}

/* Evaluates the entry's channels at a frame, and applies what changed. Flags are those of
 * UpdatePalette. */
static void UpdateEntry(MotionPaletteEntry *entry, const ElementData *element,
                        const ChannelAnimation *animation, const MotionPaletteEntry *parent,
                        const SceneData *scene, u32 flags, f32 frame, f32 frameStep) {
    if (entry->vertices == NULL) {
        return;
    }
    if (element->field_0x8 == 0) {
        return;
    }

    if (!(flags & UPDATE_NONE) && !(entry->flags & ENTRY_ALL_CHANNELS_SET)) {
        s32 oldFlags = entry->flags;
        s32 oldHidden = entry->channels.named.hidden;
        u32 oldTint = entry->channels.named.tint;
        int colorsSet = (oldFlags & ENTRY_COLOR_CHANNELS) == ENTRY_COLOR_CHANNELS;
        f32 oldPattern = entry->channels.named.pattern;

        EvaluateChannels(entry, element, animation, scene, frame, frameStep);

        if (!(oldFlags & ENTRY_TRANSFORM_SET)) {
            entry->flags |= ENTRY_APPLY | ENTRY_MATRIX_CHANGED;
        }
        if (oldHidden != entry->channels.named.hidden) {
            entry->flags |= ENTRY_APPLY | ENTRY_HIDDEN_CHANGED;
        }
        if (oldTint != entry->channels.named.tint) {
            entry->flags |= ENTRY_APPLY | ENTRY_TINT_CHANGED;
        }
        if (oldPattern != entry->channels.named.pattern) {
            SetEntryPattern(entry, element, scene);
        }

        if (!colorsSet) {
            SetEntryColors(entry);
            entry->flags |= ENTRY_APPLY | ENTRY_TINT_CHANGED;
        }
    }

    if (entry->flags & ENTRY_APPLY) {
        UpdateEntryTransform(entry, element, parent, scene);
    }
}

/* Sets the entry's channel values. The entry's own values always apply; otherwise a channel that
 * is not final yet comes from the animation track (evaluated at the frame) or from the element's
 * values (the library defaults for channels the element does not store). The scene is unused. */
static void EvaluateChannels(MotionPaletteEntry *entry, const ElementData *element,
                             const ChannelAnimation *animation, const SceneData *scene, f32 frame,
                             f32 frameStep) {
    u32 bit;
    u32 *values = entry->channels.raw;
    int track = 0;
    s32 remaining = ENTRY_CHANNELS_MASK;
    int channel;

    for (channel = 0; channel < kEntryChannelCount; channel++) {
        bit = 1 << channel;

        if (entry->overrideMask & bit) {
            values[channel] = entry->overrides[channel].bits;
            entry->flags |= bit;
            if (animation != NULL && (animation->channelMask & bit)) {
                track++;
            }
        } else if (animation != NULL && (animation->channelMask & bit)) {
            if (!(entry->flags & bit)) {
                int running = 1;
                u32 value;

                EvaluateTrack(&value, &animation->tracks[track], channel, frame, frameStep,
                              &running);
                values[channel] = value;
                if (running == 0) {
                    entry->flags |= bit;
                }
            }
            track++;
        } else if (!(entry->flags & bit)) {
            if (element->channelMask == GetFullChannelMask()) {
                values[channel] = element->channelValues[channel];
            } else {
                u32 value;

                GetChannelValue(&value, channel, element->channelMask, element->channelValues);
                values[channel] = value;
            }
            entry->flags |= bit;
        }

        remaining &= ~bit;
        if (remaining == (remaining & (entry->flags & ENTRY_CHANNELS_MASK))) {
            break;
        }
    }

    if (!(entry->flags & ENTRY_TRANSFORM_SET) &&
        (entry->flags & ENTRY_TRANSFORM_CHANNELS) == ENTRY_TRANSFORM_CHANNELS) {
        entry->flags |= ENTRY_TRANSFORM_SET;
    }
    if ((entry->flags & ENTRY_CHANNELS_MASK) == ENTRY_CHANNELS_MASK) {
        entry->flags |= ENTRY_ALL_CHANNELS_SET;
    }
}

/* Applies what changed to the entry: builds the matrix from the translation, rotation and scale
 * channels, and updates the hidden state and the tint (combined with the parent's as the element
 * asks) and the colours of the vertices. */
static void UpdateEntryTransform(MotionPaletteEntry *entry, const ElementData *element,
                                 const MotionPaletteEntry *parent, const SceneData *scene) {
    u32 flags = element->flags;

    if (entry->flags & ENTRY_MATRIX_CHANGED) {
        Matrix *matrix = GetTopMatrix();
        f32 x = entry->channels.named.translate.x;
        f32 y = entry->channels.named.translate.y;

        ScaleByScreenA(&x, &y);

        f32 aspect = 1.3333f;
        if (scene->version >= 1) {
            aspect = scene->aspect;
        }
        f32 screenAspect = GetScreenAspectB();
        y *= screenAspect / aspect;

        switch (flags & ELEMENT_TRANSLATE_MASK) {
        case ELEMENT_TRANSLATE_MASK:
            TranslateMatrix(x, y);
            break;
        case ELEMENT_TRANSLATE_X:
            TranslateMatrixX(x, y);
            break;
        case ELEMENT_TRANSLATE_Y:
            TranslateMatrixY(x, y);
            break;
        case 0:
            SetMatrixTranslation(x, y);
            break;
        }

        if (!(flags & ELEMENT_FLAG_0x2)) {
            matrix->m[0] = matrix->sub0->m[0];
            matrix->m[1] = matrix->sub0->m[1];
            matrix->m[3] = matrix->sub0->m[3];
            matrix->m[4] = matrix->sub0->m[4];
            matrix->sub1->m[0] = 1.0f;
            matrix->sub1->m[1] = 0.0f;
            matrix->sub1->m[3] = 0.0f;
            matrix->sub1->m[4] = 1.0f;
        }

        /* The scale of the parent that is applied after the entry's own (inferred from the
         * ScaleMatrix calls below). */
        f32 parentScaleX = 1.0f;
        f32 parentScaleY = 1.0f;
        int applyParentScale = 0;
        switch (flags & ELEMENT_SCALE_MASK) {
        case ELEMENT_FLAG_0x400:
            if (!(flags & ELEMENT_FLAG_0x2)) {
                matrix->m[1] = 0.0f;
                matrix->m[3] = 0.0f;
                matrix->m[4] = 1.0f;
                matrix->sub0->m[1] = 0.0f;
                matrix->sub0->m[3] = 0.0f;
                matrix->sub0->m[4] = 1.0f;
            } else {
                applyParentScale = 1;
                matrix->m[0] = matrix->sub1->m[0];
                matrix->m[1] = matrix->sub1->m[1];
                matrix->m[3] = matrix->sub1->m[3];
                matrix->m[4] = matrix->sub1->m[4];
                matrix->sub0->m[4] = 1.0f;
                parentScaleX = matrix->sub0->m[0];
            }
            break;
        case ELEMENT_FLAG_0x800:
            if (!(flags & ELEMENT_FLAG_0x2)) {
                matrix->m[0] = 1.0f;
                matrix->m[1] = 0.0f;
                matrix->m[3] = 0.0f;
                matrix->sub0->m[0] = 1.0f;
                matrix->sub0->m[1] = 0.0f;
                matrix->sub0->m[3] = 0.0f;
            } else {
                applyParentScale = 1;
                matrix->m[0] = matrix->sub1->m[0];
                matrix->m[1] = matrix->sub1->m[1];
                matrix->m[3] = matrix->sub1->m[3];
                matrix->m[4] = matrix->sub1->m[4];
                matrix->sub0->m[0] = 1.0f;
                parentScaleY = matrix->sub0->m[4];
            }
            break;
        case 0:
            if (!(flags & ELEMENT_FLAG_0x2)) {
                matrix->m[0] = 1.0f;
                matrix->m[1] = 0.0f;
                matrix->m[3] = 0.0f;
                matrix->m[4] = 1.0f;
                matrix->sub0->m[0] = 1.0f;
                matrix->sub0->m[1] = 0.0f;
                matrix->sub0->m[3] = 0.0f;
                matrix->sub0->m[4] = 1.0f;
            } else {
                matrix->m[0] = matrix->sub1->m[0];
                matrix->m[1] = matrix->sub1->m[1];
                matrix->m[3] = matrix->sub1->m[3];
                matrix->m[4] = matrix->sub1->m[4];
                matrix->sub0->m[0] = 1.0f;
                matrix->sub0->m[1] = 0.0f;
                matrix->sub0->m[3] = 0.0f;
                matrix->sub0->m[4] = 1.0f;
            }
            break;
        }

        UseTopMatrixAsSource();
        RotateMatrix(entry->channels.named.rotation);
        ScaleMatrix(entry->channels.named.scale.x, entry->channels.named.scale.y);
        if (applyParentScale) {
            ScaleMatrix(parentScaleX, parentScaleY);
        }
        ApplyScreenAspect(entry, scene);
    }

    if (entry->flags & ENTRY_HIDDEN_CHANGED) {
        entry->field_0x20 = entry->channels.named.hidden;
        if (parent != NULL && (flags & ELEMENT_COMBINE_HIDDEN)) {
            entry->field_0x20 = entry->channels.named.hidden | parent->field_0x20;
        }
    }

    if (entry->flags & ENTRY_TINT_CHANGED) {
        entry->field_0x24 = entry->channels.named.tint;
        if (parent != NULL && (flags & ELEMENT_COMBINE_TINT)) {
            ColorBytes own;
            ColorBytes inherited;
            ColorBytes combined;

            own.packed = entry->channels.named.tint;
            inherited.packed = parent->field_0x24;
            for (int i = 0; i < 4; i++) {
                combined.channel[i] = own.channel[i] * inherited.channel[i] / 255;
            }
            entry->field_0x24 = combined.packed;
        }
        TintEntryColors(entry);
    }
}

/* Sets the colour of the four corners of every quad of an entry to the entry's corner colours.
 * Entries without textures are read with a 0xC vertex stride (PlainVertex, inferred).
 * UpdateEntry expands this function in place; the out-of-line copy remains because other code
 * calls it. */
#pragma auto_inline on
void SetEntryColors(MotionPaletteEntry *entry) {
    if (entry->textures != NULL) {
        for (int quad = 0; quad < entry->quadCount; quad++) {
            for (int corner = 0; corner < 4; corner++) {
                entry->vertices[quad * 4 + corner].color = entry->channels.named.colors[corner];
            }
        }
    } else {
        for (int quad = 0; quad < entry->quadCount; quad++) {
            for (int corner = 0; corner < 4; corner++) {
                reinterpret_cast<PlainVertex *>(entry->vertices)[quad * 4 + corner].color =
                    entry->channels.named.colors[corner];
            }
        }
    }
}
#pragma auto_inline reset

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
        PlainVertex *vertices = reinterpret_cast<PlainVertex *>(entry->vertices);
        PlainVertex *drawVertices = reinterpret_cast<PlainVertex *>(entry->drawVertices);
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

void ApplyScreenAspect(MotionPaletteEntry *entry, const SceneData *scene) {
    f32 aspect = 1.3333f;
    if (scene->version >= 1) {
        aspect = scene->aspect;
    }
    aspect /= GetScreenAspectB();

    f32 offsetX;
    f32 offsetY;
    GetScaledScreenOffsetA(&offsetX, &offsetY);

    if (entry->textures != NULL) {
        Vertex *vertices = entry->vertices;
        Vertex *drawVertices = entry->drawVertices;
        int vertex = 0;
        for (int quad = 0; quad < entry->quadCount; quad++) {
            for (int corner = 0; corner < 4; corner++) {
                vertices[vertex].y /= aspect;
                TransformPoint(&drawVertices[vertex].x, &entry->matrix, &vertices[vertex].x);
                drawVertices[vertex].y *= aspect;
                vertices[vertex].y *= aspect;
                drawVertices[vertex].x += offsetX;
                drawVertices[vertex].y += offsetY;
                vertex++;
            }
        }
    } else {
        PlainVertex *vertices = reinterpret_cast<PlainVertex *>(entry->vertices);
        PlainVertex *drawVertices = reinterpret_cast<PlainVertex *>(entry->drawVertices);
        int vertex = 0;
        for (int quad = 0; quad < entry->quadCount; quad++) {
            for (int corner = 0; corner < 4; corner++) {
                vertices[vertex].y /= aspect;
                TransformPoint(&drawVertices[vertex].x, &entry->matrix, &vertices[vertex].x);
                drawVertices[vertex].y *= aspect;
                vertices[vertex].y *= aspect;
                drawVertices[vertex].x += offsetX;
                drawVertices[vertex].y += offsetY;
                vertex++;
            }
        }
    }
}

/* Sets the texture region of a type 1 element's quad from the entry's pattern: the glyph of the
 * pattern, or the blend of the glyphs of two patterns, with the element's flips applied. The quad
 * is given the layout of a textured quad if it was not one. */
void SetEntryPattern(MotionPaletteEntry *entry, const ElementData *element,
                     const SceneData *scene) {
    if (element->type != 1) {
        return;
    }

    int pattern;
    int nextPattern;
    f32 blend;
    const GlyphData *glyph = NULL;
    TexCoord texCoords[2];

    UnpackPatternBlend(&pattern, &nextPattern, &blend, entry->channels.named.pattern);
    if (pattern >= 0 && pattern < element->patternCount && element->patterns[pattern] >= 0) {
        glyph = &scene->glyphs[element->patterns[pattern]];
        if (blend != 0.0f || nextPattern < element->patternCount) {
            const GlyphData *next = &scene->glyphs[element->patterns[nextPattern]];

            for (int i = 0; i < 2; i++) {
                texCoords[i].u = glyph->corner[i].u * (1.0f - blend) + next->corner[i].u * blend;
                texCoords[i].v = glyph->corner[i].v * (1.0f - blend) + next->corner[i].v * blend;
            }
        } else {
            texCoords[0].u = glyph->corner[0].u;
            texCoords[0].v = glyph->corner[0].v;
            texCoords[1].u = glyph->corner[1].u;
            texCoords[1].v = glyph->corner[1].v;
        }
    }
    if (glyph != NULL) {
        if (element->fn_80361708()) {
            f32 swap = texCoords[0].u;
            texCoords[0].u = texCoords[1].u;
            texCoords[1].u = swap;
        }
        if (element->fn_803616FC()) {
            f32 swap = texCoords[0].v;
            texCoords[0].v = texCoords[1].v;
            texCoords[1].v = swap;
        }

        if (entry->vertices == NULL) {
            entry->vertices = entry->quadVertices;
            entry->drawVertices = entry->quadDrawVertices;
            entry->textures = &entry->quadTexture;
            entry->ownsVertices = 0;
            entry->quadCount = 1;
        } else if (entry->textures == NULL) {
            /* The quad has the layout of a quad without textures: move its vertices. */
            PlainVertex drawCopy[4];
            PlainVertex vertexCopy[4];
            PlainVertex *vertices = reinterpret_cast<PlainVertex *>(entry->vertices);
            PlainVertex *drawVertices = reinterpret_cast<PlainVertex *>(entry->drawVertices);

            for (int i = 0; i < 4; i++) {
                vertexCopy[i] = vertices[i];
                drawCopy[i] = drawVertices[i];
            }
            if (entry->ownsVertices) {
                Free(entry->vertices);
                Free(entry->drawVertices);
            }
            entry->vertices = entry->quadVertices;
            entry->drawVertices = entry->quadDrawVertices;
            entry->textures = &entry->quadTexture;
            for (int i = 0; i < 4; i++) {
                entry->vertices[i].x = vertexCopy[i].x;
                entry->vertices[i].y = vertexCopy[i].y;
                entry->vertices[i].color = vertexCopy[i].color;
                entry->drawVertices[i].x = drawCopy[i].x;
                entry->drawVertices[i].y = drawCopy[i].y;
                entry->drawVertices[i].color = drawCopy[i].color;
            }
            entry->ownsVertices = 0;
        }

        *entry->textures = glyph->texture;
        entry->vertices[0].u = texCoords[0].u;
        entry->vertices[0].v = texCoords[0].v;
        entry->vertices[1].u = texCoords[0].u;
        entry->vertices[1].v = texCoords[1].v;
        entry->vertices[2].u = texCoords[1].u;
        entry->vertices[2].v = texCoords[0].v;
        entry->vertices[3].u = texCoords[1].u;
        entry->vertices[3].v = texCoords[1].v;
        for (int i = 0; i < 4; i++) {
            entry->drawVertices[i].u = entry->vertices[i].u;
            entry->drawVertices[i].v = entry->vertices[i].v;
        }
    } else if (entry->vertices == NULL) {
        entry->vertices = entry->quadVertices;
        entry->drawVertices = entry->quadDrawVertices;
        entry->textures = NULL;
        entry->ownsVertices = 0;
        entry->quadCount = 1;
    } else if (entry->textures != NULL) {
        /* The quad has the layout of a textured quad: move its vertices. */
        Vertex drawCopy[4];
        Vertex vertexCopy[4];

        if (entry->ownsVertices) {
            Free(entry->textures);
        }
        entry->textures = NULL;
        for (int i = 0; i < 4; i++) {
            vertexCopy[i] = entry->vertices[i];
            drawCopy[i] = entry->drawVertices[i];
        }
        if (entry->ownsVertices) {
            Free(entry->vertices);
            Free(entry->drawVertices);
        }
        entry->vertices = entry->quadVertices;
        entry->drawVertices = entry->quadDrawVertices;
        PlainVertex *vertices = reinterpret_cast<PlainVertex *>(entry->vertices);
        PlainVertex *drawVertices = reinterpret_cast<PlainVertex *>(entry->drawVertices);
        for (int i = 0; i < 4; i++) {
            vertices[i].x = vertexCopy[i].x;
            vertices[i].y = vertexCopy[i].y;
            vertices[i].color = vertexCopy[i].color;
            drawVertices[i].x = drawCopy[i].x;
            drawVertices[i].y = drawCopy[i].y;
            drawVertices[i].color = drawCopy[i].color;
        }
        entry->ownsVertices = 0;
    }
}

void EvaluateTrack(u32 *out, const KeyTrack *track, int channel, f32 frame, f32 speed,
                   int *running) {
    const Keyframe *first;
    const Keyframe *second;

    if (track->keys[track->keyCount - 1].frame <= frame) {
        if (track->loopMode == KEYTRACK_LOOP) {
            if (track->keys[track->keyCount - 1].frame == 0) {
                if (speed > 0.0f) {
                    *running = 0;
                }
            } else {
                frame = fmodf(frame, track->keys[track->keyCount - 1].frame);
            }
        } else {
            if (speed > 0.0f) {
                *running = 0;
            }
            const Keyframe *last = &track->keys[track->keyCount - 1];
            *out = last->value.bits;
            return;
        }
    }

    if (track->keys[0].frame > frame) {
        if (speed < 0.0f) {
            *running = 0;
        }
        *out = track->keys[0].value.bits;
        return;
    }

    first = NULL;
    second = NULL;
    for (int i = 0; i < track->keyCount; i++) {
        first = second;
        second = &track->keys[i];
        if (second->frame > frame) {
            break;
        }
    }
    if (first == NULL) {
        first = second;
    }

    switch (channel) {
    case 0:
    case 12:
    case 13:
    case 14:
        HoldKeyValue(out, first, second, frame);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        InterpolateKeys(out, first, second, frame);
        break;
    case 6: {
        KeyValue interpolated;
        InterpolateKeys(&interpolated.bits, first, second, frame);

        f32 blend;
        if (second->value.number == first->value.number) {
            blend = 0.0f;
        } else {
            blend = (interpolated.number - first->value.number) /
                    (second->value.number - first->value.number);
        }
        KeyValue packed;
        packed.number = PackPatternBlend(static_cast<int>(first->value.number),
                                         static_cast<int>(second->value.number), blend);
        *out = packed.bits;
        break;
    }
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
        InterpolateColor(out, first, second, frame);
        break;
    default:
        HoldKeyValue(out, first, second, frame);
        break;
    }
}

/* The value at frame, interpolated between two keys by the first key's interpolation. */
static void InterpolateKeys(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame) {
    switch (first->interpolation) {
    case KEY_HOLD:
        HoldKeyValue(out, first, second, frame);
        break;
    case KEY_LINEAR:
        InterpolateLinear(out, first, second, frame);
        break;
    case KEY_HERMITE:
        InterpolateHermite(out, first, second, frame);
        break;
    default:
        HoldKeyValue(out, first, second, frame);
        break;
    }
}

/* The value of the first key, unchanged. The second key and the frame are unused: EvaluateTrack
 * passes the second key here as it does to the other interpolations. */
static void HoldKeyValue(u32 *out, const Keyframe *first, const Keyframe *second, f32 frame) {
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

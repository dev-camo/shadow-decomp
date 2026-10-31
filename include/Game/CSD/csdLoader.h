#ifndef GAME_CSD_CSDLOADER_H
#define GAME_CSD_CSDLOADER_H

#include <dolphin/types.h>
#include "Game/CSD/CTexList.h"
#include "Game/CSD/csdAccess.h"
#include "layout_check.h"

/* Functions and records of csdLoader.cpp: reading the project file and its container, and the
 * keyframe and colour helpers. Names are inferred from that code. */
namespace Chao {
namespace CSD {

/* How the value of a key changes towards the next key (names inferred from the interpolation
 * functions of csdLoader.cpp; other values hold the key's value). */
enum KeyInterpolation { KEY_HOLD = 0, KEY_LINEAR = 1, KEY_HERMITE = 2 };

/* The value of a key: a float, or the bits of an integer or of four colour bytes (the same value
 * as the integer or float of csdAccess.cpp's ChannelValue). */
union KeyValue {
    u32 bits;
    f32 number;
};
CHECK_SIZEOF(Chao::CSD::KeyValue, 0x4);

/* A key of an animation track. Only KEY_HERMITE keys use the tangents of the segment that starts at
 * the key (names inferred). */
struct Keyframe {
    /* 0x00 */ s32 frame;
    /* 0x04 */ KeyValue value;
    /* 0x08 */ s32 interpolation;
    /* 0x0C */ f32 startTangent;
    /* 0x10 */ f32 endTangent;
    /* 0x14 */ u32 field_0x14;
};
CHECK_SIZEOF(Chao::CSD::Keyframe, 0x18);

/* The keys of one animation channel, in frame order. Names are inferred from EvaluateTrack. */
struct KeyTrack {
    /* 0x00 */ s32 loopMode;
    /* 0x04 */ s32 keyCount;
    /* 0x08 */ Keyframe *keys;
};
CHECK_SIZEOF(Chao::CSD::KeyTrack, 0xC);

/* The loop mode of a track that repeats (name inferred from EvaluateTrack; other modes stop at
 * the last key). */
enum KeyTrackLoopMode { KEYTRACK_LOOP = 2 };

/* The keys of the channels of one element, for the channels of a mask: one track for each bit of
 * the mask, in channel order (names inferred from UpdateEntry and EvaluateChannels). */
struct ChannelAnimation {
    /* 0x00 */ u32 channelMask;
    /* 0x04 */ KeyTrack *tracks;
};
CHECK_SIZEOF(Chao::CSD::ChannelAnimation, 0x8);

/* The animation of a scene's layer: one ChannelAnimation for each element. */
struct LayerMotion {
    /* 0x00 */ u32 field_0x0;
    /* 0x04 */ ChannelAnimation *entries;
};
CHECK_SIZEOF(Chao::CSD::LayerMotion, 0x8);

/* The animation of a scene: one LayerMotion for each layer. */
struct SceneMotion {
    /* 0x00 */ u32 field_0x0;
    /* 0x04 */ LayerMotion *layers;
};
CHECK_SIZEOF(Chao::CSD::SceneMotion, 0x8);

/* A frame time of a pattern (sprite frame) animation packs two pattern indices and the blend
 * between them: the integer part is the index of the first pattern plus 10000 times the index of
 * the second, the fraction is the weight of the second. A first index above 10000 is an error
 * (PackPatternBlend reports "Large PatternIndex Error." and hangs). Names are inferred from the
 * error text and from how csdLoader.cpp blends the texture coordinates of the two patterns. */
void UnpackPatternBlend(int *pattern, int *nextPattern, f32 *blend, f32 packed);
f32 PackPatternBlend(int pattern, int nextPattern, f32 blend);

/* Splits the contents of a project file into its two parts, each in a new buffer from the
 * library's allocator (the second is null, with size 0, when the file has only one part).
 * Files starting with the signature "CPAF" or its byte-reversed form "FAPC" have a size word
 * before each part, byte-reversed in the second form; any other file is the first part as is. */
void ExtractFileParts(void **part1, u32 *size1, void **part2, u32 *size2, const u32 *file,
                      u32 fileSize);

/* Resolves the pointers of a project file's contents and stores the project data in *project, which
 * is left null when the file is not a project file (names are inferred). */
void ResolveProject(ProjectData **project, void *file);

/* Sets the vertex colours of an entry's quads to its corner colours, and the draw colours to the
 * vertex colours scaled by the entry's colour field (called by code outside csdLoader.cpp). */
void SetEntryColors(MotionPaletteEntry *entry);
void TintEntryColors(MotionPaletteEntry *entry);

/* Draws a scene's elements through the render state, layer by layer, element by element. The
 * texture list supplies the textures of the elements. The unused argument is passed along to
 * DrawTextElement, which ignores it (its type is unknown). Names are inferred. */
void DrawScene(const SceneData *scene, const RCPtr<CTexList> &texList, void *unused,
               const MotionPalette *palette);
void DrawLayer(const SceneData *scene, const LayerData *layer, const RCPtr<CTexList> &texList,
               void *unused, const MotionPaletteLayer *paletteLayer);
/* Draws an element of type 1 (a quad) or 2 (text) when the element's field_0x8 and the entry's
 * field_0x140 are nonzero and the entry's field_0x20 is zero. */
void DrawElement(const SceneData *scene, const ElementData *element, const RCPtr<CTexList> &texList,
                 void *unused, MotionPaletteEntry *entry);
void DrawQuadElement(const SceneData *scene, const ElementData *element,
                     const RCPtr<CTexList> &texList, MotionPaletteEntry *entry);
void DrawTextElement(const SceneData *scene, const ElementData *element,
                     const RCPtr<CTexList> &texList, void *unused, MotionPaletteEntry *entry);

/* Returns the index the scene's name table gives the name, or -1 (name inferred; the table's
 * meaning is not known). */
int FindSceneNameIndex(const SceneData *scene, const char *name);

/* Sets the texture region of a type 1 element's quad from the entry's pattern (called by code
 * outside csdLoader.cpp; name inferred). */
void SetEntryPattern(MotionPaletteEntry *entry, const ElementData *element, const SceneData *scene);

/* Rescales an entry's vertices for the screen: each is transformed by the entry's matrix into the
 * drawing vertices, with the y coordinates adjusted by the scene's aspect ratio relative to the
 * screen's and the whole offset by the screen's scaled offset (name inferred). */
void ApplyScreenAspect(MotionPaletteEntry *entry, const SceneData *scene);

/* Evaluates a channel's track at a frame and stores the raw value in *out: the last or first
 * key's value outside the keys, otherwise what the channel number selects (channels 1 to 5
 * interpolate by the key's interpolation, 6 blends two pattern indices, 7 to 11 interpolate
 * colours; others hold the key's value). A looping track wraps the frame. *running is cleared
 * when the frame is at or past the last key of a non-looping (or zero-length looping) track while
 * the speed is positive, or before the first key while it is negative (names inferred). */
void EvaluateTrack(u32 *out, const KeyTrack *track, int channel, f32 frame, f32 speed,
                   int *running);

/* Brings a palette's entries to the state of a frame of the scene's animation. The motion may be
 * null (the channels then come from the entry's own values or the element's values), the parent
 * entry is the one the scene is drawn under. The palette remembers its last frame, parent, motion
 * and fonts and skips the update when nothing changed (names inferred). */
void UpdatePalette(MotionPalette *palette, const SceneData *scene, const SceneMotion *motion,
                   f32 frame, const FontList *fonts, MotionPaletteEntry *parent);

/* ResolveProject, then NormalizeProject on the result. */
void LoadProject(ProjectData **project, void *file);

/* Completes the flags of the elements of a project's scenes. */
void NormalizeProject(ProjectData *project);

} // namespace CSD
} // namespace Chao

#endif

#ifndef GAME_CSD_CSDLOADER_H
#define GAME_CSD_CSDLOADER_H

#include <dolphin/types.h>
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

/* ResolveProject, then NormalizeProject on the result. */
void LoadProject(ProjectData **project, void *file);

/* Completes the flags of the elements of a project's scenes. */
void NormalizeProject(ProjectData *project);

} // namespace CSD
} // namespace Chao

#endif

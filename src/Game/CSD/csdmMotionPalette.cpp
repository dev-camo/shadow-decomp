/* csdmMotionPalette.cpp (original name from the library's allocation calls): the palette of a
 * scene's runtime entries as an object (csdm.h). Class and function names are inferred.
 * Definitions are in reverse address order (deferred inlining emits the last definition first). */
#include "Game/CSD/csdm.h"
#include <new>

namespace Chao {
namespace CSD {

/* Releases a palette's block: its entries, then the block itself. */
static void ReleasePaletteBlock(unsigned char *block) {
    ReleasePalette(reinterpret_cast<MotionPalette *>(block));
    if (block != NULL) {
        Free(block);
    }
}

void CMotionPaletteRef::Clear() {
    m_ref.Reset(NULL);
    m_palette = NULL;
}

CMotionPaletteRef::CMotionPaletteRef() {
    Clear();
}

CMotionPalette::CMotionPalette(const SceneData *scene, const FontList *fonts) {
    m_palette = static_cast<MotionPalette *>(Alloc(GetPaletteSize(scene), __FILE__, __LINE__));
    m_ref.Reset(reinterpret_cast<unsigned char *>(m_palette), ReleasePaletteBlock);
    InitPalette(m_palette, scene, fonts);
}

void CMotionPattern::Attach(MotionPaletteEntry *entry, const RCPtr<unsigned char> &ref) {
    Clear();
    m_ref = ref;
    m_entry = entry;
}

CMotionPattern::CMotionPattern(MotionPaletteEntry *entry, const RCPtr<unsigned char> &ref) {
    Attach(entry, ref);
}

CMotionPattern *CMotionPalette::GetPattern(CMotionPattern *result, const ElementRef &element) {
    MotionPaletteEntry *entry = &m_palette->layers[element.layer].entries[element.element];

    return ::new (result) CMotionPattern(entry, m_ref);
}

void CMotionPalette::Rebuild(const CSceneDataRef &scene, const CFontListRef &fonts) {
    MotionPalette *palette = m_palette;

    if (palette->matrixPool != NULL) {
        Free(palette->matrixPool);
        palette->matrixPool = NULL;
    }

    int matrixCount = CountPaletteMatrices(palette, scene.m_scene);
    if (matrixCount > 0) {
        palette->matrixPool = static_cast<MatrixValue *>(
            Alloc(matrixCount * sizeof(MatrixValue), __FILE__, __LINE__));
    }
    SetMatrixPool(palette->matrixPool, matrixCount);
    BuildPalette(palette, scene.m_scene, fonts.m_fonts);
    ClearMatrixPool();
}

} // namespace CSD
} // namespace Chao

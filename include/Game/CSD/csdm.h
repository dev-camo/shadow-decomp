#ifndef GAME_CSD_CSDM_H
#define GAME_CSD_CSDM_H

#include <dolphin/types.h>
#include "Game/CSD/MotionPalette.h"
#include "Game/CSD/csdBase.h"
#include "Game/CSD/csdRCObject.h"
#include "layout_check.h"

/* The classes of the csdm* files, the library's object layer over the records of MotionPalette.h.
 * The original headers' names are unknown except csdmProject.h; this header stands in for them.
 * The game's code carries no run-time type information for these classes, so every class and
 * member name here is inferred from what the code does. Virtual functions are declared and
 * defined elsewhere (csdmScene.cpp holds the vtables and the destructors of these classes);
 * members without a supported meaning keep their offset. */
namespace Chao {
namespace CSD {

/* A resource handle: a counted reference to a block and the block's address (inferred). The
 * handles of the library share this shape; each is its own class with its own virtual
 * destructor and Copy. */
class CMotionPaletteRef {
  public:
    CMotionPaletteRef();
    virtual ~CMotionPaletteRef();
    /* Takes over the other handle's reference and block. */
    virtual void Copy(const CMotionPaletteRef &other);

    /* Drops the reference and the block. */
    void Clear();

    /* 0x04 */ RCPtr<unsigned char> m_ref;
    /* 0x08 */ MotionPalette *m_palette;
};
CHECK_SIZEOF(Chao::CSD::CMotionPaletteRef, 0xC);

/* The handle of a scene's file data (inferred). */
class CSceneDataRef {
  public:
    virtual ~CSceneDataRef();
    virtual void Copy(const CSceneDataRef &other);

    /* 0x04 */ RCPtr<unsigned char> m_ref;
    /* 0x08 */ const SceneData *m_scene;
};
CHECK_SIZEOF(Chao::CSD::CSceneDataRef, 0xC);

/* The handle of a project's fonts (inferred). */
class CFontListRef {
  public:
    virtual ~CFontListRef();
    virtual void Copy(const CFontListRef &other);

    /* 0x04 */ RCPtr<unsigned char> m_ref;
    /* 0x08 */ const FontList *m_fonts;
};
CHECK_SIZEOF(Chao::CSD::CFontListRef, 0xC);

/* A reference to an element of a scene: the layer and the element within it (inferred from the
 * use as an index of the palette's layers and entries). */
struct ElementRef {
    /* 0x00 */ s32 field_0x0;
    /* 0x04 */ s32 layer;
    /* 0x08 */ s32 element;
};
CHECK_SIZEOF(Chao::CSD::ElementRef, 0xC);

/* The handle of an element's runtime entry (inferred). */
class CMotionPatternRef {
  public:
    CMotionPatternRef();
    virtual ~CMotionPatternRef();
    virtual void Copy(const CMotionPatternRef &other);

    /* Drops the reference and the entry. */
    void Clear();

    /* 0x04 */ RCPtr<unsigned char> m_ref;
    /* 0x08 */ MotionPaletteEntry *m_entry;
};
CHECK_SIZEOF(Chao::CSD::CMotionPatternRef, 0xC);

/* The entry of an element with the accessors that override its animated values (class name
 * inferred from csdmMotionPattern.cpp). The CBase base is inferred: the game's deleting
 * destructor of CMotionPattern (0x80365C6C in the USA release) frees the object through the
 * current allocator's FreeObject, while that of CMotionPatternRef uses the global operator
 * delete. */
class CMotionPattern : public CMotionPatternRef, public CBase {
  public:
    /* Wraps the entry of an element of a palette; ref counts the palette's block. */
    CMotionPattern(MotionPaletteEntry *entry, const RCPtr<unsigned char> &ref);
    virtual ~CMotionPattern() {}
    virtual void Copy(const CMotionPatternRef &other);

    /* Takes the entry and the reference to its block (name inferred). */
    void Attach(MotionPaletteEntry *entry, const RCPtr<unsigned char> &ref);

    /* 0x0C */ void *field_0xC;
    /* 0x10 */ f32 field_0x10;
    /* 0x14 */ RCPtr<MatrixValue> m_pool;
};
CHECK_SIZEOF(Chao::CSD::CMotionPattern, 0x18);

/* The palette of a scene's runtime entries, built from the scene's file data. As for
 * CMotionPattern, the CBase base is inferred from the deleting destructor (0x8036706C in the
 * USA release), which frees through FreeObject where CMotionPaletteRef's uses the global
 * operator delete. */
class CMotionPalette : public CMotionPaletteRef, public CBase {
  public:
    CMotionPalette(const SceneData *scene, const FontList *fonts);
    virtual ~CMotionPalette();

    /* Builds the entries again for another scene and fonts, reusing the palette's block (name
     * inferred). */
    void Rebuild(const CSceneDataRef &scene, const CFontListRef &fonts);

    /* Constructs the entry of an element as a CMotionPattern in result and returns it (name
     * inferred). */
    CMotionPattern *GetPattern(CMotionPattern *result, const ElementRef &element);
};
CHECK_SIZEOF(Chao::CSD::CMotionPalette, 0xC);

} // namespace CSD
} // namespace Chao

#endif

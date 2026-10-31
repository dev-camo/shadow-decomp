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
 * member name here is inferred from what the code does. The out-of-line virtual functions are
 * defined in csdmScene.cpp, except CMotionPattern's Copy, which is in csdmMotionPattern.cpp with
 * the class's vtable. CMotionPattern's destructor is inline; the game's copy of it is the one in
 * csdmScene.cpp. Members without a supported meaning keep their offset. */
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
    CSceneDataRef();
    virtual ~CSceneDataRef();
    virtual void Copy(const CSceneDataRef &other);

    /* Drops the reference and the data. */
    void Clear();

    /* Takes the data and the reference that counts its block. */
    void Attach(const SceneData *scene, const RCPtr<unsigned char> &ref);

    /* The scene's aspect ratio (name inferred; 1.3333 for data without one). */
    f32 GetAspect() const;

    /* 0x04 */ RCPtr<unsigned char> m_ref;
    /* 0x08 */ const SceneData *m_scene;
};
CHECK_SIZEOF(Chao::CSD::CSceneDataRef, 0xC);

/* The handle of a project's fonts (inferred). */
class CFontListRef {
  public:
    CFontListRef();
    virtual ~CFontListRef();
    virtual void Copy(const CFontListRef &other);

    /* Drops the reference and the fonts. */
    void Clear();

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

/* The node an element belongs to (class name inferred from csdmNode.cpp). Only the members that
 * CMotionPattern reads are known. */
class CNode {
  public:
    /* 0x00 */ u8 field_0x0[0x8];
    /* 0x08 */ const ElementData *m_element;
    /* 0x0C */ u8 field_0xC[0x28];
    /* 0x34 */ CSceneDataRef *m_scene;
};
CHECK_OFFSETOF(Chao::CSD::CNode, m_element, 0x8);
CHECK_OFFSETOF(Chao::CSD::CNode, m_scene, 0x34);

/* The defaults a node's element has for a channel, read when neither an override nor a computed
 * value exists (placeholder name: the function is in csdmNode.cpp). */
void fn_8036ADC0(ChannelWord *value, const CNode *node, int channel);

/* Makes a Vector2 (placeholder name: the function is in the unit before csdAccess.cpp). */
Vector2 fn_8035BA0C(f32 x, f32 y);

/* The entry of an element with the accessors that override its animated values (class name
 * inferred from csdmMotionPattern.cpp). An override replaces the channel's computed value until
 * it is cleared: the entry's overrides block holds the values and its mask says which exist.
 * The CBase base is inferred: the game's deleting destructor of CMotionPattern (0x80365C6C in
 * the USA release) frees the object through the current allocator's FreeObject, while that of
 * CMotionPatternRef uses the global operator delete. */
class CMotionPattern : public CMotionPatternRef, public CBase {
  public:
    /* A new entry, reset to the state of a newly created element (aspect is the scene's). */
    CMotionPattern(f32 aspect);
    /* Wraps the entry of an element of a palette; ref counts the palette's block. */
    CMotionPattern(MotionPaletteEntry *entry, const RCPtr<unsigned char> &ref);
    /* A copy of another handle of an entry (the entry and its reference; name inferred). */
    explicit CMotionPattern(const CMotionPatternRef &other);
    virtual ~CMotionPattern() {}
    virtual void Copy(const CMotionPatternRef &other);

    /* Takes the entry and the reference to its block (name inferred). */
    void Attach(MotionPaletteEntry *entry, const RCPtr<unsigned char> &ref);

    /* Stores the entry's own values for the hidden flag, the translation and the scale (names
     * inferred): unlike the setters below, they do not override the channel but replace the
     * entry's current value, and mark the entry's channels as needing an update. */
    void SetEntryHidden(s32 hidden);
    void SetEntryTranslation(f32 x, f32 y);
    void SetEntryScale(f32 x, f32 y);

    /* Brings an entry without quads up to date: its hidden flag, tint and matrix, for the
     * screen as it is now (name inferred). */
    void UpdateTransform();

    /* The accessors (names inferred). A getter returns the override, the entry's computed value
     * or the element's default, in that order. */
    void SetHidden(s32 hidden);
    void ClearHidden();
    s32 GetHidden();
    void SetTranslation(f32 x, f32 y);
    void SetTranslateX(f32 x);
    void SetTranslateY(f32 y);
    Vector2 GetTranslation();
    void SetRotation(f32 rotation);
    f32 GetRotation();
    void SetScaleX(f32 scaleX);
    void SetScaleY(f32 scaleY);
    Vector2 GetScale();
    void SetPattern(f32 pattern);
    f32 GetPattern();
    void SetTint(const u32 &tint);
    void ClearTint();
    u32 GetTint();
    void SetColor(int corner, const u32 &color);
    void ClearColor(int corner);
    u32 GetColor(int corner);
    void SetValue(int index, s32 value);
    s32 GetValue(int index);

    /* 0x0C */ CNode *m_node;
    /* 0x10 */ f32 m_aspect;
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

#ifndef GAME_CSD_CSDMSCENE_H
#define GAME_CSD_CSDMSCENE_H

#include "Game/CSD/csdm.h"

/* The classes of csdmScene.cpp, built on the handles of csdm.h (class names and members inferred;
 * this library's GameCube objects carry no run-time type information). Each handle comes with an
 * owning class, as CMotionPaletteRef comes with CMotionPalette: the owning class derives from
 * CBase, so its deleting destructor frees the object through the current allocator. */
namespace Chao {
namespace CSD {

/* The handle of one motion of a scene (the table SceneData::motions points to). */
class CSceneMotionRef {
  public:
    CSceneMotionRef();
    virtual ~CSceneMotionRef();
    virtual void Copy(const CSceneMotionRef &other);

    /* Drops the reference and the motion. */
    void Clear();

    /* 0x04 */ RCPtr<unsigned char> m_ref;
    /* 0x08 */ const SceneMotion *m_motion;
};
CHECK_SIZEOF(Chao::CSD::CSceneMotionRef, 0xC);

class CSceneMotion : public CSceneMotionRef, public CBase {
  public:
    /* A copy of another handle (the motion and its reference). */
    explicit CSceneMotion(const CSceneMotionRef &other);
    /* A handle of the motion; ref counts the motion's block. */
    CSceneMotion(const SceneMotion *motion, const RCPtr<unsigned char> &ref);
    virtual ~CSceneMotion();

    /* Takes the motion and the reference that counts its block (name inferred). */
    void Attach(const SceneMotion *motion, const RCPtr<unsigned char> &ref);
};
CHECK_SIZEOF(Chao::CSD::CSceneMotion, 0xC);

/* A copy of a handle of the fonts of a project. */
class CFontList : public CFontListRef, public CBase {
  public:
    explicit CFontList(const CFontListRef &other);
    virtual ~CFontList();
};
CHECK_SIZEOF(Chao::CSD::CFontList, 0xC);

/* The three objects a scene creates in the block it allocates for them (placed with new, destroyed
 * through the destructor; the scene keeps pointers to them, so they come and go as the scene's
 * fonts, palette and motion are set). */
struct SceneObjects {
    /* 0x00 */ CFontList fonts;
    /* 0x0C */ CMotionPalette palette;
    /* 0x18 */ CSceneMotion motion;
};
CHECK_SIZEOF(Chao::CSD::SceneObjects, 0x24);

/* A scene: its file data (the CSceneDataRef base), a palette of runtime entries built from it and
 * the playback of one of its motions. Only the members that csdmScene.cpp uses have names; the
 * base class that makes the scene a listener source (a second vtable pointer at 0xC), its listener
 * lists, the root pattern and the playback policy are not modelled yet. */
class CScene : public CSceneDataRef {
  public:
    /* The first and last frame of a motion (by name or by its index in the scene's data); a name
     * the scene does not have, or data older than version 2, gives the frame range of the motion
     * that is set (names inferred). */
    f32 GetMotionStart(const char *name);
    f32 GetMotionStart(int index) const;
    f32 GetMotionEnd(const char *name);
    f32 GetMotionEnd(int index) const;

    /* The pattern of the scene's own element, created when first needed (name inferred; its
     * definition is not sourced yet). */
    CMotionPattern *GetRootPattern();

    /* Set the root pattern's hidden flag, translation (in screen units) and scale (names
     * inferred). */
    void SetHidden(s32 hidden);
    void SetTranslation(f32 x, f32 y);
    void SetTranslation(const Vector2 &translation);
    void SetScale(f32 x, f32 y);

    /* Builds the palette again from the scene's data and fonts. */
    void RebuildPalette();

    /* Replaces the palette by a new one for the scene's data and fonts (name inferred). */
    void CreatePalette();

    /* The index of the scene's motion of that name, or -1 (name inferred). */
    int FindMotion(const char *name);

    /* Sets the motion by name (ignored when the scene has none of that name) or by index (ignored
     * when it is the current index or not below the motion count), and takes its frame range when
     * the data has one (names inferred). */
    void SetMotion(const char *name);
    void SetMotion(int index);

    /* The frame range of the motion that is set, the frame before the last update and the frame
     * set by SetFrame, which also becomes the previous one (names inferred). */
    f32 GetMotionStart() const;
    f32 GetMotionEnd() const;
    f32 GetLastFrame() const;
    void SetFrame(f32 frame);

    /* The speed the frame advances at (name inferred). */
    f32 GetSpeed() const;
    void SetSpeed(f32 speed);

    /* The motion plays while the scene is not paused (names inferred from the update, which
     * advances the frame only then). */
    void Resume();
    void Pause();

    /* 0x0C */ u8 field_0xC[0x58];
    /* 0x64 */ CSceneMotion *m_motion;
    /* 0x68 */ CFontList *m_fonts;
    /* 0x6C */ CMotionPalette *m_palette;
    /* 0x70 */ f32 m_lastFrame;
    /* 0x74 */ f32 m_frame;
    /* 0x78 */ f32 m_speed;
    /* 0x7C */ f32 m_startFrame;
    /* 0x80 */ f32 m_endFrame;
    /* 0x84 */ u8 field_0x84[0x4];
    /* 0x88 */ CMotionPattern *m_rootPattern;
    /* 0x8C */ s32 m_paused;
    /* 0x90 */ u8 field_0x90[0x18];
    /* 0xA8 */ s32 m_motionIndex;
    /* 0xAC */ f32 field_0xAC;
    /* 0xB0 */ u8 field_0xB0[0x1C];
    /* 0xCC */ SceneObjects *m_objects;
};
CHECK_OFFSETOF(Chao::CSD::CScene, m_motion, 0x64);
CHECK_OFFSETOF(Chao::CSD::CScene, m_endFrame, 0x80);
CHECK_OFFSETOF(Chao::CSD::CScene, m_paused, 0x8C);
CHECK_OFFSETOF(Chao::CSD::CScene, m_motionIndex, 0xA8);
CHECK_SIZEOF(Chao::CSD::CScene, 0xD0);

} // namespace CSD
} // namespace Chao

#endif

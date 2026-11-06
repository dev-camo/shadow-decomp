#ifndef GAME_CSD_CSDMSCENE_H
#define GAME_CSD_CSDMSCENE_H

#include "Game/CSD/csdm.h"
#include <map>

/* The csdmScene.cpp classes use handles from csdm.h (names and members remain working names where
 * evidence is absent; holder RTTI does not prove direct RTTI for each class). Each handle comes
 * with an owning class, as CMotionPaletteRef comes with CMotionPalette: it derives from CBase,
 * whose deleting destructor frees it through the current allocator. */
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
    /* The fonts of a project's file data; ref counts the block that holds them. */
    CFontList(const FontList *fonts, const RCPtr<unsigned char> &ref) {
        Attach(fonts, ref);
    }
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

class CScene;
class CProject;
class CTexList;

/* The listener's list-owning base, identified by its constructor and destructor (name unknown).
 * Scene pointers are non-owning back-references. Its pure destructor has an out-of-line body. */
class UnkSceneListenerBase_80365054 {
  public:
    UnkSceneListenerBase_80365054();
    virtual ~UnkSceneListenerBase_80365054() = 0;

    void fn_80367A70(CScene *scene);

  protected:
    /* 0x04 */ std::list<CScene *> m_scenes;
};
CHECK_SIZEOF(Chao::CSD::UnkSceneListenerBase_80365054, 0x10);

/* What a scene reports to its listeners (class name and event names inferred from the vtables
 * 0x8055FFB0 and 0x8055FF88): six event slots and a destructor that releases the listener from
 * the scenes it listens to. Persistent listeners share one list in CSceneListenerSource; the
 * scene's six callback lists hold a separate interface. The
 * first two slots report the end boundary at positive speed and the start boundary at negative
 * speed (from CMotionEndPolicy). The next two receive the scene before and after palette update;
 * the final two receive the scene around drawing. */
class CSceneListener : public UnkSceneListenerBase_80365054, public CBase {
  public:
    CSceneListener();
    virtual ~CSceneListener();
    virtual void OnReachEnd(CScene *scene) {}
    virtual void OnReachStart(CScene *scene) {}
    virtual void fn_8036F96C(CScene *scene) {}
    virtual void fn_8036F970(CScene *scene) {}
    virtual void fn_8036F974(CScene *scene) {}
    virtual void fn_8036F978(CScene *scene) {}

    /* Removes a scene back-reference without modifying that scene's listener list. */
    void fn_8036542C(CScene *scene);
};
CHECK_SIZEOF(Chao::CSD::CSceneListener, 0x10);

/* What a scene does when its motion reaches the first or the last frame (names inferred from the
 * handlers; every scene starts with MOTION_END_WRAP for both). */
enum MotionEndAction {
    MOTION_END_PAUSE = 0,   /* pause the scene */
    MOTION_END_WRAP = 1,    /* set both frame positions to the opposite boundary */
    MOTION_END_REVERSE = 2, /* reverse the direction of the playback */
    MOTION_END_REMOVE = 3   /* remove the scene from its project */
};

/* The scene's reaction to the ends of its motion (class name inferred). */
class CMotionEndPolicy : public CSceneListener {
  public:
    CMotionEndPolicy();
    virtual void OnReachEnd(CScene *scene);
    virtual void OnReachStart(CScene *scene);

    /* Copies the two boundary actions, leaving listener connections unchanged. */
    void fn_803676D4(const CMotionEndPolicy &other);

    /* 0x10 */ MotionEndAction m_endAction;
    /* 0x14 */ MotionEndAction m_startAction;
};
CHECK_SIZEOF(Chao::CSD::CMotionEndPolicy, 0x18);

/* The scene's second base and raw listener list (class and method names inferred). The native
 * destructor releases the list; its pure virtual declaration supplies the derived destructor slot. */
class CSceneListenerSource {
  public:
    CSceneListenerSource();
    virtual ~CSceneListenerSource() = 0;
    virtual CScene *GetSender();

    /* Invokes the selected listener member on each persistent listener. */
    void fn_80365478(void (CSceneListener::*event)(CScene *), CScene *scene);
    /* Removes both sides of the non-owning listener connection. */
    void fn_80365958(CSceneListener *listener);

    /* 0x04 */ std::list<CSceneListener *> m_listeners;
};
CHECK_SIZEOF(Chao::CSD::CSceneListenerSource, 0x10);

/* The callback interface held by the scene's reference-counted lists
 * (class and method names inferred).
 * The update calls the first virtual slot and removes callbacks returning false; the native
 * reference release invokes the deleting destructor in the following slot. Only this virtual
 * interface is recovered; the declaration is not used to allocate or copy callback objects. */
class CSceneCallback {
  public:
    virtual bool Invoke(CScene *scene) = 0;
    virtual ~CSceneCallback();
};
CHECK_SIZEOF(Chao::CSD::CSceneCallback, 0x4);

/* Key of the node cache: the two indices copied from ElementRef when creating or finding a node
 * (name inferred). The native comparator orders the signed layer, then the signed element. */
struct SceneNodeKey {
    /* 0x00 */ s32 layer;
    /* 0x04 */ s32 element;

    SceneNodeKey(s32 layerIndex, s32 elementIndex);
    SceneNodeKey(const SceneNodeKey &other);
};
CHECK_SIZEOF(Chao::CSD::SceneNodeKey, 0x8);

struct SceneNodeKeyCompare {
    bool operator()(const SceneNodeKey &left, const SceneNodeKey &right) const;
};
CHECK_SIZEOF(Chao::CSD::SceneNodeKeyCompare, 0x1);

/* A scene: its file data (the CSceneDataRef base), a palette of runtime entries built from it and
 * the playback of one of its motions. CSceneListenerSource is its second base at 0xC; the six
 * callback lists and node cache are constructed separately by the native scene constructor. */
class CScene : public CSceneDataRef, public CSceneListenerSource, public CBase {
  public:
    typedef std::list<RCPtr<CSceneCallback> > CallbackList;
    typedef std::map<SceneNodeKey, RCPtr<CNode>, SceneNodeKeyCompare> NodeMap;

    CScene();
    virtual ~CScene();
    /* other must be a CScene (inferred from the derived playback and handle fields read). */
    virtual void Copy(const CSceneDataRef &other);

    /* Frame update, drawing and sender conversion in their native virtual order. The update
     * applies boundary policies before updating the palette and advancing the frame. */
    virtual void fn_803669B8(f32 step);
    virtual void fn_803667A4(const RCPtr<CTexList> &textures);
    virtual CScene *GetSender();

    /* Initializes playback state and allocates the placement-object block when absent. */
    void fn_80367500();
    /* Attaches scene data, its project key and optional fonts (signature inferred). */
    void fn_803648DC(const SceneData *scene, const RCPtr<unsigned char> &ref, const char *key,
                     CProject *project, const CFontListRef *fonts);
    /* Replaces fonts in this scene and its cached nodes, then rebuilds with non-null fonts. */
    void fn_80364538(const CFontListRef *fonts);
    const char *fn_803674F8() const;
    f32 fn_80365424() const;
    /* Invokes the virtual update with the library's mutable default step. */
    void fn_80366F9C();
    /* Finds or creates the cached node for an element; null gives an empty reference. */
    RCPtr<CNode> fn_80366468(const ElementRef *elementRef);
    ElementRef fn_80366640(const char *name) const;
    RCPtr<CNode> fn_803666B4(const char *name);

    /* The first and last frame of a motion (by name or by its index in the scene's data); a name
     * the scene does not have, or data older than version 2, gives the frame range of the motion
     * that is set (names inferred). */
    f32 GetMotionStart(const char *name);
    f32 GetMotionStart(int index) const;
    f32 GetMotionEnd(const char *name);
    f32 GetMotionEnd(int index) const;

    /* The pattern of the scene's own element, created when first needed (name inferred). */
    CMotionPattern *GetRootPattern();

    /* Drops the old root, resets the 0x84 reference, then copies non-null pattern (name inferred). */
    void SetRootPattern(const CMotionPatternRef *pattern);

    /* Returns 1 if any layer's root palette entry has a non-null matrix.sub0, otherwise 0. */
    int fn_8036635C() const;

    /* Assigns the node reference, conditionally rebuilds its owning scene's palette, then drops
     * the standalone root pattern. */
    void fn_803663C0(const RCPtr<CNode> &node);

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

    /* Initialises the frame range and both frame positions from the attached data. */
    void fn_8036674C();

    /* The speed the frame advances at (name inferred). */
    f32 GetSpeed() const;
    void SetSpeed(f32 speed);

    /* The motion plays while the scene is not paused (names inferred from the update, which
     * advances the frame only then). */
    void Resume();
    void Pause();

    /* Store the two motion-end policy actions (working signatures inferred). */
    void fn_803659CC(MotionEndAction action);
    void fn_803659DC(MotionEndAction action);
    MotionEndAction fn_803659D4() const;

    /* 0x1C */ CallbackList m_endCallbacks;
    /* 0x28 */ CallbackList m_startCallbacks;
    /* 0x34 */ CallbackList m_beforeUpdateCallbacks;
    /* 0x40 */ CallbackList m_afterUpdateCallbacks;
    /* 0x4C */ CallbackList m_beforeDrawCallbacks;
    /* 0x58 */ CallbackList m_afterDrawCallbacks;
    /* Base-handle pointers: native Copy passes them to the owning classes' handle constructors. */
    /* 0x64 */ CSceneMotionRef *m_motion;
    /* 0x68 */ CFontListRef *m_fonts;
    /* 0x6C */ CMotionPalette *m_palette;
    /* 0x70 */ f32 m_lastFrame;
    /* 0x74 */ f32 m_frame;
    /* 0x78 */ f32 m_speed;
    /* 0x7C */ f32 m_startFrame;
    /* 0x80 */ f32 m_endFrame;
    /* 0x84 */ RCPtr<CNode> field_0x84;
    /* 0x88 */ CMotionPattern *m_rootPattern;
    /* 0x8C */ s32 m_paused;
    /* 0x90 */ CMotionEndPolicy m_endPolicy;
    /* 0xA8 */ s32 m_motionIndex;
    /* 0xAC */ f32 field_0xAC;
    /* 0xB0 */ NodeMap m_nodes;
    /* The project compares this key with strcmp (type inferred). */
    /* 0xC0 */ const char *field_0xC0;
    /* 0xC4 */ RCPtr<UnkNodePayload_80364C40> field_0xC4;
    /* The project the scene was created by, set while it belongs to one (name inferred: the
     * project's destructor clears it in each of its scenes). */
    /* 0xC8 */ CProject *m_project;
    /* 0xCC */ SceneObjects *m_objects;
};
CHECK_OFFSETOF(Chao::CSD::CScene, m_motion, 0x64);
CHECK_OFFSETOF(Chao::CSD::CScene, m_endCallbacks, 0x1C);
CHECK_OFFSETOF(Chao::CSD::CScene, m_nodes, 0xB0);
CHECK_OFFSETOF(Chao::CSD::CScene, m_endFrame, 0x80);
CHECK_OFFSETOF(Chao::CSD::CScene, field_0x84, 0x84);
CHECK_OFFSETOF(Chao::CSD::CScene, m_rootPattern, 0x88);
CHECK_OFFSETOF(Chao::CSD::CScene, m_paused, 0x8C);
CHECK_OFFSETOF(Chao::CSD::CScene, m_endPolicy, 0x90);
CHECK_OFFSETOF(Chao::CSD::CScene, m_motionIndex, 0xA8);
CHECK_OFFSETOF(Chao::CSD::CScene, field_0xC0, 0xC0);
CHECK_OFFSETOF(Chao::CSD::CScene, field_0xC4, 0xC4);
CHECK_OFFSETOF(Chao::CSD::CScene, m_project, 0xC8);
CHECK_SIZEOF(Chao::CSD::CScene, 0xD0);
CHECK_SIZEOF(Chao::CSD::CScene::CallbackList, 0xC);
CHECK_SIZEOF(Chao::CSD::CScene::NodeMap, 0x10);

/* Stores the default step used by the scene's no-argument update (name unknown). */
void fn_803649F8(f32 step);

} // namespace CSD
} // namespace Chao

#endif

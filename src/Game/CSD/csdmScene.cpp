/* csdmScene.cpp (original name from the library's allocation calls): the scene object and the
 * handles it is built from (csdm.h, csdmScene.h). Class and function names are inferred.
 * Definitions are in reverse address order (deferred inlining emits the last definition first). */
#include <new>
#include "Game/CSD/csdmScene.h"
#include "Game/CSD/Screen.h"
#include "Game/CSD/csdLoader.h"

namespace Chao {
namespace CSD {

void CMotionPaletteRef::Copy(const CMotionPaletteRef &other) {
    m_ref = other.m_ref;
    m_palette = other.m_palette;
}

void CMotionPatternRef::Copy(const CMotionPatternRef &other) {
    m_ref = other.m_ref;
    m_entry = other.m_entry;
}

void CSceneDataRef::Copy(const CSceneDataRef &other) {
    m_ref = other.m_ref;
    m_scene = other.m_scene;
}

void CSceneMotionRef::Copy(const CSceneMotionRef &other) {
    m_ref = other.m_ref;
    m_motion = other.m_motion;
}

CSceneMotion::CSceneMotion(const CSceneMotionRef &other) {
    Copy(other);
}

CSceneMotionRef::CSceneMotionRef() {
    Clear();
}

void CSceneMotionRef::Clear() {
    m_ref.Reset(NULL);
    m_motion = NULL;
}

void CSceneMotion::Attach(const SceneMotion *motion, const RCPtr<unsigned char> &ref) {
    Clear();
    m_ref = ref;
    m_motion = motion;
}

CSceneMotion::CSceneMotion(const SceneMotion *motion, const RCPtr<unsigned char> &ref) {
    Attach(motion, ref);
}

void CScene::SetMotion(int index) {
    if (index != m_motionIndex && index < m_scene->nameCount) {
        if (m_motion != NULL) {
            m_motion->~CSceneMotion();
            m_motion = NULL;
        }
        m_motion = ::new (&m_objects->motion) CSceneMotion(&m_scene->motions[index], m_ref);
        m_motionIndex = index;
        if (m_scene->version >= 2) {
            const FrameRange *ranges = m_scene->frameRanges;

            if (ranges != NULL) {
                m_startFrame = ranges[index].start;
                m_endFrame = ranges[index].end;
            }
        }
    }
}

void CScene::SetMotion(const char *name) {
    int index = FindMotion(name);

    if (index >= 0) {
        SetMotion(index);
    }
}

CSceneMotionRef::~CSceneMotionRef() {
    m_motion = NULL;
}

CSceneMotion::~CSceneMotion() {}

int CScene::FindMotion(const char *name) {
    return FindSceneNameIndex(m_scene, name);
}

void CScene::SetSpeed(f32 speed) {
    m_speed = speed;
}

f32 CScene::GetSpeed() const {
    return m_speed;
}

CMotionPaletteRef::~CMotionPaletteRef() {
    m_palette = NULL;
}

CMotionPalette::~CMotionPalette() {}

void CScene::CreatePalette() {
    if (m_palette != NULL) {
        m_palette->~CMotionPalette();
        m_palette = NULL;
    }
    if (m_scene != NULL) {
        if (m_fonts != NULL) {
            m_palette = ::new (&m_objects->palette) CMotionPalette(m_scene, m_fonts->m_fonts);
        } else {
            m_palette = ::new (&m_objects->palette) CMotionPalette(m_scene, NULL);
        }
    }
}

void CScene::fn_803669B8(f32 step) {
    if (m_scene == NULL) {
        return;
    }

    fn_80365478(&CSceneListener::fn_8036F96C, this);
    for (CallbackList::iterator callback = m_beforeUpdateCallbacks.begin();
         callback != m_beforeUpdateCallbacks.end();) {
        if (!(*callback)->Invoke(this)) {
            callback = m_beforeUpdateCallbacks.erase(callback);
        } else {
            ++callback;
        }
    }

    if (m_lastFrame >= m_endFrame && m_speed > 0.0f) {
        m_endPolicy.OnReachEnd(this);
        fn_80365478(&CSceneListener::OnReachEnd, this);
        for (CallbackList::iterator callback = m_endCallbacks.begin();
             callback != m_endCallbacks.end();) {
            if (!(*callback)->Invoke(this)) {
                callback = m_endCallbacks.erase(callback);
            } else {
                ++callback;
            }
        }
    }

    if (m_lastFrame <= m_startFrame && m_speed < 0.0f) {
        m_endPolicy.OnReachStart(this);
        fn_80365478(&CSceneListener::OnReachStart, this);
        for (CallbackList::iterator callback = m_startCallbacks.begin();
             callback != m_startCallbacks.end();) {
            if (!(*callback)->Invoke(this)) {
                callback = m_startCallbacks.erase(callback);
            } else {
                ++callback;
            }
        }
    }

    const SceneMotion *motion = NULL;
    if (m_motion != NULL) {
        motion = m_motion->m_motion;
    }

    MotionPaletteEntry *parent = NULL;
    if (field_0x84.Get() != NULL) {
        parent = field_0x84->m_pattern->m_entry;
    } else if (m_rootPattern != NULL) {
        m_rootPattern->UpdateTransform();
        parent = m_rootPattern->m_entry;
    }

    const FontList *fonts = NULL;
    if (m_fonts != NULL) {
        fonts = m_fonts->m_fonts;
    }

    NodeMap::iterator node;
    for (node = m_nodes.begin(); node != m_nodes.end(); ++node) {
        node->second->fn_8036A728();
    }
    UpdatePalette(m_palette->m_palette, m_scene, motion, m_frame, fonts, parent);

    fn_80365478(&CSceneListener::fn_8036F970, this);
    for (CallbackList::iterator callback = m_afterUpdateCallbacks.begin();
         callback != m_afterUpdateCallbacks.end();) {
        if (!(*callback)->Invoke(this)) {
            callback = m_afterUpdateCallbacks.erase(callback);
        } else {
            ++callback;
        }
    }

    if (m_paused == 0) {
        m_lastFrame = m_frame;
        m_frame += step * (m_speed * m_scene->field_0x8);
        if (m_frame > m_endFrame) {
            m_frame = m_endFrame;
        } else if (m_frame < m_startFrame) {
            m_frame = m_startFrame;
        }
    }

    for (node = m_nodes.begin(); node != m_nodes.end(); ++node) {
        if (node->second->fn_80366F78() != 0) {
            node->second->fn_8036A954();
        }
    }
}

const SceneData *CSceneDataRef::fn_8036679C() const {
    return m_scene;
}

void CScene::fn_8036674C() {
    m_startFrame = fn_8036679C()->field_0xC;
    m_endFrame = fn_8036679C()->field_0x10;
    m_frame = m_lastFrame = m_startFrame;
}

void CScene::SetFrame(f32 frame) {
    m_lastFrame = frame;
    m_frame = frame;
}

f32 CScene::GetLastFrame() const {
    return m_lastFrame;
}

f32 CScene::GetMotionStart() const {
    return m_startFrame;
}

f32 CScene::GetMotionEnd() const {
    return m_endFrame;
}

void CScene::fn_803663C0(const RCPtr<CNode> &node) {
    field_0x84 = node;
    if (field_0x84.Get() != NULL && fn_8036635C()) {
        field_0x84->fn_8036A20C();
    }
    if (m_rootPattern != NULL) {
        delete m_rootPattern;
        m_rootPattern = NULL;
    }
}

int CScene::fn_8036635C() const {
    const SceneData *scene = m_scene;
    const MotionPalette *palette = m_palette->m_palette;

    for (int i = 0; i < palette->layerCount; i++) {
        int rootIndex = scene->layers[i].rootIndex;

        if (palette->layers[i].entries[rootIndex].matrix.sub0 != NULL) {
            return 1;
        }
    }
    return 0;
}

void CMotionPatternRef::Clear() {
    m_ref.Reset(NULL);
    m_entry = NULL;
}

CMotionPatternRef::CMotionPatternRef() {
    Clear();
}

CMotionPattern::CMotionPattern(const CMotionPatternRef &other) {
    Copy(other);
}

void CScene::SetRootPattern(const CMotionPatternRef *pattern) {
    if (m_rootPattern != NULL) {
        delete m_rootPattern;
        m_rootPattern = NULL;
    }
    field_0x84.Reset(NULL);
    if (pattern != NULL) {
        m_rootPattern = CSD_NEW CMotionPattern(*pattern);
    }
}

CMotionPatternRef::~CMotionPatternRef() {
    m_entry = NULL;
}

CMotionPattern *CScene::GetRootPattern() {
    if (m_rootPattern != NULL) {
        return m_rootPattern;
    }

    CMotionPattern *pattern = CSD_NEW CMotionPattern(GetAspect());
    SetRootPattern(pattern);
    delete pattern;
    return m_rootPattern;
}

f32 CSceneDataRef::GetAspect() const {
    if (m_scene->version >= 1) {
        return m_scene->aspect;
    }
    return 1.3333f;
}

void CMotionPattern::SetEntryHidden(s32 hidden) {
    m_entry->channels.named.hidden = hidden;
    m_entry->flags &= ~(1 << CHANNEL_HIDDEN);
    m_entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
}

void CScene::SetHidden(s32 hidden) {
    GetRootPattern()->SetEntryHidden(hidden);
}

void CScene::SetTranslation(const Vector2 &translation) {
    SetTranslation(translation.x, translation.y);
}

void CScene::SetTranslation(f32 x, f32 y) {
    FromScreenA(&x, &y);
    GetRootPattern()->SetEntryTranslation(x, y);
}

void CMotionPattern::SetEntryTranslation(f32 x, f32 y) {
    m_entry->channels.named.translate.x = x;
    m_entry->channels.named.translate.y = y;
    m_entry->flags &= ~(1 << CHANNEL_TRANSLATE_X);
    m_entry->flags &= ~(1 << CHANNEL_TRANSLATE_Y);
    m_entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
}

void CMotionPattern::SetEntryScale(f32 x, f32 y) {
    m_entry->channels.named.scale.x = x;
    m_entry->channels.named.scale.y = y;
    m_entry->flags &= ~(1 << CHANNEL_SCALE_X);
    m_entry->flags &= ~(1 << CHANNEL_SCALE_Y);
    m_entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
}

void CScene::SetScale(f32 x, f32 y) {
    GetRootPattern()->SetEntryScale(x, y);
}

void CScene::Pause() {
    m_paused = 1;
}

void CScene::Resume() {
    m_paused = 0;
}

void CScene::fn_803659DC(MotionEndAction action) {
    m_endPolicy.m_endAction = action;
}

void CScene::fn_803659CC(MotionEndAction action) {
    m_endPolicy.m_startAction = action;
}

CSceneDataRef::CSceneDataRef() {
    Clear();
}

CSceneDataRef::~CSceneDataRef() {
    m_scene = NULL;
}

CMotionEndPolicy::CMotionEndPolicy()
    : m_endAction(MOTION_END_WRAP), m_startAction(MOTION_END_WRAP) {}

void CSceneDataRef::Clear() {
    m_ref.Reset(NULL);
    m_scene = NULL;
}

void CSceneDataRef::Attach(const SceneData *scene, const RCPtr<unsigned char> &ref) {
    Clear();
    m_ref = ref;
    m_scene = scene;
}

CFontList::~CFontList() {}

void CFontListRef::Clear() {
    m_ref.Reset(NULL);
    m_fonts = NULL;
}

CFontListRef::CFontListRef() {
    Clear();
}

CFontListRef::~CFontListRef() {
    m_fonts = NULL;
}

void CFontListRef::Copy(const CFontListRef &other) {
    m_ref = other.m_ref;
    m_fonts = other.m_fonts;
}

CFontList::CFontList(const CFontListRef &other) {
    Copy(other);
}

void CScene::RebuildPalette() {
    m_palette->Rebuild(*this, *m_fonts);
}

f32 CScene::GetMotionStart(int index) const {
    if (m_scene->version >= 2) {
        return m_scene->frameRanges[index].start;
    }
    return m_startFrame;
}

f32 CScene::GetMotionStart(const char *name) {
    int index = FindMotion(name);

    if (index >= 0) {
        return GetMotionStart(index);
    }
    return m_startFrame;
}

f32 CScene::GetMotionEnd(int index) const {
    if (m_scene->version >= 2) {
        return m_scene->frameRanges[index].end;
    }
    return m_endFrame;
}

f32 CScene::GetMotionEnd(const char *name) {
    int index = FindMotion(name);

    if (index >= 0) {
        return GetMotionEnd(index);
    }
    return m_endFrame;
}

} // namespace CSD
} // namespace Chao

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

CMotionPatternRef::~CMotionPatternRef() {
    m_entry = NULL;
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

CSceneDataRef::CSceneDataRef() {
    Clear();
}

CSceneDataRef::~CSceneDataRef() {
    m_scene = NULL;
}

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

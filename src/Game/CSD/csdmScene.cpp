/* csdmScene.cpp (original name from the library's allocation calls): scene objects,
 * reference handles and callback dispatch. Names not established by RTTI are inferred.
 * Definitions with established native identities are in reverse address order
 * (deferred inlining emits the last definition first). The callback destructor's
 * placement is provisional.
 * Persistent listeners are non-owning; scene destruction only detaches them. */
#include <new>
#include "Game/CSD/CPlatform.h"
#include "Game/CSD/csdmScene.h"
#include "Game/CSD/Screen.h"
#include "Game/CSD/csdLoader.h"

namespace Chao {
namespace CSD {

static f32 lbl_8055FCF8 = 0.016666f;

CSceneCallback::~CSceneCallback() {}

bool SceneNodeKeyCompare::operator()(const SceneNodeKey &left, const SceneNodeKey &right) const {
    if (left.layer == right.layer) {
        return left.element < right.element;
    }
    return left.layer < right.layer;
}

void CMotionPaletteRef::Copy(const CMotionPaletteRef &other) {
    m_ref = other.m_ref;
    m_palette = other.m_palette;
}

void CMotionPatternRef::Copy(const CMotionPatternRef &other) {
    m_ref = other.m_ref;
    m_entry = other.m_entry;
}

void UnkSceneListenerBase_80365054::fn_80367A70(CScene *scene) {
    m_scenes.remove(scene);
}

UnkSceneListenerBase_80365054::~UnkSceneListenerBase_80365054() {}

CSceneListenerSource::~CSceneListenerSource() {}

CScene::~CScene() {
    fn_80365478(&CSceneListener::fn_80367A70, GetSender());
    m_listeners.clear();

    NodeMap::iterator entry;
    for (entry = m_nodes.begin(); entry != m_nodes.end();) {
        (entry++)->second->m_scene = NULL;
    }

    if (m_motion != NULL) {
        m_motion->~CSceneMotionRef();
        m_motion = NULL;
    }
    if (m_fonts != NULL) {
        m_fonts->~CFontListRef();
        m_fonts = NULL;
    }
    if (m_palette != NULL) {
        m_palette->~CMotionPalette();
        m_palette = NULL;
    }
    Free(m_objects);
    if (m_rootPattern != NULL) {
        delete m_rootPattern;
        m_rootPattern = NULL;
    }
}

void CSceneDataRef::Copy(const CSceneDataRef &other) {
    m_ref = other.m_ref;
    m_scene = other.m_scene;
}

void CMotionEndPolicy::fn_803676D4(const CMotionEndPolicy &other) {
    m_endAction = other.m_endAction;
    m_startAction = other.m_startAction;
}

void CScene::Copy(const CSceneDataRef &other) {
    const CScene &scene = static_cast<const CScene &>(other);

    CSceneDataRef::Copy(other);
    if (m_motion != NULL) {
        m_motion->~CSceneMotionRef();
        m_motion = NULL;
    }
    if (scene.m_motion != NULL) {
        m_motion = ::new (&m_objects->motion) CSceneMotion(*scene.m_motion);
    }
    if (m_fonts != NULL) {
        m_fonts->~CFontListRef();
        m_fonts = NULL;
    }
    if (scene.m_fonts != NULL) {
        m_fonts = ::new (&m_objects->fonts) CFontList(*scene.m_fonts);
    }
    CreatePalette();
    field_0x84 = scene.field_0x84;
    m_lastFrame = scene.m_lastFrame;
    m_frame = scene.m_frame;
    m_speed = scene.m_speed;
    m_startFrame = scene.m_startFrame;
    m_endFrame = scene.m_endFrame;
    m_motionIndex = scene.m_motionIndex;
    m_paused = scene.m_paused;
    m_endPolicy.fn_803676D4(scene.m_endPolicy);
}

void CScene::fn_80367500() {
    m_motion = NULL;
    m_fonts = NULL;
    m_motionIndex = -1;
    m_lastFrame = 0.0f;
    m_frame = 0.0f;
    m_speed = 1.0f;
    m_palette = NULL;
    field_0x84.Reset(NULL);
    m_rootPattern = NULL;
    m_paused = 0;
    field_0xC0 = NULL;
    field_0xAC = 0.0f;
    m_project = NULL;
    if (m_objects == NULL) {
        m_objects = static_cast<SceneObjects *>(Alloc(sizeof(SceneObjects), __FILE__, __LINE__));
    }
}

const char *CScene::fn_803674F8() const {
    return field_0xC0;
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
            m_motion->~CSceneMotionRef();
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

void CScene::fn_80366F9C() {
    fn_803669B8(lbl_8055FCF8);
}

size_t CNodeListenerSource::fn_80366F78() {
    return m_listeners.size();
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

void CScene::fn_803667A4(const RCPtr<CTexList> &textures) {
    if (m_scene == NULL) {
        return;
    }

    fn_80365478(&CSceneListener::fn_8036F974, this);
    for (CallbackList::iterator callback = m_beforeDrawCallbacks.begin();
         callback != m_beforeDrawCallbacks.end();) {
        if (!(*callback)->Invoke(this)) {
            callback = m_beforeDrawCallbacks.erase(callback);
        } else {
            ++callback;
        }
    }

    GetPlatform()->Begin();
    const FontList *fonts = NULL;
    if (m_fonts != NULL) {
        fonts = m_fonts->m_fonts;
    }
    DrawScene(m_scene, textures, fonts, m_palette->m_palette);
    GetPlatform()->End();

    fn_80365478(&CSceneListener::fn_8036F978, this);
    for (CallbackList::iterator callback = m_afterDrawCallbacks.begin();
         callback != m_afterDrawCallbacks.end();) {
        if (!(*callback)->Invoke(this)) {
            callback = m_afterDrawCallbacks.erase(callback);
        } else {
            ++callback;
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

RCPtr<CNode> CScene::fn_803666B4(const char *name) {
    ElementRef element = fn_80366640(name);

    if (element.field_0x0 != NULL) {
        return fn_80366468(&element);
    }
    return RCPtr<CNode>(NULL);
}

ElementRef CScene::fn_80366640(const char *name) const {
    const NodeEntry *node = FindNode(name, m_scene);
    ElementRef result;

    if (node != NULL) {
        result.field_0x0 = node->name;
        result.layer = node->field_0x4;
        result.element = node->field_0x8;
    } else {
        /* The original only clears the name; failed lookups leave both indices unspecified. */
        result.field_0x0 = NULL;
    }
    return result;
}

RCPtr<CNode> CScene::fn_80366468(const ElementRef *elementRef) {
    if (elementRef != NULL) {
        SceneNodeKey key(elementRef->layer, elementRef->element);
        NodeMap::iterator found = m_nodes.find(key);

        if (found != m_nodes.end()) {
            return found->second;
        }
        ElementData *element = m_scene->layers[elementRef->layer].elements[elementRef->element];
        RCPtr<CNode> node(CSD_NEW CNode(element, m_ref, m_palette, *elementRef, this, m_fonts),
                          RCPtr<CNode>::DeleteRelease());
        m_nodes.insert(std::make_pair(key, node));
        return node;
    }
    return RCPtr<CNode>(NULL);
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

MotionEndAction CScene::fn_803659D4() const {
    return m_endPolicy.m_endAction;
}

void CScene::fn_803659CC(MotionEndAction action) {
    m_endPolicy.m_startAction = action;
}

void CSceneListenerSource::fn_80365958(CSceneListener *listener) {
    listener->fn_8036542C(GetSender());
    m_listeners.remove(listener);
}

SceneNodeKey::SceneNodeKey(const SceneNodeKey &other)
    : layer(other.layer), element(other.element) {}

SceneNodeKey::SceneNodeKey(s32 layerIndex, s32 elementIndex)
    : layer(layerIndex), element(elementIndex) {}

CScene *CSceneListenerSource::GetSender() {
    return NULL;
}

void CSceneListenerSource::fn_80365478(void (CSceneListener::*event)(CScene *), CScene *scene) {
    std::list<CSceneListener *>::iterator listener;

    for (listener = m_listeners.begin(); listener != m_listeners.end(); ++listener) {
        ((*listener)->*event)(scene);
    }
}

void CSceneListener::fn_8036542C(CScene *scene) {
    m_scenes.remove(scene);
}

f32 CScene::fn_80365424() const {
    return field_0xAC;
}

CSceneDataRef::CSceneDataRef() {
    Clear();
}

CSceneDataRef::~CSceneDataRef() {
    m_scene = NULL;
}

CSceneListenerSource::CSceneListenerSource() {}

UnkSceneListenerBase_80365054::UnkSceneListenerBase_80365054() {}

CSceneListener::CSceneListener() {}

CMotionEndPolicy::CMotionEndPolicy()
    : m_endAction(MOTION_END_WRAP), m_startAction(MOTION_END_WRAP) {}

void fn_803649F8(f32 step) {
    lbl_8055FCF8 = step;
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

void CScene::fn_803648DC(const SceneData *scene, const RCPtr<unsigned char> &ref, const char *key,
                         CProject *project, const CFontListRef *fonts) {
    fn_80367500();
    CSceneDataRef::Attach(scene, ref);
    field_0xC0 = key;
    if (fonts != NULL) {
        m_fonts = ::new (&m_objects->fonts) CFontList(*fonts);
    }
    CreatePalette();
    fn_8036674C();
    field_0xAC = 1.0f + scene->field_0x4;
    m_project = project;
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

void CScene::fn_80364538(const CFontListRef *fonts) {
    if (m_fonts != NULL) {
        m_fonts->~CFontListRef();
        m_fonts = NULL;
    }

    for (NodeMap::iterator node = m_nodes.begin(); node != m_nodes.end(); ++node) {
        node->second->SetFonts(fonts);
    }

    if (fonts != NULL) {
        m_fonts = ::new (&m_objects->fonts) CFontList(*fonts);
        BuildPalette(m_palette->m_palette, m_scene, m_fonts->m_fonts);
    }
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

CScene *CScene::GetSender() {
    return this;
}

} // namespace CSD
} // namespace Chao

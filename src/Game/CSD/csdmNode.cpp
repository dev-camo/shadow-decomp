/* csdmNode.cpp (original name from the library's allocation calls): the node of a scene element
 * (csdm.h). Names without native evidence are inferred. Definitions are in reverse address order
 * (deferred inlining emits the last definition first). */
#include <new>
#include "Game/CSD/csdmNode.h"
#include "Game/CSD/Screen.h"
#include "Game/CSD/csdAccess.h"
#include "Game/CSD/csdLoader.h"

namespace Chao {
namespace CSD {

void CNodeListener::fn_8036B9FC(CNode *node) {
    m_nodes.remove(node);
}

void CElementDataRef::Copy(const CElementDataRef &other) {
    m_ref = other.m_ref;
    m_element = other.m_element;
}

CNodeListenerSource::~CNodeListenerSource() {}

CElementDataRef::CElementDataRef() {
    Clear();
}

CNodeListenerSource::CNodeListenerSource() {}

void CElementDataRef::Clear() {
    m_ref.Reset(NULL);
    m_element = NULL;
}

void CElementDataRef::Attach(ElementData *element, const RCPtr<unsigned char> &ref) {
    Clear();
    m_ref = ref;
    m_element = element;
}

CNode::CNode(ElementData *element, const RCPtr<unsigned char> &ref, CMotionPalette *palette,
             const ElementRef &elementRef, CScene *scene, const CFontListRef *fonts)
    : m_fonts(NULL) {
    Attach(element, ref);
    m_objects = static_cast<NodeObjects *>(Alloc(sizeof(NodeObjects), __FILE__, __LINE__));
    m_pattern = palette->GetPattern(&m_objects->pattern, elementRef);
    m_pattern->m_aspect = scene->GetAspect();
    m_pattern->m_node = this;
    field_0x30 = elementRef.field_0x0;
    m_scene = scene;
    if (fonts != NULL) {
        m_fonts = ::new (&m_objects->fonts) CFontList(*fonts);
    }
}

CElementDataRef::~CElementDataRef() {
    m_element = NULL;
}

CNode::~CNode() {
    fn_8036AAC8(&CNodeListener::fn_8036B9FC, GetSender());
    m_listeners.clear();
    if (m_pattern != NULL) {
        m_pattern->~CMotionPattern();
        m_pattern = NULL;
    }
    if (m_fonts != NULL) {
        m_fonts->~CFontList();
        m_fonts = NULL;
    }
    Free(m_objects);
}

void CNode::fn_8036AEE0(u32 value) {
    m_element->field_0x8 = value;
}

Vector2 CNode::GetTranslation() const {
    const Vector2 &translation = m_pattern->GetTranslation();
    Vector2 screen = translation;

    ToScreenA(&screen.x, &screen.y);
    return screen;
}

void CNode::SetTranslation(const Vector2 &translation) {
    SetTranslation(translation.x, translation.y);
}

void CNode::SetTranslation(f32 x, f32 y) {
    FromScreenA(&x, &y);
    m_pattern->SetTranslation(x, y);
}

void fn_8036ADC0(ChannelWord *value, const CNode *node, int channel) {
    const ElementData *element = node->m_element;

    if (element->channelMask == GetFullChannelMask()) {
        value->bits = element->channelValues[channel];
    } else {
        GetChannelValue(&value->bits, channel, element->channelMask, element->channelValues);
    }
}

s32 CNode::GetHidden() const {
    return m_pattern->GetHidden();
}

/* Matching workaround: the original keeps five global setter bodies, with no calls to
 * them from this object, and expands them in the channel switch. Scoped automatic
 * inlining preserves those bodies; explicit inline drops them. Unit-wide automatic
 * inlining grows SetTranslation(const Vector2 &) from 40 to 84 bytes. Revisit with
 * the original inline settings. */
#pragma auto_inline on
void CNode::SetHidden(s32 hidden) {
    m_pattern->SetHidden(hidden);
}
#pragma auto_inline reset

f32 CNode::GetRotation() const {
    return m_pattern->GetRotation();
}

/* A pattern index without blending: the packed value (csdLoader.h) has the index as both its
 * first and its second pattern. */
void CNode::SetPattern(int pattern) {
    m_pattern->SetPattern(PackPatternBlend(pattern, pattern, 0.0f));
}

u32 CNode::GetTint() const {
    return m_pattern->GetTint();
}

// Matching workaround: as for SetHidden
#pragma auto_inline on
void CNode::SetTint(const u32 &tint) {
    m_pattern->SetTint(tint);
}
#pragma auto_inline reset

void CNode::ClearTint() {
    m_pattern->ClearTint();
}

u32 CNode::GetColor(int corner) const {
    return m_pattern->GetColor(corner);
}

// Matching workaround: as for SetHidden
#pragma auto_inline on
void CNode::SetColor(int corner, const u32 &color) {
    m_pattern->SetColor(corner, color);
}
#pragma auto_inline reset

void CNode::ClearColor(int corner) {
    m_pattern->ClearColor(corner);
}

s32 CNode::GetValue(int index) const {
    return m_pattern->GetValue(index);
}

void CNodeListenerSource::fn_8036AAC8(void (CNodeListener::*event)(CNode *), CNode *node) {
    std::list<CNodeListener *>::iterator listener;

    for (listener = m_listeners.begin(); listener != m_listeners.end(); ++listener) {
        ((*listener)->*event)(node);
    }
}

void CNode::fn_8036A954() {
    fn_8036AAC8(&CNodeListener::fn_0xC, this);
    for (CallbackList::iterator callback = m_callbacks.begin(); callback != m_callbacks.end();) {
        if (!(*callback)->Invoke(this)) {
            callback = m_callbacks.erase(callback);
        } else {
            ++callback;
        }
    }
}

void CNode::SetFonts(const CFontListRef *fonts) {
    if (m_fonts != NULL) {
        m_fonts->~CFontList();
        m_fonts = NULL;
    }
    if (fonts != NULL) {
        m_fonts = ::new (&m_objects->fonts) CFontList(*fonts);
    }
}

void CNode::ClearHidden() {
    m_pattern->ClearHidden();
}

void CNode::fn_8036A728() {
    for (ChannelMap::iterator node = m_channels.begin(); node != m_channels.end(); ++node) {
        fn_8036A44C(node);
    }
}

void CNode::fn_8036A44C(const ChannelMap::iterator &node) {
    switch (node->first) {
    case CHANNEL_HIDDEN:
        SetHidden(node->second->GetHidden());
        break;
    case CHANNEL_TRANSLATE_X:
        SetTranslateX(node->second->GetTranslateX());
        break;
    case CHANNEL_TRANSLATE_Y:
        SetTranslateY(node->second->GetTranslateY());
        break;
    case CHANNEL_ROTATION:
        m_pattern->SetRotation(node->second->GetRotation());
        break;
    case CHANNEL_SCALE_X:
        SetScaleX(node->second->GetScaleX());
        break;
    case CHANNEL_SCALE_Y:
        SetScaleY(node->second->GetScaleY());
        break;
    case CHANNEL_PATTERN: {
        int first;
        int second;
        f32 blend;

        UnpackPatternBlend(&first, &second, &blend, node->second->m_pattern->GetPattern());
        m_pattern->SetPattern(PackPatternBlend(first, second, blend));
        break;
    }
    case CHANNEL_TINT:
        SetTint(node->second->GetTint());
        break;
    case CHANNEL_COLOR_0:
        SetColor(0, node->second->GetColor(0));
        break;
    case CHANNEL_COLOR_0 + 1:
        SetColor(1, node->second->GetColor(1));
        break;
    case CHANNEL_COLOR_0 + 2:
        SetColor(2, node->second->GetColor(2));
        break;
    case CHANNEL_COLOR_0 + 3:
        SetColor(3, node->second->GetColor(3));
        break;
    case CHANNEL_FIELD_0x30:
        m_pattern->SetValue(0, node->second->GetValue(0));
        break;
    case CHANNEL_FIELD_0x30 + 1:
        m_pattern->SetValue(1, node->second->GetValue(1));
        break;
    case CHANNEL_FIELD_0x30 + 2:
        m_pattern->SetValue(2, node->second->GetValue(2));
        break;
    }
}

f32 CNode::GetTranslateX() const {
    const Vector2 &translation = m_pattern->GetTranslation();
    Vector2 screen = translation;

    ToScreenA(&screen.x, &screen.y);
    return screen.x;
}

/* The single-axis setters convert a coordinate pair (FromScreenA) whose other coordinate is
 * zero. */
void CNode::SetTranslateX(f32 value) {
    f32 x = value;
    f32 y = 0.0f;

    FromScreenA(&x, &y);
    m_pattern->SetTranslateX(x);
}

f32 CNode::GetTranslateY() const {
    const Vector2 &translation = m_pattern->GetTranslation();
    Vector2 screen = translation;

    ToScreenA(&screen.x, &screen.y);
    return screen.y;
}

void CNode::SetTranslateY(f32 value) {
    f32 x = 0.0f;
    f32 y = value;

    FromScreenA(&x, &y);
    m_pattern->SetTranslateY(y);
}

f32 CNode::GetScaleX() const {
    return m_pattern->GetScale().x;
}

// Matching workaround: as for SetHidden
#pragma auto_inline on
void CNode::SetScaleX(f32 scaleX) {
    m_pattern->SetScaleX(scaleX);
}
#pragma auto_inline reset

f32 CNode::GetScaleY() const {
    return m_pattern->GetScale().y;
}

// Matching workaround: as for SetHidden
#pragma auto_inline on
void CNode::SetScaleY(f32 scaleY) {
    m_pattern->SetScaleY(scaleY);
}
#pragma auto_inline reset

void CNode::fn_8036A20C() {
    u32 flags = m_element->flags;

    m_element->flags = flags & ~ELEMENT_SCALE_MASK;
    m_scene->RebuildPalette();
    m_element->flags = flags;

    MotionPaletteEntry *entry = m_pattern->m_entry;

    entry->flags &= ~ENTRY_TRANSFORM_SET;
    entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
    MarkEntryDirty(entry);
}

CNode *CNode::GetSender() {
    return this;
}

} // namespace CSD
} // namespace Chao

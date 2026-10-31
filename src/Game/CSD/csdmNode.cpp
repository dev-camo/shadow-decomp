/* csdmNode.cpp (original name from the library's allocation calls): the node of a scene element
 * (csdm.h). Class and function names are inferred. Definitions are in reverse address order
 * (deferred inlining emits the last definition first). */
#include <new>
#include "Game/CSD/csdmNode.h"
#include "Game/CSD/Screen.h"
#include "Game/CSD/csdAccess.h"
#include "Game/CSD/csdLoader.h"

namespace Chao {
namespace CSD {

void CElementDataRef::Copy(const CElementDataRef &other) {
    m_ref = other.m_ref;
    m_element = other.m_element;
}

CElementDataRef::CElementDataRef() {
    Clear();
}

void CElementDataRef::Clear() {
    m_ref.Reset(NULL);
    m_element = NULL;
}

void CElementDataRef::Attach(ElementData *element, const RCPtr<unsigned char> &ref) {
    Clear();
    m_ref = ref;
    m_element = element;
}

CElementDataRef::~CElementDataRef() {
    m_element = NULL;
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

void CNode::SetHidden(s32 hidden) {
    m_pattern->SetHidden(hidden);
}

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

void CNode::SetTint(const u32 &tint) {
    m_pattern->SetTint(tint);
}

void CNode::ClearTint() {
    m_pattern->ClearTint();
}

u32 CNode::GetColor(int corner) const {
    return m_pattern->GetColor(corner);
}

void CNode::SetColor(int corner, const u32 &color) {
    m_pattern->SetColor(corner, color);
}

void CNode::ClearColor(int corner) {
    m_pattern->ClearColor(corner);
}

s32 CNode::GetValue(int index) const {
    return m_pattern->GetValue(index);
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

void CNode::SetScaleX(f32 scaleX) {
    m_pattern->SetScaleX(scaleX);
}

f32 CNode::GetScaleY() const {
    return m_pattern->GetScale().y;
}

void CNode::SetScaleY(f32 scaleY) {
    m_pattern->SetScaleY(scaleY);
}

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

} // namespace CSD
} // namespace Chao

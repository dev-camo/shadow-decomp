/* csdmMotionPattern.cpp (original name from the library's allocation calls): the accessors that
 * override the animated values of an element's entry (csdm.h). Class and function names are
 * inferred. Definitions are in reverse address order (deferred inlining emits the last
 * definition first).
 *
 * Every setter allocates the entry's overrides block when it is the first to need it. The
 * allocation lines differ between the setters (the original passes a different __LINE__ in
 * each), so the setters repeat that code instead of sharing a helper. */
#include "Game/CSD/csdm.h"
#include "Game/CSD/Matrix.h"
#include "Game/CSD/Screen.h"
#include "Game/CSD/csdAccess.h"
#include "Game/CSD/csdLoader.h"

namespace Chao {
namespace CSD {

/* The release policy of the matrix pool of an entry that has no quads. */
static RCPtr<MatrixValue>::FreeRelease s_freeRelease;

/* Releases an entry created by the constructor: its pieces, then the block itself. */
static void ReleaseEntryBlock(unsigned char *block) {
    ReleaseEntry(reinterpret_cast<MotionPaletteEntry *>(block));
    if (block != NULL) {
        Free(block);
    }
}

CMotionPattern::CMotionPattern(f32 aspect) : m_aspect(aspect) {
    MotionPaletteEntry *entry =
        static_cast<MotionPaletteEntry *>(Alloc(sizeof(MotionPaletteEntry), __FILE__, __LINE__));
    ResetEntry(entry);

    RCPtr<unsigned char> block(reinterpret_cast<unsigned char *>(entry), ReleaseEntryBlock);
    Attach(entry, block);
}

void CMotionPattern::Copy(const CMotionPatternRef &other) {
    const CMotionPattern &pattern = static_cast<const CMotionPattern &>(other);

    CMotionPatternRef::Copy(other);
    m_node = pattern.m_node;
    m_aspect = pattern.m_aspect;
}

Vector2 CMotionPattern::GetTranslation() {
    MotionPaletteEntry *entry = m_entry;

    if (entry->overrideMask & (1 << CHANNEL_TRANSLATE_X)) {
        Vector2 override;

        override.x = entry->overrides[CHANNEL_TRANSLATE_X].number;
        override.y = entry->overrides[CHANNEL_TRANSLATE_Y].number;
        return override;
    }

    Vector2 translation;

    if (entry->field_0x140 != 0) {
        translation = entry->channels.named.translate;
    } else {
        ChannelWord x;
        ChannelWord y;

        fn_8036ADC0(&y, m_node, CHANNEL_TRANSLATE_Y);
        fn_8036ADC0(&x, m_node, CHANNEL_TRANSLATE_X);
        translation = fn_8035BA0C(x.number, y.number);
    }
    return translation;
}

void CMotionPattern::UpdateTransform() {
    MotionPaletteEntry *entry = m_entry;
    f32 sizeAWidth;
    f32 sizeAHeight;
    f32 sizeBWidth;
    f32 sizeBHeight;
    f32 offsetX;
    f32 offsetY;

    entry->flags &= ~kEntryChangeMask;
    GetScreenSizeA(&sizeAWidth, &sizeAHeight);
    GetScreenSizeB(&sizeBWidth, &sizeBHeight);
    GetScaledScreenOffsetA(&offsetX, &offsetY);

    if (entry->screenState == NULL) {
        m_pool.Reset(static_cast<MatrixValue *>(Alloc(2 * sizeof(MatrixValue), __FILE__, __LINE__)),
                     s_freeRelease);
        SetMatrixPool(&*m_pool, 2);
        AllocEntryMatrices(entry);
        ClearMatrixPool();
        entry->screenState =
            static_cast<ScreenState *>(Alloc(sizeof(ScreenState), __FILE__, __LINE__));
        entry->ownsVertices = 1;
    } else if (entry->screenState->sizeA[0] != sizeAWidth ||
               entry->screenState->sizeA[1] != sizeAHeight ||
               entry->screenState->sizeB[0] != sizeBWidth ||
               entry->screenState->sizeB[1] != sizeBHeight ||
               entry->screenState->offsetA[0] != offsetX ||
               entry->screenState->offsetA[1] != offsetY) {
        entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
        entry->flags &= ~(1 << CHANNEL_TRANSLATE_X);
        entry->flags &= ~(1 << CHANNEL_TRANSLATE_Y);
    }

    if (entry->flags & ENTRY_ALL_CHANNELS_SET) {
        return;
    }

    if (!(entry->flags & (1 << CHANNEL_HIDDEN))) {
        entry->field_0x20 = entry->channels.named.hidden;
        entry->flags |= 1 << CHANNEL_HIDDEN;
        entry->flags |= ENTRY_HIDDEN_CHANGED;
        entry->flags |= ENTRY_APPLY;
    }
    if (!(entry->flags & ENTRY_TINT_CHANNEL)) {
        entry->field_0x24 = entry->channels.named.tint;
        entry->flags |= ENTRY_TINT_CHANNEL;
        entry->flags |= ENTRY_TINT_CHANGED;
        entry->flags |= ENTRY_APPLY;
    }
    if ((entry->flags & ENTRY_TRANSFORM_CHANNELS) != ENTRY_TRANSFORM_CHANNELS) {
        PushMatrix(&entry->matrix);

        f32 x = entry->channels.named.translate.x;
        f32 y = entry->channels.named.translate.y;

        ScaleByScreenA(&x, &y);

        f32 screenAspect = GetScreenAspectB();

        y *= screenAspect / m_aspect;
        TranslateMatrix(x, y);
        UseTopMatrixAsSource();
        RotateMatrix(entry->channels.named.rotation);
        ScaleMatrix(entry->channels.named.scale.x, entry->channels.named.scale.y);
        PopMatrix();
        entry->flags |= ENTRY_TRANSFORM_CHANNELS;
        entry->flags |= ENTRY_MATRIX_CHANGED;
        entry->flags |= ENTRY_APPLY;
    }
    entry->flags |= ENTRY_ALL_CHANNELS_SET;

    entry->screenState->sizeA[0] = sizeAWidth;
    entry->screenState->sizeA[1] = sizeAHeight;
    entry->screenState->sizeB[0] = sizeBWidth;
    entry->screenState->sizeB[1] = sizeBHeight;
    entry->screenState->offsetA[0] = offsetX;
    entry->screenState->offsetA[1] = offsetY;
}

void CMotionPattern::SetTranslation(f32 x, f32 y) {
    MotionPaletteEntry *entry = m_entry;
    u32 bits = (1 << CHANNEL_TRANSLATE_X) | (1 << CHANNEL_TRANSLATE_Y);

    if ((entry->overrideMask & (1 << CHANNEL_TRANSLATE_X)) &&
        entry->overrides[CHANNEL_TRANSLATE_X].number == x &&
        (entry->overrideMask & (1 << CHANNEL_TRANSLATE_Y)) &&
        entry->overrides[CHANNEL_TRANSLATE_Y].number == y) {
        return;
    }
    entry->overrideMask |= bits;
    if (entry->overrides == NULL) {
        entry->overrides =
            static_cast<ChannelWord *>(Alloc(sizeof(EntryChannels), __FILE__, __LINE__));
    }
    entry->overrides[CHANNEL_TRANSLATE_X].number = x;
    entry->overrides[CHANNEL_TRANSLATE_Y].number = y;
    entry->flags &= ~bits;
    entry->flags &= ~ENTRY_TRANSFORM_SET;
    entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
    MarkEntryDirty(entry);
}

void CMotionPattern::SetHidden(s32 hidden) {
    MotionPaletteEntry *entry = m_entry;

    if ((entry->overrideMask & (1 << CHANNEL_HIDDEN)) &&
        entry->overrides[CHANNEL_HIDDEN].integer == hidden) {
        return;
    }
    entry->overrideMask |= 1 << CHANNEL_HIDDEN;
    if (entry->overrides == NULL) {
        entry->overrides =
            static_cast<ChannelWord *>(Alloc(sizeof(EntryChannels), __FILE__, __LINE__));
    }
    entry->overrides[CHANNEL_HIDDEN].integer = hidden;
    entry->flags &= ~(1 << CHANNEL_HIDDEN);
    entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
    MarkEntryDirty(entry);
}

void CMotionPattern::ClearHidden() {
    MotionPaletteEntry *entry = m_entry;

    if (entry->overrideMask & (1 << CHANNEL_HIDDEN)) {
        entry->overrideMask &= ~(1 << CHANNEL_HIDDEN);
        entry->flags &= ~(1 << CHANNEL_HIDDEN);
        entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
        MarkEntryDirty(entry);
    }
}

s32 CMotionPattern::GetHidden() {
    MotionPaletteEntry *entry = m_entry;

    if (entry->overrideMask & (1 << CHANNEL_HIDDEN)) {
        return entry->overrides[CHANNEL_HIDDEN].integer;
    }
    if (entry->field_0x140 != 0) {
        return entry->channels.named.hidden;
    }

    ChannelWord value;

    fn_8036ADC0(&value, m_node, CHANNEL_HIDDEN);
    return value.integer;
}

f32 CMotionPattern::GetRotation() {
    MotionPaletteEntry *entry = m_entry;

    if (entry->overrideMask & (1 << CHANNEL_ROTATION)) {
        return entry->overrides[CHANNEL_ROTATION].number;
    }
    if (entry->field_0x140 != 0) {
        return entry->channels.named.rotation;
    }

    ChannelWord value;

    fn_8036ADC0(&value, m_node, CHANNEL_ROTATION);
    return value.number;
}

void CMotionPattern::SetRotation(f32 rotation) {
    MotionPaletteEntry *entry = m_entry;

    if ((entry->overrideMask & (1 << CHANNEL_ROTATION)) &&
        entry->overrides[CHANNEL_ROTATION].number == rotation) {
        return;
    }
    entry->overrideMask |= 1 << CHANNEL_ROTATION;
    if (entry->overrides == NULL) {
        entry->overrides =
            static_cast<ChannelWord *>(Alloc(sizeof(EntryChannels), __FILE__, __LINE__));
    }
    entry->overrides[CHANNEL_ROTATION].number = rotation;
    entry->flags &= ~(1 << CHANNEL_ROTATION);
    entry->flags &= ~ENTRY_TRANSFORM_SET;
    entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
    MarkEntryDirty(entry);
}

Vector2 CMotionPattern::GetScale() {
    MotionPaletteEntry *entry = m_entry;

    if (entry->overrideMask & (1 << CHANNEL_SCALE_X)) {
        Vector2 override;

        override.x = entry->overrides[CHANNEL_SCALE_X].number;
        override.y = entry->overrides[CHANNEL_SCALE_Y].number;
        return override;
    }

    Vector2 scale;

    if (entry->field_0x140 != 0) {
        scale = entry->channels.named.scale;
    } else {
        ChannelWord x;
        ChannelWord y;

        fn_8036ADC0(&y, m_node, CHANNEL_SCALE_Y);
        fn_8036ADC0(&x, m_node, CHANNEL_SCALE_X);
        scale = fn_8035BA0C(x.number, y.number);
    }
    return scale;
}

u32 CMotionPattern::GetTint() {
    MotionPaletteEntry *entry = m_entry;

    if (entry->overrideMask & (1 << CHANNEL_TINT)) {
        return entry->overrides[CHANNEL_TINT].bits;
    }
    if (entry->field_0x140 != 0) {
        return entry->channels.named.tint;
    }

    ChannelWord value;

    fn_8036ADC0(&value, m_node, CHANNEL_TINT);
    return value.bits;
}

void CMotionPattern::SetTint(const u32 &tint) {
    MotionPaletteEntry *entry = m_entry;

    if ((entry->overrideMask & (1 << CHANNEL_TINT)) &&
        entry->overrides[CHANNEL_TINT].bits == tint) {
        return;
    }
    entry->overrideMask |= 1 << CHANNEL_TINT;
    if (entry->overrides == NULL) {
        entry->overrides =
            static_cast<ChannelWord *>(Alloc(sizeof(EntryChannels), __FILE__, __LINE__));
    }
    entry->overrides[CHANNEL_TINT].bits = tint;
    entry->flags &= ~ENTRY_TINT_CHANNEL;
    entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
    MarkEntryDirty(entry);
}

void CMotionPattern::ClearTint() {
    MotionPaletteEntry *entry = m_entry;

    if (entry->overrideMask & (1 << CHANNEL_TINT)) {
        entry->overrideMask &= ~(1 << CHANNEL_TINT);
        entry->flags &= ~ENTRY_TINT_CHANNEL;
        entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
        MarkEntryDirty(entry);
    }
}

f32 CMotionPattern::GetPattern() {
    MotionPaletteEntry *entry = m_entry;

    if (entry->overrideMask & (1 << CHANNEL_PATTERN)) {
        return entry->overrides[CHANNEL_PATTERN].number;
    }
    if (entry->field_0x140 != 0) {
        return entry->channels.named.pattern;
    }

    ChannelWord value;

    fn_8036ADC0(&value, m_node, CHANNEL_PATTERN);
    return value.number;
}

void CMotionPattern::SetPattern(f32 pattern) {
    MotionPaletteEntry *entry = m_entry;

    if ((entry->overrideMask & (1 << CHANNEL_PATTERN)) &&
        entry->overrides[CHANNEL_PATTERN].number == pattern) {
        return;
    }
    entry->overrideMask |= 1 << CHANNEL_PATTERN;
    if (entry->overrides == NULL) {
        entry->overrides =
            static_cast<ChannelWord *>(Alloc(sizeof(EntryChannels), __FILE__, __LINE__));
    }
    entry->overrides[CHANNEL_PATTERN].number = pattern;
    entry->channels.named.pattern = pattern;
    if (m_node->m_scene != NULL) {
        SetEntryPattern(m_entry, m_node->m_element, m_node->m_scene->m_scene);
    }
    entry->flags &= ~(1 << CHANNEL_PATTERN);
    entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
    MarkEntryDirty(entry);
}

u32 CMotionPattern::GetColor(int corner) {
    MotionPaletteEntry *entry = m_entry;
    u32 bit = (1 << CHANNEL_COLOR_0) << corner;

    if (entry->overrideMask & bit) {
        return entry->overrides[CHANNEL_COLOR_0 + corner].bits;
    }
    if (entry->field_0x140 != 0) {
        return entry->channels.named.colors[corner];
    }

    ChannelWord value;

    fn_8036ADC0(&value, m_node, CHANNEL_COLOR_0 + corner);
    return value.bits;
}

void CMotionPattern::SetColor(int corner, const u32 &color) {
    MotionPaletteEntry *entry = m_entry;
    u32 bit = (1 << CHANNEL_COLOR_0) << corner;

    if ((entry->overrideMask & bit) && entry->overrides[CHANNEL_COLOR_0 + corner].bits == color) {
        return;
    }
    entry->overrideMask |= bit;
    if (entry->overrides == NULL) {
        entry->overrides =
            static_cast<ChannelWord *>(Alloc(sizeof(EntryChannels), __FILE__, __LINE__));
    }
    entry->overrides[CHANNEL_COLOR_0 + corner].bits = color;
    entry->channels.named.colors[corner] = color;
    SetEntryColors(m_entry);
    TintEntryColors(m_entry);
    entry->flags |= bit;
}

void CMotionPattern::ClearColor(int corner) {
    MotionPaletteEntry *entry = m_entry;
    u32 bit = (1 << CHANNEL_COLOR_0) << corner;

    if (entry->overrideMask & bit) {
        entry->overrideMask &= ~bit;
        entry->flags &= ~bit;
        entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
        MarkEntryDirty(entry);
    }
}

s32 CMotionPattern::GetValue(int index) {
    MotionPaletteEntry *entry = m_entry;
    u32 bit = (1 << CHANNEL_FIELD_0x30) << index;

    if (entry->overrideMask & bit) {
        return entry->overrides[CHANNEL_FIELD_0x30 + index].integer;
    }
    if (entry->field_0x140 != 0) {
        return entry->channels.named.field_0x30[index];
    }

    ChannelWord value;

    fn_8036ADC0(&value, m_node, CHANNEL_FIELD_0x30 + index);
    return value.integer;
}

void CMotionPattern::SetValue(int index, s32 value) {
    MotionPaletteEntry *entry = m_entry;

    if ((entry->overrideMask & ((1 << CHANNEL_FIELD_0x30) << index)) &&
        entry->overrides[CHANNEL_FIELD_0x30 + index].integer == value) {
        return;
    }
    entry->overrideMask |= (1 << CHANNEL_FIELD_0x30) << index;
    if (entry->overrides == NULL) {
        entry->overrides =
            static_cast<ChannelWord *>(Alloc(sizeof(EntryChannels), __FILE__, __LINE__));
    }
    entry->overrides[CHANNEL_FIELD_0x30 + index].integer = value;
    entry->channels.named.field_0x30[index] = value;
    entry->flags |= (1 << CHANNEL_FIELD_0x30) << index;
}

void CMotionPattern::SetTranslateX(f32 x) {
    MotionPaletteEntry *entry = m_entry;

    if ((entry->overrideMask & (1 << CHANNEL_TRANSLATE_X)) &&
        entry->overrides[CHANNEL_TRANSLATE_X].number == x) {
        return;
    }
    entry->overrideMask |= 1 << CHANNEL_TRANSLATE_X;
    if (entry->overrides == NULL) {
        entry->overrides =
            static_cast<ChannelWord *>(Alloc(sizeof(EntryChannels), __FILE__, __LINE__));
    }
    entry->overrides[CHANNEL_TRANSLATE_X].number = x;
    entry->flags &= ~(1 << CHANNEL_TRANSLATE_X);
    entry->flags &= ~ENTRY_TRANSFORM_SET;
    entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
    MarkEntryDirty(entry);
}

void CMotionPattern::SetTranslateY(f32 y) {
    MotionPaletteEntry *entry = m_entry;

    if ((entry->overrideMask & (1 << CHANNEL_TRANSLATE_Y)) &&
        entry->overrides[CHANNEL_TRANSLATE_Y].number == y) {
        return;
    }
    entry->overrideMask |= 1 << CHANNEL_TRANSLATE_Y;
    if (entry->overrides == NULL) {
        entry->overrides =
            static_cast<ChannelWord *>(Alloc(sizeof(EntryChannels), __FILE__, __LINE__));
    }
    entry->overrides[CHANNEL_TRANSLATE_Y].number = y;
    entry->flags &= ~(1 << CHANNEL_TRANSLATE_Y);
    entry->flags &= ~ENTRY_TRANSFORM_SET;
    entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
    MarkEntryDirty(entry);
}

void CMotionPattern::SetScaleX(f32 scaleX) {
    MotionPaletteEntry *entry = m_entry;

    if ((entry->overrideMask & (1 << CHANNEL_SCALE_X)) &&
        entry->overrides[CHANNEL_SCALE_X].number == scaleX) {
        return;
    }
    entry->overrideMask |= 1 << CHANNEL_SCALE_X;
    if (entry->overrides == NULL) {
        entry->overrides =
            static_cast<ChannelWord *>(Alloc(sizeof(EntryChannels), __FILE__, __LINE__));
    }
    entry->overrides[CHANNEL_SCALE_X].number = scaleX;
    entry->flags &= ~(1 << CHANNEL_SCALE_X);
    entry->flags &= ~ENTRY_TRANSFORM_SET;
    entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
    MarkEntryDirty(entry);
}

void CMotionPattern::SetScaleY(f32 scaleY) {
    MotionPaletteEntry *entry = m_entry;

    if ((entry->overrideMask & (1 << CHANNEL_SCALE_Y)) &&
        entry->overrides[CHANNEL_SCALE_Y].number == scaleY) {
        return;
    }
    entry->overrideMask |= 1 << CHANNEL_SCALE_Y;
    if (entry->overrides == NULL) {
        entry->overrides =
            static_cast<ChannelWord *>(Alloc(sizeof(EntryChannels), __FILE__, __LINE__));
    }
    entry->overrides[CHANNEL_SCALE_Y].number = scaleY;
    entry->flags &= ~(1 << CHANNEL_SCALE_Y);
    entry->flags &= ~ENTRY_TRANSFORM_SET;
    entry->flags &= ~ENTRY_ALL_CHANNELS_SET;
    MarkEntryDirty(entry);
}

} // namespace CSD
} // namespace Chao

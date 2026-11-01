/* Effect loader and manager/element methods; original filename unknown. */
#include "Game/EffectManager.h"
#include "Game/UnkEffectFileData.h"
#include <cstdint>
#include <cstring>

namespace Effect {
namespace System {
namespace {

/* The file uses 32-bit relative offsets in slots that become native GameCube pointers. */
template <class T> inline void RelocateEffectPointer(T *&pointer, unsigned char *base) {
    if (pointer != 0) {
        pointer = reinterpret_cast<T *>(reinterpret_cast<std::uintptr_t>(pointer) +
                                        reinterpret_cast<std::uintptr_t>(base));
    }
}

} // namespace

EffectManager::EffectManager() : field_0x24(0), field_0x25(0), field_0x28(0) {}

EffectManager::~EffectManager() {}

EffectElement::EffectElement(const UnkEffectElementData *definition, EffectLaunchInfo *info,
                             EffectManager *manager)
    : field_0x4(0), field_0x5(0), m_size(sizeof(EffectElement)), m_elapsed(0.0f), m_timeScale(1.0f),
      m_timeScaleSource(0), m_flags(ELEMENT_ACTIVE), m_manager(manager), field_0x30(0),
      m_definition(definition), m_matrixSource(0), m_translationSource(0), m_rotationSource(0) {
    fn_8040C148(&m_transform, &lbl_805E0720);
    fn_8040C148(&m_baseTransform, &lbl_805E0720);
    m_previousTranslation.Set(0.0f, 0.0f, 0.0f);
    m_translationVelocity.Set(0.0f, 0.0f, 0.0f);
    m_translation.Set(0.0f, 0.0f, 0.0f);
    m_rotation.Set(0.0f, 0.0f, 0.0f);
    m_scale.Set(1.0f, 1.0f, 1.0f);
    m_velocity.Set(0.0f, 0.0f, 0.0f);
}

EffectElement::~EffectElement() {
    m_updateNext.reset();
    m_updatePrev.reset();
    m_renderNext.reset();
    m_renderPrev.reset();
}

bool EffectManager::fn_80425860(UnkEffectFileData *file) {
    if (file == 0) {
        return false;
    }
    if (std::strncmp(file->signature, "EFFD", 4) != 0) {
        field_0x28 = 2;
        return false;
    }
    if (file->field_0x4 < 1) {
        field_0x28 = 3;
        return false;
    }
    if (file->field_0x6 != 0) {
        return true;
    }
    unsigned char *base = reinterpret_cast<unsigned char *>(file);
    RelocateEffectPointer(file->first, base);
    for (UnkEffectElementData *record = file->first; record != 0; record = record->next) {
        switch (record->field_0x20) {
        case 2: {
            UnkEffectElementData2 *data = static_cast<UnkEffectElementData2 *>(record);
            RelocateEffectPointer(data->field_0x48, base);
            RelocateEffectPointer(data->field_0x58, base);
            RelocateEffectPointer(data->field_0x70, base);
            RelocateEffectPointer(data->field_0x68, base);
            break;
        }
        case 3: {
            UnkEffectElementData3 *data = static_cast<UnkEffectElementData3 *>(record);
            RelocateEffectPointer(data->field_0x38, base);
            break;
        }
        case 4: {
            UnkEffectElementData4 *data = static_cast<UnkEffectElementData4 *>(record);
            RelocateEffectPointer(data->field_0x3C, base);
            RelocateEffectPointer(data->field_0x40, base);
            break;
        }
        case 5: {
            UnkEffectElementData5 *data = static_cast<UnkEffectElementData5 *>(record);
            RelocateEffectPointer(data->field_0x48, base);
            RelocateEffectPointer(data->field_0x58, base);
            RelocateEffectPointer(data->field_0x5C, base);
            RelocateEffectPointer(data->field_0x74, base);
            RelocateEffectPointer(data->field_0x6C, base);
            break;
        }
        case 6: {
            UnkEffectElementData6 *data = static_cast<UnkEffectElementData6 *>(record);
            RelocateEffectPointer(data->field_0x54, base);
            RelocateEffectPointer(data->field_0x6C, base);
            RelocateEffectPointer(data->field_0x64, base);
            break;
        }
        case 7: {
            UnkEffectElementData7 *data = static_cast<UnkEffectElementData7 *>(record);
            RelocateEffectPointer(data->field_0x50, base);
            RelocateEffectPointer(data->field_0x54, base);
            break;
        }
        case 8: {
            UnkEffectElementData8 *data = static_cast<UnkEffectElementData8 *>(record);
            RelocateEffectPointer(data->field_0x34, base);
            break;
        }
        case 9: {
            UnkEffectElementData9 *data = static_cast<UnkEffectElementData9 *>(record);
            RelocateEffectPointer(data->field_0x44, base);
            RelocateEffectPointer(data->field_0x48, base);
            RelocateEffectPointer(data->field_0x4C, base);
            break;
        }
        case 10: {
            UnkEffectElementData10 *data = static_cast<UnkEffectElementData10 *>(record);
            RelocateEffectPointer(data->field_0x38, base);
            RelocateEffectPointer(data->field_0x3C, base);
            break;
        }
        }
        RelocateEffectPointer(record->next, base);
    }
    file->field_0x6 = 1;
    return true;
}

} // namespace System
} // namespace Effect

extern "C" void fn_80425E68(Effect::System::EffectElement *effect, const Fn8040Matrix34 *matrix) {
    fn_8040C148(&effect->m_baseTransform, matrix);
}

extern "C" void fn_80425E4C(Effect::System::EffectElement *effect, const Fn8040Vector3 *position) {
    effect->m_baseTransform.values[0][3] = position->x;
    effect->m_baseTransform.values[1][3] = position->y;
    effect->m_baseTransform.values[2][3] = position->z;
}

extern "C" void fn_80425D94(Effect::System::EffectElement *effect, const Fn8040Vector3 *rotation) {
    Fn8040Matrix34 matrix;
    fn_8040C148(&matrix, &lbl_805E0720);
    Fn8040Vector3 translation(effect->m_baseTransform.values[0][3],
                              effect->m_baseTransform.values[1][3],
                              effect->m_baseTransform.values[2][3]);
    fn_8040B80C(&matrix, rotation->z);
    fn_8040B848(&matrix, rotation->y);
    fn_8040B884(&matrix, rotation->x);
    matrix.values[0][3] = translation.x;
    matrix.values[1][3] = translation.y;
    matrix.values[2][3] = translation.z;
    fn_8040C148(&effect->m_baseTransform, &matrix);
}

extern "C" void fn_80425C34(Effect::System::EffectElement *effect, float deltaTime) {
    if (effect->m_matrixSource != 0) {
        fn_80425E68(effect, effect->m_matrixSource);
    } else {
        if (effect->m_translationSource != 0) {
            const Fn8040Vector3 *translation = effect->m_translationSource;
            effect->m_baseTransform.values[0][3] = translation->x;
            effect->m_baseTransform.values[1][3] = translation->y;
            effect->m_baseTransform.values[2][3] = translation->z;
        }
        if (effect->m_rotationSource != 0) {
            fn_80425D94(effect, effect->m_rotationSource);
        }
    }
    fn_8040C148(&effect->m_transform, &effect->m_baseTransform);
    AddVector(&effect->m_translation, fn_80093230(&effect->m_velocity, deltaTime));
    fn_8040B59C(&effect->m_transform, &effect->m_translation);
    fn_8040B80C(&effect->m_transform, effect->m_rotation.z);
    fn_8040B848(&effect->m_transform, effect->m_rotation.y);
    fn_8040B884(&effect->m_transform, effect->m_rotation.x);
    fn_8040B790(&effect->m_transform, &effect->m_scale);
    Fn8040Vector3 position(effect->m_transform.values[0][3], effect->m_transform.values[1][3],
                           effect->m_transform.values[2][3]);
    if ((effect->m_flags & Effect::System::ELEMENT_FLAG_2) != 0) {
        Fn8040Vector3 difference;
        fn_8040CD74(&difference, &position, &effect->m_previousTranslation);
        Fn8040Vector3 differenceCopy;
        fn_800091D0(&differenceCopy, &difference);
        Fn8040Vector3 velocity;
        fn_8040CDA8(&velocity, &differenceCopy, 1.0f / deltaTime);
        fn_800091D0(&effect->m_translationVelocity, &velocity);
    }
    fn_800091D0(&effect->m_previousTranslation, &position);
}

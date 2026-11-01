/* EffectGlobalParam methods; original filename unknown. */
#include "Game/EffectGlobalParam.h"
#include "Game/UnkEffectFileData.h"

namespace Effect {
namespace System {

EffectGlobalParam::EffectGlobalParam(UnkEffectElementData8 *definition, EffectLaunchInfo *info,
                                     EffectManager *manager)
    : EffectElement(definition, info, manager), m_definitionFlags(0) {
    m_size -= sizeof(EffectElement);
    m_size += sizeof(EffectGlobalParam);
    if ((m_flags & ELEMENT_ACTIVE) == 0) {
        return;
    }
    field_0x108 = static_cast<const UnkEffectGlobalParamData *>(definition->field_0x34);
    m_duration = definition->field_0x30;
    m_definitionFlags = definition->field_0x2C;
}

EffectGlobalParam::~EffectGlobalParam() {}

void EffectGlobalParam::Update(float deltaTime) {
    if ((m_flags & ELEMENT_ACTIVE) == 0) {
        return;
    }
    if ((m_flags & ELEMENT_FLAG_4) != 0) {
        return;
    }
    if ((m_flags & ELEMENT_FLAG_2) != 0) {
        m_elapsed += deltaTime;
    }
    if (m_elapsed > 0.0001f + m_duration) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    if (m_duration > 0.0001f) {
        m_progress = m_elapsed / m_duration;
    } else {
        m_progress = 1.0f;
    }
    if (m_progress > 1.0f) {
        m_progress = 1.0f;
    }
}

} // namespace System
} // namespace Effect

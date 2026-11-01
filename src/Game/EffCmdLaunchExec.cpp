/* EffCmdLaunchExec methods; the original source filename is unknown. */
#include "Game/EffectCommandExec.h"
#include "Game/EffectElement.h"
#include "Game/EffectManager.h"
#include "Game/EffectObject.h"
#include "Game/EffectSystem.h"
#include "Game/fn_80424D18.h"

namespace Effect {
namespace System {

EffCmdLaunchExec::EffCmdLaunchExec(const UnkCommandData *command, unsigned int *flags,
                                   EffectManager *manager, EffectLaunchInfo *info,
                                   EffectObject *parent)
    : EffectCommandExec(command), m_flags(flags), m_elapsed(0.0f), m_parent(parent) {
    const UnkLaunchData *data = static_cast<const UnkLaunchData *>(m_command);
    /* A zero base duration also suppresses the random duration range. */
    if (data->duration != 0) {
        m_duration = data->duration;
        if (data->durationRange != 0) {
            m_duration += data->durationRange * fn_8042A5D0()->Random();
        }
    } else {
        m_duration = 0.0f;
    }
    if (data->sharedRandom != 0) {
        float random = fn_8042A5D0()->Random();
        m_translation.x = data->translationRange.x * random + data->translation.x;
        m_translation.y = data->translationRange.y * random + data->translation.y;
        m_translation.z = data->translationRange.z * random + data->translation.z;
        random = fn_8042A5D0()->Random();
        m_rotation.x = data->rotationRange.x * random + data->rotation.x;
        m_rotation.y = data->rotationRange.y * random + data->rotation.y;
        m_rotation.z = data->rotationRange.z * random + data->rotation.z;
        random = fn_8042A5D0()->Random();
        m_scale.x = data->scaleRange.x * random + data->scale.x;
        m_scale.y = data->scaleRange.y * random + data->scale.y;
        m_scale.z = data->scaleRange.z * random + data->scale.z;
    } else {
        m_translation.x = data->translationRange.x * fn_8042A5D0()->Random() + data->translation.x;
        m_translation.y = data->translationRange.y * fn_8042A5D0()->Random() + data->translation.y;
        m_translation.z = data->translationRange.z * fn_8042A5D0()->Random() + data->translation.z;
        m_rotation.x = data->rotationRange.x * fn_8042A5D0()->Random() + data->rotation.x;
        m_rotation.y = data->rotationRange.y * fn_8042A5D0()->Random() + data->rotation.y;
        m_rotation.z = data->rotationRange.z * fn_8042A5D0()->Random() + data->rotation.z;
        m_scale.x = data->scaleRange.x * fn_8042A5D0()->Random() + data->scale.x;
        m_scale.y = data->scaleRange.y * fn_8042A5D0()->Random() + data->scale.y;
        m_scale.z = data->scaleRange.z * fn_8042A5D0()->Random() + data->scale.z;
    }
    m_child = manager->Create(info, data->id);
    boost::shared_ptr<EffectElement> child = m_child.lock();
    if (child.get() != 0) {
        child->SetTimeScale(fn_80424D18(m_parent));
    }
}

EffCmdLaunchExec::~EffCmdLaunchExec() {
    boost::shared_ptr<EffectElement> child = m_child.lock();
    if (child.get() != 0) {
        child->ClearFlags(ELEMENT_ACTIVE);
    }
}

bool EffCmdLaunchExec::Update(float deltaTime) {
    boost::shared_ptr<EffectElement> child = m_child.lock();
    if (child.get() == 0 || !child->HasFlags(ELEMENT_ACTIVE)) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    EffectObject *parent = m_parent;
    if ((parent->m_flags & ELEMENT_STOP_REQUESTED) != 0 &&
        !child->HasFlags(ELEMENT_STOP_REQUESTED)) {
        child->RequestStop();
    }
    if ((parent->m_flags & ELEMENT_FLAG_8) != 0 && !child->HasFlags(ELEMENT_FLAG_8)) {
        child->SetFlags(ELEMENT_FLAG_8);
    } else if ((parent->m_flags & ELEMENT_FLAG_8) == 0 && child->HasFlags(ELEMENT_FLAG_8)) {
        child->ClearFlags(ELEMENT_FLAG_8);
    }
    if ((parent->m_flags & ELEMENT_FLAG_4) != 0 && !child->HasFlags(ELEMENT_FLAG_4)) {
        child->SetFlags(ELEMENT_FLAG_4);
    } else if ((parent->m_flags & ELEMENT_FLAG_4) == 0 && child->HasFlags(ELEMENT_FLAG_4)) {
        child->ClearFlags(ELEMENT_FLAG_4);
    }
    child->SetTimeScale(fn_80424D18(m_parent));
    if (field_0x10 == 0) {
        field_0x10 = 1;
        fn_8040C148(&m_transform, &parent->m_transform);
        fn_8040B59C(&m_transform, &m_translation);
        fn_8040B80C(&m_transform, m_rotation.z);
        fn_8040B848(&m_transform, m_rotation.y);
        fn_8040B884(&m_transform, m_rotation.x);
        fn_8040B790(&m_transform, &m_scale);
        if ((parent->m_definitionFlags & OBJECT_LINK_TRANSFORM) != 0) {
            child->SetMatrixSource(&m_transform);
        } else {
            fn_80425E68(child.get(), &m_transform);
        }
        return true;
    }
    if (child->HasFlags(ELEMENT_FLAG_4)) {
        return true;
    }
    m_elapsed += deltaTime;
    if (m_duration > 0.0001f && m_elapsed > m_duration) {
        if (m_command->type == 18 &&
            static_cast<const UnkTimedLaunchData *>(m_command)->stopAtDuration != 0 &&
            !child->HasFlags(ELEMENT_STOP_REQUESTED)) {
            child->RequestStop();
        }
        if (!child->HasFlags(ELEMENT_STOP_REQUESTED)) {
            if (m_command->stop != 0) {
                *m_flags |= COMMAND_STOP_REQUESTED;
            }
            return false;
        }
    }
    if ((parent->m_definitionFlags & OBJECT_LINK_TRANSFORM) != 0) {
        fn_8040C148(&m_transform, &parent->m_transform);
        fn_8040B59C(&m_transform, &m_translation);
        fn_8040B80C(&m_transform, m_rotation.z);
        fn_8040B848(&m_transform, m_rotation.y);
        fn_8040B884(&m_transform, m_rotation.x);
        fn_8040B790(&m_transform, &m_scale);
    }
    return true;
}

} // namespace System
} // namespace Effect

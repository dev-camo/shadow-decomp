/* EffCmd3DSclExec methods; the original source filename is unknown. */
#include "Game/EffectCommandExec.h"
#include "Game/EffectSystem.h"

namespace Effect {
namespace System {

EffCmd3DSclExec::EffCmd3DSclExec(const UnkCommandData *command, unsigned int *flags,
                                 Fn8040Vector3 *target)
    : EffectCommandExec(command), m_flags(flags), m_target(target), m_elapsed(0.0f) {
    const UnkVector3Data *data = static_cast<const UnkVector3Data *>(m_command);
    if (data->sharedRandom != 0) {
        float random = fn_8042A5D0()->Random();
        m_rate.x = data->range.x * random + data->value.x;
        m_rate.y = data->range.y * random + data->value.y;
        m_rate.z = data->range.z * random + data->value.z;
    } else {
        m_rate.x = data->range.x * fn_8042A5D0()->Random() + data->value.x;
        m_rate.y = data->range.y * fn_8042A5D0()->Random() + data->value.y;
        m_rate.z = data->range.z * fn_8042A5D0()->Random() + data->value.z;
    }
    m_duration = data->duration;
    if (data->durationRange != 0) {
        m_duration += data->durationRange * fn_8042A5D0()->Random();
    }
    if (m_duration < 0.0001f) {
        if (data->relative != 0) {
            m_target->x *= m_rate.x;
            m_target->y *= m_rate.y;
            m_target->z *= m_rate.z;
        } else {
            fn_800091D0(m_target, &m_rate);
        }
        m_target = 0;
    } else {
        if (data->relative != 0) {
            m_rate.x *= m_target->x;
            m_rate.y *= m_target->y;
            m_rate.z *= m_target->z;
        }
        fn_8040CE04(&m_rate, target);
        fn_8040CE38(&m_rate, 1.0f / m_duration);
        m_duration += 0.0001f;
    }
}

bool EffCmd3DSclExec::Update(float deltaTime) {
    if (m_target == 0) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    if (field_0x10 == 0) {
        field_0x10 = 1;
        return true;
    }
    m_elapsed += deltaTime;
    if (m_elapsed > m_duration) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    Fn8040Vector3 step;
    fn_800091A4(&step, &m_rate, deltaTime);
    fn_8040CFF8(m_target, &step);
    return true;
}

} // namespace System
} // namespace Effect

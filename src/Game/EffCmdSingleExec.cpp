/* EffCmdSingleExec methods; the original source filename is unknown. */
#include "Game/EffectCommandExec.h"
#include "Game/EffectSystem.h"

namespace Effect {
namespace System {

EffCmdSingleExec::EffCmdSingleExec(const UnkCommandData *command, unsigned int *flags,
                                   float *target)
    : EffectCommandExec(command), m_flags(flags), m_target(target), m_elapsed(0.0f) {
    const UnkSingleData *data = static_cast<const UnkSingleData *>(m_command);
    m_rate = data->range * fn_8042A5D0()->Random() + data->value;
    m_duration = data->duration;
    if (data->durationRange != 0) {
        m_duration += data->durationRange * fn_8042A5D0()->Random();
    }
    if (m_duration < 0.0001f) {
        if (data->relative != 0) {
            *m_target += m_rate;
        } else {
            *m_target = m_rate;
        }
        m_target = 0;
    } else {
        if (data->relative == 0) {
            m_rate -= *target;
        }
        m_rate /= m_duration;
        m_duration += 0.0001f;
    }
}

bool EffCmdSingleExec::Update(float deltaTime) {
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
    *m_target += m_rate * deltaTime;
    return true;
}

} // namespace System
} // namespace Effect

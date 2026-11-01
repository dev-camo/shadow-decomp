/* EffCmdMatColorExec methods; the original source filename is unknown. */
#include "Game/EffectCommandExec.h"
#include "Game/EffectSystem.h"

namespace Effect {
namespace System {

EffCmdMatColorExec::EffCmdMatColorExec(const UnkCommandData *command, unsigned int *flags,
                                       int *target)
    : EffectCommandExec(command), m_flags(flags), m_target(target), m_elapsed(0.0f) {
    const UnkMatColorData *data = static_cast<const UnkMatColorData *>(m_command);
    if (data->sharedRandom != 0) {
        float random = fn_8042A5D0()->Random();
        m_rate[0] = data->value[0];
        if (data->range[0] != 0) {
            m_rate[0] += static_cast<int>(data->range[0] * random);
        }
        m_rate[1] = data->value[1];
        if (data->range[1] != 0) {
            m_rate[1] += static_cast<int>(data->range[1] * random);
        }
        m_rate[2] = data->value[2];
        if (data->range[2] != 0) {
            m_rate[2] += static_cast<int>(data->range[2] * random);
        }
    } else {
        m_rate[0] = data->value[0];
        if (data->range[0] != 0) {
            m_rate[0] += static_cast<int>(data->range[0] * fn_8042A5D0()->Random());
        }
        m_rate[1] = data->value[1];
        if (data->range[1] != 0) {
            m_rate[1] += static_cast<int>(data->range[1] * fn_8042A5D0()->Random());
        }
        m_rate[2] = data->value[2];
        if (data->range[2] != 0) {
            m_rate[2] += static_cast<int>(data->range[2] * fn_8042A5D0()->Random());
        }
    }
    m_duration = data->duration;
    if (data->durationRange != 0) {
        m_duration += data->durationRange * fn_8042A5D0()->Random();
    }
    if (data->relative != 0) {
        for (int i = 0; i < 3; ++i) {
            m_rate[i] <<= MATERIAL_FRACTION_BITS;
            m_rate[i] += m_target[i];
            if (m_rate[i] > MATERIAL_CHANNEL_MAX) {
                m_rate[i] = MATERIAL_CHANNEL_MAX;
            } else if (m_rate[i] < MATERIAL_CHANNEL_CUTOFF) {
                m_rate[i] = 0;
            }
            m_rate[i] -= m_target[i];
        }
    } else {
        for (int i = 0; i < 3; ++i) {
            m_rate[i] <<= MATERIAL_FRACTION_BITS;
            if (m_rate[i] > MATERIAL_CHANNEL_MAX) {
                m_rate[i] = MATERIAL_CHANNEL_MAX;
            } else if (m_rate[i] < MATERIAL_CHANNEL_CUTOFF) {
                m_rate[i] = 0;
            }
        }
    }
    /* Immediate commands write the computed relative deltas directly, too. */
    if (m_duration < 0.0001f) {
        for (int i = 0; i < 3; ++i) {
            m_target[i] = m_rate[i];
        }
        m_target = 0;
    } else {
        if (data->relative == 0) {
            for (int i = 0; i < 3; ++i) {
                m_rate[i] -= m_target[i];
            }
        }
        float inverseDuration = 1.0f / m_duration;
        for (int i = 0; i < 3; ++i) {
            m_rate[i] = static_cast<int>(m_rate[i] * inverseDuration);
        }
        m_duration += 0.0001f;
    }
}

bool EffCmdMatColorExec::Update(float deltaTime) {
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
    for (int i = 0; i < 3; ++i) {
        if (m_rate[i] != 0) {
            m_target[i] += static_cast<int>(m_rate[i] * deltaTime);
            if (m_target[i] > MATERIAL_CHANNEL_MAX) {
                m_target[i] = MATERIAL_CHANNEL_MAX;
                m_rate[i] = 0;
            } else if (m_target[i] < MATERIAL_CHANNEL_CUTOFF) {
                m_target[i] = 0;
                m_rate[i] = 0;
            }
        }
    }
    return true;
}

} // namespace System
} // namespace Effect

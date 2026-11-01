/* EffCmdMatAlphaExec methods; the original source filename is unknown. */
#include "Game/EffectCommandExec.h"
#include "Game/EffectSystem.h"

namespace Effect {
namespace System {

EffCmdMatAlphaExec::EffCmdMatAlphaExec(const UnkCommandData *command, unsigned int *flags,
                                       int *target)
    : EffectCommandExec(command), m_flags(flags), m_target(target), m_elapsed(0.0f) {
    const UnkMatAlphaData *data = static_cast<const UnkMatAlphaData *>(m_command);
    m_rate = data->value;
    m_rate += static_cast<int>(data->range * fn_8042A5D0()->Random());
    /* A nonzero range takes a second sample in the original. */
    if (data->range != 0) {
        m_rate += static_cast<int>(data->range * fn_8042A5D0()->Random());
    }
    m_duration = data->duration;
    if (data->durationRange != 0) {
        m_duration += data->durationRange * fn_8042A5D0()->Random();
    }
    m_rate <<= MATERIAL_FRACTION_BITS;
    if (data->relative != 0) {
        m_rate += *m_target;
        if (m_rate > MATERIAL_CHANNEL_MAX) {
            m_rate = MATERIAL_CHANNEL_MAX;
        } else if (m_rate < MATERIAL_CHANNEL_CUTOFF) {
            m_rate = 0;
        }
        m_rate -= *m_target;
    } else {
        if (m_rate > MATERIAL_CHANNEL_MAX) {
            m_rate = MATERIAL_CHANNEL_MAX;
        } else if (m_rate < MATERIAL_CHANNEL_CUTOFF) {
            m_rate = 0;
        }
    }
    /* Immediate commands write the computed relative delta directly, too. */
    if (m_duration < 0.0001f) {
        *m_target = m_rate;
        m_target = 0;
    } else {
        if (data->relative == 0) {
            m_rate -= *m_target;
        }
        m_rate = static_cast<int>(m_rate / m_duration);
        m_duration += 0.0001f;
    }
}

bool EffCmdMatAlphaExec::Update(float deltaTime) {
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
    if (m_rate != 0) {
        *m_target += static_cast<int>(m_rate * deltaTime);
        if (*m_target > MATERIAL_CHANNEL_MAX) {
            *m_target = MATERIAL_CHANNEL_MAX;
            m_rate = 0;
        } else if (*m_target < MATERIAL_CHANNEL_CUTOFF) {
            /* Dropping the target reports termination on the next update. */
            m_target = 0;
            m_rate = 0;
        }
    }
    return true;
}

} // namespace System
} // namespace Effect

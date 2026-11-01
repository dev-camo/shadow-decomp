/* Wait executor methods; the surrounding command unit is not yet reconstructed. */
#include "Game/EffectCommandExec.h"

namespace Effect {
namespace System {

EffCmdWaitExec::EffCmdWaitExec(const UnkCommandData *command, unsigned int *flags,
                               EffectCommandExec *last)
    : EffectCommandExec(command), m_flags(flags), m_elapsed(0.0f) {
    const UnkWaitData *data = static_cast<const UnkWaitData *>(m_command);
    m_duration = data->duration;
    if (data->duration != 0) {
        m_duration += 0.0001f;
    }
    m_last = last;
}

bool EffCmdWaitExec::Update(float deltaTime) {
    if (m_duration < 0.0001f) {
        if (m_last == 0 || m_last != m_prev) {
            if (m_command->stop != 0) {
                *m_flags |= COMMAND_STOP_REQUESTED;
            }
            return false;
        }
    } else {
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
    }
    return true;
}

} // namespace System
} // namespace Effect

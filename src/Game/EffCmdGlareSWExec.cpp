/* Glare switch command methods; kept separate until the surrounding command unit matches. */
#include "Game/EffectCommandExec.h"

namespace Effect {
namespace System {

EffCmdGlareSWExec::EffCmdGlareSWExec(const UnkCommandData *command, unsigned int *flags)
    : EffectCommandExec(command), m_flags(flags) {
    const UnkGlareSWData *data = static_cast<const UnkGlareSWData *>(m_command);
    *m_flags |= COMMAND_GLARE_CHANGED;
    if (data->enabled != 0) {
        *m_flags |= COMMAND_GLARE_ENABLED;
    } else {
        *m_flags &= ~COMMAND_GLARE_ENABLED;
    }
}

bool EffCmdGlareSWExec::Update(float) {
    if (m_command->stop != 0) {
        *m_flags |= COMMAND_STOP_REQUESTED;
    }
    return false;
}

} // namespace System
} // namespace Effect

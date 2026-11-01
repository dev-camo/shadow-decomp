/* Stop-request command methods; the original source filename is unknown. */
#include "Game/EffectCommandExec.h"
#include "Game/EffectElement.h"

namespace Effect {
namespace System {

EffCmdRequestStopExec::EffCmdRequestStopExec(const UnkCommandData *command, unsigned int *flags,
                                             EffectElement *target)
    : EffectCommandExec(command), m_flags(flags) {
    if (target != 0) {
        target->RequestStop();
    }
}

bool EffCmdRequestStopExec::Update(float) {
    if (m_command->stop != 0) {
        *m_flags |= COMMAND_STOP_REQUESTED;
    }
    return false;
}

} // namespace System
} // namespace Effect

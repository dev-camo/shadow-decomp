/* Callback and loop executors from the effect command interpreter. */
#include "Game/EffectCommandExec.h"

namespace Effect {
namespace System {

EffCmdLoopStartExec::EffCmdLoopStartExec(const UnkCommandData *command, unsigned int *flags)
    : EffectCommandExec(command), m_flags(flags) {
    m_count = m_remaining = static_cast<const UnkLoopStartData *>(m_command)->count;
    m_finished = false;
}

bool EffCmdLoopStartExec::Update(float) {
    if (m_finished) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    return true;
}

EffCmdLoopEndExec::EffCmdLoopEndExec(const UnkCommandData *command, unsigned int *flags,
                                     EffectCommandExec *last, const UnkCommandData **restart)
    : EffectCommandExec(command), m_flags(flags), m_finished(false) {
    EffectCommandExec *current = last;
    while (current != 0) {
        if (dynamic_cast<EffCmdLoopStartExec *>(current) != 0) {
            EffCmdLoopStartExec *loop = dynamic_cast<EffCmdLoopStartExec *>(current);
            if (loop->m_finished) {
                current = current->m_prev;
                continue;
            }
            if (restart == 0) {
                current = 0;
                loop->m_finished = true;
            } else if (loop->m_count != 0) {
                if (loop->m_remaining == 0) {
                    current = 0;
                    loop->m_finished = true;
                } else {
                    loop->m_remaining--;
                    *restart = loop->m_command;
                }
            } else {
                *restart = loop->m_command;
            }
            break;
        }
        current = current->m_prev;
    }
    if (current == 0) {
        m_finished = true;
    }
}

bool EffCmdLoopEndExec::Update(float) {
    if (m_finished && m_command->stop != 0) {
        *m_flags |= COMMAND_STOP_REQUESTED;
    }
    return false;
}

EffCmdCallbackExec::EffCmdCallbackExec(const UnkCommandData *command, unsigned int *flags)
    : EffectCommandExec(command), m_flags(flags) {}

bool EffCmdCallbackExec::Update(float) {
    if (m_command->stop != 0) {
        *m_flags |= COMMAND_STOP_REQUESTED;
    }
    return false;
}

} // namespace System
} // namespace Effect

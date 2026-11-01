/* EffectArranger routines; original filename unknown. */
#include "Game/EffectArranger.h"
#include "Game/EffectCommandExec.h"
#include "Game/UnkEffectFileData.h"
#include "Game/UnkEffectPool.h"
#include "Game/fn_8042A444.h"

namespace Effect {
namespace System {

/* Inferred predicate: whether a command list still contains a launch. */
static inline bool HasLaunchCommand(EffectCommandExec *command) {
    for (; command != 0; command = command->m_next) {
        if (command->m_command->type == COMMAND_LAUNCH ||
            command->m_command->type == COMMAND_TIMED_LAUNCH) {
            return true;
        }
    }
    return false;
}

void EffectArranger::Update(float deltaTime) {
    if ((m_flags & ELEMENT_ACTIVE) == 0) {
        return;
    }
    if ((m_flags & ELEMENT_FLAG_4) != 0) {
        fn_8042A5C0(fn_8042A5D0(), m_launchPool);
        EffectCommandExec *command = m_commandHead;
        while (command != 0) {
            if (command->m_command->type != COMMAND_LAUNCH &&
                command->m_command->type != COMMAND_TIMED_LAUNCH) {
                command = command->m_next;
                continue;
            }
            if (!command->Update(deltaTime)) {
                if (command->m_prev != 0) {
                    command->m_prev->m_next = command->m_next;
                }
                if (command->m_next != 0) {
                    command->m_next->m_prev = command->m_prev;
                }
                if (command == m_commandHead) {
                    m_commandHead = command->m_next;
                }
                if (command == m_commandTail) {
                    m_commandTail = command->m_prev;
                }
                EffectCommandExec *next = command->m_next;
                delete command;
                command = next;
            } else {
                command = command->m_next;
            }
        }
        return;
    }
    const UnkEffectElementData3 *definition =
        static_cast<const UnkEffectElementData3 *>(m_definition);
    if ((m_flags & ELEMENT_FLAG_2) != 0) {
        m_elapsed += deltaTime;
    }
    if ((m_definitionFlags & 1) == 0 && m_elapsed > 0.0001f + definition->field_0x34) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    fn_80426700();
    if ((m_commandFlags & COMMAND_STOP_REQUESTED) == 0) {
        fn_8042A5C0(fn_8042A5D0(), m_commandPool);
        EffectCommandExec *command = m_commandHead;
        while (command != 0) {
            if (command->m_command->type == COMMAND_LAUNCH ||
                command->m_command->type == COMMAND_TIMED_LAUNCH) {
                command = command->m_next;
                continue;
            }
            if (!command->Update(deltaTime)) {
                if (command->m_prev != 0) {
                    command->m_prev->m_next = command->m_next;
                }
                if (command->m_next != 0) {
                    command->m_next->m_prev = command->m_prev;
                }
                if (command == m_commandHead) {
                    m_commandHead = command->m_next;
                }
                if (command == m_commandTail) {
                    m_commandTail = command->m_prev;
                }
                EffectCommandExec *next = command->m_next;
                delete command;
                command = next;
            } else {
                command = command->m_next;
            }
        }
    }
    if ((m_commandFlags & COMMAND_STOP_REQUESTED) != 0) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    fn_80425C34(this, deltaTime);
    fn_8042A5C0(fn_8042A5D0(), m_launchPool);
    EffectCommandExec *command = m_commandHead;
    while (command != 0) {
        if (command->m_command->type != COMMAND_LAUNCH &&
            command->m_command->type != COMMAND_TIMED_LAUNCH) {
            command = command->m_next;
            continue;
        }
        if (!command->Update(deltaTime)) {
            if (command->m_prev != 0) {
                command->m_prev->m_next = command->m_next;
            }
            if (command->m_next != 0) {
                command->m_next->m_prev = command->m_prev;
            }
            if (command == m_commandHead) {
                m_commandHead = command->m_next;
            }
            if (command == m_commandTail) {
                m_commandTail = command->m_prev;
            }
            EffectCommandExec *next = command->m_next;
            delete command;
            command = next;
        } else {
            command = command->m_next;
        }
    }
    if ((m_flags & ELEMENT_STOP_REQUESTED) != 0) {
        bool hasLaunchCommand = HasLaunchCommand(m_commandHead);
        if (!hasLaunchCommand && ((m_definitionFlags & 0x40000) == 0 || m_commandHead == 0)) {
            m_flags &= ~ELEMENT_ACTIVE;
        }
    }
}

void EffectArranger::fn_80426700() {
    fn_8042A5C0(fn_8042A5D0(), m_commandPool);
    while (m_command != 0) {
        if (m_commandTail != 0 && dynamic_cast<EffCmdWaitExec *>(m_commandTail) != 0) {
            return;
        }
        EffectCommandExec *command = 0;
        unsigned int stride;
        switch (m_command->type) {
        case COMMAND_3D_POSITION:
            command = new EffCmd3DPosExec(m_command, &m_commandFlags, &m_translation);
            stride = sizeof(UnkVector3Data);
            break;
        case COMMAND_3D_ROTATION:
            command = new EffCmd3DRotExec(m_command, &m_commandFlags, &m_rotation);
            stride = sizeof(UnkVector3Data);
            break;
        case COMMAND_3D_SCALE:
            command = new EffCmd3DSclExec(m_command, &m_commandFlags, &m_scale);
            stride = sizeof(UnkVector3Data);
            break;
        case COMMAND_3D_VELOCITY:
            command = new EffCmd3DVelExec(m_command, &m_commandFlags, &m_velocity);
            stride = sizeof(UnkVector3Data);
            break;
        case COMMAND_WAIT:
            command = new EffCmdWaitExec(m_command, &m_commandFlags, m_commandTail);
            stride = sizeof(UnkWaitData);
            break;
        case COMMAND_LOOP_START:
            command = new EffCmdLoopStartExec(m_command, &m_commandFlags);
            stride = sizeof(UnkLoopStartData);
            ++field_0x4;
            if (field_0x5 != 0 || (m_flags & ELEMENT_STOP_REQUESTED) != 0) {
                ++field_0x5;
            }
            break;
        case COMMAND_LOOP_END: {
            const UnkCommandData *restart = 0;
            if (field_0x5 != 0 || (m_flags & ELEMENT_STOP_REQUESTED) != 0) {
                command = new EffCmdLoopEndExec(m_command, &m_commandFlags, m_commandTail, 0);
                if (field_0x5 != 0) {
                    --field_0x5;
                }
            } else {
                command =
                    new EffCmdLoopEndExec(m_command, &m_commandFlags, m_commandTail, &restart);
            }
            if (restart != 0) {
                m_command = restart;
                stride = sizeof(UnkLoopStartData);
            } else {
                stride = sizeof(UnkCommandData);
                --field_0x4;
            }
            break;
        }
        case COMMAND_CALLBACK:
            command = new EffCmdCallbackExec(m_command, &m_commandFlags);
            stride = COMMAND_CALLBACK_STRIDE;
            break;
        case COMMAND_LAUNCH:
            if ((m_flags & ELEMENT_STOP_REQUESTED) == 0 || (m_definitionFlags & 0x40000) != 0) {
                fn_8042A5C0(fn_8042A5D0(), m_launchPool);
                command =
                    new EffCmdLaunchExec(m_command, &m_commandFlags, m_manager, m_launchInfo, this);
                fn_8042A5C0(fn_8042A5D0(), m_commandPool);
            }
            stride = sizeof(UnkLaunchData);
            break;
        case COMMAND_TIMED_LAUNCH:
            if ((m_flags & ELEMENT_STOP_REQUESTED) == 0 || (m_definitionFlags & 0x40000) != 0) {
                fn_8042A5C0(fn_8042A5D0(), m_launchPool);
                command =
                    new EffCmdLaunchExec(m_command, &m_commandFlags, m_manager, m_launchInfo, this);
                fn_8042A5C0(fn_8042A5D0(), m_commandPool);
            }
            stride = sizeof(UnkTimedLaunchData);
            break;
        case COMMAND_REQUEST_STOP:
            command = new EffCmdRequestStopExec(m_command, &m_commandFlags, this);
            stride = sizeof(UnkCommandData);
            break;
        default:
            return;
        }
        if (command != 0) {
            if (m_commandTail != 0) {
                m_commandTail->m_next = command;
                command->m_prev = m_commandTail;
                m_commandTail = command;
            } else {
                m_commandHead = m_commandTail = command;
            }
        } else {
            m_commandFlags |= COMMAND_STOP_REQUESTED;
            return;
        }
        if (m_command->hasNext != 0 && stride != 0) {
            m_command = reinterpret_cast<const UnkCommandData *>(
                reinterpret_cast<const unsigned char *>(m_command) + stride);
        } else {
            m_command = 0;
        }
    }
}

EffectArranger::EffectArranger(const UnkEffectElementData3 *definition, EffectLaunchInfo *info,
                               EffectManager *manager)
    : EffectElement(definition, info, manager), m_commandFlags(0), m_definitionFlags(0),
      m_launchInfo(0), m_command(0), m_commandHead(0), m_commandTail(0), m_commandPool(0),
      m_launchPool(0) {
    m_size -= sizeof(EffectElement);
    m_size += sizeof(EffectArranger);
    if ((m_flags & ELEMENT_ACTIVE) == 0) {
        return;
    }
    m_definitionFlags = definition->field_0x2C;
    m_commandPool = new UnkEffectPool(0x30, definition->field_0x30);
    if (m_commandPool == 0 ||
        (definition->field_0x30 != 0 &&
         m_commandPool->m_blockStride * (m_commandPool->m_usedCount + m_commandPool->m_freeCount) ==
             0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    m_size +=
        m_commandPool->m_blockStride * (m_commandPool->m_usedCount + m_commandPool->m_freeCount) +
        sizeof(UnkEffectPool);
    m_launchPool = new UnkEffectPool(0x80, definition->field_0x32);
    if (m_launchPool == 0 ||
        (definition->field_0x32 != 0 &&
         m_launchPool->m_blockStride * (m_launchPool->m_usedCount + m_launchPool->m_freeCount) ==
             0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    /* The original accounts for the command pool again here. */
    m_size +=
        m_commandPool->m_blockStride * (m_commandPool->m_usedCount + m_commandPool->m_freeCount) +
        sizeof(UnkEffectPool);
    m_command = definition->field_0x38;
}

EffectArranger::~EffectArranger() {
    EffectCommandExec *command = m_commandHead;
    while (command != 0) {
        EffectCommandExec *next = command->m_next;
        /* The original returns only type 0x11 to the launch pool; timed launches (0x12)
         * are released through the command pool. */
        if (command->m_command->type == COMMAND_LAUNCH) {
            fn_8042A5C0(fn_8042A5D0(), m_launchPool);
        } else {
            fn_8042A5C0(fn_8042A5D0(), m_commandPool);
        }
        delete command;
        command = next;
    }
    if (m_commandPool != 0) {
        delete m_commandPool;
    }
    if (m_launchPool != 0) {
        delete m_launchPool;
    }
}

} // namespace System
} // namespace Effect

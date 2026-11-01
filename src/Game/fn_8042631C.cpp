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
        if (command->m_command->type == 0x11 || command->m_command->type == 0x12) {
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
            if (command->m_command->type != 0x11 && command->m_command->type != 0x12) {
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
            if (command->m_command->type == 0x11 || command->m_command->type == 0x12) {
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
        if (command->m_command->type != 0x11 && command->m_command->type != 0x12) {
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
         m_commandPool->field_0x1C * (m_commandPool->field_0x14 + m_commandPool->field_0x10) ==
             0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    m_size += m_commandPool->field_0x1C * (m_commandPool->field_0x14 + m_commandPool->field_0x10) +
              sizeof(UnkEffectPool);
    m_launchPool = new UnkEffectPool(0x80, definition->field_0x32);
    if (m_launchPool == 0 ||
        (definition->field_0x32 != 0 &&
         m_launchPool->field_0x1C * (m_launchPool->field_0x14 + m_launchPool->field_0x10) == 0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    /* The original accounts for the command pool again here. */
    m_size += m_commandPool->field_0x1C * (m_commandPool->field_0x14 + m_commandPool->field_0x10) +
              sizeof(UnkEffectPool);
    m_command = static_cast<const UnkCommandData *>(definition->field_0x38);
}

EffectArranger::~EffectArranger() {
    EffectCommandExec *command = m_commandHead;
    while (command != 0) {
        EffectCommandExec *next = command->m_next;
        if (command->m_command->type == 0x11) {
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

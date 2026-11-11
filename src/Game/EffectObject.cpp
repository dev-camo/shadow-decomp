/* EffectObject methods; original filename unknown. */
#include "Game/EffectObject.h"
#include "Game/EffectCommandExec.h"
#include "Game/UnkEffectFileData.h"
#include "Game/UnkEffectPool.h"

namespace Effect {
namespace System {

void UnkEffectPool::operator delete(void *block) {
    fn_8042A5D0()->Free(block);
}

EffectObject::EffectObject(UnkEffectElementData4 *definition, EffectLaunchInfo *info,
                           EffectManager *manager)
    : EffectElement(definition, info, manager), m_commandFlags(0), m_definitionFlags(0),
      m_command(0), m_commandHead(0), m_commandTail(0), m_pool(0) {
    m_size -= sizeof(EffectElement);
    m_size += sizeof(EffectObject);
    if (!(m_flags & ELEMENT_ACTIVE)) {
        return;
    }
    m_definitionFlags = definition->field_0x2C;
    m_pool = new UnkEffectPool(sizeof(EffCmdVector3Exec), definition->field_0x30);
    if (m_pool == 0 || (definition->field_0x30 != 0 &&
                        m_pool->m_blockStride * (m_pool->m_usedCount + m_pool->m_freeCount) == 0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    m_size +=
        m_pool->m_blockStride * (m_pool->m_usedCount + m_pool->m_freeCount) + sizeof(UnkEffectPool);
    m_color[0] = (definition->field_0x39[0] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_color[1] = (definition->field_0x39[1] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_color[2] = (definition->field_0x39[2] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_alpha = (definition->field_0x38 & 0xFF) << MATERIAL_FRACTION_BITS;
    m_command = definition->field_0x40;
}

EffectObject::~EffectObject() {
    EffectCommandExec *command = m_commandHead;
    fn_8042A5C0(fn_8042A5D0(), m_pool);
    while (command != 0) {
        EffectCommandExec *next = command->m_next;
        delete command;
        command = next;
    }
    if (m_pool != 0) {
        delete m_pool;
    }
}

void EffectObject::fn_80421834() {
    while (m_command != 0) {
        if (m_commandTail != 0 && dynamic_cast<EffCmdWaitExec *>(m_commandTail) != 0) {
            return;
        }
        EffectCommandExec *command;
        unsigned int stride;
        switch (m_command->type) {
        case COMMAND_GLARE_SWITCH:
            command = new EffCmdGlareSWExec(m_command, &m_commandFlags);
            stride = COMMAND_GLARE_SWITCH_STRIDE;
            break;
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
        case COMMAND_MAT_COLOR:
            command = new EffCmdMatColorExec(m_command, &m_commandFlags, m_color);
            stride = COMMAND_MAT_COLOR_STRIDE;
            break;
        case COMMAND_MAT_ALPHA:
            command = new EffCmdMatAlphaExec(m_command, &m_commandFlags, &m_alpha);
            stride = COMMAND_MAT_ALPHA_STRIDE;
            break;
        case COMMAND_WAIT:
            command = new EffCmdWaitExec(m_command, &m_commandFlags, m_commandTail);
            stride = sizeof(UnkWaitData);
            break;
        case COMMAND_LOOP_START:
            command = new EffCmdLoopStartExec(m_command, &m_commandFlags);
            stride = sizeof(UnkLoopStartData);
            ++field_0x4;
            if (field_0x5 != 0 || (m_flags & ELEMENT_STOP_REQUESTED)) {
                ++field_0x5;
            }
            break;
        case COMMAND_LOOP_END: {
            const UnkCommandData *restart = 0;
            if (field_0x5 != 0 || (m_flags & ELEMENT_STOP_REQUESTED)) {
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
                --field_0x4;
                stride = sizeof(UnkCommandData);
            }
            break;
        }
        case COMMAND_CALLBACK:
            command = new EffCmdCallbackExec(m_command, &m_commandFlags);
            stride = COMMAND_CALLBACK_STRIDE;
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

void EffectObject::Update(float deltaTime) {
    if (!(m_flags & ELEMENT_ACTIVE)) {
        return;
    }
    if (m_flags & ELEMENT_FLAG_4) {
        return;
    }
    if ((m_flags & ELEMENT_STOP_REQUESTED) &&
        (!(m_definitionFlags & 0x40000) || m_commandHead == 0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    const UnkEffectElementData4 *definition =
        static_cast<const UnkEffectElementData4 *>(m_definition);
    if (m_flags & ELEMENT_FLAG_2) {
        m_elapsed += deltaTime;
    }
    if (!(m_definitionFlags & 1) && m_elapsed > 0.0001f + definition->field_0x34) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    fn_8042A5C0(fn_8042A5D0(), m_pool);
    fn_80421834();
    if (!(m_commandFlags & COMMAND_STOP_REQUESTED)) {
        EffectCommandExec *next;
        EffectCommandExec *command = m_commandHead;
        while (command != 0) {
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
                next = command->m_next;
                delete command;
                command = next;
            } else {
                command = command->m_next;
            }
        }
    }
    if (m_commandFlags & COMMAND_STOP_REQUESTED) {
        m_flags &= ~ELEMENT_ACTIVE;
    } else {
        fn_80425C34(this, deltaTime);
    }
}

} // namespace System
} // namespace Effect

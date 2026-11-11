/* EffectRay methods; class identities come from RTTI, original filename unknown. */
#include "Game/EffectRay.h"
#include "Game/EffectCommandExec.h"
#include "Game/EffectRayExecUnit.h"
#include "Game/UnkEffectFileData.h"
#include "Game/UnkEffectPool.h"
#include "Game/fn_804220CC.h"
#include <cmath>

namespace Effect {
namespace System {

EffCmdTopColorExec::~EffCmdTopColorExec() {}

EffCmdTopAlphaExec::~EffCmdTopAlphaExec() {}

EffCmdLastColorExec::~EffCmdLastColorExec() {}

EffCmdLastAlphaExec::~EffCmdLastAlphaExec() {}

EffCmdLengthExec::~EffCmdLengthExec() {}

EffCmdWidthExec::~EffCmdWidthExec() {}

EffectRayExecUnit::EffectRayExecUnit(const UnkEffectElementData6 *definition,
                                     UnkEffectPool *commandPool)
    : m_flags(0), m_elapsed(0.0f), m_length(definition->field_0x3C),
      m_width(definition->field_0x40), m_prev(0), m_next(0), m_command(0), m_commandHead(0),
      m_commandTail(0), m_commandPool(commandPool) {
    m_color[0] = (definition->field_0x50[1] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_color[1] = (definition->field_0x50[2] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_color[2] = (definition->field_0x50[3] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_alpha = (definition->field_0x50[0] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_topColor[0] = m_topColor[1] = m_topColor[2] = m_topAlpha = MATERIAL_CHANNEL_MAX;
    m_lastColor[0] = m_lastColor[1] = m_lastColor[2] = m_lastAlpha = MATERIAL_CHANNEL_MAX;
}

EffectRayExecUnit::~EffectRayExecUnit() {
    EffectCommandExec *command = m_commandHead;
    EffectCommandExec *next;
    UnkEffectPool *previousPool = fn_8042A5C0(fn_8042A5D0(), m_commandPool);
    while (command != 0) {
        next = command->m_next;
        delete command;
        command = next;
    }
    fn_8042A5C0(fn_8042A5D0(), previousPool);
}

void EffectRayExecUnit::fn_80429B48() {
    while (m_command != 0) {
        if (m_commandTail != 0 && dynamic_cast<EffCmdWaitExec *>(m_commandTail) != 0) {
            return;
        }
        EffectCommandExec *command = 0;
        unsigned int stride = 0;
        switch (m_command->type) {
        case COMMAND_3D_POSITION:
            command = new EffCmd3DPosExec(m_command, &m_flags, &m_position);
            stride = sizeof(UnkVector3Data);
            break;
        case COMMAND_3D_VELOCITY:
            command = new EffCmd3DVelExec(m_command, &m_flags, &m_velocity);
            stride = sizeof(UnkVector3Data);
            break;
        case COMMAND_MAT_COLOR:
            command = new EffCmdMatColorExec(m_command, &m_flags, m_color);
            stride = COMMAND_MAT_COLOR_STRIDE;
            break;
        case COMMAND_MAT_ALPHA:
            command = new EffCmdMatAlphaExec(m_command, &m_flags, &m_alpha);
            stride = COMMAND_MAT_ALPHA_STRIDE;
            break;
        case COMMAND_TOP_COLOR:
            command = new EffCmdTopColorExec(m_command, &m_flags, m_topColor);
            stride = COMMAND_MAT_COLOR_STRIDE;
            break;
        case COMMAND_TOP_ALPHA:
            command = new EffCmdTopAlphaExec(m_command, &m_flags, &m_topAlpha);
            stride = COMMAND_MAT_ALPHA_STRIDE;
            break;
        case COMMAND_LAST_COLOR:
            command = new EffCmdLastColorExec(m_command, &m_flags, m_lastColor);
            stride = COMMAND_MAT_COLOR_STRIDE;
            break;
        case COMMAND_LAST_ALPHA:
            command = new EffCmdLastAlphaExec(m_command, &m_flags, &m_lastAlpha);
            stride = COMMAND_MAT_ALPHA_STRIDE;
            break;
        case COMMAND_WAIT:
            command = new EffCmdWaitExec(m_command, &m_flags, m_commandTail);
            stride = sizeof(UnkWaitData);
            break;
        case COMMAND_LOOP_START:
            command = new EffCmdLoopStartExec(m_command, &m_flags);
            stride = sizeof(UnkLoopStartData);
            break;
        case COMMAND_LOOP_END: {
            const UnkCommandData *restart = 0;
            command = new EffCmdLoopEndExec(m_command, &m_flags, m_commandTail, &restart);
            if (restart != 0) {
                m_command = restart;
                stride = sizeof(UnkLoopStartData);
            } else {
                stride = sizeof(UnkCommandData);
            }
            break;
        }
        case COMMAND_CALLBACK:
            command = new EffCmdCallbackExec(m_command, &m_flags);
            stride = COMMAND_CALLBACK_STRIDE;
            break;
        case COMMAND_LENGTH:
            command = new EffCmdLengthExec(m_command, &m_flags, &m_length);
            stride = sizeof(UnkSingleData);
            break;
        case COMMAND_WIDTH:
            command = new EffCmdWidthExec(m_command, &m_flags, &m_width);
            stride = sizeof(UnkSingleData);
            break;
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
            m_flags |= COMMAND_STOP_REQUESTED;
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

EffectRay::EffectRay(UnkEffectElementData6 *definition, EffectLaunchInfo *info,
                     EffectManager *manager)
    : EffectElement(definition, info, manager), m_head(0), m_tail(0), m_commandFlags(0),
      m_definitionFlags(0), m_count(0), m_friction(0.0f), m_command(0), m_commandHead(0),
      m_commandTail(0), m_commandPool(0), m_rayPool(0) {
    m_size -= sizeof(EffectElement);
    m_size += sizeof(EffectRay);
    if ((m_flags & ELEMENT_ACTIVE) == 0) {
        return;
    }
    m_capacity = definition->emitter.capacity;
    m_genQuantity = definition->emitter.generationRate;
    m_definitionFlags |= definition->emitter.flags;
    m_definitionFlags |= definition->field_0x2C;
    m_definitionFlags |= definition->field_0x48;
    if (definition->emitter.shape != 0) {
        m_definitionFlags |= definition->emitter.shape->flags;
    }
    if ((m_definitionFlags & 0x400) != 0) {
        m_friction = definition->field_0x44;
    }
    m_rayPool = new UnkEffectPool(sizeof(EffectRayExecUnit), m_capacity);
    if (m_rayPool == 0 ||
        (m_capacity != 0 &&
         m_rayPool->m_blockStride * (m_rayPool->m_usedCount + m_rayPool->m_freeCount) == 0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    m_size += m_rayPool->m_blockStride * (m_rayPool->m_usedCount + m_rayPool->m_freeCount) +
              sizeof(UnkEffectPool);
    int commandCount = definition->emitter.commandCount;
    if (commandCount == 0) {
        commandCount = m_capacity * fn_804220CC(definition->field_0x54);
        commandCount += fn_804220CC(definition->emitter.command);
    }
    m_commandPool = new UnkEffectPool(0x30, commandCount);
    if (m_commandPool == 0 ||
        (commandCount != 0 &&
         m_commandPool->m_blockStride * (m_commandPool->m_usedCount + m_commandPool->m_freeCount) ==
             0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    m_size +=
        m_commandPool->m_blockStride * (m_commandPool->m_usedCount + m_commandPool->m_freeCount) +
        sizeof(UnkEffectPool);
    m_command = definition->emitter.command;
}

EffectRay::~EffectRay() {
    fn_8042A5C0(fn_8042A5D0(), m_rayPool);
    EffectRayExecUnit *ray = m_head;
    while (ray != 0) {
        EffectRayExecUnit *next = ray->m_next;
        delete ray;
        ray = next;
    }
    m_head = 0;
    m_tail = 0;
    EffectCommandExec *command = m_commandHead;
    fn_8042A5C0(fn_8042A5D0(), m_commandPool);
    while (command != 0) {
        EffectCommandExec *next = command->m_next;
        delete command;
        command = next;
    }
    if (m_rayPool != 0) {
        delete m_rayPool;
    }
    if (m_commandPool != 0) {
        delete m_commandPool;
    }
}

void EffectRay::fn_804293B4() {
    while (m_command != 0) {
        if (m_commandTail != 0 && dynamic_cast<EffCmdWaitExec *>(m_commandTail) != 0) {
            return;
        }
        EffectCommandExec *command = 0;
        unsigned int stride = 0;
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
        case COMMAND_FRICTION:
            command = new EffCmdFrictionExec(m_command, &m_commandFlags, &m_friction);
            stride = sizeof(UnkSingleData);
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
        case COMMAND_GEN_QUANTITY:
            command = new EffCmdGenQuantityExec(m_command, &m_commandFlags, &m_genQuantity);
            stride = sizeof(UnkSingleData);
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

void EffectRay::fn_80428DFC(float deltaTime) {
    int quantity = 0;
    float generated = m_genQuantity * deltaTime;
    const UnkEffectElementData6 *definition =
        static_cast<const UnkEffectElementData6 *>(m_definition);
    if (generated < 1.0f) {
        if (fn_8042A5D0()->Random() < generated) {
            quantity = 1;
        }
    } else {
        quantity = generated;
    }
    if (quantity <= 0) {
        return;
    }
    for (int i = 0; i < quantity; ++i) {
        if (m_capacity == m_count) {
            break;
        }
        fn_8042A5C0(fn_8042A5D0(), m_rayPool);
        EffectRayExecUnit *ray = new EffectRayExecUnit(
            static_cast<const UnkEffectElementData6 *>(m_definition), m_commandPool);
        if (ray == 0) {
            break;
        }
        ++m_count;
        if (m_tail != 0) {
            m_tail->m_next = ray;
            ray->m_prev = m_tail;
            m_tail = ray;
        } else {
            m_head = m_tail = ray;
        }
        float speed = definition->field_0x34 * fn_8042A5D0()->Random() + definition->field_0x30;
        const UnkEffectParticleShape *shape = definition->emitter.shape;
        /* As in the original spawning path, the shape pointer is required here. */
        switch (shape->type) {
        default: {
            ray->m_position = m_previousTranslation;
            /* The original default-shape path writes only y before this vector assignment;
             * Fn8040Vector3's empty default constructor leaves x and z uninitialized. */
            Fn8040Vector3 velocity;
            velocity.y = speed;
            ray->m_velocity = velocity;
            break;
        }
        case 0: {
            const UnkEffectParticleShape0 *data =
                static_cast<const UnkEffectParticleShape0 *>(shape);
            float angleRange = data->angle;
            float angle;
            if ((m_definitionFlags & 0x40) != 0 && quantity > 1) {
                if (angleRange > 360.0f - 0.0001f) {
                    angle = angleRange * (static_cast<float>(i) / quantity);
                } else {
                    angle = angleRange * (static_cast<float>(i) / (quantity - 1));
                }
                angle -= angleRange / 2.0f;
            } else {
                angle = angleRange * fn_8042A5D0()->Random() - data->angle / 2.0f;
            }
            float radius = 0.0f;
            float spread = 0.0f;
            if (data->radius > 0.0001f) {
                if ((m_definitionFlags & 0x80) != 0) {
                    radius = data->radius;
                } else {
                    radius = data->radius * fn_8042A5D0()->Random();
                }
                spread = data->spread * (radius / data->radius);
            }
            float sine = std::sin(angle);
            float height = 0.0f;
            float cosine = std::cos(angle);
            if (data->height > 0.0001f) {
                height = data->height * fn_8042A5D0()->Random();
            }
            ray->m_position.Set(sine * radius, height, cosine * radius);
            float spreadSine = std::sin(spread);
            float spreadCosine = std::cos(spread);
            ray->m_velocity.Set(sine * (spreadSine * speed), spreadCosine * speed,
                                cosine * (spreadSine * speed));
            break;
        }
        case 1: {
            const UnkEffectParticleShape1 *data =
                static_cast<const UnkEffectParticleShape1 *>(shape);
            float angleRange = data->angle;
            float angle = angleRange * fn_8042A5D0()->Random() - data->angle;
            float radius = 0.0f;
            if (data->radius > 0.0001f) {
                if ((m_definitionFlags & 0x80) != 0) {
                    radius = data->radius;
                } else {
                    radius = data->radius * fn_8042A5D0()->Random();
                }
            }
            float spread = data->spread * fn_8042A5D0()->Random();
            float spreadSine = std::sin(spread);
            float spreadCosine = std::cos(spread);
            float x = spreadSine * std::sin(angle);
            float z = spreadSine * std::cos(angle);
            ray->m_position.Set(x * radius, spreadCosine * radius, z * radius);
            ray->m_velocity.Set(x * speed, spreadCosine * speed, z * speed);
            if ((m_definitionFlags & 0x20) != 0) {
                ray->m_velocity.x = -ray->m_velocity.x;
                ray->m_velocity.y = -ray->m_velocity.y;
                ray->m_velocity.z = -ray->m_velocity.z;
            }
            break;
        }
        case 2: {
            const UnkEffectParticleShape2 *data =
                static_cast<const UnkEffectParticleShape2 *>(shape);
            float width = data->width;
            ray->m_position.x = width * fn_8042A5D0()->Random() - width * 0.5f;
            float height = data->height;
            ray->m_position.y = height * fn_8042A5D0()->Random() - height * 0.5f;
            float depth = data->depth;
            ray->m_position.z = depth * fn_8042A5D0()->Random() - depth * 0.5f;
            ray->m_velocity.Set(0.0f, speed, 0.0f);
            break;
        }
        }
        if ((m_definitionFlags & 0x800) == 0) {
            ray->m_position = fn_8040B67C(&m_transform, &ray->m_position);
            ray->m_velocity = fn_8040B618(m_transform, ray->m_velocity);
        }
        ray->m_origin = ray->m_position;
        ray->m_command = definition->field_0x54;
    }
}

void EffectRay::fn_80428B20(float deltaTime) {
    const UnkEffectElementData6 *definition =
        static_cast<const UnkEffectElementData6 *>(m_definition);
    if ((m_definitionFlags & 0x100) == 0) {
        float lifetime = definition->field_0x38;
        for (EffectRayExecUnit *ray = m_head; ray != 0; ray = ray->m_next) {
            if ((m_flags & ELEMENT_FLAG_2) != 0) {
                ray->m_elapsed += deltaTime;
            }
            if (ray->m_elapsed > 0.0001f + lifetime) {
                ray->m_flags |= COMMAND_STOP_REQUESTED;
            }
        }
    }
    EffectRayExecUnit *commandRay = m_head;
    fn_8042A5C0(fn_8042A5D0(), m_commandPool);
    for (; commandRay != 0; commandRay = commandRay->m_next) {
        if ((commandRay->m_flags & COMMAND_STOP_REQUESTED) != 0) {
            continue;
        }
        commandRay->fn_80429B48();
        if ((commandRay->m_flags & COMMAND_STOP_REQUESTED) != 0) {
            continue;
        }
        EffectCommandExec *command = commandRay->m_commandHead;
        while (command != 0) {
            if (!command->Update(deltaTime)) {
                if (command->m_prev != 0) {
                    command->m_prev->m_next = command->m_next;
                }
                if (command->m_next != 0) {
                    command->m_next->m_prev = command->m_prev;
                }
                if (command == commandRay->m_commandHead) {
                    commandRay->m_commandHead = command->m_next;
                }
                if (command == commandRay->m_commandTail) {
                    commandRay->m_commandTail = command->m_prev;
                }
                EffectCommandExec *next = command->m_next;
                delete command;
                command = next;
            } else {
                command = command->m_next;
            }
        }
    }
    EffectRayExecUnit *next;
    EffectRayExecUnit *ray = m_head;
    fn_8042A5C0(fn_8042A5D0(), m_rayPool);
    while (ray != 0) {
        next = ray->m_next;
        if ((ray->m_flags & COMMAND_STOP_REQUESTED) != 0) {
            if (ray->m_prev != 0) {
                ray->m_prev->m_next = ray->m_next;
            }
            if (ray->m_next != 0) {
                ray->m_next->m_prev = ray->m_prev;
            }
            if (ray == m_head) {
                m_head = ray->m_next;
            }
            if (ray == m_tail) {
                m_tail = ray->m_prev;
            }
            delete ray;
            --m_count;
        }
        ray = next;
    }
    for (ray = m_head; ray != 0; ray = ray->m_next) {
        AddVector(&ray->m_position, ray->m_velocity * deltaTime);
        if (m_friction != 0.0f) {
            float speed = fn_8040CF50(&ray->m_velocity);
            if (speed > 0.0001f) {
                float loss = speed * m_friction;
                if (loss > speed) {
                    fn_8040CE38(&ray->m_velocity, 0.0f);
                } else {
                    fn_8040CE38(&ray->m_velocity, (speed - loss) / speed);
                }
            }
        }
    }
}

void EffectRay::Update(float deltaTime) {
    if ((m_flags & ELEMENT_ACTIVE) != 0 && (m_flags & ELEMENT_FLAG_4) == 0) {
        const UnkEffectElementData6 *definition =
            static_cast<const UnkEffectElementData6 *>(m_definition);
        if ((m_flags & ELEMENT_FLAG_2) != 0) {
            m_elapsed += deltaTime;
        }
        if ((m_definitionFlags & 1) == 0 && m_elapsed > 0.0001f + definition->emitter.duration) {
            m_flags &= ~ELEMENT_ACTIVE;
            return;
        }
        fn_8042A5C0(fn_8042A5D0(), m_commandPool);
        fn_804293B4();
        if ((m_commandFlags & COMMAND_STOP_REQUESTED) == 0) {
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
        if ((m_commandFlags & COMMAND_STOP_REQUESTED) != 0) {
            m_flags &= ~ELEMENT_ACTIVE;
            return;
        }
        fn_80425C34(this, deltaTime);
        if ((m_flags & ELEMENT_STOP_REQUESTED) != 0) {
            if ((m_definitionFlags & 0x40000) != 0 && m_commandHead != 0) {
                fn_80428DFC(deltaTime);
                fn_80428B20(deltaTime);
            } else {
                fn_80428B20(deltaTime);
                if (m_count == 0) {
                    m_flags &= ~ELEMENT_ACTIVE;
                }
            }
        } else {
            fn_80428DFC(deltaTime);
            fn_80428B20(deltaTime);
        }
    }
}

} // namespace System
} // namespace Effect

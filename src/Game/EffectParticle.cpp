/* EffectParticle methods; original filename unknown. */
#include "Game/EffectParticle.h"
#include "Game/EffectCommandExec.h"
#include "Game/UnkEffectFileData.h"
#include "Game/UnkEffectPool.h"
#include "Game/fn_804220CC.h"
#include <cmath>

namespace Effect {
namespace System {

EffCmdGravityExec::~EffCmdGravityExec() {}

EffCmdFrictionExec::~EffCmdFrictionExec() {}

EffCmdGenQuantityExec::~EffCmdGenQuantityExec() {}

EffCmd2DRotExec::~EffCmd2DRotExec() {}

EffectParticleExecUnit::EffectParticleExecUnit(const UnkEffectElementData2 *definition,
                                               UnkEffectPool *commandPool)
    : m_rotation(0.0f, 0.0f, 0.0f),
      m_scale(definition->particle.scale, definition->particle.scale, definition->particle.scale),
      m_uvLower(0.0f, 0.0f), field_0x3C(0.0f), m_uvUpper(1.0f, 1.0f), field_0x48(1.0f), m_flags(0),
      m_elapsed(0.0f), m_prev(0), m_next(0), m_command(0), m_commandHead(0), m_commandTail(0),
      m_commandPool(commandPool) {
    m_color[0] = (definition->field_0x54[1] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_color[1] = (definition->field_0x54[2] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_color[2] = (definition->field_0x54[3] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_color[3] = (definition->field_0x54[0] & 0xFF) << MATERIAL_FRACTION_BITS;
}

EffectParticleExecUnit::EffectParticleExecUnit(const UnkEffectElementData5 *definition,
                                               UnkEffectPool *commandPool)
    : m_rotation(0.0f, 0.0f, 0.0f),
      m_scale(definition->particle.scale, definition->particle.scale, definition->particle.scale),
      m_uvLower(0.0f, 0.0f), field_0x3C(0.0f), m_uvUpper(1.0f, 1.0f), field_0x48(1.0f), m_flags(0),
      m_elapsed(0.0f), m_prev(0), m_next(0), m_command(0), m_commandHead(0), m_commandTail(0),
      m_commandPool(commandPool) {
    m_color[0] = (definition->field_0x54[1] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_color[1] = (definition->field_0x54[2] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_color[2] = (definition->field_0x54[3] & 0xFF) << MATERIAL_FRACTION_BITS;
    m_color[3] = (definition->field_0x54[0] & 0xFF) << MATERIAL_FRACTION_BITS;
}

EffectParticleExecUnit::~EffectParticleExecUnit() {
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

void EffectParticleExecUnit::fn_804281D4() {
    while (m_command != 0) {
        if (m_commandTail != 0 && dynamic_cast<EffCmdWaitExec *>(m_commandTail) != 0) {
            return;
        }
        EffectCommandExec *command = 0;
        unsigned int stride = 0;
        switch (m_command->type) {
        case COMMAND_PTN_ANIM:
            command = new EffCmdPtnAnimExec(m_command, &m_flags, &m_uvLower, &m_uvUpper);
            stride = sizeof(UnkPtnAnimData);
            break;
        case COMMAND_3D_POSITION:
            command = new EffCmd3DPosExec(m_command, &m_flags, &m_position);
            stride = sizeof(UnkVector3Data);
            break;
        case COMMAND_3D_ROTATION:
            command = new EffCmd3DRotExec(m_command, &m_flags, &m_rotation);
            stride = sizeof(UnkVector3Data);
            break;
        case COMMAND_2D_ROTATION:
            command = new EffCmd2DRotExec(m_command, &m_flags, &m_rotation.z);
            stride = sizeof(UnkSingleData);
            break;
        case COMMAND_3D_SCALE:
            command = new EffCmd3DSclExec(m_command, &m_flags, &m_scale);
            stride = sizeof(UnkVector3Data);
            break;
        case COMMAND_2D_SCALE:
            command = new EffCmd2DSclExec(m_command, &m_flags, &m_scale);
            stride = sizeof(UnkVector2Data);
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
            command = new EffCmdMatAlphaExec(m_command, &m_flags, &m_color[3]);
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

EffectParticle::EffectParticle(const UnkEffectElementData *definition, EffectLaunchInfo *info,
                               EffectManager *manager)
    : EffectElement(definition, info, manager), m_head(0), m_tail(0), m_commandFlags(0),
      m_definitionFlags(0), m_count(0), field_0x114(0.0f), field_0x118(0.0f), m_command(0),
      m_commandHead(0), m_commandTail(0), m_commandPool(0), m_particlePool(0) {
    m_size -= sizeof(EffectElement);
    m_size += sizeof(EffectParticle);
    if ((m_flags & ELEMENT_ACTIVE) == 0) {
        return;
    }
    /* Only the two original particle definition variants provide these embedded records. */
    switch (m_definition->field_0x20) {
    case 2: {
        const UnkEffectElementData2 *data = static_cast<const UnkEffectElementData2 *>(definition);
        m_definitionFlags |= data->field_0x4C;
        field_0x124 = &data->particle;
        field_0x120 = &data->emitter;
        break;
    }
    case 5: {
        const UnkEffectElementData5 *data = static_cast<const UnkEffectElementData5 *>(definition);
        m_definitionFlags |= data->field_0x4C;
        field_0x124 = &data->particle;
        field_0x120 = &data->emitter;
        break;
    }
    }
    m_maxCount = field_0x120->capacity;
    field_0x11C = field_0x120->generationRate;
    m_definitionFlags |= field_0x120->flags;
    m_definitionFlags |= field_0x124->flags;
    if (field_0x120->shape != 0) {
        m_definitionFlags |= field_0x120->shape->flags;
    }
    if ((m_definitionFlags & 0x200) != 0) {
        field_0x114 = field_0x124->gravity;
    }
    if ((m_definitionFlags & 0x400) != 0) {
        field_0x118 = field_0x124->friction;
    }
    m_particlePool = new UnkEffectPool(sizeof(EffectParticleExecUnit), m_maxCount);
    if (m_particlePool == 0 ||
        (m_maxCount != 0 && m_particlePool->m_blockStride *
                                    (m_particlePool->m_usedCount + m_particlePool->m_freeCount) ==
                                0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    m_size += m_particlePool->m_blockStride *
                  (m_particlePool->m_usedCount + m_particlePool->m_freeCount) +
              sizeof(UnkEffectPool);
    int commandCount = field_0x120->commandCount;
    if (commandCount == 0) {
        commandCount = m_maxCount * fn_804220CC(field_0x124->command);
        commandCount += fn_804220CC(field_0x120->command);
    }
    /* The largest allocated command executor used by this effect occupies 0x30 bytes. */
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
    m_command = field_0x120->command;
}

EffectParticle::~EffectParticle() {
    fn_8042A5C0(fn_8042A5D0(), m_particlePool);
    EffectParticleExecUnit *particle = m_head;
    while (particle != 0) {
        EffectParticleExecUnit *next = particle->m_next;
        delete particle;
        particle = next;
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
    if (m_particlePool != 0) {
        delete m_particlePool;
    }
    if (m_commandPool != 0) {
        delete m_commandPool;
    }
}

void EffectParticle::fn_80427970() {
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
        case COMMAND_GRAVITY:
            command = new EffCmdGravityExec(m_command, &m_commandFlags, &field_0x114);
            stride = sizeof(UnkSingleData);
            break;
        case COMMAND_FRICTION:
            command = new EffCmdFrictionExec(m_command, &m_commandFlags, &field_0x118);
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
            command = new EffCmdGenQuantityExec(m_command, &m_commandFlags, &field_0x11C);
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

void EffectParticle::fn_804272E8(float deltaTime) {
    int quantity = 0;
    float generated = field_0x11C * deltaTime;
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
    float translationSpeed = 0.0f;
    if ((m_definitionFlags & 0x1000) != 0) {
        translationSpeed = fn_8040CF50(&m_translationVelocity);
    }
    for (int i = 0; i < quantity; ++i) {
        if (m_maxCount == m_count) {
            break;
        }
        fn_8042A5C0(fn_8042A5D0(), m_particlePool);
        /* The original constructors supply only types 2 and 5 to this class. */
        EffectParticleExecUnit *particle;
        switch (m_definition->field_0x20) {
        case 2:
            particle = new EffectParticleExecUnit(
                static_cast<const UnkEffectElementData2 *>(m_definition), m_commandPool);
            break;
        case 5:
            particle = new EffectParticleExecUnit(
                static_cast<const UnkEffectElementData5 *>(m_definition), m_commandPool);
            break;
        }
        if (particle == 0) {
            break;
        }
        ++m_count;
        if (m_tail != 0) {
            m_tail->m_next = particle;
            particle->m_prev = m_tail;
            m_tail = particle;
        } else {
            m_head = m_tail = particle;
        }
        const UnkEffectParticleData124 *parameters = field_0x124;
        float speed = parameters->speedRange * fn_8042A5D0()->Random() + parameters->speed;
        const UnkEffectParticleShape *shape = field_0x120->shape;
        /* As in the original spawning path, the shape pointer is required here. */
        switch (shape->type) {
        default: {
            particle->m_position = m_previousTranslation;
            /* The original default-shape path writes only y before this vector assignment;
             * Fn8040Vector3's empty default constructor leaves x and z uninitialized. */
            Fn8040Vector3 velocity;
            velocity.y = speed;
            particle->m_velocity = velocity;
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
            particle->m_position.Set(sine * radius, height, cosine * radius);
            float spreadSine = std::sin(spread);
            float spreadCosine = std::cos(spread);
            particle->m_velocity.Set(sine * (spreadSine * speed), spreadCosine * speed,
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
            particle->m_position.Set(x * radius, spreadCosine * radius, z * radius);
            particle->m_velocity.Set(x * speed, spreadCosine * speed, z * speed);
            if ((m_definitionFlags & 0x20) != 0) {
                particle->m_velocity.x = -particle->m_velocity.x;
                particle->m_velocity.y = -particle->m_velocity.y;
                particle->m_velocity.z = -particle->m_velocity.z;
            }
            break;
        }
        case 2: {
            const UnkEffectParticleShape2 *data =
                static_cast<const UnkEffectParticleShape2 *>(shape);
            float width = data->width;
            particle->m_position.x = width * fn_8042A5D0()->Random() - width * 0.5f;
            float height = data->height;
            particle->m_position.y = height * fn_8042A5D0()->Random() - height * 0.5f;
            float depth = data->depth;
            particle->m_position.z = depth * fn_8042A5D0()->Random() - depth * 0.5f;
            particle->m_velocity.Set(0.0f, speed, 0.0f);
            break;
        }
        }
        if ((m_definitionFlags & 0x800) == 0) {
            particle->m_position = fn_8040B67C(m_transform, particle->m_position);
            particle->m_velocity = fn_8040B618(m_transform, particle->m_velocity);
            if (translationSpeed > 0.0001f) {
                float projection = fn_8040CF90(&particle->m_velocity, &m_translationVelocity);
                float velocityLength = fn_8040CF50(&particle->m_velocity);
                float alignment = projection / (translationSpeed * velocityLength);
                AddVector(&particle->m_velocity,
                          m_translationVelocity * (alignment > 0.0 ? alignment : -alignment));
            }
        }
        particle->m_command = field_0x124->command;
    }
}

void EffectParticle::fn_80426FF4(float deltaTime) {
    if ((m_definitionFlags & 0x100) == 0) {
        float lifetime = field_0x124->lifetime;
        for (EffectParticleExecUnit *particle = m_head; particle != 0;
             particle = particle->m_next) {
            if ((m_flags & ELEMENT_FLAG_2) != 0) {
                particle->m_elapsed += deltaTime;
            }
            if (particle->m_elapsed > 0.0001f + lifetime) {
                particle->m_flags |= COMMAND_STOP_REQUESTED;
            }
        }
    }
    EffectParticleExecUnit *commandParticle = m_head;
    fn_8042A5C0(fn_8042A5D0(), m_commandPool);
    for (; commandParticle != 0; commandParticle = commandParticle->m_next) {
        if ((commandParticle->m_flags & COMMAND_STOP_REQUESTED) != 0) {
            continue;
        }
        commandParticle->fn_804281D4();
        if ((commandParticle->m_flags & COMMAND_STOP_REQUESTED) != 0) {
            continue;
        }
        EffectCommandExec *command = commandParticle->m_commandHead;
        while (command != 0) {
            if (!command->Update(deltaTime)) {
                if (command->m_prev != 0) {
                    command->m_prev->m_next = command->m_next;
                }
                if (command->m_next != 0) {
                    command->m_next->m_prev = command->m_prev;
                }
                if (command == commandParticle->m_commandHead) {
                    commandParticle->m_commandHead = command->m_next;
                }
                if (command == commandParticle->m_commandTail) {
                    commandParticle->m_commandTail = command->m_prev;
                }
                EffectCommandExec *next = command->m_next;
                delete command;
                command = next;
            } else {
                command = command->m_next;
            }
        }
    }
    EffectParticleExecUnit *next;
    EffectParticleExecUnit *particle = m_head;
    fn_8042A5C0(fn_8042A5D0(), m_particlePool);
    while (particle != 0) {
        next = particle->m_next;
        if ((particle->m_flags & COMMAND_STOP_REQUESTED) != 0) {
            if (particle->m_prev != 0) {
                particle->m_prev->m_next = particle->m_next;
            }
            if (particle->m_next != 0) {
                particle->m_next->m_prev = particle->m_prev;
            }
            if (particle == m_head) {
                m_head = particle->m_next;
            }
            if (particle == m_tail) {
                m_tail = particle->m_prev;
            }
            delete particle;
            --m_count;
        }
        particle = next;
    }
    for (particle = m_head; particle != 0; particle = particle->m_next) {
        AddVector(&particle->m_position, particle->m_velocity * deltaTime);
        if (field_0x118 != 0.0f) {
            float speed = fn_8040CF50(&particle->m_velocity);
            if (speed > 0.0001f) {
                float loss = speed * field_0x118;
                if (loss > speed) {
                    fn_8040CE38(&particle->m_velocity, 0.0f);
                } else {
                    fn_8040CE38(&particle->m_velocity, (speed - loss) / speed);
                }
            }
        }
        if (field_0x114 != 0.0f) {
            particle->m_velocity.y -= deltaTime * field_0x114;
        }
    }
}

void EffectParticle::Update(float deltaTime) {
    if ((m_flags & ELEMENT_ACTIVE) != 0 && (m_flags & ELEMENT_FLAG_4) == 0) {
        if ((m_flags & ELEMENT_FLAG_2) != 0) {
            m_elapsed += deltaTime;
        }
        if ((m_definitionFlags & 1) == 0 && m_elapsed > 0.0001f + field_0x120->duration) {
            m_flags &= ~ELEMENT_ACTIVE;
            return;
        }
        fn_8042A5C0(fn_8042A5D0(), m_commandPool);
        fn_80427970();
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
                fn_804272E8(deltaTime);
                fn_80426FF4(deltaTime);
            } else {
                fn_80426FF4(deltaTime);
                if (m_count == 0) {
                    m_flags &= ~ELEMENT_ACTIVE;
                }
            }
        } else {
            fn_804272E8(deltaTime);
            fn_80426FF4(deltaTime);
        }
    }
}

} // namespace System
} // namespace Effect

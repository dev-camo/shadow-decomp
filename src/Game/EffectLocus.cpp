/* EffectLocus methods; original filename unknown. */
#include "Game/EffectLocus.h"
#include "Game/EffectLocusExecUnit.h"
#include "Game/EffectCommandExec.h"
#include "Game/UnkEffectFileData.h"
#include "Game/UnkEffectPool.h"
#include "Game/fn_8007A34C.h"
#include <cmath>

namespace Effect {
namespace System {

/* Class identities and inheritance are recorded in the original RTTI. */
class EffCmdTopColorExec : public EffCmdMatColorExec {
  public:
    EffCmdTopColorExec(const UnkCommandData *command, unsigned int *flags, int *target)
        : EffCmdMatColorExec(command, flags, target) {}
    virtual ~EffCmdTopColorExec();
};
CHECK_SIZEOF(EffCmdTopColorExec, 0x30);

class EffCmdTopAlphaExec : public EffCmdMatAlphaExec {
  public:
    EffCmdTopAlphaExec(const UnkCommandData *command, unsigned int *flags, int *target)
        : EffCmdMatAlphaExec(command, flags, target) {}
    virtual ~EffCmdTopAlphaExec();
};
CHECK_SIZEOF(EffCmdTopAlphaExec, 0x28);

class EffCmdLastColorExec : public EffCmdMatColorExec {
  public:
    EffCmdLastColorExec(const UnkCommandData *command, unsigned int *flags, int *target)
        : EffCmdMatColorExec(command, flags, target) {}
    virtual ~EffCmdLastColorExec();
};
CHECK_SIZEOF(EffCmdLastColorExec, 0x30);

class EffCmdLastAlphaExec : public EffCmdMatAlphaExec {
  public:
    EffCmdLastAlphaExec(const UnkCommandData *command, unsigned int *flags, int *target)
        : EffCmdMatAlphaExec(command, flags, target) {}
    virtual ~EffCmdLastAlphaExec();
};
CHECK_SIZEOF(EffCmdLastAlphaExec, 0x28);

EffectLocusExecUnit::EffectLocusExecUnit() {
    fn_8040C148(&m_transform, &lbl_805E0720);
    m_age = 0.0f;
    m_prev = 0;
    m_next = 0;
}

EffectLocusExecUnit::~EffectLocusExecUnit() {}

EffectLocus::EffectLocus(UnkEffectElementData7 *definition, EffectLaunchInfo *info,
                         EffectManager *manager)
    : EffectElement(definition, info, manager), m_head(0), m_tail(0), m_previousUnit(0),
      m_commandFlags(0), m_definitionFlags(0), m_unitCount(0), m_accumulatedTime(0.0f),
      m_command(0), m_commandHead(0), m_commandTail(0), m_commandPool(0), m_nodePool(0) {
    m_size -= sizeof(EffectElement);
    m_size += sizeof(EffectLocus);
    if ((m_flags & ELEMENT_ACTIVE) == 0) {
        return;
    }
    if (definition->field_0x34 < 0) {
        m_capacity = definition->field_0x48 / (1 - definition->field_0x34) + 1;
        m_interval = static_cast<float>(1 - definition->field_0x34);
    } else {
        m_capacity = definition->field_0x48 * (definition->field_0x34 + 1) + 1;
        m_interval = 1.0f / (definition->field_0x34 + 1);
    }
    m_definitionFlags |= definition->field_0x44;
    m_definitionFlags |= definition->field_0x2C;
    m_definitionFlags |= definition->field_0x38;
    m_nodePool = new UnkEffectPool(sizeof(EffectLocusExecUnit), m_capacity);
    if (m_nodePool == 0 ||
        (m_capacity != 0 &&
         m_nodePool->m_blockStride * (m_nodePool->m_usedCount + m_nodePool->m_freeCount) == 0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    m_size += m_nodePool->m_blockStride * (m_nodePool->m_usedCount + m_nodePool->m_freeCount) +
              sizeof(UnkEffectPool);
    int commandCount = definition->field_0x4A;
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
    m_command = definition->field_0x50;
    m_pointCount = 0;
    const UnkCommandData *point = definition->field_0x54;
    while (point != 0) {
        unsigned int stride;
        if (point->type == 0x15) {
            ++m_pointCount;
            stride = 0x20;
        } else {
            stride = 0;
        }
        if (point->hasNext != 0 && stride != 0) {
            point = reinterpret_cast<const UnkCommandData *>(
                reinterpret_cast<const unsigned char *>(point) + stride);
        } else {
            point = 0;
        }
    }
    if ((m_definitionFlags & 0x2000) != 0) {
        ++m_pointCount;
    }
    if (m_pointCount < 2) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    m_color[0] = definition->field_0x40[1] << MATERIAL_FRACTION_BITS;
    m_color[1] = definition->field_0x40[2] << MATERIAL_FRACTION_BITS;
    m_color[2] = definition->field_0x40[3] << MATERIAL_FRACTION_BITS;
    m_alpha = definition->field_0x40[0] << MATERIAL_FRACTION_BITS;
    m_topColor[0] = m_topColor[1] = m_topColor[2] = m_topAlpha = MATERIAL_CHANNEL_MAX;
    m_lastColor[0] = m_lastColor[1] = m_lastColor[2] = m_lastAlpha = MATERIAL_CHANNEL_MAX;
}

EffectLocus::~EffectLocus() {
    fn_8042A5C0(fn_8042A5D0(), m_nodePool);
    EffectLocusExecUnit *unit = m_head;
    while (unit != 0) {
        EffectLocusExecUnit *next = unit->m_next;
        delete unit;
        unit = next;
    }
    m_head = 0;
    m_tail = 0;
    if (m_previousUnit != 0) {
        delete m_previousUnit;
        m_previousUnit = 0;
    }
    EffectCommandExec *command = m_commandHead;
    fn_8042A5C0(fn_8042A5D0(), m_commandPool);
    while (command != 0) {
        EffectCommandExec *next = command->m_next;
        delete command;
        command = next;
    }
    if (m_nodePool != 0) {
        delete m_nodePool;
    }
    if (m_commandPool != 0) {
        delete m_commandPool;
    }
}

void EffectLocus::fn_8042BD8C() {
    while (m_command != 0) {
        if (m_commandTail != 0 && dynamic_cast<EffCmdWaitExec *>(m_commandTail) != 0) {
            return;
        }
        EffectCommandExec *command = 0;
        unsigned int stride;
        switch (m_command->type) {
        case 2:
            command = new EffCmdGlareSWExec(m_command, &m_commandFlags);
            stride = 8;
            break;
        case 3:
            command = new EffCmd3DPosExec(m_command, &m_commandFlags, &m_translation);
            stride = 0x24;
            break;
        case 4:
            command = new EffCmd3DRotExec(m_command, &m_commandFlags, &m_rotation);
            stride = 0x24;
            break;
        case 6:
            command = new EffCmd3DSclExec(m_command, &m_commandFlags, &m_scale);
            stride = 0x24;
            break;
        case 8:
            command = new EffCmd3DVelExec(m_command, &m_commandFlags, &m_velocity);
            stride = 0x24;
            break;
        case 0xB:
            command = new EffCmdMatColorExec(m_command, &m_commandFlags, m_color);
            stride = 0x14;
            break;
        case 0xC:
            command = new EffCmdMatAlphaExec(m_command, &m_commandFlags, &m_alpha);
            stride = 0x10;
            break;
        case 0x16:
            command = new EffCmdTopColorExec(m_command, &m_commandFlags, m_topColor);
            stride = 0x14;
            break;
        case 0x17:
            command = new EffCmdTopAlphaExec(m_command, &m_commandFlags, &m_topAlpha);
            stride = 0x10;
            break;
        case 0x18:
            command = new EffCmdLastColorExec(m_command, &m_commandFlags, m_lastColor);
            stride = 0x14;
            break;
        case 0x19:
            command = new EffCmdLastAlphaExec(m_command, &m_commandFlags, &m_lastAlpha);
            stride = 0x10;
            break;
        case 0xD:
            command = new EffCmdWaitExec(m_command, &m_commandFlags, m_commandTail);
            stride = 8;
            break;
        case 0xE:
            command = new EffCmdLoopStartExec(m_command, &m_commandFlags);
            stride = 8;
            ++field_0x4;
            if (field_0x5 != 0 || (m_flags & ELEMENT_STOP_REQUESTED) != 0) {
                ++field_0x5;
            }
            break;
        case 0xF: {
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
                stride = 8;
            } else {
                stride = 4;
                --field_0x4;
            }
            break;
        }
        case 0x10:
            command = new EffCmdCallbackExec(m_command, &m_commandFlags);
            stride = 8;
            break;
        case 0x13:
            command = new EffCmdRequestStopExec(m_command, &m_commandFlags, this);
            stride = 4;
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

void EffectLocus::fn_8042B938(float deltaTime) {
    Fn8040Matrix34 current;
    Fn8040Vector4 previousRotation;
    Fn8040Vector4 currentRotation;
    Fn8040Vector3 scale;
    if ((m_definitionFlags & 0x800) != 0) {
        fn_8040C148(&current, &lbl_805E0720);
    } else {
        fn_8040C148(&current, &m_transform);
    }
    Fn8040Vector4 currentTemporary;
    fn_8040C55C(&currentTemporary, &current);
    fn_8007A34C(&currentRotation, &currentTemporary);
    float currentScaleX = std::sqrt(current.values[2][0] * current.values[2][0] +
                                    (current.values[0][0] * current.values[0][0] +
                                     current.values[1][0] * current.values[1][0]));
    float currentScaleY = std::sqrt(current.values[2][1] * current.values[2][1] +
                                    (current.values[0][1] * current.values[0][1] +
                                     current.values[1][1] * current.values[1][1]));
    float currentScaleZ = std::sqrt(current.values[2][2] * current.values[2][2] +
                                    (current.values[0][2] * current.values[0][2] +
                                     current.values[1][2] * current.values[1][2]));
    fn_8042A5C0(fn_8042A5D0(), m_nodePool);
    if (m_previousUnit == 0) {
        // The original assumes both initial allocations succeed.
        m_previousUnit = new EffectLocusExecUnit;
        fn_8040C148(&m_previousUnit->m_transform, &current);
        EffectLocusExecUnit *unit = new EffectLocusExecUnit;
        m_tail = unit;
        m_head = unit;
        fn_8040C148(&unit->m_transform, &current);
        unit->m_age = m_accumulatedTime;
        m_accumulatedTime = 0.0f;
        ++m_unitCount;
        ++m_addedCount;
    } else {
        m_accumulatedTime += deltaTime;
        // The original does not reject zero or negative intervals.
        while (m_accumulatedTime > m_interval) {
            EffectLocusExecUnit *unit = new EffectLocusExecUnit;
            if (unit == 0) {
                EffectLocusExecUnit *old = m_head;
                m_head = old->m_next;
                if (m_head != 0) {
                    m_head->m_prev = 0;
                }
                delete old;
                --m_unitCount;
                ++m_removedCount;
                unit = new EffectLocusExecUnit;
                if (unit == 0) {
                    break;
                }
            }
            Fn8040Matrix34 *previous;
            if (m_tail != 0) {
                previous = &m_tail->m_transform;
            } else {
                previous = &m_previousUnit->m_transform;
            }
            ++m_unitCount;
            ++m_addedCount;
            if (m_tail != 0) {
                m_tail->m_next = unit;
                unit->m_prev = m_tail;
                m_tail = unit;
            } else {
                m_tail = unit;
                m_head = unit;
            }
            float t = m_interval / m_accumulatedTime;
            Fn8040Vector4 previousTemporary;
            fn_8040C55C(&previousTemporary, previous);
            fn_8007A34C(&previousRotation, &previousTemporary);
            scale.x = std::sqrt(previous->values[2][0] * previous->values[2][0] +
                                (previous->values[0][0] * previous->values[0][0] +
                                 previous->values[1][0] * previous->values[1][0]));
            scale.y = std::sqrt(previous->values[2][1] * previous->values[2][1] +
                                (previous->values[0][1] * previous->values[0][1] +
                                 previous->values[1][1] * previous->values[1][1]));
            scale.z = std::sqrt(previous->values[2][2] * previous->values[2][2] +
                                (previous->values[0][2] * previous->values[0][2] +
                                 previous->values[1][2] * previous->values[1][2]));
            Fn8040Vector4 rotation;
            fn_8040C3EC(&rotation, &previousRotation, &currentRotation, t);
            Fn8040Matrix34 transform;
            fn_8040B228(&transform, &rotation);
            fn_8040C148(&unit->m_transform, &transform);
            float previousWeight = 1.0f - t;
            unit->m_transform.values[0][3] =
                previousWeight * previous->values[0][3] + t * current.values[0][3];
            unit->m_transform.values[1][3] =
                previousWeight * previous->values[1][3] + t * current.values[1][3];
            unit->m_transform.values[2][3] =
                previousWeight * previous->values[2][3] + t * current.values[2][3];
            scale.x = previousWeight * scale.x + t * currentScaleX;
            scale.y = previousWeight * scale.y + t * currentScaleY;
            scale.z = previousWeight * scale.z + t * currentScaleZ;
            fn_8040B790(&unit->m_transform, &scale);
            m_accumulatedTime -= m_interval;
            unit->m_age = m_accumulatedTime;
        }
        fn_8040C148(&m_previousUnit->m_transform, &current);
    }
}

inline void EffectLocus::ExpireUnits(float deltaTime) {
    const UnkEffectElementData7 *definition =
        static_cast<const UnkEffectElementData7 *>(m_definition);
    if ((m_definitionFlags & 0x100) != 0) {
        return;
    }
    float lifetime = 0.0001f + definition->field_0x30;
    EffectLocusExecUnit *unit = m_head;
    while (unit != 0) {
        EffectLocusExecUnit *next = unit->m_next;
        if ((m_flags & ELEMENT_FLAG_2) != 0) {
            unit->m_age += deltaTime;
        }
        if (unit->m_age > lifetime) {
            if (unit->m_prev != 0) {
                unit->m_prev->m_next = unit->m_next;
            }
            if (unit->m_next != 0) {
                unit->m_next->m_prev = unit->m_prev;
            }
            if (unit == m_head) {
                m_head = unit->m_next;
            }
            if (unit == m_tail) {
                m_tail = unit->m_prev;
            }
            delete unit;
            --m_unitCount;
            ++m_removedCount;
        }
        unit = next;
    }
}

void EffectLocus::Update(float deltaTime) {
    if ((m_flags & ELEMENT_ACTIVE) == 0) {
        return;
    }
    if ((m_flags & ELEMENT_FLAG_4) != 0) {
        return;
    }
    m_addedCount = 0;
    m_removedCount = 0;
    if ((m_flags & ELEMENT_FLAG_2) != 0) {
        m_elapsed += deltaTime;
    }
    const UnkEffectElementData7 *definition =
        static_cast<const UnkEffectElementData7 *>(m_definition);
    if ((m_definitionFlags & 1) == 0 && m_elapsed > definition->field_0x4C + 0.0001f) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    fn_8042A5C0(fn_8042A5D0(), m_commandPool);
    fn_8042BD8C();
    if ((m_commandFlags & COMMAND_STOP_REQUESTED) == 0) {
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
    if ((m_flags & ELEMENT_STOP_REQUESTED) != 0) {
        if ((m_definitionFlags & 0x40000) != 0 && m_commandHead != 0) {
            fn_8042B938(deltaTime);
            ExpireUnits(deltaTime);
        } else {
            if ((m_definitionFlags & 0x100) != 0 && m_previousUnit != 0 && m_head != 0) {
                m_accumulatedTime += deltaTime;
                fn_8042A5C0(fn_8042A5D0(), m_nodePool);
                while (m_accumulatedTime > m_interval && m_head != 0) {
                    EffectLocusExecUnit *unit = m_head;
                    m_head = unit->m_next;
                    if (m_head != 0) {
                        m_head->m_prev = 0;
                    }
                    delete unit;
                    --m_unitCount;
                    ++m_removedCount;
                    m_accumulatedTime -= m_interval;
                }
                if (m_head == 0) {
                    m_tail = 0;
                }
            }
            // The original expiration path uses the currently selected pool.
            ExpireUnits(deltaTime);
            if (m_unitCount == 0) {
                m_flags &= ~ELEMENT_ACTIVE;
            }
        }
    } else {
        fn_8042B938(deltaTime);
        ExpireUnits(deltaTime);
    }
}

} // namespace System
} // namespace Effect

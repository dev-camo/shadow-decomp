/* EffectShimmer methods; original filename unknown. */
#include "Game/EffectShimmer.h"
#include "Game/EffectCommandExec.h"
#include "Game/UnkEffectFileData.h"
#include "Game/UnkEffectPool.h"

namespace Effect {
namespace System {

/* Accessed payload prefixes; original type names and complete extents are unknown. */
struct UnkEffectShimmerData {
    /* 0x00 */ int type;
};
CHECK_SIZEOF(UnkEffectShimmerData, 0x4);

struct UnkEffectShimmerData0 : UnkEffectShimmerData {
    /* 0x04 */ int field_0x4[2];
    /* 0x0C */ float field_0xC[2];
    /* 0x14 */ Fn8040Vector2 amplitude;
    /* 0x1C */ Fn8040Vector2 cycle;
    /* 0x24 */ Fn8040Vector2 waveLength;
};
CHECK_SIZEOF(UnkEffectShimmerData0, 0x2C);

struct UnkEffectShimmerData1 : UnkEffectShimmerData {
    /* 0x04 */ int field_0x4[2];
    /* 0x0C */ unsigned char field_0xC;
    /* 0x10 */ float field_0x10[2];
    /* 0x18 */ float amplitude;
    /* 0x1C */ float cycle;
    /* 0x20 */ float waveLength;
};
CHECK_SIZEOF(UnkEffectShimmerData1, 0x24);

/* Class identities and inheritance are recorded in the original RTTI. */
class EffCmd2DAmplitudeExec : public EffCmdVector2Exec {
  public:
    EffCmd2DAmplitudeExec(const UnkCommandData *command, unsigned int *flags, Fn8040Vector3 *target)
        : EffCmdVector2Exec(command, flags, target) {}
    virtual ~EffCmd2DAmplitudeExec();
};
CHECK_SIZEOF(EffCmd2DAmplitudeExec, 0x30);

class EffCmd2DCycleExec : public EffCmdVector2Exec {
  public:
    EffCmd2DCycleExec(const UnkCommandData *command, unsigned int *flags, Fn8040Vector3 *target)
        : EffCmdVector2Exec(command, flags, target) {}
    virtual ~EffCmd2DCycleExec();
};
CHECK_SIZEOF(EffCmd2DCycleExec, 0x30);

class EffCmd2DWaveLengthExec : public EffCmdVector2Exec {
  public:
    EffCmd2DWaveLengthExec(const UnkCommandData *command, unsigned int *flags,
                           Fn8040Vector3 *target)
        : EffCmdVector2Exec(command, flags, target) {}
    virtual ~EffCmd2DWaveLengthExec();
};
CHECK_SIZEOF(EffCmd2DWaveLengthExec, 0x30);

class EffCmdAmplitudeExec : public EffCmdSingleExec {
  public:
    EffCmdAmplitudeExec(const UnkCommandData *command, unsigned int *flags, float *target)
        : EffCmdSingleExec(command, flags, target) {}
    virtual ~EffCmdAmplitudeExec();
};
CHECK_SIZEOF(EffCmdAmplitudeExec, 0x28);

class EffCmdCycleExec : public EffCmdSingleExec {
  public:
    EffCmdCycleExec(const UnkCommandData *command, unsigned int *flags, float *target)
        : EffCmdSingleExec(command, flags, target) {}
    virtual ~EffCmdCycleExec();
};
CHECK_SIZEOF(EffCmdCycleExec, 0x28);

class EffCmdWaveLengthExec : public EffCmdSingleExec {
  public:
    EffCmdWaveLengthExec(const UnkCommandData *command, unsigned int *flags, float *target)
        : EffCmdSingleExec(command, flags, target) {}
    virtual ~EffCmdWaveLengthExec();
};
CHECK_SIZEOF(EffCmdWaveLengthExec, 0x28);

EffCmd2DAmplitudeExec::~EffCmd2DAmplitudeExec() {}

EffCmd2DCycleExec::~EffCmd2DCycleExec() {}

EffCmd2DWaveLengthExec::~EffCmd2DWaveLengthExec() {}

EffCmdAmplitudeExec::~EffCmdAmplitudeExec() {}

EffCmdCycleExec::~EffCmdCycleExec() {}

EffCmdWaveLengthExec::~EffCmdWaveLengthExec() {}

EffectShimmer::EffectShimmer(UnkEffectElementData10 *definition, EffectLaunchInfo *info,
                             EffectManager *manager)
    : EffectElement(definition, info, manager), m_commandFlags(0), m_definitionFlags(0),
      m_command(0), m_commandHead(0), m_commandTail(0), m_commandPool(0) {
    m_size -= sizeof(EffectElement);
    m_size += sizeof(EffectShimmer);
    if ((m_flags & ELEMENT_ACTIVE) == 0) {
        return;
    }
    m_parameters = static_cast<UnkEffectShimmerData *>(definition->field_0x38);
    m_definitionFlags = definition->field_0x2C;
    m_duration = definition->field_0x34;
    m_commandPool = new UnkEffectPool(0x30, definition->field_0x32);
    if (m_commandPool == 0 ||
        (definition->field_0x32 != 0 &&
         m_commandPool->m_blockStride * (m_commandPool->m_usedCount + m_commandPool->m_freeCount) ==
             0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    m_size +=
        m_commandPool->m_blockStride * (m_commandPool->m_usedCount + m_commandPool->m_freeCount) +
        sizeof(UnkEffectPool);
    m_command = static_cast<const UnkCommandData *>(definition->field_0x3C);
    UnkEffectShimmerData *parameters = static_cast<UnkEffectShimmerData *>(definition->field_0x38);
    switch (parameters->type) {
    case 0: {
        UnkEffectShimmerData0 *data = static_cast<UnkEffectShimmerData0 *>(parameters);
        field_0x108[0] = data->field_0x4[0];
        field_0x108[1] = data->field_0x4[1];
        field_0x110[0] = data->field_0xC[0];
        field_0x110[1] = data->field_0xC[1];
        m_amplitude.x = data->amplitude.x;
        m_amplitude.y = data->amplitude.y;
        m_cycle.x = data->cycle.x;
        m_cycle.y = data->cycle.y;
        m_waveLength.x = data->waveLength.x;
        m_waveLength.y = data->waveLength.y;
        break;
    }
    case 1: {
        UnkEffectShimmerData1 *data = static_cast<UnkEffectShimmerData1 *>(parameters);
        field_0x108[0] = data->field_0x4[0];
        field_0x108[1] = data->field_0x4[1];
        field_0x110[0] = data->field_0x10[0];
        field_0x110[1] = data->field_0x10[1];
        m_amplitude.x = data->amplitude;
        m_cycle.x = data->cycle;
        m_waveLength.x = data->waveLength;
        field_0x13C = data->field_0xC;
        break;
    }
    }
    // Only the supplied vector components are initialized.
    m_alpha = definition->field_0x30 << MATERIAL_FRACTION_BITS;
    for (int i = 0; i < 2; ++i) {
        if (field_0x108[i] < 1) {
            field_0x108[i] = 1;
        }
    }
}

EffectShimmer::~EffectShimmer() {
    EffectCommandExec *command = m_commandHead;
    fn_8042A5C0(fn_8042A5D0(), m_commandPool);
    while (command != 0) {
        EffectCommandExec *next = command->m_next;
        delete command;
        command = next;
    }
    if (m_commandPool != 0) {
        delete m_commandPool;
    }
}

void EffectShimmer::fn_8042AAB0() {
    while (m_command != 0) {
        if (m_commandTail != 0 && dynamic_cast<EffCmdWaitExec *>(m_commandTail) != 0) {
            return;
        }
        EffectCommandExec *command = 0;
        unsigned int stride;
        switch (m_command->type) {
        case 0xC:
            command = new EffCmdMatAlphaExec(m_command, &m_commandFlags, &m_alpha);
            stride = 0x10;
            break;
        case 0x1C:
            command = new EffCmd2DAmplitudeExec(m_command, &m_commandFlags, &m_amplitude);
            stride = 0x1C;
            break;
        case 0x1D:
            command = new EffCmd2DCycleExec(m_command, &m_commandFlags, &m_cycle);
            stride = 0x1C;
            break;
        case 0x1E:
            command = new EffCmd2DWaveLengthExec(m_command, &m_commandFlags, &m_waveLength);
            stride = 0x1C;
            break;
        case 0x1F:
            command = new EffCmdAmplitudeExec(m_command, &m_commandFlags, &m_amplitude.x);
            stride = 0x14;
            break;
        case 0x20:
            command = new EffCmdCycleExec(m_command, &m_commandFlags, &m_cycle.x);
            stride = 0x14;
            break;
        case 0x21:
            command = new EffCmdWaveLengthExec(m_command, &m_commandFlags, &m_waveLength.x);
            stride = 0x14;
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

void EffectShimmer::Update(float deltaTime) {
    if ((m_flags & ELEMENT_ACTIVE) == 0) {
        return;
    }
    if ((m_flags & ELEMENT_FLAG_4) != 0) {
        return;
    }
    if ((m_flags & ELEMENT_STOP_REQUESTED) != 0 &&
        ((m_definitionFlags & 0x40000) == 0 || m_commandHead == 0)) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    if ((m_flags & ELEMENT_FLAG_2) != 0) {
        m_elapsed += deltaTime;
    }
    if ((m_definitionFlags & 1) == 0 && m_elapsed > 0.0001f + m_duration) {
        m_flags &= ~ELEMENT_ACTIVE;
        return;
    }
    fn_8042A5C0(fn_8042A5D0(), m_commandPool);
    fn_8042AAB0();
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
}

} // namespace System
} // namespace Effect

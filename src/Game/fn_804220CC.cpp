/* Effect command implementations sharing RTTI and literals. Original filename unknown. */
#include "Game/EffectCommandExec.h"
#include "Game/EffectElement.h"
#include "Game/fn_80424D18.h"
#include "Game/EffectSystem.h"
#include "Game/EffectManager.h"
#include "Game/EffectArranger.h"
#include "Game/fn_804220CC.h"

namespace Effect {
namespace System {

EffCmdSingleExec::~EffCmdSingleExec() {}
EffCmdVector2Exec::~EffCmdVector2Exec() {}
EffCmdPtnAnimExec::~EffCmdPtnAnimExec() {}
EffCmdGlareSWExec::~EffCmdGlareSWExec() {}
EffCmd3DSclExec::~EffCmd3DSclExec() {}
EffCmd2DSclExec::~EffCmd2DSclExec() {}
EffCmdMatColorExec::~EffCmdMatColorExec() {}
EffCmdMatAlphaExec::~EffCmdMatAlphaExec() {}
EffCmdWaitExec::~EffCmdWaitExec() {}
EffCmdLoopStartExec::~EffCmdLoopStartExec() {}
EffCmdLoopEndExec::~EffCmdLoopEndExec() {}
EffCmdCallbackExec::~EffCmdCallbackExec() {}

} // namespace System
} // namespace Effect

/* Time-multiplier getter; the original method and source filename are unknown. */
float fn_80424D18(Effect::System::EffectElement *element) {
    return element->m_timeScale;
}

namespace Effect {
namespace System {

EffCmdSingleExec::EffCmdSingleExec(const UnkCommandData *command, unsigned int *flags,
                                   float *target)
    : EffectCommandExec(command), m_flags(flags), m_target(target), m_elapsed(0.0f) {
    const UnkSingleData *data = static_cast<const UnkSingleData *>(m_command);
    m_rate = data->range * fn_8042A5D0()->Random() + data->value;
    m_duration = data->duration;
    if (data->durationRange != 0) {
        m_duration += data->durationRange * fn_8042A5D0()->Random();
    }
    if (m_duration < 0.0001f) {
        if (data->relative != 0) {
            *m_target += m_rate;
        } else {
            *m_target = m_rate;
        }
        m_target = 0;
    } else {
        if (data->relative == 0) {
            m_rate -= *target;
        }
        m_rate /= m_duration;
        m_duration += 0.0001f;
    }
}

bool EffCmdSingleExec::Update(float deltaTime) {
    if (m_target == 0) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    if (field_0x10 == 0) {
        field_0x10 = 1;
        return true;
    }
    m_elapsed += deltaTime;
    if (m_elapsed > m_duration) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    *m_target += m_rate * deltaTime;
    return true;
}

EffCmdVector2Exec::EffCmdVector2Exec(const UnkCommandData *command, unsigned int *flags,
                                     Fn8040Vector3 *target)
    : EffectCommandExec(command), m_flags(flags), m_target(target), m_elapsed(0.0f) {
    const UnkVector2Data *data = static_cast<const UnkVector2Data *>(m_command);
    if (data->sharedRandom != 0) {
        float random = fn_8042A5D0()->Random();
        m_rate.x = data->range.x * random + data->value.x;
        m_rate.y = data->range.y * random + data->value.y;
    } else {
        m_rate.x = data->range.x * fn_8042A5D0()->Random() + data->value.x;
        m_rate.y = data->range.y * fn_8042A5D0()->Random() + data->value.y;
    }
    /* The original leaves m_rate.z unchanged, although its helpers use all three components. */
    m_duration = data->duration;
    if (data->durationRange != 0) {
        m_duration += data->durationRange * fn_8042A5D0()->Random();
    }
    if (m_duration < 0.0001f) {
        if (data->relative != 0) {
            fn_8040CFF8(m_target, &m_rate);
        } else {
            fn_800091D0(m_target, &m_rate);
        }
        m_target = 0;
    } else {
        if (data->relative == 0) {
            fn_8040CE04(&m_rate, m_target);
        }
        fn_8040CE38(&m_rate, 1.0f / m_duration);
        m_duration += 0.0001f;
    }
}

bool EffCmdVector2Exec::Update(float deltaTime) {
    if (m_target == 0) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    if (field_0x10 == 0) {
        field_0x10 = 1;
        return true;
    }
    m_elapsed += deltaTime;
    if (m_elapsed > m_duration) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    Fn8040Vector3 step;
    fn_800091A4(&step, &m_rate, deltaTime);
    fn_8040CFF8(m_target, &step);
    return true;
}

EffCmdVector3Exec::EffCmdVector3Exec(const UnkCommandData *command, unsigned int *flags,
                                     Fn8040Vector3 *target)
    : EffectCommandExec(command), m_flags(flags), m_target(target), m_elapsed(0.0f) {
    const UnkVector3Data *data = static_cast<const UnkVector3Data *>(m_command);
    if (data->sharedRandom != 0) {
        float random = fn_8042A5D0()->Random();
        m_rate.x = data->range.x * random + data->value.x;
        m_rate.y = data->range.y * random + data->value.y;
        m_rate.z = data->range.z * random + data->value.z;
    } else {
        m_rate.x = data->range.x * fn_8042A5D0()->Random() + data->value.x;
        m_rate.y = data->range.y * fn_8042A5D0()->Random() + data->value.y;
        m_rate.z = data->range.z * fn_8042A5D0()->Random() + data->value.z;
    }
    m_duration = data->duration;
    if (data->durationRange != 0) {
        m_duration += data->durationRange * fn_8042A5D0()->Random();
    }
    if (m_duration < 0.0001f) {
        if (data->relative != 0) {
            fn_8040CFF8(m_target, &m_rate);
        } else {
            fn_800091D0(m_target, &m_rate);
        }
        m_target = 0;
    } else {
        if (data->relative == 0) {
            fn_8040CE04(&m_rate, m_target);
        }
        fn_8040CE38(&m_rate, 1.0f / m_duration);
        m_duration += 0.0001f;
    }
}

bool EffCmdVector3Exec::Update(float deltaTime) {
    if (m_target == 0) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    if (field_0x10 == 0) {
        field_0x10 = 1;
        return true;
    }
    m_elapsed += deltaTime;
    if (m_elapsed > m_duration) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    Fn8040Vector3 step;
    fn_800091A4(&step, &m_rate, deltaTime);
    fn_8040CFF8(m_target, &step);
    return true;
}

EffCmdPtnAnimExec::EffCmdPtnAnimExec(const UnkCommandData *command, unsigned int *flags,
                                     Fn8040Vector2 *lower, Fn8040Vector2 *upper)
    : EffectCommandExec(command), m_flags(flags), m_lower(lower), m_upper(upper), m_current(0),
      m_elapsed(0.0f) {
    const UnkPtnAnimData *data = static_cast<const UnkPtnAnimData *>(m_command);
    switch (data->mode) {
    case 0:
        m_divisions = 2;
        break;
    case 1:
        m_divisions = 4;
        break;
    case 2:
        m_divisions = 8;
        break;
    default:
        /* The original still computes the rectangle with this zero divisor. */
        m_divisions = 0;
        break;
    }
    m_current = m_start = data->start;
    if (data->startRange != 0) {
        m_current += static_cast<int>(data->startRange * fn_8042A5D0()->Random());
    }
    m_end = data->end;
    m_interval = data->interval;
    if (data->intervalRange != 0) {
        m_interval += data->intervalRange * fn_8042A5D0()->Random();
    }
    unsigned char column = m_current % m_divisions;
    unsigned char row = m_current / m_divisions;
    float cellSize = 1.0f / m_divisions;
    m_lower->x = column * cellSize;
    m_lower->y = row * cellSize;
    m_upper->x = (column + 1) * cellSize;
    m_upper->y = (row + 1) * cellSize;
    /* The encoded interval disables updates even if its random range was nonzero. */
    if (data->interval == 0) {
        m_divisions = 0;
    }
}

bool EffCmdPtnAnimExec::Update(float deltaTime) {
    if (m_divisions == 0) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    if (field_0x10 == 0) {
        field_0x10 = 1;
        return true;
    }
    m_elapsed += deltaTime;
    float threshold = m_interval + 0.0001f;
    if (m_elapsed < threshold) {
        return true;
    }
    const UnkPtnAnimData *data = static_cast<const UnkPtnAnimData *>(m_command);
    if (data->loop != 0) {
        do {
            m_elapsed -= m_interval;
            if (m_current++ == m_end) {
                m_current = m_start;
            }
        } while (m_elapsed > threshold);
    } else {
        do {
            m_elapsed -= m_interval;
        } while (m_current++ != m_end && m_elapsed > threshold);
        /* The postincrement above can wrap at 255 before this completion test. */
        if (m_current > m_end) {
            if (m_command->stop != 0) {
                *m_flags |= COMMAND_STOP_REQUESTED;
            }
            return false;
        }
    }
    unsigned char column = m_current % m_divisions;
    unsigned char row = m_current / m_divisions;
    float cellSize = 1.0f / m_divisions;
    m_lower->x = column * cellSize;
    m_lower->y = row * cellSize;
    m_upper->x = (column + 1) * cellSize;
    m_upper->y = (row + 1) * cellSize;
    return true;
}

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

EffCmd3DSclExec::EffCmd3DSclExec(const UnkCommandData *command, unsigned int *flags,
                                 Fn8040Vector3 *target)
    : EffectCommandExec(command), m_flags(flags), m_target(target), m_elapsed(0.0f) {
    const UnkVector3Data *data = static_cast<const UnkVector3Data *>(m_command);
    if (data->sharedRandom != 0) {
        float random = fn_8042A5D0()->Random();
        m_rate.x = data->range.x * random + data->value.x;
        m_rate.y = data->range.y * random + data->value.y;
        m_rate.z = data->range.z * random + data->value.z;
    } else {
        m_rate.x = data->range.x * fn_8042A5D0()->Random() + data->value.x;
        m_rate.y = data->range.y * fn_8042A5D0()->Random() + data->value.y;
        m_rate.z = data->range.z * fn_8042A5D0()->Random() + data->value.z;
    }
    m_duration = data->duration;
    if (data->durationRange != 0) {
        m_duration += data->durationRange * fn_8042A5D0()->Random();
    }
    if (m_duration < 0.0001f) {
        if (data->relative != 0) {
            m_target->x *= m_rate.x;
            m_target->y *= m_rate.y;
            m_target->z *= m_rate.z;
        } else {
            fn_800091D0(m_target, &m_rate);
        }
        m_target = 0;
    } else {
        if (data->relative != 0) {
            m_rate.x *= m_target->x;
            m_rate.y *= m_target->y;
            m_rate.z *= m_target->z;
        }
        fn_8040CE04(&m_rate, target);
        fn_8040CE38(&m_rate, 1.0f / m_duration);
        m_duration += 0.0001f;
    }
}

bool EffCmd3DSclExec::Update(float deltaTime) {
    if (m_target == 0) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    if (field_0x10 == 0) {
        field_0x10 = 1;
        return true;
    }
    m_elapsed += deltaTime;
    if (m_elapsed > m_duration) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    Fn8040Vector3 step;
    fn_800091A4(&step, &m_rate, deltaTime);
    fn_8040CFF8(m_target, &step);
    return true;
}

EffCmd2DSclExec::EffCmd2DSclExec(const UnkCommandData *command, unsigned int *flags,
                                 Fn8040Vector3 *target)
    : EffectCommandExec(command), m_flags(flags), m_target(target), m_elapsed(0.0f) {
    const UnkVector2Data *data = static_cast<const UnkVector2Data *>(m_command);
    if (data->sharedRandom != 0) {
        float random = fn_8042A5D0()->Random();
        m_rate.x = data->range.x * random + data->value.x;
        m_rate.y = data->range.y * random + data->value.y;
    } else {
        m_rate.x = data->range.x * fn_8042A5D0()->Random() + data->value.x;
        m_rate.y = data->range.y * fn_8042A5D0()->Random() + data->value.y;
    }
    /* The original leaves m_rate.z unchanged, although its helpers use all three components. */
    m_duration = data->duration;
    if (data->durationRange != 0) {
        m_duration += data->durationRange * fn_8042A5D0()->Random();
    }
    if (m_duration < 0.0001f) {
        if (data->relative != 0) {
            m_target->x *= m_rate.x;
            m_target->y *= m_rate.y;
        } else {
            fn_800091D0(m_target, &m_rate);
        }
        m_target = 0;
    } else {
        if (data->relative != 0) {
            m_rate.x *= m_target->x;
            m_rate.y *= m_target->y;
        }
        fn_8040CE04(&m_rate, m_target);
        fn_8040CE38(&m_rate, 1.0f / m_duration);
        m_duration += 0.0001f;
    }
}

bool EffCmd2DSclExec::Update(float deltaTime) {
    if (m_target == 0) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    if (field_0x10 == 0) {
        field_0x10 = 1;
        return true;
    }
    m_elapsed += deltaTime;
    if (m_elapsed > m_duration) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    Fn8040Vector3 step;
    fn_800091A4(&step, &m_rate, deltaTime);
    fn_8040CFF8(m_target, &step);
    return true;
}

EffCmdMatColorExec::EffCmdMatColorExec(const UnkCommandData *command, unsigned int *flags,
                                       int *target)
    : EffectCommandExec(command), m_flags(flags), m_target(target), m_elapsed(0.0f) {
    const UnkMatColorData *data = static_cast<const UnkMatColorData *>(m_command);
    if (data->sharedRandom != 0) {
        float random = fn_8042A5D0()->Random();
        m_rate[0] = data->value[0];
        if (data->range[0] != 0) {
            m_rate[0] += static_cast<int>(data->range[0] * random);
        }
        m_rate[1] = data->value[1];
        if (data->range[1] != 0) {
            m_rate[1] += static_cast<int>(data->range[1] * random);
        }
        m_rate[2] = data->value[2];
        if (data->range[2] != 0) {
            m_rate[2] += static_cast<int>(data->range[2] * random);
        }
    } else {
        m_rate[0] = data->value[0];
        if (data->range[0] != 0) {
            m_rate[0] += static_cast<int>(data->range[0] * fn_8042A5D0()->Random());
        }
        m_rate[1] = data->value[1];
        if (data->range[1] != 0) {
            m_rate[1] += static_cast<int>(data->range[1] * fn_8042A5D0()->Random());
        }
        m_rate[2] = data->value[2];
        if (data->range[2] != 0) {
            m_rate[2] += static_cast<int>(data->range[2] * fn_8042A5D0()->Random());
        }
    }
    m_duration = data->duration;
    if (data->durationRange != 0) {
        m_duration += data->durationRange * fn_8042A5D0()->Random();
    }
    if (data->relative != 0) {
        for (int i = 0; i < 3; ++i) {
            m_rate[i] <<= MATERIAL_FRACTION_BITS;
            m_rate[i] += m_target[i];
            if (m_rate[i] > MATERIAL_CHANNEL_MAX) {
                m_rate[i] = MATERIAL_CHANNEL_MAX;
            } else if (m_rate[i] < MATERIAL_CHANNEL_CUTOFF) {
                m_rate[i] = 0;
            }
            m_rate[i] -= m_target[i];
        }
    } else {
        for (int i = 0; i < 3; ++i) {
            m_rate[i] <<= MATERIAL_FRACTION_BITS;
            if (m_rate[i] > MATERIAL_CHANNEL_MAX) {
                m_rate[i] = MATERIAL_CHANNEL_MAX;
            } else if (m_rate[i] < MATERIAL_CHANNEL_CUTOFF) {
                m_rate[i] = 0;
            }
        }
    }
    /* Immediate commands write the computed relative deltas directly, too. */
    if (m_duration < 0.0001f) {
        for (int i = 0; i < 3; ++i) {
            m_target[i] = m_rate[i];
        }
        m_target = 0;
    } else {
        if (data->relative == 0) {
            for (int i = 0; i < 3; ++i) {
                m_rate[i] -= m_target[i];
            }
        }
        float inverseDuration = 1.0f / m_duration;
        for (int i = 0; i < 3; ++i) {
            m_rate[i] = static_cast<int>(m_rate[i] * inverseDuration);
        }
        m_duration += 0.0001f;
    }
}

bool EffCmdMatColorExec::Update(float deltaTime) {
    if (m_target == 0) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    if (field_0x10 == 0) {
        field_0x10 = 1;
        return true;
    }
    m_elapsed += deltaTime;
    if (m_elapsed > m_duration) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    for (int i = 0; i < 3; ++i) {
        if (m_rate[i] != 0) {
            m_target[i] += static_cast<int>(m_rate[i] * deltaTime);
            if (m_target[i] > MATERIAL_CHANNEL_MAX) {
                m_target[i] = MATERIAL_CHANNEL_MAX;
                m_rate[i] = 0;
            } else if (m_target[i] < MATERIAL_CHANNEL_CUTOFF) {
                m_target[i] = 0;
                m_rate[i] = 0;
            }
        }
    }
    return true;
}

EffCmdMatAlphaExec::EffCmdMatAlphaExec(const UnkCommandData *command, unsigned int *flags,
                                       int *target)
    : EffectCommandExec(command), m_flags(flags), m_target(target), m_elapsed(0.0f) {
    const UnkMatAlphaData *data = static_cast<const UnkMatAlphaData *>(m_command);
    m_rate = data->value;
    m_rate += static_cast<int>(data->range * fn_8042A5D0()->Random());
    /* A nonzero range takes a second sample in the original. */
    if (data->range != 0) {
        m_rate += static_cast<int>(data->range * fn_8042A5D0()->Random());
    }
    m_duration = data->duration;
    if (data->durationRange != 0) {
        m_duration += data->durationRange * fn_8042A5D0()->Random();
    }
    m_rate <<= MATERIAL_FRACTION_BITS;
    if (data->relative != 0) {
        m_rate += *m_target;
        if (m_rate > MATERIAL_CHANNEL_MAX) {
            m_rate = MATERIAL_CHANNEL_MAX;
        } else if (m_rate < MATERIAL_CHANNEL_CUTOFF) {
            m_rate = 0;
        }
        m_rate -= *m_target;
    } else {
        if (m_rate > MATERIAL_CHANNEL_MAX) {
            m_rate = MATERIAL_CHANNEL_MAX;
        } else if (m_rate < MATERIAL_CHANNEL_CUTOFF) {
            m_rate = 0;
        }
    }
    /* Immediate commands write the computed relative delta directly, too. */
    if (m_duration < 0.0001f) {
        *m_target = m_rate;
        m_target = 0;
    } else {
        if (data->relative == 0) {
            m_rate -= *m_target;
        }
        m_rate = static_cast<int>(m_rate / m_duration);
        m_duration += 0.0001f;
    }
}

bool EffCmdMatAlphaExec::Update(float deltaTime) {
    if (m_target == 0) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    if (field_0x10 == 0) {
        field_0x10 = 1;
        return true;
    }
    m_elapsed += deltaTime;
    if (m_elapsed > m_duration) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    if (m_rate != 0) {
        *m_target += static_cast<int>(m_rate * deltaTime);
        if (*m_target > MATERIAL_CHANNEL_MAX) {
            *m_target = MATERIAL_CHANNEL_MAX;
            m_rate = 0;
        } else if (*m_target < MATERIAL_CHANNEL_CUTOFF) {
            /* Dropping the target reports termination on the next update. */
            m_target = 0;
            m_rate = 0;
        }
    }
    return true;
}

EffCmdWaitExec::EffCmdWaitExec(const UnkCommandData *command, unsigned int *flags,
                               EffectCommandExec *last)
    : EffectCommandExec(command), m_flags(flags), m_elapsed(0.0f) {
    const UnkWaitData *data = static_cast<const UnkWaitData *>(m_command);
    m_duration = data->duration;
    if (data->duration != 0) {
        m_duration += 0.0001f;
    }
    m_last = last;
}

bool EffCmdWaitExec::Update(float deltaTime) {
    if (m_duration < 0.0001f) {
        if (m_last == 0 || m_last != m_prev) {
            if (m_command->stop != 0) {
                *m_flags |= COMMAND_STOP_REQUESTED;
            }
            return false;
        }
    } else {
        if (field_0x10 == 0) {
            field_0x10 = 1;
            return true;
        }
        m_elapsed += deltaTime;
        if (m_elapsed > m_duration) {
            if (m_command->stop != 0) {
                *m_flags |= COMMAND_STOP_REQUESTED;
            }
            return false;
        }
    }
    return true;
}

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

EffCmdLaunchExec::EffCmdLaunchExec(const UnkCommandData *command, unsigned int *flags,
                                   EffectManager *manager, EffectLaunchInfo *info,
                                   EffectArranger *parent)
    : EffectCommandExec(command), m_flags(flags), m_elapsed(0.0f), m_parent(parent) {
    const UnkLaunchData *data = static_cast<const UnkLaunchData *>(m_command);
    /* A zero base duration also suppresses the random duration range. */
    if (data->duration != 0) {
        m_duration = data->duration;
        if (data->durationRange != 0) {
            m_duration += data->durationRange * fn_8042A5D0()->Random();
        }
    } else {
        m_duration = 0.0f;
    }
    if (data->sharedRandom != 0) {
        float random = fn_8042A5D0()->Random();
        m_translation.x = data->translationRange.x * random + data->translation.x;
        m_translation.y = data->translationRange.y * random + data->translation.y;
        m_translation.z = data->translationRange.z * random + data->translation.z;
        random = fn_8042A5D0()->Random();
        m_rotation.x = data->rotationRange.x * random + data->rotation.x;
        m_rotation.y = data->rotationRange.y * random + data->rotation.y;
        m_rotation.z = data->rotationRange.z * random + data->rotation.z;
        random = fn_8042A5D0()->Random();
        m_scale.x = data->scaleRange.x * random + data->scale.x;
        m_scale.y = data->scaleRange.y * random + data->scale.y;
        m_scale.z = data->scaleRange.z * random + data->scale.z;
    } else {
        m_translation.x = data->translationRange.x * fn_8042A5D0()->Random() + data->translation.x;
        m_translation.y = data->translationRange.y * fn_8042A5D0()->Random() + data->translation.y;
        m_translation.z = data->translationRange.z * fn_8042A5D0()->Random() + data->translation.z;
        m_rotation.x = data->rotationRange.x * fn_8042A5D0()->Random() + data->rotation.x;
        m_rotation.y = data->rotationRange.y * fn_8042A5D0()->Random() + data->rotation.y;
        m_rotation.z = data->rotationRange.z * fn_8042A5D0()->Random() + data->rotation.z;
        m_scale.x = data->scaleRange.x * fn_8042A5D0()->Random() + data->scale.x;
        m_scale.y = data->scaleRange.y * fn_8042A5D0()->Random() + data->scale.y;
        m_scale.z = data->scaleRange.z * fn_8042A5D0()->Random() + data->scale.z;
    }
    m_child = manager->Create(info, data->id);
    boost::shared_ptr<EffectElement> child = m_child.lock();
    if (child.get() != 0) {
        child->SetTimeScale(fn_80424D18(m_parent));
    }
}

EffCmdLaunchExec::~EffCmdLaunchExec() {
    boost::shared_ptr<EffectElement> child = m_child.lock();
    if (child.get() != 0) {
        child->ClearFlags(ELEMENT_ACTIVE);
    }
}

bool EffCmdLaunchExec::Update(float deltaTime) {
    boost::shared_ptr<EffectElement> child = m_child.lock();
    if (child.get() == 0 || !child->HasFlags(ELEMENT_ACTIVE)) {
        if (m_command->stop != 0) {
            *m_flags |= COMMAND_STOP_REQUESTED;
        }
        return false;
    }
    EffectArranger *parent = m_parent;
    if ((parent->m_flags & ELEMENT_STOP_REQUESTED) != 0 &&
        !child->HasFlags(ELEMENT_STOP_REQUESTED)) {
        child->RequestStop();
    }
    if ((parent->m_flags & ELEMENT_FLAG_8) != 0 && !child->HasFlags(ELEMENT_FLAG_8)) {
        child->SetFlags(ELEMENT_FLAG_8);
    } else if ((parent->m_flags & ELEMENT_FLAG_8) == 0 && child->HasFlags(ELEMENT_FLAG_8)) {
        child->ClearFlags(ELEMENT_FLAG_8);
    }
    if ((parent->m_flags & ELEMENT_FLAG_4) != 0 && !child->HasFlags(ELEMENT_FLAG_4)) {
        child->SetFlags(ELEMENT_FLAG_4);
    } else if ((parent->m_flags & ELEMENT_FLAG_4) == 0 && child->HasFlags(ELEMENT_FLAG_4)) {
        child->ClearFlags(ELEMENT_FLAG_4);
    }
    child->SetTimeScale(fn_80424D18(m_parent));
    if (field_0x10 == 0) {
        field_0x10 = 1;
        fn_8040C148(&m_transform, &parent->m_transform);
        fn_8040B59C(&m_transform, &m_translation);
        fn_8040B80C(&m_transform, m_rotation.z);
        fn_8040B848(&m_transform, m_rotation.y);
        fn_8040B884(&m_transform, m_rotation.x);
        fn_8040B790(&m_transform, &m_scale);
        if ((parent->m_definitionFlags & ARRANGER_LINK_TRANSFORM) != 0) {
            child->SetMatrixSource(&m_transform);
        } else {
            fn_80425E68(child.get(), &m_transform);
        }
        return true;
    }
    if (child->HasFlags(ELEMENT_FLAG_4)) {
        return true;
    }
    m_elapsed += deltaTime;
    if (m_duration > 0.0001f && m_elapsed > m_duration) {
        if (m_command->type == 18 &&
            static_cast<const UnkTimedLaunchData *>(m_command)->stopAtDuration != 0 &&
            !child->HasFlags(ELEMENT_STOP_REQUESTED)) {
            child->RequestStop();
        }
        if (!child->HasFlags(ELEMENT_STOP_REQUESTED)) {
            if (m_command->stop != 0) {
                *m_flags |= COMMAND_STOP_REQUESTED;
            }
            return false;
        }
    }
    if ((parent->m_definitionFlags & ARRANGER_LINK_TRANSFORM) != 0) {
        fn_8040C148(&m_transform, &parent->m_transform);
        fn_8040B59C(&m_transform, &m_translation);
        fn_8040B80C(&m_transform, m_rotation.z);
        fn_8040B848(&m_transform, m_rotation.y);
        fn_8040B884(&m_transform, m_rotation.x);
        fn_8040B790(&m_transform, &m_scale);
    }
    return true;
}

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

/* Command-stream traversal adjacent to Effect::System RTTI data. The original file and
 * record type names are unknown; lengths are encoded record sizes in bytes. */
unsigned int fn_804220CC(const unsigned char *command) {
    unsigned int count = 0;

    while (command != 0) {
        unsigned int size = 0;

        switch (command[0]) {
        case 0:
        case 2:
            break;
        case 1:
            size = 0x10;
            break;
        case 3:
            size = 0x24;
            break;
        case 4:
            size = 0x24;
            break;
        case 5:
            size = 0x14;
            break;
        case 6:
            size = 0x24;
            break;
        case 7:
            size = 0x1C;
            break;
        case 8:
            size = 0x24;
            break;
        case 9:
            size = 0x14;
            break;
        case 10:
            size = 0x14;
            break;
        case 11:
            size = 0x14;
            break;
        case 12:
            size = 0x10;
            break;
        case 13:
            size = 0x8;
            break;
        case 14:
            size = 0x8;
            break;
        case 15:
            size = 0x4;
            break;
        case 16:
            size = 0x8;
            break;
        case 17:
            size = 0x58;
            break;
        case 18:
            size = 0x5C;
            break;
        case 19:
            size = 0x4;
            break;
        case 20:
            size = 0x14;
            break;
        case 21:
            size = 0x20;
            break;
        case 22:
            size = 0x14;
            break;
        case 23:
            size = 0x10;
            break;
        case 24:
            size = 0x14;
            break;
        case 25:
            size = 0x10;
            break;
        case 26:
            size = 0x14;
            break;
        case 27:
            size = 0x14;
            break;
        case 28:
            size = 0x1C;
            break;
        case 29:
            size = 0x1C;
            break;
        case 30:
            size = 0x1C;
            break;
        case 31:
            size = 0x14;
            break;
        case 32:
            size = 0x14;
            break;
        case 33:
            size = 0x14;
            break;
        }

        if (command[1] != 0 && size != 0) {
            command += size;
        } else {
            command = 0;
        }
        count++;
    }
    return count;
}

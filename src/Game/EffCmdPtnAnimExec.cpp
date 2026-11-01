/* Pattern animation command methods; the original source filename is unknown. */
#include "Game/EffectCommandExec.h"
#include "Game/EffectSystem.h"

namespace Effect {
namespace System {

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

} // namespace System
} // namespace Effect

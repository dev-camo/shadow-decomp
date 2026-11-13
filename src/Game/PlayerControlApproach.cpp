/* Player::Control::Approach (class name from RTTI). The file name is inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/PlayerControlApproach.h"
#include "Game/CharInfo.h"
#include "Game/Player.h"
#include "Game/PlayerCommands.h"
#include "Game/PlayerControlCommands.h"
#include "Game/Target.h"
#include "Game/fn_8040_vector_types.h"

namespace Player {
namespace Control {

const char *Approach::s_name = "Approach";

Approach::Approach(const boost::shared_ptr<PlayerBase> &player) : ControlBase(player, s_name, 1) {}

Approach::~Approach() {}

void Approach::fn_8009E1A4(int, const InitializeArgs &args) {
    const InitializeArgsApproach &approachArgs = dynamic_cast<const InitializeArgsApproach &>(args);
    m_action = approachArgs.m_action;
    m_target = approachArgs.m_target;

    boost::shared_ptr<Target> target = m_target.lock();
    if (target.get() != 0) {
        m_targetInfo = target->m_info;
        fn_80095A64(m_player->field_0x248, m_targetInfo);
    }
    m_time = 0.0f;
}

void Approach::fn_8009E17C(int) {
    fn_80095A40(m_player->field_0x248);
}

/* Ends, by asking the player's controller for the control with id 0, when the target or its
 * character info no longer exists, when bit 2 of the player's bit set at 0xA8 is clear or when
 * m_time, after adding dt, has reached 1. Otherwise it takes the vector from the player's position
 * to the point that the target gives for that position (fn_801E73EC), clears its y, normalises it
 * (fn_8040CE74) and sends the player an AccelerateInputCommand with half of it (the command is
 * null when its allocation fails). When the dot product of the normalised vector and the
 * player's vector from fn_800796DC is at least 0.966 and the character info that fn_80095A10
 * gives for the player's object at 0x248 is the target's, it sends the command of m_action, if
 * any, and ends in the same way. */
void Approach::Update(float dt) {
    if (m_target.expired() || m_targetInfo.expired() || !fn_8006CD78(m_player->field_0xA8, 2)) {
        fn_8009EBA4(m_player->field_0x224, InitializeArgs(0));
        return;
    }

    m_time += dt;
    if (m_time >= 1.0f) {
        fn_8009EBA4(m_player->field_0x224, InitializeArgs(0));
        return;
    }

    boost::shared_ptr<Target> target = m_target.lock();
    boost::shared_ptr<CharInfo> targetInfo = m_targetInfo.lock();

    Fn8040Vector3 direction = target->fn_801E73EC(&m_player->m_position) - m_player->m_position;
    direction.y = 0.0f;
    fn_8040CE74(&direction);
    float dot = fn_8040CF90(direction, fn_800796DC(m_player.get()));
    fn_8040CE38(&direction, 0.5f);
    m_player->fn_801DC980(new AccelerateInputCommand(direction, 1));

    if (dot >= 0.966f) {
        boost::shared_ptr<CharInfo> info = fn_80095A10(m_player->field_0x248).lock();
        if (info == targetInfo) {
            switch (m_action) {
            case 1:
                m_player->fn_801DC980(new PickupObjectCommand(m_target, 2));
                break;
            case 2:
                m_player->fn_801DC980(new TurnOverCommand(m_target, 2));
                break;
            }
            fn_8009EBA4(m_player->field_0x224, InitializeArgs(0));
            return;
        }
    }
}

ControlBase *Approach::Create(const boost::shared_ptr<PlayerBase> &player) {
    return new Approach(player);
}

} // namespace Control
} // namespace Player

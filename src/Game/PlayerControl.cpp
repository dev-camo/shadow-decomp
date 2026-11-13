/* Player::Control::ControlBase's constructor (class name from RTTI). In USA (GUPE8P) it is the
 * only function between the two template functions at 0x8009DC10 and 0x8009DC3C, which belong
 * to the code before it, and Approach::Create, the first function of Player::Control::Approach's
 * file. It is taken to be a file of its own (inferred: the bytes would be the same with the
 * constructor defined last in Approach's file). The file name is inferred from the namespace. */
#include "Game/PlayerControl.h"

namespace Player {
namespace Control {

ControlBase::ControlBase(const boost::shared_ptr<PlayerBase> &player, const char *name, int id)
    : m_player(player), m_name(name), m_id(id) {}

} // namespace Control
} // namespace Player

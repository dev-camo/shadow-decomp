/* Player::WeaponBase (class described in Game/PlayerWeaponBase.h). File name inferred from the
 * namespace and class, as Player::Watch is in PlayerWatch.cpp. Definitions are in reverse address
 * order (deferred inlining emits functions last-defined first). */
#include "Game/PlayerWeaponBase.h"

namespace Player {

WeaponBase::WeaponBase(Task *parent, const boost::shared_ptr<PlayerBase> &owner)
    : Task(parent), m_owner(owner) {
    m_owner->field_0x238 = this;
}

WeaponBase::~WeaponBase() {
    m_owner->field_0x238 = 0;
}

} // namespace Player

/* Attack factory; its original source scope and unused scalar meaning are unknown. */
#include "Game/CharaColliAttack.h"

using SonicteamUSA::System::CharaColli::CharaColliAttack;

boost::shared_ptr<CharaColliAttack> fn_8005A300(int arg0, boost::weak_ptr<CharInfo> owner,
                                                AttackCallback callback0, AttackCallback callback1,
                                                float arg4) {
    boost::shared_ptr<CharaColliAttack> attack =
        boost::shared_ptr<CharaColliAttack>(new CharaColliAttack());

    if (attack.get() != 0) {
        attack->fn_80054DD0(5, 0x40, callback0, callback1, arg4);
        attack->m_owner = owner;
    }

    return attack;
}

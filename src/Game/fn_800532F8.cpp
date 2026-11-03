/* Forward an attack value to the attack list at 0x44. */
#include "Game/CharaColliAttack.h"

using SonicteamUSA::System::CharaColli::CharaColliAttack;

void CharaColliAttack::fn_800532F8(boost::weak_ptr<CharaColliAttack> attack) {
    field_0x44.fn_80053340(attack);
}

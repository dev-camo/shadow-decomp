/* CharaColliAttack setup for its second callback. */
#include "Game/CharaColliAttack.h"

using SonicteamUSA::System::CharaColli::CharaColliAttack;

void CharaColliAttack::fn_80096B64(int arg0, const AttackCallback &callback) {
    field_0x55 = 1;
    field_0x3C = arg0;
    field_0x1C.field_0x0 = callback;
}

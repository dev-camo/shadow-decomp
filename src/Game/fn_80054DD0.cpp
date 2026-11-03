/* CharaColliAttack setup for its first and third callbacks. */
#include "Game/CharaColliAttack.h"

using SonicteamUSA::System::CharaColli::CharaColliAttack;

void CharaColliAttack::fn_80054DD0(int arg0, int arg1, AttackCallback callback0,
                                   AttackCallback callback1, float arg4) {
    field_0x54 = 1;
    field_0x38 = arg0;
    field_0x34 = arg4;
    field_0x40 = arg1;
    field_0x10.field_0x0 = callback0;
    field_0x28.field_0x0 = callback1;
}

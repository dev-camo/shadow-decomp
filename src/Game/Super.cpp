/* Player::Behavior::Super (class name from RTTI): a player behaviour (behaviour in the class
 * comment). File name inferred from the class. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first). */
#include "Game/Super.h"
#include "Game/CharCommand.h"
#include "Game/Player.h"
#include "Game/PlayerMotion.h"
#include "Game/UnkManager_80078C88.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8040_vector_types.h"
#include <cmath>

/* The length of the player's vector at 0x18 at which fn_802EB060's factor reaches 1, and the
 * length of the part along the vector at 0x1C8 of the offset of the player's vector at 0xDC from
 * its position, beyond which fn_802EB0C4 adds that part to the vector at 0x24 (names inferred).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the unknown way the original
 * initialises these floats at startup. The original's static initialiser (0x802EB5D4) stores both
 * of them to .sbss; a constant initialiser would place them in .sdata with no stores. The bytes do
 * not establish the variables' original type: plain floats initialised by an expression would fit
 * too. Revisit when the original initialiser expression is found. */
static const float s_fullSpeed = UnkStartupFloat_800E2570(750.0f).value;
static const float s_offsetLimit = UnkStartupFloat_800E2570(10.0f).value;

namespace Player {
namespace Behavior {

/* The entry of fn_80078C88's manager that Super starts (fn_8027AA78), stops (fn_8027AA4C) and
 * waits for (fn_8027AAA4); placeholder name, the entry's meaning is unknown. */
static const int kSuperManagerEntry = 0xE;

const char *Super::s_name = "Super";

Super::Super(const boost::shared_ptr<PlayerBase> &player) : Behavior(player, s_name, 0x22) {}

Super::~Super() {}

bool Super::fn_802EB530() {
    return true;
}

void Super::fn_802EB4B8() {
    fn_8027AA78(fn_80078C88(), kSuperManagerEntry);
    m_player->field_0xA0.Set(2);
    m_player->field_0xA0.Set(7);
    m_player->field_0xA0.Set(9);
    field_0x24 = 0.0f;
    field_0x28 = 0.0f;
    field_0x1C = 0;
}

void Super::fn_802EB454() {
    fn_8027AA4C(fn_80078C88(), kSuperManagerEntry);
    m_player->field_0xA0.Reset(2);
    m_player->field_0xA0.Reset(7);
    m_player->field_0xA0.Reset(9);
}

void Super::fn_80072420(CharCommand *command) {
    if (field_0x1C == 0) {
        command->m_flags |= 2;
    }
}

void Super::Update(float dt) {
    switch (field_0x1C) {
    case 0:
        fn_802EB39C();
        break;
    case 1:
        fn_802EB0C4(dt);
        break;
    }
}

void Super::fn_802EB3E0() {
    field_0x1C = 1;
    field_0x20 = 0;
}

void Super::fn_802EB39C() {
    if (fn_8027AAA4(fn_80078C88(), kSuperManagerEntry)) {
        fn_802EB3E0();
    }
}

/* In order:
 * - Only while field_0x20 is 0 (fn_802EB3E0 clears it): calls the motion's fn_8006CB7C with 0x2B
 *   and 10.0f and increments field_0x20.
 * - field_0x24: fn_802EB060 while bit 0x1B of the player's bit set is clear; otherwise 250 when
 *   bit 0x3C is set or when the squared length of the vector at 0xB8 is below 0.9 * 0.9;
 *   otherwise 250 + 150 * (dot - 0.766) / (1 - 0.766), where dot is the dot product of the
 *   player's facing direction (fn_800796DC) and that vector normalised, or 250 when dot is below
 *   0.766.
 * - Adds the vector at 0xB8 times field_0x24 to the player's vector at 0x24. When the part of
 *   the offset of the vector at 0xDC from the player's position along the vector at 0x1C8 is
 *   longer than s_offsetLimit (compared squared), also adds that part, normalised, times 25.
 * - Calls fn_80079720, clears bit 2 of the bit set and calls fn_8009728C with 0.
 * - When bit 0x3C is set: sets field_0x28 to 15 and passes fn_80093FC0's vector and
 *   field_0x28 * dt to fn_80093BA0. Otherwise field_0x28 is 4 while the squared length of the
 *   vector at 0x18 is at most 150 * 150, 2 when it is above 300 * 300, and in between
 *   2 + 2 * (length - 300) / -150; then, only when bit 0x1B is set, it passes field_0x28 * dt to
 *   fn_80093AF8. */
void Super::fn_802EB0C4(float dt) {
    /* The original compares field_0x20 with beq and b, which a switch with one case produces
     * (an if gives a single bne). */
    switch (field_0x20) {
    case 0:
        m_player->field_0x230->fn_8006CB7C(0x2B, 10.0f);
        field_0x20++;
        break;
    }

    Fn8040Vector3 vectorB8(m_player->field_0xB8);
    if (!fn_8006CD78(m_player->field_0xA8, 0x1B)) {
        fn_802EB060();
    } else if (fn_8006CD78(m_player->field_0xA8, 0x3C)) {
        field_0x24 = 250.0f;
    } else if (fn_8040CF2C(&m_player->field_0xB8) < 0.9f * 0.9f) {
        field_0x24 = 250.0f;
    } else {
        const Fn8040Vector3 &direction = fn_8040CEE8(vectorB8);
        const Fn8040Vector3 &facing = fn_800796DC(m_player.get());
        float dot = fn_8040CF90(&facing, &direction);
        float factor;
        if (dot < 0.766f) {
            factor = 0.0f;
        } else {
            factor = (dot - 0.766f) / (1.0f - 0.766f);
        }
        field_0x24 = 150.0f * factor + 250.0f;
    }

    AddVector(&m_player->field_0x24, vectorB8 * field_0x24);

    Fn8040Vector3 offset = m_player->field_0xDC - m_player->m_position;
    Fn8040Vector3 along;
    fn_8040F990(&along, 0, &offset, &m_player->field_0x1C8);
    if (fn_8040CF2C(&along) > s_offsetLimit * s_offsetLimit) {
        AddVector(&m_player->field_0x24, fn_8040CEE8(along) * 25.0f);
    }

    fn_80079720(m_player.get());
    fn_80076CC8(m_player->field_0xA8, 2);
    fn_8009728C(m_player, 0);

    if (fn_8006CD78(m_player->field_0xA8, 0x3C)) {
        field_0x28 = 15.0f;
        fn_80093BA0(m_player, fn_80093FC0(m_player), field_0x28 * dt);
    } else {
        float speedSquared = fn_8040CF2C(&m_player->field_0x18);
        if (speedSquared <= 22500.0f) {
            field_0x28 = 4.0f;
        } else if (speedSquared > 90000.0f) {
            field_0x28 = 2.0f;
        } else {
            float speed = std::sqrt(speedSquared);
            field_0x28 = 2.0f * ((speed - 300.0f) / -150.0f) + 2.0f;
        }

        if (fn_8006CD78(m_player->field_0xA8, 0x1B)) {
            fn_80093AF8(m_player, field_0x28 * dt);
        }
    }
}

void Super::fn_802EB060() {
    float factor = fn_8040CF50(&m_player->field_0x18) / s_fullSpeed;
    factor = fn_800704AC(factor, 1.0f);
    field_0x24 = 150.0f * factor + 250.0f;
}

Behavior *Super::Create(const boost::shared_ptr<PlayerBase> &player) {
    return new Super(player);
}

} // namespace Behavior
} // namespace Player

/* The record's second function, an empty function (only blr). Its signature is inferred: an empty
 * body cannot show the parameters or the result. */
extern "C" void fn_802EB014(void) {}

const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_80507B50 = {Player::Behavior::Super::Create,
                                                                   fn_802EB014, 0};

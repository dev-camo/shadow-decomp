/* Player::Shadow::ShadowBehavior (class name from RTTI): the factory of Shadow's behaviours, and
 * the table of behaviour records it reads. File name inferred from the class. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/PlayerShadowBehavior.h"
#include "Game/Player.h"
#include "Game/Super.h"

/* The records of the behaviours whose files are not recovered yet (placeholder names; the
 * addresses are USA (GUPE8P) ones). Each declaration moves to its behaviour's header when that
 * file is recovered, as Super's has. */
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0518;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0000;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AFDD0;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0858;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0988;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0694;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B07A4;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0D08;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B03E0;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B04CC;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0F44;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B10B8;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AFCBC;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B1234;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0944;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0D60;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B13DC;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AFF64;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0DB4;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AFACC;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B08F4;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0AF8;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0C90;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B08A0;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B0DF8;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804B011C;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AF784;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AFB60;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AF894;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AF9CC;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AFA7C;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AFB10;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AFD74;
extern const Player::Behavior::UnkBehaviorRecord_80521CC8 lbl_804AF420;

/* The number of entries of the behaviour table (name inferred): the constructor and the
 * destructor of ShadowBehavior visit this many. */
static const int kBehaviorTableSize = 37;

/* The behaviour table (placeholder name): a pointer to the record of each behaviour id from 0 to
 * 0x22. The last two entries have no initialiser and are null. */
const Player::Behavior::UnkBehaviorRecord_80521CC8 *lbl_80521CC8[kBehaviorTableSize] = {
    &lbl_804B0518, &lbl_804B0000, &lbl_804AFDD0, &lbl_804B0858, &lbl_804B0988, &lbl_804B0694,
    &lbl_804B07A4, &lbl_804B0D08, &lbl_804B03E0, &lbl_804B04CC, &lbl_804B0F44, &lbl_804B10B8,
    &lbl_804AFCBC, &lbl_804B1234, &lbl_804B0944, &lbl_804B0D60, &lbl_804B13DC, &lbl_804AFF64,
    &lbl_804B0DB4, &lbl_804AFACC, &lbl_804B08F4, &lbl_804B0AF8, &lbl_804B0C90, &lbl_804B08A0,
    &lbl_804B0DF8, &lbl_804B011C, &lbl_804AF784, &lbl_804AFB60, &lbl_804AF894, &lbl_804AF9CC,
    &lbl_804AFA7C, &lbl_804AFB10, &lbl_804AFD74, &lbl_804AF420, &lbl_80507B50,
};

namespace Player {
namespace Shadow {

ShadowBehavior::ShadowBehavior(const boost::shared_ptr<PlayerBase> &player) : m_player(player) {
    for (int i = 0; i < kBehaviorTableSize; i++) {
        const Behavior::UnkBehaviorRecord_80521CC8 *record = lbl_80521CC8[i];
        if (record != 0 && record->field_0x4 != 0) {
            record->field_0x4(m_player);
        }
    }
}

ShadowBehavior::~ShadowBehavior() {
    for (int i = 0; i < kBehaviorTableSize; i++) {
        const Behavior::UnkBehaviorRecord_80521CC8 *record = lbl_80521CC8[i];
        if (record != 0 && record->field_0x8 != 0) {
            record->field_0x8(m_player);
        }
    }
}

boost::shared_ptr<Behavior::Behavior> ShadowBehavior::Create(int id) {
    if (fn_8006CD78(m_player->field_0xA8, 0x25)) {
        switch (id) {
        case 0:
        case 2:
        case 3:
            /* Player::Behavior::Super's id. */
            id = 0x22;
            break;
        }
    }

    const Behavior::UnkBehaviorRecord_80521CC8 *record = lbl_80521CC8[id];
    if (record != 0 && record->create != 0) {
        boost::shared_ptr<Behavior::Behavior> behavior(record->create(m_player));
        behavior->SetField0x14(behavior);
        return behavior;
    }
    return boost::shared_ptr<Behavior::Behavior>();
}

} // namespace Shadow
} // namespace Player

boost::shared_ptr<Player::Behavior::Factory>
fn_800AE330(const boost::shared_ptr<Player::PlayerBase> &player) {
    return boost::shared_ptr<Player::Behavior::Factory>(new Player::Shadow::ShadowBehavior(player));
}

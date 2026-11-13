/* The stage's gravity direction: its table of directions, the state object that holds the current
 * index and the function that changes it (the original file name is unknown). Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/UnkGravity_80293928.h"
#include "Game/CharInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkManager_8006F754.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class UnkGravityState_805F0340 {
  public:
    UnkGravityState_805F0340() : index(0) {}
    /* Nontrivial destruction is inferred from the native class-delete pattern. */
    ~UnkGravityState_805F0340() {}

    /* 0x00 */ int index;
};
CHECK_SIZEOF(UnkGravityState_805F0340, 0x4);

UnkGravityState_805F0340 *lbl_UnkGravityState_805F0340;

Fn8040Vector3 lbl_80584360(0.0f, 1.0f, 0.0f);

/* The directions that an index selects (name inferred): +y, -y, +x, -x, +z and -z. The first is
 * a copy of lbl_80584360, as in the original's start-up function (USA (GUPE8P) 0x80293BA4). */
static Fn8040Vector3 s_directions[6] = {
    lbl_80584360,
    Fn8040Vector3(0.0f, -1.0f, 0.0f),
    Fn8040Vector3(1.0f, 0.0f, 0.0f),
    Fn8040Vector3(-1.0f, 0.0f, 0.0f),
    Fn8040Vector3(0.0f, 0.0f, 1.0f),
    Fn8040Vector3(0.0f, 0.0f, -1.0f),
};

const Fn8040Vector3 *fn_80293B60(int index) {
    return &s_directions[index];
}

void fn_80293B1C(void) {
    if (lbl_UnkGravityState_805F0340 == 0) {
        lbl_UnkGravityState_805F0340 = new UnkGravityState_805F0340;
    }
}

void fn_80293AE4(void) {
    if (lbl_UnkGravityState_805F0340 != 0) {
        delete lbl_UnkGravityState_805F0340;
        lbl_UnkGravityState_805F0340 = 0;
    }
}

int fn_80293AC8(void) {
    if (lbl_UnkGravityState_805F0340 != 0) {
        return lbl_UnkGravityState_805F0340->index;
    }
    return 0;
}

void fn_80293928(int index) {
    if (lbl_UnkGravityState_805F0340 != 0) {
        lbl_UnkGravityState_805F0340->index = index;

        Fn8040Vector3 direction = s_directions[lbl_UnkGravityState_805F0340->index];
        boost::shared_ptr<CharInfo> player = fn_800A8B74(fn_80074194(), lbl_805EE588).lock();

        if (player.get() != 0) {
            player->fn_801DC980(new Player::SetGravityCommand(&direction, 4));
        }

        for (int id = 1; id < 14; id++) {
            boost::weak_ptr<CharInfo> partner = fn_8007488C(fn_8006F754(), id);

            if (!partner.expired()) {
                boost::shared_ptr<CharInfo> info = partner.lock();

                info->fn_801DC980(new Player::SetGravityCommand(&direction, 4));
            }
        }
    }
}

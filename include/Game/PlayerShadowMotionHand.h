#ifndef GAME_PLAYERSHADOWMOTIONHAND_H
#define GAME_PLAYERSHADOWMOTIONHAND_H

#include "Game/Task.h"
#include "boost/shared_ptr.hpp"
#include <vector>
#include "layout_check.h"

struct _stMtnPlayWork;
struct Fn8040Matrix34;

namespace Player {
class PlayerBase;
struct UnkExternalMotionArgs_8009A6A4;

namespace Shadow {

struct UnkMotionResources_800B0D3C;
/* The 0x124-byte object that MotionHand's constructor creates (0x800B0C3C constructs it; type name
 * unknown). */
class UnkMotionHandImpl_800B0C3C;

/* Player::Shadow::MotionHand (class name and Task base from RTTI): a task that ShadowMotion creates
 * (0x800B4BA8) and forwards calls to. Only the functions that ShadowMotion calls are declared;
 * their names are placeholders and their signatures follow ShadowMotion's calls. */
class MotionHand : public Task {
  public:
    MotionHand(Task *parent, const boost::shared_ptr<PlayerBase> &owner,
               const UnkMotionResources_800B0D3C &resources);

    void fn_800AFFF0();
    void fn_800B0014();
    void fn_800B0038(_stMtnPlayWork *&first, _stMtnPlayWork *&second);
    /* A matrix for the motion node id, used where ShadowMotion's motion control has no node of
     * that id (0x800B43F0). */
    Fn8040Matrix34 *fn_800B0084(int nodeId);
    void fn_800B00F0(float arg0);
    void fn_800B0124(const UnkExternalMotionArgs_8009A6A4 &arg0, float arg1);
    void fn_800B0148(int arg0, float arg1);
    void fn_800B019C(int arg0);
    void fn_800B01C0(std::vector<void *> &objects);
    void fn_800B01E4();

    /* 0x28 */ UnkMotionHandImpl_800B0C3C *field_0x28;
    /* 0x2C */ u8 field_0x2C[4]; /* Unrecovered storage. */
};
CHECK_SIZEOF(MotionHand, 0x30);

} // namespace Shadow
} // namespace Player

#endif

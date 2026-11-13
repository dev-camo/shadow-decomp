#ifndef GAME_PLAYERSHADOWMOTION_H
#define GAME_PLAYERSHADOWMOTION_H

#include "Game/PlayerMotion.h"
#include "Game/PlayerMotionEventArgs.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include <vector>
#include "layout_check.h"

class Unk80015A7C;
struct MotionNodeCallbackData;

namespace Player {
class PlayerBase;

namespace Shadow {

class MotionHand;
class MotionEye;
class MotionSub;

/* A key and an index for the resource lookups fn_8001277C, fn_80012608 and fn_80012684 (placeholder
 * names). The copy constructor of UnkMotionResources_800B0D3C is not expanded where it is used
 * (its out-of-line copy is at USA (GUPE8P) 0x800B0D3C); the user-declared constructor that makes
 * the copies of this type count as inline calls is inferred from that. */
struct UnkResourceKey_800B0D3C {
    UnkResourceKey_800B0D3C() {}

    /* 0x0 */ int field_0x0;
    /* 0x4 */ int field_0x4;
};
CHECK_SIZEOF(UnkResourceKey_800B0D3C, 0x8);

/* The resource keys of Shadow's motion tasks (placeholder name): Player::PlayerShadow builds them
 * and passes them to the factory of ShadowMotion, which copies them and passes them on to MotionHand
 * and MotionEye. The grouping into members follows the loops of the copy constructor at USA
 * (GUPE8P) 0x800B0D3C (one key, two arrays of five, four keys); ShadowMotion reads the model
 * (field_0x0), the first motion (field_0x68) and the motion pack (field_0x70). */
struct UnkMotionResources_800B0D3C {
    /* 0x00 */ UnkResourceKey_800B0D3C field_0x0;
    /* 0x08 */ UnkResourceKey_800B0D3C field_0x8[5];
    /* 0x30 */ UnkResourceKey_800B0D3C field_0x30[5];
    /* 0x58 */ UnkResourceKey_800B0D3C field_0x58;
    /* 0x60 */ UnkResourceKey_800B0D3C field_0x60;
    /* 0x68 */ UnkResourceKey_800B0D3C field_0x68;
    /* 0x70 */ UnkResourceKey_800B0D3C field_0x70;
};
CHECK_SIZEOF(UnkMotionResources_800B0D3C, 0x78);

/* The callback of an IK target (placeholder name). Its constructor, destructor, assignment and
 * Clear are inline functions whose out-of-line copies are at USA (GUPE8P) 0x800B12C0, 0x800B116C,
 * 0x800B4D30 and 0x800B4F0C. The assignment from a callback is inferred from its copy, which passes
 * its argument on to the callback's own assignment. Clear (name inferred) is inferred from its
 * copy, which takes no argument, calls the callback's clear() and returns this object; its callers
 * (0x800B4EE8, 0x800B4F68) pass no second argument. */
class UnkIKCallback_800B12C0 {
  public:
    UnkIKCallback_800B12C0 &operator=(const IKEventCallback &callback) {
        m_function = callback;
        return *this;
    }
    UnkIKCallback_800B12C0 &Clear() {
        m_function.clear();
        return *this;
    }

    /* 0x0 */ IKEventCallback m_function;
};
CHECK_SIZEOF(UnkIKCallback_800B12C0, 0xC);

/* An IK target of ShadowMotion (placeholder name): three joint matrices that the IK solvers
 * compute, the target matrix, the callback that supplies a target position instead, whether the
 * target is set (field_0xCC) and the weight of the solution (field_0xD0). */
class UnkIKRecord_800B1264 {
  public:
    UnkIKRecord_800B1264();
    ~UnkIKRecord_800B1264();

    /* 0x00 */ Fn8040Matrix34 field_0x0[3];
    /* 0x90 */ Fn8040Matrix34 field_0x90;
    /* 0xC0 */ UnkIKCallback_800B12C0 field_0xC0;
    /* 0xCC */ bool field_0xCC;
    /* 0xD0 */ float field_0xD0;
};
CHECK_SIZEOF(UnkIKRecord_800B1264, 0xD4);

/* A rotation that ShadowMotion turns towards a target in one of its node callbacks (placeholder
 * name; behaviour inferred). */
class UnkRotation_800B1618 {
  public:
    UnkRotation_800B1618();
    void fn_800B1308(MotionNodeCallbackData *data, float dt);

    /* 0x0 */ Fn8040Vector4 field_0x0;
};
CHECK_SIZEOF(UnkRotation_800B1618, 0x10);

/* Player::Shadow::ShadowMotion (class name and base from RTTI): Shadow's motion task. It loads the
 * model and motions named by its resource keys (fn_800B4B44, fn_800B44B4), creates the MotionHand,
 * MotionEye and MotionSub tasks, registers its node callbacks with the motion control and runs one
 * of the state functions of the state table each frame. Names of the methods not in the RTTI are
 * placeholders. */
class ShadowMotion : public PlayerMotion {
  public:
    /* A state function: the state table gives each state an update function called before the
     * motion control steps and one called after (placeholder name). */
    typedef void (ShadowMotion::*StateFunction)();

    ShadowMotion(Task *parent, const boost::shared_ptr<PlayerBase> &owner,
                 const UnkMotionResources_800B0D3C &resources);
    virtual ~ShadowMotion();
    virtual void Update(float dt);
    virtual void fn_8006CB7C(int arg0, float arg1);
    virtual void UnkSlot3(std::vector<void *> &objects);
    virtual void fn_8006C090(int arg0);
    virtual void fn_8006C094(const UnkExternalMotionArgs_8009A6A4 &arg0, float arg1);
    virtual void fn_8006C098(int arg0);
    virtual void fn_8006C0A0(int arg0, const Fn8040Matrix34 &matrix);
    virtual void fn_8006C09C(int arg0, const IKEventCallback &callback);
    virtual void fn_8006C0A4(int arg0, float arg1);
    virtual float fn_8006C0A8(int arg0);
    virtual void fn_800AA748(_stMtnPlayWork *&first, _stMtnPlayWork *&second);
    virtual int fn_800AA740();
    virtual void UnkSlot13();
    virtual void UnkSlot14();

    void fn_800B27E4();
    void fn_800B2814();
    void fn_800B2844();
    void fn_800B2A18();
    void fn_800B2A84();
    void fn_800B2B44();
    void fn_800B2B48();
    void fn_800B2C0C();
    void fn_800B2C70();
    void fn_800B2D38();
    void fn_800B2E28();
    void fn_800B2EA4();
    void fn_800B2F48();
    void fn_800B30F8();
    void fn_800B31E0();
    void fn_800B32B4();
    void fn_800B334C();
    void fn_800B33D8();
    void fn_800B34E0();
    void fn_800B3538();
    void fn_800B3624();
    void fn_800B36F8();
    void fn_800B38C8();
    void fn_800B3AAC();
    void fn_800B3CA4();
    void fn_800B3D40();
    void fn_800B4088();
    void fn_800B42A8(IKEventArgs &args);
    void fn_800B4328();
    void fn_800B442C();
    void fn_800B44B4();
    void fn_800B4754();
    void fn_800B4804(float dt);
    void fn_800B4B44();

    /* 0x458 */ int field_0x458;
    /* 0x45C */ Unk80015A7C *field_0x45C;
    /* 0x460 */ StateFunction field_0x460;
    /* 0x46C */ StateFunction field_0x46C;
    /* 0x478 */ float field_0x478;
    /* 0x47C */ UnkRotation_800B1618 field_0x47C;
    /* 0x48C */ bool field_0x48C;
    /* 0x490 */ UnkIKRecord_800B1264 field_0x490[4];
    /* 0x7E0 */ MotionHand *field_0x7E0;
    /* 0x7E4 */ MotionEye *field_0x7E4;
    /* 0x7E8 */ MotionSub *field_0x7E8;
    /* 0x7EC */ UnkMotionResources_800B0D3C field_0x7EC;
    /* 0x864 */ u8 field_0x864[4]; /* Unrecovered storage. */
};
CHECK_SIZEOF(ShadowMotion, 0x868);
CHECK_OFFSETOF(ShadowMotion, field_0x458, 0x458);
CHECK_OFFSETOF(ShadowMotion, field_0x460, 0x460);
CHECK_OFFSETOF(ShadowMotion, field_0x47C, 0x47C);
CHECK_OFFSETOF(ShadowMotion, field_0x490, 0x490);
CHECK_OFFSETOF(ShadowMotion, field_0x7E0, 0x7E0);
CHECK_OFFSETOF(ShadowMotion, field_0x7EC, 0x7EC);

} // namespace Shadow
} // namespace Player

extern "C" {
/* Creates a Player::Shadow::ShadowMotion (placeholder name). Player::PlayerShadow's constructor
 * calls it at 0x800ADAF4 and overwrites r3 at 0x800ADAF8 without using a result. */
void fn_800B10CC(Task *parent, const boost::shared_ptr<Player::PlayerBase> &owner,
                 const Player::Shadow::UnkMotionResources_800B0D3C &resources);
}

#endif

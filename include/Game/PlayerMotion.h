#ifndef GAME_PLAYERMOTION_H
#define GAME_PLAYERMOTION_H

#include "Game/Task.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_80027D5C.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <vector>
#include "layout_check.h"

struct _stMtnPlayWork;
namespace Effect {
namespace System {
class EffectElement;
}
} // namespace Effect

namespace Player {
class PlayerBase;
struct IKEventArgs;
/* The addressed three-word input of SetExternalMotionCommand (the command's words at 0x14 to 0x1F,
 * passed by the code at USA (GUPE8P) 0x8009A6A8). Its type name and the meaning of the last two
 * words remain unknown; Player::Shadow::ShadowMotion's implementation of the slot (0x800B4F98)
 * makes the first word its motion index. The const reference used by the virtual slot is
 * inferred. */
struct UnkExternalMotionArgs_8009A6A4 {
    /* 0x0 */ int field_0x0;
    /* 0x4 */ u8 field_0x4[8]; /* Unrecovered storage. */
};
CHECK_SIZEOF(UnkExternalMotionArgs_8009A6A4, 0xC);

/* The records at PlayerMotion +0xD0 have stride 0x38. Their matrix pointer is assigned by the
 * derived motion task; their vectors are updated by fn_800AAB78. Names and flag types inferred. */
struct UnkMotionRecord_800AAB74 {
    UnkMotionRecord_800AAB74();
    void fn_800AAB78();

    /* 0x00 */ Fn8040Matrix34 *field_0x0;
    /* 0x04 */ Fn8040Vector3 field_0x4;
    /* 0x10 */ Fn8040Vector3 field_0x10;
    /* 0x1C */ Fn8040Vector3 field_0x1C;
    /* 0x28 */ Fn8040Vector3 field_0x28;
    /* 0x34 */ bool field_0x34;
    /* 0x35 */ bool field_0x35;
};
CHECK_SIZEOF(UnkMotionRecord_800AAB74, 0x38);

/* Class and base names from RTTI. The task registers itself at its player's +0x230 and clears
 * that registration on destruction. Virtual signatures and member types are inferred from
 * native derived implementations and callers; placeholder method names retain their addresses.
 * The seven default slots are inline. Their out-of-line copies at 0x8006C090-0x8006C0B0 in USA
 * (GUPE8P) open the Player::Npc::Sonic::MotionImpl file, after the preceding file's static
 * initialiser at 0x8006C054. */
class PlayerMotion : public Task {
  public:
    PlayerMotion(Task *parent, const boost::shared_ptr<PlayerBase> &owner);
    virtual ~PlayerMotion();
    virtual void Update(float dt);
    virtual void fn_8006CB7C(int arg0, float arg1) = 0;
    virtual void UnkSlot3(std::vector<void *> &objects) = 0;
    virtual void fn_8006C090(int arg0) {}
    virtual void fn_8006C094(const UnkExternalMotionArgs_8009A6A4 &arg0, float arg1) {}
    virtual void fn_8006C098(int arg0) {}
    virtual void fn_8006C0A0(int arg0, const Fn8040Matrix34 &matrix) {}
    virtual void fn_8006C09C(int arg0, const boost::function1<void, IKEventArgs &> &callback) {}
    virtual void fn_8006C0A4(int arg0, float arg1) {}
    virtual float fn_8006C0A8(int arg0) {
        return 0.0f;
    }
    virtual void fn_800AA748(_stMtnPlayWork *&first, _stMtnPlayWork *&second);
    virtual int fn_800AA740();
    virtual void UnkSlot13() = 0;
    virtual void UnkSlot14() = 0;

    void fn_800AA7C0(float value);
    void fn_800AA7B8(float value);
    int fn_800AA7B0();
    UnkMotionRecord_800AAB74 *fn_800AA760(int index);
    _stMtnPlayWork *fn_800AA758();
    void fn_800AA700(bool enabled);
    void fn_800AA6C0(bool enabled);
    bool fn_800AA640();
    void fn_800AA638(int value);
    void fn_800AA5D4();
    void fn_800AA58C(float value);
    Unk801A35ACParameters *fn_800AA584();
    void fn_800AA404();
    void fn_800AA3A4();
    void fn_800AA0D4();
    float fn_800AA064();
    void fn_800A9F9C(float fraction);
    float fn_800A9EF8();
    void fn_800A9EA0();
    void fn_800A9E48();

    /* 0x028 */ boost::weak_ptr< ::Effect::System::EffectElement> field_0x28[6];
    /* 0x058 */ boost::shared_ptr<PlayerBase> m_owner; /* name inferred */
    /* 0x060 */ UnkBitFlags_80014E04 field_0x60;
    /* Unrecovered storage; no object composition is inferred for this interval. */
    /* 0x064 */ u8 field_0x64[0xC];
    /* ShadowMotion supplies these two matrix addresses to the record array. */
    /* 0x070 */ Fn8040Matrix34 field_0x70;
    /* 0x0A0 */ Fn8040Matrix34 field_0xA0;
    /* 0x0D0 */ UnkMotionRecord_800AAB74 field_0xD0[14];
    /* 0x3E0 */ unsigned int field_0x3E0;
    /* 0x3E4 */ int field_0x3E4;
    /* 0x3E8 */ _stMtnPlayWork *field_0x3E8;
    /* 0x3EC */ unsigned int field_0x3EC;
    /* 0x3F0 */ int field_0x3F0;
    /* 0x3F4 */ Unk801A35ACParameters field_0x3F4;
    /* 0x420 */ int field_0x420[2];
    /* 0x428 */ int field_0x428;
    /* 0x42C */ int field_0x42C;
    /* 0x430 */ float field_0x430;
    /* 0x434 */ float field_0x434;
    /* 0x438 */ float field_0x438;
    /* 0x43C */ float field_0x43C;
    /* 0x440 */ int field_0x440[2];
    /* 0x448 */ float field_0x448;
    /* 0x44C */ float field_0x44C;
    /* 0x450 */ int field_0x450;

  private:
    /* Declared and not defined; the executable has no copy of either. */
    PlayerMotion(const PlayerMotion &);
    PlayerMotion &operator=(const PlayerMotion &);
};
CHECK_SIZEOF(PlayerMotion, 0x458);
CHECK_OFFSETOF(PlayerMotion, m_owner, 0x58);
CHECK_OFFSETOF(PlayerMotion, field_0x70, 0x70);
CHECK_OFFSETOF(PlayerMotion, field_0xA0, 0xA0);
CHECK_OFFSETOF(PlayerMotion, field_0xD0, 0xD0);
CHECK_OFFSETOF(PlayerMotion, field_0x3E8, 0x3E8);
CHECK_OFFSETOF(PlayerMotion, field_0x3F4, 0x3F4);
CHECK_OFFSETOF(PlayerMotion, field_0x420, 0x420);
CHECK_OFFSETOF(PlayerMotion, field_0x450, 0x450);

} // namespace Player

#endif

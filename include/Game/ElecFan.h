#ifndef GAME_ELECFAN_H
#define GAME_ELECFAN_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

struct RpClump;

/* The gadget parameters in the order of the descriptor's table (names from that table). */
struct ElecFanParams {
    /* 0x0 */ float height;
    /* 0x4 */ float radius;
    /* 0x8 */ float power;
};
CHECK_SIZEOF(ElecFanParams, 0xC);

/* Class name from RTTI; member names and method names marked "(name inferred)" are inferred. */
class ElecFan : public Gadget {
  public:
    ElecFan(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecFan();
    virtual void UpdateBody(float dt);
    /* Draws the range of the fan as a debug cylinder (behaviour inferred). */
    virtual void fn_800CBC1C(float);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Allocates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, com id and parameters (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Pushes the nearest character info upwards while it is inside the range (name inferred). */
    void BlowCharacter();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::weak_ptr<CharInfo> m_target; /* the character info being blown */
    /* 0x40 */ ElecFanParams m_params;
    /* 0x4C */ u8 m_comId;
    /* Bit 0 is set in the frames in which the fan blows a character info and bit 2 while the fan
     * runs (its switch is on, or it has no com id) (inferred). */
    /* 0x50 */ UnkBitFlags_80014E04 m_state;
    /* Unclassified storage in the original 0x58-byte allocation; no accesses are identified. */
    /* 0x54 */ unsigned char field_0x54[4];
};
CHECK_SIZEOF(ElecFan, 0x58);

/* Class name from RTTI: the short-lived task that the fan starts each frame while it runs. Through
 * the callback it hands to a draw layer, it draws the clump of DE0403DENSIRYUU.DFF rising from a
 * point on the fan's rim and fading out, and it marks itself finished once its timer reaches
 * 0.333333 (behaviour inferred). */
class ElecFanDraw : public Task {
  public:
    ElecFanDraw(Task *parent, Fn8040Vector3 position, float angle, float radius, float halfHeight);
    virtual ~ElecFanDraw();
    virtual void Update(float dt);

    /* The draw layer's callback (name inferred). */
    void OnDraw(RednerDrawArgs &args);

    static const char *s_name;

    /* 0x28 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0x30 */ RpClump *m_clump;
    /* 0x34 */ float m_timer;
    /* 0x38 */ float m_radius;     /* the fan's radius */
    /* 0x3C */ float m_halfHeight; /* half the fan's height */
    /* 0x40 */ float m_boundRadius;
    /* 0x44 */ Fn8040Vector3 m_position;
    /* 0x50 */ float m_angle; /* radians about the y axis */
    /* Unclassified storage in the original 0x58-byte allocation; no accesses are identified. */
    /* 0x54 */ unsigned char field_0x54[4];
};
CHECK_SIZEOF(ElecFanDraw, 0x58);

#endif

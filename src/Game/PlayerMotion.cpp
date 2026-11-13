/* Player::PlayerMotion (class name from RTTI). File name inferred from the class. The task owns
 * its player reference and weak effect handles; its motion control is supplied by derived tasks.
 * Definitions follow reverse address order for deferred inlining. */
#include "Game/PlayerMotion.h"
#include "Game/EffectElement.h"
#include "Game/Player.h"
#include "Game/PlayerAttack.h"
#include "Game/PlayerEffect.h"
#include "Game/PlayerMotionEventArgs.h"
#include "Game/PlayerWeaponBase.h"
#include "Game/SoundBase.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_80411714_resource_groups.h"
#include "Game/fn_804157E8_motion_control.h"
#include <cstring>

namespace Player {

void UnkMotionRecord_800AAB74::fn_800AAB78() {
    if (field_0x35) {
        if (field_0x0 != 0) {
            Fn8040Vector3 position(fn_8040FB08(field_0x0));
            field_0x10 = fn_8040B618(*field_0x0, Fn8040Vector3(0.0f, 0.0f, -1.0f));
            field_0x1C = fn_8040B618(*field_0x0, Fn8040Vector3(0.0f, 1.0f, 0.0f));
            field_0x28 = position - field_0x4;
            field_0x4 = position;
        } else {
            field_0x10 = Fn8040Vector3(0.0f, 0.0f, -1.0f);
            field_0x1C = Fn8040Vector3(0.0f, 1.0f, 0.0f);
            fn_8040CE60(&field_0x28);
            fn_8040CE60(&field_0x4);
        }
    }
}

UnkMotionRecord_800AAB74::UnkMotionRecord_800AAB74() {}

PlayerMotion::PlayerMotion(Task *parent, const boost::shared_ptr<PlayerBase> &owner)
    : Task(parent), m_owner(owner) {
    m_owner->field_0x230 = this;
    m_name = "Motion";
    field_0x60.Clear();
    field_0x3E0 = 0;
    field_0x3E8 = 0;
    field_0x3F0 = 0;
    field_0x450 = 0;
    std::memset(field_0xD0, 0, sizeof(field_0xD0));
    for (int i = 0; i < 2; ++i) {
        field_0x420[i] = -1;
    }
    field_0x428 = 0x30;
    field_0x42C = 0x30;
    field_0x430 = 0.0f;
    field_0x434 = -1.0f;
    field_0x438 = 0.0f;
    field_0x43C = 1.0f;
    for (int i = 0; i < 2; ++i) {
        field_0x440[i] = -1;
    }
    field_0x448 = 0.0f;
}

PlayerMotion::~PlayerMotion() {
    m_owner->field_0x230 = 0;
    for (int i = 0; i < 6; ++i) {
        if (!field_0x28[i].expired()) {
            field_0x28[i].lock()->ClearFlags(::Effect::System::ELEMENT_ACTIVE);
        }
    }
}

void PlayerMotion::Update(float dt) {
    float step = *m_owner->field_0x60;
    float value = fn_8006C0A8(1);
    if (field_0x60.Test(0)) {
        value += 60.0f * (0.333f * step);
    } else {
        value -= 60.0f * (0.0625f * step);
    }
    value = fn_80070460(value, 0.0f, 1.0f);
    fn_8006C0A4(1, value);

    bool state = m_owner->fn_80073A20();
    static const int indices[] = {0, 3, 4};
    for (unsigned int i = 0; i < 3; ++i) {
        boost::shared_ptr< ::Effect::System::EffectElement> effect(field_0x28[indices[i]].lock());
        if (effect) {
            if (state) {
                effect->SetFlags(::Effect::System::ELEMENT_FLAG_8);
            } else {
                effect->ClearFlags(::Effect::System::ELEMENT_FLAG_8);
            }
        }
    }
}

void PlayerMotion::fn_800AA7C0(float value) {
    field_0x434 = value;
}

void PlayerMotion::fn_800AA7B8(float value) {
    field_0x438 = value;
}

int PlayerMotion::fn_800AA7B0() {
    return field_0x42C;
}

UnkMotionRecord_800AAB74 *PlayerMotion::fn_800AA760(int index) {
    UnkMotionRecord_800AAB74 *record = &field_0xD0[index];
    if (!record->field_0x35) {
        record->field_0x35 = true;
        record->fn_800AAB78();
    }
    return record;
}

_stMtnPlayWork *PlayerMotion::fn_800AA758() {
    return field_0x3E8;
}

void PlayerMotion::fn_800AA748(_stMtnPlayWork *&first, _stMtnPlayWork *&second) {
    first = 0;
    second = 0;
}

int PlayerMotion::fn_800AA740() {
    return 0;
}

void PlayerMotion::fn_800AA700(bool enabled) {
    if (enabled) {
        field_0x60.Set(1);
    } else {
        field_0x60.Reset(1);
    }
}

void PlayerMotion::fn_800AA6C0(bool enabled) {
    if (enabled) {
        field_0x60.Set(0);
    } else {
        field_0x60.Reset(0);
    }
}

bool PlayerMotion::fn_800AA640() {
    float value = fn_8006C0A8(1);
    bool result = false;
    if (field_0x60.Test(0) && value >= 1.0f) {
        result = true;
    }
    return result;
}

void PlayerMotion::fn_800AA638(int value) {
    field_0x3F0 = value;
}

void PlayerMotion::fn_800AA5D4() {
    if (m_owner->field_0x238 != 0 && m_owner->field_0x238->UnkSlot17()) {
        field_0x3F4.field_0x24 = 3;
    } else {
        field_0x3F4.field_0x24 = field_0x3F0;
    }
}

void PlayerMotion::fn_800AA58C(float value) {
    value = fn_80070460(value, 0.0f, 1.0f);
    field_0x3F4.field_0x0.alpha = value;
}

Unk801A35ACParameters *PlayerMotion::fn_800AA584() {
    return &field_0x3F4;
}

void PlayerMotion::fn_800AA404() {
    if (field_0x3E8 != 0) {
        bool changed = false;
        for (int i = 0; i < 2; ++i) {
            if (field_0x440[i] != field_0x420[i]) {
                changed = true;
            }
        }
        if (changed) {
            for (int i = 0; i < 2; ++i) {
                field_0x440[i] = -1;
            }
            field_0x440[0] = field_0x420[0];
            stMtnRequestFrameSpeed(field_0x3E8, 0, 0.0f);
            if (field_0x430 > 1.0f) {
                stMtnPrepareBlendWithTarget(field_0x3E8, 0, -1, -1);
                stMtnRequestBlendFrame(field_0x3E8, 0, field_0x430);
            } else {
                stMtnAbortBlend(field_0x3E8, 0);
            }
            if (field_0x440[0] >= 0) {
                fn_80417568(field_0x3E8, static_cast<unsigned short>(field_0x440[0]));
            }
            stMtnControlWorkFlag(field_0x3E8, 0, 11, 0);
            stMtnRequestBlendRatio(field_0x3E8, 0, 1.0f);
        }
        if (field_0x440[1] != field_0x420[1] &&
            stMtnPrepareBlendWithTarget(field_0x3E8, 1, -1, -1)) {
            field_0x440[1] = field_0x420[1];
            if (field_0x440[1] >= 0) {
                fn_80417568(field_0x3E8, static_cast<unsigned short>(field_0x440[1]));
            }
        }
        stMtnRequestBlendRatio(field_0x3E8, 0, field_0x448);
    }
}

void PlayerMotion::fn_800AA3A4() {
    field_0x440[0] = field_0x420[0];
    field_0x440[1] = field_0x420[1];
    stMtnRequestMotion(field_0x3E8, 0, static_cast<unsigned short>(field_0x440[1]));
    stMtnRequestMotion(field_0x3E8, 1, static_cast<unsigned short>(field_0x440[0]));
}

} // namespace Player

extern "C" bool fn_800AA390(const Player::MotionEventCallback *callback) {
    return !callback->empty();
}

extern "C" bool fn_800AA370(const Player::MotionEventCallback *callback) {
    return fn_800AA390(callback);
}

extern "C" void fn_800AA344(Player::MotionEventCallback *callback, Player::MotionEventArgs &event) {
    if (!callback->empty()) {
        (*callback)(event);
    }
}

/* The accessed prefix of the payload of motion event 0xD000; field names remain unknown. */
struct UnkMotionPayload_800AA0D4 {
    /* 0x0 */ int field_0x0;
    /* 0x4 */ int field_0x4;
    /* 0x8 */ u8 field_0x8[4]; /* Unrecovered storage. */
    /* 0xC */ float field_0xC;
};
CHECK_SIZEOF(UnkMotionPayload_800AA0D4, 0x10);

namespace Player {

void PlayerMotion::fn_800AA0D4() {
    _stMtnPlayWork *control = field_0x3E8;
    const UnkMotionPayload_800AA0D4 *attackEvent = 0;
    bool eventD001 = false;
    unsigned short eventId;
    void *payload;
    while (fn_80415854(control, 0, &eventId, &payload, 0)) {
        control = 0;
        if (fn_800AA370(&m_owner->field_0x74)) {
            MotionEventArgs event(eventId, payload);
            fn_800AA344(&m_owner->field_0x74, event);
        }
        switch (eventId) {
        case 0x5000: {
            int arg0 = *static_cast<int *>(payload);
            switch (arg0) {
            case 4:
            case 5:
            case 6:
                fn_800A29C0(m_owner->field_0x23C, arg0);
                break;
            }
            break;
        }
        case 0x5001: {
            int *args = static_cast<int *>(payload);
            m_owner->field_0x240->fn_80305CE4(args[0], args[1]);
            break;
        }
        case 0xD000:
            attackEvent = static_cast<UnkMotionPayload_800AA0D4 *>(payload);
            break;
        case 0xD001:
            eventD001 = true;
            break;
        case 0xD004:
            fn_800A2978(m_owner->field_0x23C, payload);
            break;
        }
    }
    if (m_owner->field_0x234 != 0 && !fn_8006CD78(m_owner->field_0xA8, 0x35)) {
        if (attackEvent != 0) {
            fn_80077974(m_owner->field_0x234, attackEvent->field_0x0, attackEvent->field_0x4,
                        attackEvent->field_0xC);
        } else {
            fn_8007792C(m_owner->field_0x234);
        }
    }
    if (fn_8006CD78(m_owner->field_0xA8, 0x1F) && m_owner->field_0x238->UnkSlot6() == 5) {
        if (eventD001) {
            m_owner->field_0x238->UnkSlot8();
        } else {
            m_owner->field_0x238->UnkSlot10();
        }
    }
    for (int i = 0; i < 16; ++i) {
        if (stMtnControlWorkFlag(field_0x3E8, 0, i, 1)) {
            m_owner->field_0xB4.Set(i);
        } else {
            m_owner->field_0xB4.Reset(i);
        }
    }
}

float PlayerMotion::fn_800AA064() {
    MotionRange range;
    fn_80415D40(field_0x3E8, 0, &range);
    float length = range.endFrame - range.startFrame;
    if (fn_8000A950(length)) {
        return 0.0f;
    }
    return (range.currentFrame - range.startFrame) / length;
}

void PlayerMotion::fn_800A9F9C(float fraction) {
    MotionRange range;
    if (field_0x440[1] >= 0) {
        for (int i = 0; i < 2; ++i) {
            fn_804125F4(&range, static_cast<unsigned short>(field_0x440[1 - i]));
            stMtnRequestCurrentFrame(
                field_0x3E8, i, fraction * (range.endFrame - range.startFrame) + range.startFrame);
        }
    } else if (fn_804125F4(&range, static_cast<unsigned short>(field_0x440[0]))) {
        stMtnRequestCurrentFrame(field_0x3E8, 0,
                                 fraction * (range.endFrame - range.startFrame) + range.startFrame);
    }
}

float PlayerMotion::fn_800A9EF8() {
    float speed;
    if (field_0x440[1] < 0) {
        speed = field_0x43C;
    } else {
        /* A failed lookup leaves its range untouched; both original calls are unchecked. */
        MotionRange range0;
        MotionRange range1;
        fn_804125F4(&range1, static_cast<unsigned short>(field_0x440[1]));
        fn_804125F4(&range0, static_cast<unsigned short>(field_0x440[0]));
        float length0 = range0.endFrame - range0.startFrame;
        float length1 = range1.endFrame - range1.startFrame;
        float ratio = length0 / length1;
        speed = field_0x43C / ((ratio - 1.0f) * (1.0f - field_0x448) + 1.0f);
    }
    return speed * m_owner->field_0x60[1] + 0.000001f;
}

void PlayerMotion::fn_800A9EA0() {
    for (int i = 0; i < 14; ++i) {
        UnkMotionRecord_800AAB74 &record = field_0xD0[i];
        if (record.field_0x34) {
            record.fn_800AAB78();
        }
    }
}

void PlayerMotion::fn_800A9E48() {
    for (int i = 0; i < 14; ++i) {
        UnkMotionRecord_800AAB74 &record = field_0xD0[i];
        if (!record.field_0x34) {
            record.fn_800AAB78();
        }
    }
}

} // namespace Player

extern "C" bool fn_800A9E1C(const boost::shared_ptr<Player::PlayerBase> &player) {
    return player->field_0xB4.Test(3);
}

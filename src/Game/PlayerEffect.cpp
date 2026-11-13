/* Game/PlayerEffect.cpp (file name from the type names of its classes,
 * "@unnamed@PlayerEffect_cpp@::RangeEffect" and the others): Player::Effect, the implementation
 * object that it owns and the effect tasks that the implementation creates. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). The task class
 * names come from RTTI; the names of the members (m_*), of the file's tables (s_*), of
 * CameraDemo::OnSwitch and OnUpdate and of Awake::End and Belt::Stop are inferred from their use. */
#include "Game/PlayerEffect.h"
#include "Game/CameraModeUser.h"
#include "Game/EffectElement.h"
#include "Game/EffectManagerPJS.h"
#include "Game/ExternalCameraRequest.h"
#include "Game/Player.h"
#include "Game/PlayerBarrier.h"
#include "Game/PlayerBase.h"
#include "Game/PlayerMotion.h"
#include "Game/PlayerShadowElectric.h"
#include "Game/PlayerShadowSuper.h"
#include "Game/SetClumpWorld.h"
#include "Game/AngleConversion.h"
#include "Game/SoundBase.h"
#include "Game/TestEffBelt.h"
#include "Game/UnkEffectFileData.h"
#include "Game/UnkEffectRW3Parameters.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80078C88.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkManager_80079808.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_80015A7C.h"
#include "Game/fn_80038E84.h"
#include "Game/fn_802285A0.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_8040_vector_types.h"
#include "Game/fn_8003A840.h"
#include "Game/fn_80169350.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "layout_check.h"
#include <cstdio>
#include <cstdlib>

using Effect::PJS::EffectManagerPJS;
using Effect::System::EffectElement;
using Player::PlayerBase;
using Player::PlayerMotion;
using Player::UnkMotionRecord_800AAB74;
using SonicteamUSA::System::Camera::CameraMode;
using SonicteamUSA::System::Camera::CameraModeUser;

namespace {

/* The camera motion of CameraDemo, by its type. */
const int s_cameraDemoMotions[3] = {0x1C, 0x1D, 0x1D};
/* The motion records whose matrices place Jet's four models. */
const int s_jetRecords[4] = {7, 8, 9, 10};
/* The defaults that fn_800A26F0 restores in s_effects. */
const UnkEffectEntry_805217E0 s_defaultEffects[16] = {
    {0x20, 0x56, -1, 0x7004}, {-1, -1, -1, 0x7005},     {-1, -1, -1, 0x7006},
    {0x22, 0x35, -1, 0x7007}, {0x21, 0x2E, -1, 0x7008}, {0x21, 0x2E, -1, 0x7009},
    {0x20, 0x32, -1, 0x700A}, {-1, -1, -1, 0x700B},     {-1, -1, -1, 0x700C},
    {0x67, 0x64, -1, 0x700D}, {0x2B, 0x2F, -1, 0x700E}, {-1, 0x59, -1, 0x700F},
    {-1, 0x1A6, -1, 0x7010},  {-1, -1, -1, 0x7011},     {-1, -1, -1, 0x7012},
    {-1, -1, -1, -1},
};

/* RangeEffect's three effects: the motion record that places each and its effect id (placeholder
 * name). */
struct UnkRangeEffectEntry_804B26CC {
    /* 0x0 */ int record;
    /* 0x4 */ int id;
};
CHECK_SIZEOF(UnkRangeEffectEntry_804B26CC, 0x8);
const UnkRangeEffectEntry_804B26CC s_rangeEffects[3] = {{6, 0x38}, {4, 0x38}, {8, 0x1E}};

/* The table that the effects use, indexed by the type at 0x1C of the player's record at 0x278. */
UnkEffectEntry_805217E0 s_effects[16] = {
    {0x20, 0x56, -1, 0x7004}, {-1, -1, -1, 0x7005},     {-1, -1, -1, 0x7006},
    {0x22, 0x35, -1, 0x7007}, {0x21, 0x2E, -1, 0x7008}, {0x21, 0x2E, -1, 0x7009},
    {0x20, 0x32, -1, 0x700A}, {-1, -1, -1, 0x700B},     {-1, -1, -1, 0x700C},
    {0x67, 0x64, -1, 0x700D}, {0x2B, 0x2F, -1, 0x700E}, {-1, 0x59, -1, 0x700F},
    {-1, 0x1A6, -1, 0x7010},  {-1, -1, -1, 0x7011},     {-1, -1, -1, 0x7012},
    {-1, -1, -1, -1},
};

/* Three effects that follow motion records while their flag is set (placeholder names). */
class RangeEffect : public Task {
  public:
    struct Entry {
        /* 0x0 */ boost::weak_ptr<EffectElement> m_effect;
        /* 0x8 */ bool m_active;
    };

    RangeEffect(Task *parent, const boost::shared_ptr<PlayerBase> &player, RangeEffect **slot);
    virtual ~RangeEffect();
    virtual void Update(float dt);

    void fn_800A3AA4(const int *index);
    void fn_800A39C0(int index);

    /* 0x28 */ boost::shared_ptr<PlayerBase> m_player;
    /* 0x30 */ RangeEffect **m_slot;
    /* 0x34 */ Entry m_entries[3];
};
CHECK_SIZEOF(RangeEffect, 0x58);

class Footmark : public Task {
  public:
    Footmark(Task *parent, const boost::shared_ptr<PlayerBase> &player, Footmark **slot);
    virtual ~Footmark();
    virtual void Update(float dt);

    int fn_800A3F18();

    /* 0x28 */ boost::shared_ptr<PlayerBase> m_player;
    /* 0x30 */ Footmark **m_slot;
    /* 0x34 */ boost::weak_ptr<EffectElement> m_effect;
};
CHECK_SIZEOF(Footmark, 0x40);

class Slow : public Task {
  public:
    Slow(Task *parent, const boost::shared_ptr<PlayerBase> &player, Slow **slot);
    virtual ~Slow();
    virtual void Update(float dt);

    void fn_800A43C8();

    /* 0x28 */ boost::shared_ptr<PlayerBase> m_player;
    /* 0x30 */ Slow **m_slot;
    /* 0x34 */ bool m_ending;
    /* 0x38 */ float m_level;
};
CHECK_SIZEOF(Slow, 0x40);

/* One entry of Awake's parameters, by its type (placeholder name). */
struct UnkAwakeParams_804B2904 {
    /* 0x00 */ int field_0x0;
    /* 0x04 */ int field_0x4;
    /* 0x08 */ int field_0x8;
    /* 0x0C */ int field_0xC;
    /* 0x10 */ int field_0x10;
    /* 0x14 */ int field_0x14;
    /* 0x18 */ int field_0x18;
    /* 0x1C */ const char *field_0x1C;
};
CHECK_SIZEOF(UnkAwakeParams_804B2904, 0x20);

class Awake : public Task {
  public:
    Awake(Task *parent, const boost::shared_ptr<PlayerBase> &player, Awake **slot, int type);
    virtual ~Awake();
    virtual void Update(float dt);

    void fn_800A48F8();
    void fn_800A48AC();
    void fn_800A481C();
    void fn_800A47D0();

    void End() {
        if (m_state < 500) {
            m_state = 500;
        }
        if (m_slot != 0) {
            *m_slot = 0;
        }
        m_slot = 0;
    }

    /* 0x28 */ boost::shared_ptr<PlayerBase> m_player;
    /* 0x30 */ Unk80015A7C *m_30;
    /* 0x34 */ Awake **m_slot;
    /* 0x38 */ float m_level;
    /* 0x3C */ int m_state;
    /* 0x40 */ int m_type;
    /* 0x44 */ float m_time;
    /* 0x48 */ bool m_48;
};
CHECK_SIZEOF(Awake, 0x50);

class Grind : public Task {
  public:
    Grind(Task *parent, const boost::shared_ptr<PlayerBase> &player, Grind **slot);
    virtual ~Grind();
    virtual void Update(float dt);

    void fn_800A519C();
    void fn_800A5148(bool start);
    void fn_800A5048();

    /* 0x28 */ boost::shared_ptr<PlayerBase> m_player;
    /* 0x30 */ Grind **m_slot;
    /* 0x34 */ bool m_playing;
    /* 0x38 */ UnkSoundHandle_801D3DC8 m_sound;
};
CHECK_SIZEOF(Grind, 0x58);

class Jet : public Task {
  public:
    Jet(Task *parent, const boost::shared_ptr<PlayerBase> &player, Jet **slot, bool type);
    virtual ~Jet();
    virtual void Update(float dt);

    void fn_800A53D0();
    void fn_800A530C();

    /* 0x28 */ boost::shared_ptr<PlayerBase> m_player;
    /* 0x30 */ Jet **m_slot;
    /* 0x34 */ int m_state;
    /* 0x38 */ bool m_visible;
    /* 0x39 */ bool m_type;
    /* 0x3C */ UnkSoundHandle_801D3DC8 m_sound;
    /* 0x58 */ float m_level;
    /* 0x5C */ RpClump *m_clumps[4];
    /* 0x6C */ RwFrame *m_frames[4];
    /* 0x7C */ float m_angles[4];
};
CHECK_SIZEOF(Jet, 0x90);

class Belt : public Task {
  public:
    Belt(Task *parent, const boost::shared_ptr<PlayerBase> &player, float arg2);
    virtual ~Belt();
    virtual void Update(float dt);

    void Stop() {
        if (m_belt != 0) {
            m_belt->fn_802DDC94();
            m_belt = 0;
        }
    }

    /* 0x28 */ Test::EffBelt *m_belt;
    /* 0x2C */ boost::shared_ptr<PlayerBase> m_player;
};
CHECK_SIZEOF(Belt, 0x38);

class CameraDemo : public Task {
  public:
    CameraDemo(Task *parent, const boost::shared_ptr<PlayerBase> &player, int type);
    virtual ~CameraDemo();
    virtual void Update(float dt);

    void fn_800A6DF4();
    void OnSwitch(CameraModeUser::SwitchEventArgs &args);
    void OnUpdate(CameraModeUser::UpdateEventArgs &args);
    void fn_800A6D4C();
    void fn_800A65AC();
    void fn_800A5F4C();
    void fn_800A5E8C();
    void fn_800A5C74();

    /* 0x28 */ boost::shared_ptr<PlayerBase> m_player;
    /* 0x30 */ int m_type;
    /* 0x34 */ UnkCameraMotion_80012510 *m_motion;
    /* 0x38 */ float m_time;
    /* 0x3C */ float m_speed;
    /* 0x40 */ int m_state;
    /* 0x44 */ boost::weak_ptr<PJSCamera::ExternalCameraRequest> m_request;
    /* 0x4C */ boost::shared_ptr<CameraModeUser> m_cameraMode;
};
CHECK_SIZEOF(CameraDemo, 0x58);

} // namespace

namespace Player {

class EffectImpl {
  public:
    EffectImpl(const boost::shared_ptr<PlayerBase> &player);
    ~EffectImpl();

    void fn_800A37CC(Effect *effect);
    Task *fn_800A3774(Effect *effect, int type);
    void fn_800A3708(Effect *effect, float arg1);
    void fn_800A36B4();
    void fn_800A3640(Effect *effect);
    void fn_800A361C();
    void fn_800A35A8(Effect *effect);
    void fn_800A3584();
    void fn_800A352C(Effect *effect);
    void fn_800A3500();
    void fn_800A34D4(bool start);
    void fn_800A3448(Effect *effect);
    void fn_800A33BC(Effect *effect);
    void fn_800A332C();
    void fn_800A3140();
    void fn_800A30E0(Effect *effect);
    void fn_800A30C8();
    void fn_800A2FC8(int type);
    void fn_800A2F74(int index, const Fn8040Vector3 *position, const Fn8040Vector3 *vector);
    void fn_800A2F48(void *payload);

    void fn_800A2B78(Effect *effect) {
        if (m_super == 0) {
            new Shadow::Super(effect, m_player, &m_super);
        }
    }
    void fn_800A2B44() {
        if (m_super != 0) {
            m_super->fn_8027E0D8();
        }
    }
    void fn_800A2AD8(Effect *effect) {
        if (m_electric == 0) {
            new Shadow::Electric(effect, m_player, &m_electric);
        }
        m_electricCount++;
    }
    void fn_800A2A78() {
        m_electricCount--;
        if (m_electricCount < 0) {
            m_electricCount = 0;
        }
        if (m_electricCount == 0) {
            if (m_electric != 0) {
                m_electric->fn_8028D834();
            }
        }
    }
    void fn_800A2920(int arg1) {
        if (m_barrier != 0) {
            switch (arg1) {
            case 1:
            case 2:
            case 3:
            case 4:
                m_barrier->fn_8033DD04(arg1);
                break;
            }
        }
    }
    void fn_800A28EC() {
        if (m_barrier != 0) {
            m_barrier->fn_8033DCE0();
        }
    }
    void fn_800A28B0() {
        if (m_barrier != 0) {
            m_barrier->fn_8033DCBC();
        }
    }

    /* 0x00 */ boost::shared_ptr<PlayerBase> m_player;
    /* 0x08 */ Belt *m_belt;
    /* 0x0C */ Jet *m_jet1;
    /* 0x10 */ Jet *m_jet0;
    /* 0x14 */ Grind *m_grind;
    /* 0x18 */ Awake *m_awake;
    /* 0x1C */ Shadow::Super *m_super;
    /* 0x20 */ int m_electricCount;
    /* 0x24 */ Shadow::Electric *m_electric;
    /* 0x28 */ boost::weak_ptr<EffectElement> m_effect;
    /* 0x30 */ Slow *m_slow;
    /* 0x34 */ Footmark *m_footmark;
    /* 0x38 */ RangeEffect *m_range;
    /* 0x3C */ Barrier *m_barrier;
};
CHECK_SIZEOF(EffectImpl, 0x40);

} // namespace Player

using Player::EffectImpl;

namespace {

CameraDemo::CameraDemo(Task *parent, const boost::shared_ptr<PlayerBase> &player, int type)
    : Task(parent), m_player(player), m_type(type) {
    m_name = "CameraDemo";
    m_motion = 0;
    UnkResourceManager_800122D4 *resources = fn_800122D4();
    m_motion = static_cast<UnkCameraMotion_80012510 *>(
        fn_80012510(resources, fn_8027AB1C(fn_80078C88(), 0xC), s_cameraDemoMotions[m_type]));
    fn_800A6D4C();
    fn_80078F2C(m_player.get(), m_player->field_0x54, 1);
}

CameraDemo::~CameraDemo() {
    fn_801CEAD4(m_request);
    fn_802039D0(&lbl_8057D6A0.field_0xA8, 0);
    fn_80078F2C(m_player.get(), m_player->field_0x54, 0);
}

void CameraDemo::Update(float dt) {
    m_speed = 60.0f * dt * m_player->field_0x60[1];
    switch (m_state) {
    case 0:
        if (fn_80011B7C(m_motion, &m_time, &m_speed) != 0) {
            if (m_type == 1) {
                fn_800A5C74();
                m_state++;
            } else {
                m_flags |= 1;
            }
        }
        break;
    }
    if (!fn_8006CD78(m_player->field_0xA8, 0x16) && !fn_8006CD78(m_player->field_0xA8, 0x14)) {
        m_flags |= 1;
    }
}

void CameraDemo::fn_800A6DF4() {
    if (m_motion != 0) {
        Fn8040Matrix34 matrix = fn_8040B034(m_player->m_position);
        fn_8040BAC4(&matrix, &m_player->field_0x1E4);
        fn_8040B848(&matrix, m_player->m_rotation.y);
        fn_8040B884(&matrix, m_player->m_rotation.x);
        fn_8040B80C(&matrix, m_player->m_rotation.z);
        fn_8040B52C(&matrix, 0.0f, -m_player->field_0x274, 0.0f);
        fn_8000FBFC(m_cameraMode, m_motion, &matrix, m_time);
    }
}

void CameraDemo::OnSwitch(CameraModeUser::SwitchEventArgs &) {
    fn_800A6DF4();
}

void CameraDemo::OnUpdate(CameraModeUser::UpdateEventArgs &) {
    fn_800A6DF4();
}

void CameraDemo::fn_800A6D4C() {
    m_time = 0.0f;
    m_state = 0;
    switch (m_type) {
    case 0:
        fn_800A5F4C();
        break;
    case 1:
        fn_800A5E8C();
        break;
    }
    fn_802039D0(&lbl_8057D6A0.field_0xA8, 1);
}

void CameraDemo::fn_800A65AC() {
    m_cameraMode = fn_80009704(0, 0);
    if (m_cameraMode.get() != 0) {
        m_cameraMode->field_0xAC =
            CameraSwitchCallback(boost::bind(&CameraDemo::OnSwitch, this, _1));
        m_cameraMode->m_callback =
            CameraUpdateCallback(boost::bind(&CameraDemo::OnUpdate, this, _1));
    }
    if (m_motion != 0) {
        m_time = m_motion->field_0x4;
    }
}

void CameraDemo::fn_800A5F4C() {
    fn_800A65AC();
    UnkCameraParams_8000B31C params(0.25f, 0.0f);
    m_request = fn_801CED24(m_cameraMode, 0, 0, 0, 1, &params, 1);
}

void CameraDemo::fn_800A5E8C() {
    fn_800A65AC();
    m_request = fn_801CED24(m_cameraMode, 0, 0, 0, -1, 0, 0);
}

void CameraDemo::fn_800A5C74() {
    fn_801CEAD4(m_request);
    fn_802039D0(&lbl_8057D6A0.field_0xA8, 0);
    /* The angle, the six floats and the two parameter pairs are camera settings whose meanings
     * are unknown, as are fn_8026F6A4's 0xB and 0. */
    UnkCameraModeParams_8000EBB0 params;
    params.SetField0x1C(fn_8000F2E8(DegreesToRadians(140.0f)));
    params.field_0x4C[0] = 200.0f;
    params.field_0x4C[1] = 15.0f;
    params.field_0x4C[2] = 10.0f;
    params.field_0x4C[3] = 360.0f;
    params.field_0x4C[4] = 60.0f;
    params.field_0x4C[5] = 180.0f;
    UnkCameraParams_8000B31C first(1.0f, 2.0f);
    UnkCameraParams_8000B31C second(0.0f, 0.5f);
    boost::shared_ptr<CameraMode> mode = fn_8026F6A4(0xB, &params, 0);
    m_request = fn_801CED24(mode, 0, 2, &first, 3, &second, 0);
}

Belt::Belt(Task *parent, const boost::shared_ptr<PlayerBase> &player, float arg2)
    : Task(parent), m_player(player) {
    m_name = "Belt";
    const UnkCharacterRecord_804F1834 *character = &lbl_804F1834[m_player->field_0x64];
    /* The meaning of 0x3C is unknown (see Test::EffBelt). */
    m_belt = new Test::EffBelt(
        m_player, 0x3C,
        UnkDebugColor_802E0E84(character->field_0x20, character->field_0x24, character->field_0x28),
        arg2);

    PlayerMotion *motion = m_player->field_0x230;
    if (motion != 0) {
        boost::weak_ptr<EffectElement> &effect = motion->field_0x28[3];
        if (!effect.expired()) {
            effect.lock()->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }
        effect = fn_800A2854(m_player->field_0x23C, 0, character->field_0x1C);
        if (m_player->fn_80073A20()) {
            boost::shared_ptr<EffectElement> element = effect.lock();
            if (element.get() != 0) {
                element->SetFlags(Effect::System::ELEMENT_FLAG_8);
            }
        }
    }
}

Belt::~Belt() {
    if (m_belt != 0) {
        m_belt->m_flags |= 1;
    }
    PlayerMotion *motion = m_player->field_0x230;
    if (motion != 0) {
        boost::weak_ptr<EffectElement> &effect = motion->field_0x28[3];
        if (!effect.expired()) {
            effect.lock()->SetFlags(Effect::System::ELEMENT_STOP_REQUESTED);
        }
    }
}

void Belt::Update(float) {
    if (m_belt == 0 || !fn_8004EAC0(m_belt)) {
        m_flags |= 1;
    } else {
        m_belt->fn_802DDCC8(m_player.get());
    }
}

Jet::Jet(Task *parent, const boost::shared_ptr<PlayerBase> &player, Jet **slot, bool type)
    : Task(parent), m_player(player), m_slot(slot), m_type(type) {
    m_name = "Jet";
    if (m_slot != 0) {
        *m_slot = this;
    }
    m_state = 0;
    m_visible = false;
    m_level = 0.0f;

    UnkResourceManager_800122D4 *resources = fn_800122D4();
    UnkManager_8003A840 *worlds = fn_8003A840();
    m_clumps[0] = static_cast<RpClump *>(
        fn_80012700(resources, fn_8027AB1C(fn_80078C88(), 0xC), "SHADOW_FOOTEFF_R.DFF"));
    m_clumps[1] = 0;
    m_clumps[2] = 0;
    m_clumps[3] = 0;
    if (m_clumps[0] != 0) {
        m_clumps[1] = fn_80457090(m_clumps[0]);
        m_clumps[2] = fn_80457090(m_clumps[0]);
        m_clumps[0] = fn_80457090(m_clumps[0]);
    }
    for (int i = 0; i < 4; i++) {
        if (m_clumps[i] != 0) {
            int index = fn_80169350(fn_80169364(m_clumps[i]));
            SetClumpWorld(fn_800593D0(worlds, index), m_clumps[i]);
            m_frames[i] = fn_804872D8(static_cast<RwFrame *>(m_clumps[i]->object.parent));
        }
        m_angles[i] = 0.0f;
    }
}

Jet::~Jet() {
    if (m_slot != 0) {
        *m_slot = 0;
    }
    for (int i = 0; i < 4; i++) {
        if (m_clumps[i] != 0) {
            RpWorld *world = fn_8046256C(m_clumps[i]);
            if (world != 0) {
                fn_804623EC(world, m_clumps[i]);
            }
            fn_80457468(m_clumps[i]);
        }
    }
    m_sound.fn_801D455C();
}

void Jet::Update(float) {
    float speed = *m_player->field_0x60;
    switch (m_state) {
    case 0:
        m_sound.fn_801D455C();
        if (m_type) {
            m_sound = m_player->field_0x240->fn_80305C50(6, this);
        }
        m_state++;
        break;
    case 1:
        m_level += 15.0f * speed;
        if (m_level >= 1.0f) {
            m_level = 1.0f;
            m_state = 100;
        }
        break;
    case 100:
        break;
    case 500:
        m_sound.fn_801D455C();
        m_state++;
    case 501:
        m_level -= 4.0f * speed;
        if (m_level <= 0.0f) {
            m_level = 0.0f;
            m_flags |= 1;
            if (m_slot != 0) {
                *m_slot = 0;
            }
            m_slot = 0;
        }
        break;
    }
    fn_800A53D0();
    fn_800A530C();
}

void Jet::fn_800A53D0() {
    for (int i = 0; i < 4; i++) {
        if (m_clumps[i] != 0) {
            RwRGBAReal color;
            color.alpha = m_level;
            color.red = -1.0f;
            color.green = -1.0f;
            color.blue = -1.0f;
            fn_80039114(m_clumps[i], &color);
            m_angles[i] += fn_8040EC80(DegreesToRadians(40.0f), DegreesToRadians(50.0f));
            UnkMotionRecord_800AAB74 *record = m_player->field_0x230->fn_800AA760(s_jetRecords[i]);
            RwFrame *frame = m_frames[i];
            Fn8040Matrix34 rotation;
            fn_8040B138(&rotation, m_angles[i]);
            frame->modelling = fn_800591F0(fn_8001E044(record->field_0x0, &rotation));
            fn_8048722C(m_frames[i]);
        }
    }
}

void Jet::fn_800A530C() {
    if (m_player->fn_80073A20()) {
        if (!m_visible) {
            m_visible = true;
            for (int i = 0; i < 4; i++) {
                if (m_clumps[i] != 0) {
                    fn_80038FC0(m_clumps[i]);
                }
            }
        }
    } else if (m_visible) {
        m_visible = false;
        for (int i = 0; i < 4; i++) {
            if (m_clumps[i] != 0) {
                fn_80038F94(m_clumps[i]);
            }
        }
    }
}

Grind::Grind(Task *parent, const boost::shared_ptr<PlayerBase> &player, Grind **slot)
    : Task(parent), m_player(player), m_slot(slot) {
    m_name = "Grind";
    if (m_slot != 0) {
        *m_slot = this;
    }
    m_playing = false;
    fn_800A5048();
}

Grind::~Grind() {
    if (m_slot != 0) {
        *m_slot = 0;
    }
    if (m_playing) {
        m_sound.fn_801D455C();
        m_playing = false;
    }
}

void Grind::Update(float) {}

void Grind::fn_800A519C() {
    if (m_playing) {
        m_sound.fn_801D455C();
        m_playing = false;
    }
    m_flags |= 1;
}

void Grind::fn_800A5148(bool start) {
    if (start) {
        fn_800A5048();
    } else if (m_playing) {
        m_sound.fn_801D455C();
        m_playing = false;
    }
}

void Grind::fn_800A5048() {
    if (!m_playing) {
        /* For two values of this word (meanings unknown), the grind sound is 0x7503, started
         * after sound 0x7502, instead of the player's sound 0xB; the sound ids' meanings are
         * unknown too. */
        switch (lbl_8057D6A0.field_0xA8.field_0x0) {
        case 0x12E:
        case 0x194:
            fn_801D560C(fn_80053164(), 0x7502, 0, 0);
            m_sound = fn_801D3EF0(0x7503, this);
            break;
        default:
            m_player->field_0x240->fn_80305CA8(0xA);
            m_sound = m_player->field_0x240->fn_80305C50(0xB, this);
            break;
        }
        fn_8034DF48(0xA, m_player->field_0x54);
        m_playing = true;
    }
}

const UnkAwakeParams_804B2904 s_awakeParams[2] = {
    {0x23, 0x4B, 0x54, 0x1A, 3, 1, 0, "EFFSHADOWEVIL.DFF"},
    {0x22, 0x3F, 0x49, 0x19, 4, 0, 1, "EFFSHADOWHERO.DFF"},
};

Awake::Awake(Task *parent, const boost::shared_ptr<PlayerBase> &player, Awake **slot, int type)
    : Task(parent), m_player(player), m_slot(slot), m_type(type) {
    const UnkAwakeParams_804B2904 *params;
    m_name = "Awake";
    m_30 = 0;
    m_48 = false;
    _stMtnPlayWork *motionWork = 0;
    if (m_player->field_0x230 != 0) {
        motionWork = m_player->field_0x230->fn_800AA758();
    }
    m_level = 0.0f;
    m_time = 0.0f;
    if (m_slot != 0) {
        *m_slot = this;
    }

    params = &s_awakeParams[m_type];
    UnkResourceManager_800122D4 *resources = fn_800122D4();
    RpClump *clump = static_cast<RpClump *>(
        fn_80012700(resources, fn_8027AB1C(fn_80078C88(), 0xC), params->field_0x1C));
    if (clump != 0 && motionWork != 0) {
        m_30 = new Unk80015A7C(clump, motionWork);
    }

    fn_8007939C(m_player.get(), 1);
    fn_80079334(m_player.get(), 1);
    PlayerMotion *motion = m_player->field_0x230;
    if (motion != 0) {
        boost::weak_ptr<EffectElement> &effect = motion->field_0x28[4];
        if (!effect.expired()) {
            effect.lock()->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }
        effect = fn_800A2854(m_player->field_0x23C, 1, params->field_0x4);
        if (m_player->fn_80073A20()) {
            fn_800A48F8();
        }
    }
    fn_80076CA8(m_player->field_0xA8, params->field_0x0);
    m_player->field_0x240->fn_80305CA8(params->field_0xC);
    fn_801D3888(params->field_0x10);
    fn_8016F0B4(&fn_80079808()->field_0x70C, params->field_0x18);
    fn_8034DE74(8, m_player->field_0x54, 2.0f);
    m_state = 0;
}

Awake::~Awake() {
    fn_8007939C(m_player.get(), 0);
    fn_80079334(m_player.get(), 0);
    PlayerMotion *motion = m_player->field_0x230;
    if (motion != 0) {
        boost::weak_ptr<EffectElement> &effect = motion->field_0x28[4];
        if (!effect.expired()) {
            effect.lock()->SetFlags(Effect::System::ELEMENT_STOP_REQUESTED);
        }
    }
    const UnkAwakeParams_804B2904 *params = &s_awakeParams[m_type];
    fn_801D3838(params->field_0x10);
    fn_8016F094(&fn_80079808()->field_0x70C, params->field_0x18);
    fn_800A47D0();
    if (m_30 != 0) {
        delete m_30;
    }
    if (m_slot != 0) {
        *m_slot = 0;
    }
}

void Awake::Update(float) {
    const UnkAwakeParams_804B2904 *params = &s_awakeParams[m_type];
    float speed = *m_player->field_0x60;
    UnkManager_80079808 *manager = fn_80079808();
    if (!fn_8006CD78(m_player->field_0xA8, 0x16) && !fn_8006CD78(m_player->field_0xA8, 0x14) &&
        !fn_8006CD78(m_player->field_0xA8, 0x15)) {
        fn_8016F0CC(&manager->field_0x70C, params->field_0x14, -1500.0f * speed);
    }
    if (manager->field_0x70C.field_0x0[params->field_0x14][0] <= 0.0f) {
        fn_80076CC8(m_player->field_0xA8, params->field_0x0);
    }
    if (!fn_8006CD78(m_player->field_0xA8, params->field_0x0)) {
        if (m_state < 500) {
            m_state = 500;
        }
    }
    m_time += speed;
    switch (m_state) {
    case 0:
        if (m_time >= 1.5f) {
            PlayerMotion *motion = m_player->field_0x230;
            if (motion != 0) {
                boost::weak_ptr<EffectElement> &effect = motion->field_0x28[4];
                if (!effect.expired()) {
                    effect.lock()->ClearFlags(Effect::System::ELEMENT_ACTIVE);
                }
                effect = fn_800A2854(m_player->field_0x23C, 1, s_awakeParams[m_type].field_0x8);
                m_48 = true;
                fn_800A48AC();
            }
            fn_800A481C();
            m_state = 100;
        }
        break;
    case 100:
        m_level += 4.0f * speed;
        if (m_level >= 1.0f) {
            m_level = 1.0f;
            m_state++;
        }
        break;
    case 101:
        break;
    case 500:
        if (!fn_8006CD78(m_player->field_0xA8, 0x16) && !fn_8006CD78(m_player->field_0xA8, 0x14) &&
            !fn_8006CD78(m_player->field_0xA8, 0x15)) {
            m_state++;
        }
        break;
    case 501:
        m_level -= 2.0f * speed;
        if (m_level <= 0.0f) {
            m_level = 0.0f;
            m_flags |= 1;
        }
        break;
    }
    if (m_30 != 0) {
        RwRGBAReal color;
        color.alpha = m_level;
        color.red = -1.0f;
        color.green = -1.0f;
        color.blue = -1.0f;
        fn_80039114(m_30->field_0x0, &color);
        fn_80039B94(m_30->field_0x0, speed, -1.0f);
        m_30->fn_800159AC();
    }
    if (m_player->fn_80073A20()) {
        fn_800A48F8();
    } else {
        fn_800A48AC();
    }
}

void Awake::fn_800A48F8() {
    if (!m_48) {
        m_48 = true;
        if (m_30 != 0 && m_30->field_0x0 != 0) {
            fn_80038FC0(m_30->field_0x0);
        }
    }
}

void Awake::fn_800A48AC() {
    if (m_48) {
        m_48 = false;
        if (m_30 != 0 && m_30->field_0x0 != 0) {
            fn_80038F94(m_30->field_0x0);
        }
    }
}

void Awake::fn_800A481C() {
    if (m_30 != 0) {
        RpClump *clump = m_30->field_0x0;
        RpWorld *world = fn_800593D0(fn_8003A840(), fn_80169350(fn_80169364(clump)));
        if (world != 0) {
            SetClumpWorld(world, clump);
        }
    }
}

void Awake::fn_800A47D0() {
    if (m_30 != 0) {
        RpClump *clump = m_30->field_0x0;
        RpWorld *world = fn_8046256C(clump);
        if (world != 0) {
            fn_804623EC(world, clump);
        }
    }
}

Slow::Slow(Task *parent, const boost::shared_ptr<PlayerBase> &player, Slow **slot)
    : Task(parent), m_player(player), m_slot(slot) {
    m_name = "Slow";
    if (m_slot != 0) {
        *m_slot = this;
    }
    m_ending = false;
    m_level = 0.0f;
    fn_80076CA8(m_player->field_0xA8, 0x15);
    fn_802039C0(&lbl_8057D6A0.field_0xA8, 1);
    Effect::RW3::UnkEffectRW3Param2 parameters;
    parameters = EffectManagerPJS::Instance()->fn_804388A4();
    parameters.field_0x0 = 1;
    parameters.field_0x1 = 1;
    EffectManagerPJS::Instance()->fn_8043891C(parameters);
}

Slow::~Slow() {
    fn_80076CC8(m_player->field_0xA8, 0x15);
    fn_802039C0(&lbl_8057D6A0.field_0xA8, 0);
    lbl_8057D6A0.field_0x24 = lbl_8057D6A0.field_0x30 = 1.0f;
    fn_801D3838(2);
    Effect::RW3::UnkEffectRW3Param2 parameters;
    parameters = EffectManagerPJS::Instance()->fn_804388A4();
    parameters.field_0x0 = 0;
    parameters.field_0x1 = 0;
    EffectManagerPJS::Instance()->fn_8043891C(parameters);
    if (m_slot != 0) {
        *m_slot = 0;
    }
}

void Slow::Update(float dt) {
    if (m_ending) {
        m_level -= dt / 0.5f;
        if (m_level < 0.0f) {
            m_level = 0.0f;
        }
    } else {
        m_level += dt / 0.5f;
        if (m_level > 1.0f) {
            m_level = 1.0f;
        }
    }
    lbl_8057D6A0.field_0x24 = lbl_8057D6A0.field_0x30 = 0.125f * m_level;
    fn_800A43C8();
    fn_8016F0CC(&fn_80079808()->field_0x70C, 0, -2000.0f * *m_player->field_0x60);
    if (m_level <= 0.0f) {
        m_flags |= 1;
    }
}

void Slow::fn_800A43C8() {
    Effect::RW3::UnkEffectRW3Param1 parameters;
    parameters.field_0x0 = m_level <= 0.0f ? 0 : 1;
    parameters.field_0x1 = 64.0f * m_level;
    parameters.field_0x4 = 1.05f;
    EffectManagerPJS::Instance()->fn_8042E0C4(&parameters);
}

Footmark::Footmark(Task *parent, const boost::shared_ptr<PlayerBase> &player, Footmark **slot)
    : Task(parent), m_player(player), m_slot(slot) {
    m_name = "Footmark";
    if (m_slot != 0) {
        *m_slot = this;
    }
}

Footmark::~Footmark() {
    if (m_slot != 0) {
        *m_slot = 0;
    }
    if (!m_effect.expired()) {
        m_effect.lock()->RequestStop();
    }
}

void Footmark::Update(float) {
    boost::shared_ptr<EffectElement> element = m_effect.lock();
    if (fn_8006CD78(m_player->field_0xA8, 2) && fn_8006CD78(m_player->field_0xA8, 3)) {
        UnkMotionRecord_800AAB74 *right = m_player->field_0x230->fn_800AA760(0xA);
        UnkMotionRecord_800AAB74 *left = m_player->field_0x230->fn_800AA760(8);
        Fn8040Vector3 position;
        Fn8040Vector3 direction;
        position = (right->field_0x4 + left->field_0x4) * 0.5f;
        direction = fn_8040CEE8((right->field_0x1C + left->field_0x1C) * -0.5f);
        int id = fn_800A3F18();
        if (element.get() != 0 && id != element->m_definition->id) {
            element->RequestStop();
            element.reset();
            m_effect.reset();
        }
        if (element.get() == 0 && id >= 0) {
            m_effect = fn_800A27D4(m_player->field_0x23C, &position, &direction, id);
            element = m_effect.lock();
        }
        if (element.get() != 0) {
            Fn8040Vector2 angles = fn_8040F664(fn_80016284(direction));
            fn_80425E4C(element.get(), &position);
            fn_80425D94(element.get(), Fn8040Vector3(angles.x, angles.y, 0.0f));
            if (m_player->fn_80073A20()) {
                element->SetFlags(Effect::System::ELEMENT_FLAG_8);
            } else {
                element->ClearFlags(Effect::System::ELEMENT_FLAG_8);
            }
        }
    } else if (element.get() != 0) {
        element->RequestStop();
        m_effect.reset();
    }
}

int Footmark::fn_800A3F18() {
    return s_effects[m_player->field_0x278->field_0x1C].field_0x4;
}

RangeEffect::RangeEffect(Task *parent, const boost::shared_ptr<PlayerBase> &player,
                         RangeEffect **slot)
    : Task(parent), m_player(player), m_slot(slot) {
    m_name = "RangeEffect";
    if (m_slot != 0) {
        *m_slot = this;
    }
    for (int i = 0; i < 3; i++) {
        m_entries[i].m_active = false;
    }
}

RangeEffect::~RangeEffect() {
    if (m_slot != 0) {
        *m_slot = 0;
    }
    boost::shared_ptr<EffectElement> element;
    for (int i = 0; i < 3; i++) {
        element = m_entries[i].m_effect.lock();
        if (element.get() != 0) {
            element->RequestStop();
            m_entries[i].m_effect.reset();
            m_entries[i].m_active = false;
        }
    }
}

void RangeEffect::Update(float) {
    boost::shared_ptr<EffectElement> element;
    for (int i = 0; i < 3; i++) {
        element = m_entries[i].m_effect.lock();
        if (element.get() != 0) {
            if (!element->HasFlags(Effect::System::ELEMENT_ACTIVE)) {
                m_entries[i].m_effect.reset();
            } else if (!m_entries[i].m_active) {
                element->RequestStop();
                m_entries[i].m_effect.reset();
            } else {
                fn_800A39C0(i);
            }
            m_entries[i].m_active = false;
        }
    }
}

void RangeEffect::fn_800A3AA4(const int *index) {
    boost::shared_ptr<EffectElement> element = m_entries[*index].m_effect.lock();
    if (element.get() == 0) {
        m_entries[*index].m_effect = fn_800A2754(m_player->field_0x23C, s_rangeEffects[*index].id);
        fn_800A39C0(*index);
    }
    m_entries[*index].m_active = true;
}

void RangeEffect::fn_800A39C0(int index) {
    boost::shared_ptr<EffectElement> element = m_entries[index].m_effect.lock();
    if (element.get() != 0) {
        UnkMotionRecord_800AAB74 *record =
            m_player->field_0x230->fn_800AA760(s_rangeEffects[index].record);
        fn_80425E68(element.get(), *record->field_0x0);
        if (m_player->fn_80073A20()) {
            element->SetFlags(Effect::System::ELEMENT_FLAG_8);
        } else {
            element->ClearFlags(Effect::System::ELEMENT_FLAG_8);
        }
    }
}

} // namespace

namespace Player {

EffectImpl::EffectImpl(const boost::shared_ptr<PlayerBase> &player)
    : m_player(player), m_belt(0), m_jet1(0), m_jet0(0), m_grind(0), m_awake(0), m_super(0),
      m_electricCount(0), m_electric(0), m_slow(0), m_footmark(0), m_range(0), m_barrier(0) {}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if a destructor called from checked_delete turns out to be inlined in the
 * original. */
#pragma auto_inline on
EffectImpl::~EffectImpl() {
    if (m_jet1 != 0) {
        m_jet1->m_flags |= 1;
    }
    if (m_belt != 0) {
        m_belt->m_flags |= 1;
    }
    if (m_footmark != 0) {
        m_footmark->m_flags |= 1;
    }
    if (m_awake != 0) {
        m_awake->End();
    }
    if (m_slow != 0) {
        m_slow->m_ending = true;
    }
}
#pragma auto_inline reset

void EffectImpl::fn_800A37CC(Effect *effect) {
    new Footmark(effect, m_player, &m_footmark);
    new RangeEffect(effect, m_player, &m_range);
    m_barrier = new Barrier(effect, m_player);
}

Task *EffectImpl::fn_800A3774(Effect *effect, int type) {
    return new CameraDemo(effect, m_player, type);
}

void EffectImpl::fn_800A3708(Effect *effect, float arg1) {
    if (m_belt == 0) {
        m_belt = new Belt(effect, m_player, arg1);
    }
}

void EffectImpl::fn_800A36B4() {
    if (m_belt != 0) {
        m_belt->Stop();
        m_belt = 0;
    }
}

void EffectImpl::fn_800A3640(Effect *effect) {
    if (m_jet1 == 0) {
        new Jet(effect, m_player, &m_jet1, true);
    } else if (m_jet1->m_state > 100) {
        m_jet1->m_state = 0;
    }
}

void EffectImpl::fn_800A361C() {
    if (m_jet1 != 0 && m_jet1->m_state < 500) {
        m_jet1->m_state = 500;
    }
}

void EffectImpl::fn_800A35A8(Effect *effect) {
    if (m_jet0 == 0) {
        new Jet(effect, m_player, &m_jet0, false);
    } else if (m_jet0->m_state > 100) {
        m_jet0->m_state = 0;
    }
}

void EffectImpl::fn_800A3584() {
    if (m_jet0 != 0 && m_jet0->m_state < 500) {
        m_jet0->m_state = 500;
    }
}

void EffectImpl::fn_800A352C(Effect *effect) {
    if (m_grind == 0) {
        new Grind(effect, m_player, &m_grind);
    }
}

void EffectImpl::fn_800A3500() {
    if (m_grind != 0) {
        m_grind->fn_800A519C();
    }
}

void EffectImpl::fn_800A34D4(bool start) {
    if (m_grind != 0) {
        m_grind->fn_800A5148(start);
    }
}

void EffectImpl::fn_800A3448(Effect *effect) {
    if (m_awake != 0) {
        m_awake->End();
    }
    new Awake(effect, m_player, &m_awake, 0);
}

void EffectImpl::fn_800A33BC(Effect *effect) {
    if (m_awake != 0) {
        m_awake->End();
    }
    new Awake(effect, m_player, &m_awake, 1);
}

/* The meanings of the effect manager parameters here, in fn_800A3140 and in Slow::fn_800A43C8,
 * and of the effect id 0x51 are unknown. */
void EffectImpl::fn_800A332C() {
    ::Effect::RW3::UnkEffectRW3Param1 parameters;
    parameters.field_0x0 = 1;
    parameters.field_0x1 = 0x80;
    parameters.field_0x4 = 1.05f;
    EffectManagerPJS::Instance()->fn_8042E0C4(&parameters);
    m_effect = fn_800A2854(m_player->field_0x23C, 1, 0x51);
}

void EffectImpl::fn_800A3140() {
    ::Effect::RW3::UnkEffectRW3Param1 parameters;
    parameters.field_0x0 = 0;
    parameters.field_0x1 = 0x80;
    parameters.field_0x4 = 1.05f;
    EffectManagerPJS::Instance()->fn_8042E0C4(&parameters);
    if (!m_effect.expired()) {
        m_effect.lock()->RequestStop();
    }
    fn_801D3838(1);
}

void EffectImpl::fn_800A30E0(Effect *effect) {
    if (m_slow != 0) {
        m_slow->m_ending = true;
    }
    new Slow(effect, m_player, &m_slow);
}

void EffectImpl::fn_800A30C8() {
    if (m_slow != 0) {
        m_slow->m_ending = true;
    }
}

void EffectImpl::fn_800A2FC8(int type) {
    if (fn_8006CD78(m_player->field_0xA8, 2) && !m_player->fn_80073A20()) {
        UnkPlayerRecord_800A2FC8 *record = m_player->field_0x278;
        int sound = -1;
        switch (type) {
        case 4:
        case 5:
            sound = s_effects[record->field_0x1C].field_0xC;
            break;
        case 6:
            sound = 0x3001; /* meaning unknown */
            break;
        }
        if (sound >= 0) {
            fn_801D560C(fn_80053164(), sound, 0, 0);
        }
        if (s_effects[record->field_0x1C].field_0x0 >= 0) {
            fn_800A27D4(m_player->field_0x23C, &record->field_0x0, &record->field_0xC,
                        s_effects[record->field_0x1C].field_0x0);
        }
    }
}

void EffectImpl::fn_800A2F74(int index, const Fn8040Vector3 *position,
                             const Fn8040Vector3 *vector) {
    int id = s_effects[index].field_0x8;
    if (id >= 0) {
        fn_800A27D4(m_player->field_0x23C, position, vector, id);
    }
}

void EffectImpl::fn_800A2F48(void *payload) {
    if (m_range != 0) {
        m_range->fn_800A3AA4(static_cast<const int *>(payload));
    }
}

Effect::Effect(Task *parent, const boost::shared_ptr<PlayerBase> &player)
    : Task(parent), m_impl(new EffectImpl(player)) {
    m_name = "Effect";
    m_impl->fn_800A37CC(this);
    m_impl->m_player->field_0x23C = this;
}

Effect::~Effect() {
    m_impl->m_player->field_0x23C = 0;
}

void Effect::Update(float) {
    if (m_impl->m_player->field_0x230 != 0) {
        m_impl->m_player->field_0x230->fn_800AA5D4();
    }
}

} // namespace Player

Task *fn_800A2D7C(Player::Effect *effect, int type) {
    return effect->m_impl->fn_800A3774(effect, type);
}

void fn_800A2D54(Player::Effect *effect, float arg1) {
    effect->m_impl->fn_800A3708(effect, arg1);
}

void fn_800A2D30(Player::Effect *effect) {
    effect->m_impl->fn_800A36B4();
}

void fn_800A2D08(Player::Effect *effect) {
    effect->m_impl->fn_800A3640(effect);
}

void fn_800A2CE4(Player::Effect *effect) {
    effect->m_impl->fn_800A361C();
}

void fn_800A2CBC(Player::Effect *effect) {
    effect->m_impl->fn_800A35A8(effect);
}

void fn_800A2C98(Player::Effect *effect) {
    effect->m_impl->fn_800A3584();
}

void fn_800A2C70(Player::Effect *effect) {
    effect->m_impl->fn_800A352C(effect);
}

void fn_800A2C4C(Player::Effect *effect) {
    effect->m_impl->fn_800A3500();
}

void fn_800A2C28(Player::Effect *effect, bool start) {
    effect->m_impl->fn_800A34D4(start);
}

void fn_800A2C00(Player::Effect *effect) {
    effect->m_impl->fn_800A3448(effect);
}

void fn_800A2BD8(Player::Effect *effect) {
    effect->m_impl->fn_800A33BC(effect);
}

void fn_800A2B78(Player::Effect *effect) {
    effect->m_impl->fn_800A2B78(effect);
}

void fn_800A2B44(Player::Effect *effect) {
    effect->m_impl->fn_800A2B44();
}

void fn_800A2AD8(Player::Effect *effect) {
    effect->m_impl->fn_800A2AD8(effect);
}

void fn_800A2A78(Player::Effect *effect) {
    effect->m_impl->fn_800A2A78();
}

void fn_800A2A54(Player::Effect *effect) {
    effect->m_impl->fn_800A332C();
}

void fn_800A2A30(Player::Effect *effect) {
    effect->m_impl->fn_800A3140();
}

void fn_800A2A08(Player::Effect *effect) {
    effect->m_impl->fn_800A30E0(effect);
}

void fn_800A29E4(Player::Effect *effect) {
    effect->m_impl->fn_800A30C8();
}

void fn_800A29C0(Player::Effect *effect, int arg0) {
    effect->m_impl->fn_800A2FC8(arg0);
}

void fn_800A299C(Player::Effect *effect, int index, const Fn8040Vector3 *position,
                 const Fn8040Vector3 *vector) {
    effect->m_impl->fn_800A2F74(index, position, vector);
}

void fn_800A2978(Player::Effect *effect, void *payload) {
    effect->m_impl->fn_800A2F48(payload);
}

void fn_800A2920(Player::Effect *effect, int arg1) {
    effect->m_impl->fn_800A2920(arg1);
}

void fn_800A28EC(Player::Effect *effect) {
    effect->m_impl->fn_800A28EC();
}

void fn_800A28B0(Player::Effect *effect) {
    effect->m_impl->fn_800A28B0();
}

boost::weak_ptr<EffectElement> fn_800A2854(Player::Effect *effect, int index, int id) {
    return fn_800A2814(effect, effect->m_impl->m_player->field_0x230->fn_800AA760(index)->field_0x0,
                       id);
}

boost::weak_ptr<EffectElement> fn_800A2814(Player::Effect *effect, const Fn8040Matrix34 *matrix,
                                           int id) {
    return fn_801D2C28(0, matrix, id, effect->m_impl->m_player->field_0x60 + 1);
}

boost::weak_ptr<EffectElement> fn_800A27D4(Player::Effect *effect, const Fn8040Vector3 *position,
                                           const Fn8040Vector3 *vector, int id) {
    return fn_801D2AFC(0, position, vector, id, effect->m_impl->m_player->field_0x60 + 1);
}

boost::weak_ptr<EffectElement> fn_800A2794(Player::Effect *effect, const Fn8040Vector3 *position,
                                           const UnkRotation3 *rotation, int id) {
    return fn_801D2B38(0, position, rotation, id, effect->m_impl->m_player->field_0x60 + 1);
}

boost::weak_ptr<EffectElement> fn_800A2754(Player::Effect *effect, int id) {
    return fn_801D2BB0(0, id, effect->m_impl->m_player->field_0x60 + 1);
}

void fn_800A2730(int index, s32 value0, s32 value4, s32 value8, s32 valueC) {
    s_effects[index].field_0x0 = value0;
    s_effects[index].field_0x4 = value4;
    s_effects[index].field_0x8 = value8;
    s_effects[index].field_0xC = valueC;
}

void fn_800A26F0(int index) {
    s_effects[index] = s_defaultEffects[index];
}

UnkObject_800A26E0::UnkObject_800A26E0() : field_0x0(0), field_0x4(0) {}

UnkObject_800A26E0::~UnkObject_800A26E0() {}

UnkObject_80079BA4::~UnkObject_80079BA4() {}

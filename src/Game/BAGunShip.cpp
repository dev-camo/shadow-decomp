/* The BAGunShip gadget (class described in Game/BAGunShip.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to the attack object and to the probe are boost::function objects built
 * with boost::bind(&BAGunShip::method, this, _1); their manager, invoker and copy functions come
 * from the boost headers. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/BAGunShip.h"
#include "Game/AngleConversion.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkFrameSearch_80169E8C.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkParams_80051D10.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_80221BA8.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBase;

static const char *s_modelFile = "BAGUNSHIP.BIN";
static const char *s_crashFile = "DM0000GUNSHIPBRK.DFF";

/* Nothing in this file reads this float; the original only stores 500 in it at startup. The
 * BDHologram file sets the same value at startup as the default of its parameter "distance".
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of this variable.
 * The original sets it with a start-up function in .ctors (a constant float initialiser would be
 * emitted as initialised .sdata data instead), and the shared stand-in reproduces that function.
 * Revisit when the original wrapper type or a reader of the value is found. */
static UnkStartupFloat_800E2570 s_unusedValue(500.0f);

namespace BAGunShip {

const char *BAGunShip::s_name = "BAGunShip";

BAGunShip::BAGunShip(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x8D, 4);
    ReadOwner(owner);

    m_model = fn_8015F800(1, s_modelFile);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80160C28(0);
    m_model->fn_8015FF60();
    m_motion = m_model->fn_801602B4(0);
    m_model->fn_80161BD8(0, boost::weak_ptr<CharInfo>(m_charInfo), 0);
    m_model->fn_80161230(0, -1);

    boost::weak_ptr<CharaColliAttack> attackWeak = m_model->fn_80161428(0);

    if (boost::shared_ptr<CharaColliAttack> attack = attackWeak.lock()) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 = AttackCallback(boost::bind(&BAGunShip::OnAttack, this, _1));
    }

    {
        Fn8040Vector3 offset(0.0f, 0.0f, 0.0f);
        UnkRotation3 rotation(fn_8000AB90(90.0f), 0.0f, 0.0f);
        boost::shared_ptr<CharaColliBase> probe =
            fn_800C7A84(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), m_params.height,
                        m_params.radius, 0x40100, 10, 0, &offset, &rotation, 0);

        m_handle = fn_8015B8C4(probe);
        m_handle->field_0x0.fn_800F5980(
            GadgetColliDetectCallback(boost::bind(&BAGunShip::OnHit, this, _1)));
    }

    UnkFrameSearch_80169E8C search;

    /* Matching workaround: frame is cleared again after the id is set, although the constructor
     * has cleared it; the original stores 0 to it twice (the second store at USA (GUPE8P)
     * 0x802FFA44, after the store of 7 to the id). Revisit if the original type of the search
     * record or its initialisation is found. */
    search.id = 7;
    search.frame = 0;
    m_model->fn_8015FA78(fn_80169E8C, &search);
    m_muzzleFrame = search.frame;

    if (m_model.get() != 0) {
        m_target = fn_80166AF4();

        UnkWeakHolder_800CC6C8 weakTarget;
        weakTarget.ptr = m_target;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
    }

    m_object = fn_801E72DC(0x800);
    m_object->ClearFlags(1);
    UpdateModelMatrix();
}

BAGunShip::~BAGunShip() {
    fn_801E7108(m_object);
}

void BAGunShip::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    m_hitTimer -= dt;
    if (m_hitTimer < 0.0f) {
        m_hitTimer = 0.0f;
    }
    m_shotTimer -= dt;
    if (m_shotTimer < 0.0f) {
        m_shotTimer = 0.0f;
    }

    if (m_gunFlags.Test(3)) {
        fn_801D53C4(fn_80053164(), 0x792F, &m_charInfo->m_position, this, 3, 0, 2);
    }

    switch (m_state) {
    case 0:
        if (m_comId > 0) {
            if (fn_8016904C(fn_800CB69C(), m_comId) == 1) {
                m_state = 2;
            }
        } else {
            m_state = 2;
        }

        if (!m_model->fn_801626F4()) {
            m_model->fn_80162790();
            /* The original does not check m_target here. */
            m_target->fn_80166C70();
        }

        /* The original rebuilds the object on every update in this state. */
        {
            UnkObjectParams_8027C3C4 params;

            params.comId = m_params.comId;
            params.num = m_params.num;
            params.delay = m_params.delay;
            m_object8027C3C4 = fn_8027C3C4(params);
        }
        break;
    case 1:
        if (m_model->fn_801626F4()) {
            m_timer -= dt;

            if (m_timer <= 0.0f) {
                m_model->fn_8016271C();
                m_model->fn_8016111C(0, -1);
                /* The original does not check m_target for this call. */
                m_target->fn_80166B70();

                if (m_target.get() != 0) {
                    m_target->fn_801672E4(-1);
                    m_target->fn_80167010(-1);
                }

                fn_801D54F4(fn_80053164(), 0x792E, &m_charInfo->m_position, 0, 1);
                m_gunFlags.Set(3);
                m_object->SetFlags(1);
            }
        }

        if (!m_motion.expired()) {
            if (boost::shared_ptr<_stMtnPlayWork> motion = m_motion.lock()) {
                if (stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                    if (m_gunFlags.Test(2)) {
                        m_state = 4;
                    } else {
                        m_state = 3;
                    }

                    m_model->fn_80160A84(0, 2);

                    if (m_object8027C3C4.get() != 0) {
                        m_object8027C3C4->fn_8027C67C();
                    }

                    m_timer = m_params.second;
                }
            }
        }
        break;
    case 2:
        break;
    case 3:
        m_timer -= dt;

        if (m_timer <= 0.0f) {
            m_model->fn_80160A84(0, 3);
            m_state = 5;

            if (m_object8027C3C4.get() != 0) {
                m_object8027C3C4->fn_8027C650();
            }
        } else if (m_gunFlags.Test(0)) {
            if (m_shotTimer <= 0.0f) {
                m_shotTimer = Shoot();
                m_gunFlags.Reset(1);
            } else if (m_shotTimer <= 1.0f) {
                if (!m_gunFlags.Test(1)) {
                    m_gunFlags.Set(1);
                    if (m_target.get() != 0) {
                        m_target->fn_801672E4(1);
                    }
                }
            }
        }
        break;
    case 4:
        /* Matching workaround: the shot block of case 3 is written out again here, as the original
         * has its own copy (its two Shoot calls are at USA (GUPE8P) 0x802FED64 and 0x802FEDF0).
         * As an inline member function the block would be emitted out of line (it calls inline
         * functions and the unit builds with -inline level=1), and falling through from case 3
         * shrinks UpdateBody by 0x8C bytes. Revisit if the original's form of the block (a macro
         * or an inline the compiler expands here) is found. */
        if (m_gunFlags.Test(0)) {
            if (m_shotTimer <= 0.0f) {
                m_shotTimer = Shoot();
                m_gunFlags.Reset(1);
            } else if (m_shotTimer <= 1.0f) {
                if (!m_gunFlags.Test(1)) {
                    m_gunFlags.Set(1);
                    if (m_target.get() != 0) {
                        m_target->fn_801672E4(1);
                    }
                }
            }
        }
        break;
    case 5:
        if (!m_motion.expired()) {
            if (boost::shared_ptr<_stMtnPlayWork> motion = m_motion.lock()) {
                if (stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                    m_state = 7;
                    m_gunFlags.Reset(3);
                }
            }
        }
        break;
    case 6: {
        StartCrashEffect();

        // Copies of the position and rotation are passed: passing the members directly gives a
        // stack frame 16 bytes smaller than the original's.
        Fn8040Vector3 position = m_charInfo->m_position;
        UnkRotation3 rotation = m_charInfo->m_rotation;
        Fn8040Vector3 up(0.0f, 1.0f, 0.0f);
        fn_80221BA8(position, rotation, up, 0x10, -1);

        if (m_object8027C3C4.get() != 0) {
            m_object8027C3C4->fn_8027C650();
        }

        fn_80169B80(3, &m_charInfo->m_position);
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }
    case 7:
        fn_8015A794(this);
        m_flags |= 1;
        break;
    }

    if (m_object8027C3C4.get() != 0) {
        m_object8027C3C4->fn_8027C6A8(dt);
    }

    UpdateModelMatrix();
}

void BAGunShip::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
    }

    if (m_handle.get() != 0) {
        m_handle->m_probe->SetSize(m_params.radius, -2, 0.0f, m_params.height);
        fn_8015BE64(m_handle.get());
    }
}

void BAGunShip::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E50(owner, &m_comId);
    m_state = 0;
    m_timer = 0.0f;
    m_hitTimer = 0.0f;
    m_health = 40.0f;
    m_shotTimer = m_params.interval;

    if (m_params.attack != 0) {
        m_gunFlags.Set(0);
    } else {
        m_gunFlags.Reset(0);
    }

    if (m_params.second <= 0.0f) {
        m_gunFlags.Set(2);
    } else {
        m_gunFlags.Reset(2);
    }
}

void BAGunShip::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
    }

    if (m_object.get() != 0) {
        fn_800CCE38(m_object.get(), m_charInfo->m_position);
    }
}

void BAGunShip::OnHit(ColliDetectArgs &args) {
    if (m_state == 2 && fn_8015AC84(args)) {
        m_timer = 1.0f;
        m_state = 1;
        m_model->fn_80160A84(0, 1);
        UpdateModelMatrix();
    }
}

void BAGunShip::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 5;

    if (m_hitTimer > 0.0f) {
        return;
    }

    if (m_state == 3 || m_state == 4) {
        if (boost::shared_ptr<CharaColliAttack> attack = param.field_0x0.lock()) {
            if ((attack->field_0x40 & 0x3D0) == 0) {
                return;
            }

            m_health -= param.field_0x50;
            m_hitTimer = 0.5f;

            if (m_health > 0.0f) {
                param.field_0x54 = 1;
                fn_801D54F4(fn_80053164(), 0x7931, &m_charInfo->m_position, 0, 1);
            } else {
                param.field_0x54 = 3;
                m_state = 6;

                if (boost::shared_ptr<CharInfo> attacker = attack->m_owner.lock()) {
                    fn_8020642C(attacker, 0x4E);
                }

                fn_801D54F4(fn_80053164(), 0x7932, &m_charInfo->m_position, 0, 1);
            }
        }
    }
}

void BAGunShip::StartCrashEffect() {
    RpClump *clump = static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, s_crashFile));

    if (clump != 0) {
        Fn8040Vector3 angles(0.0f, 0.0f, 0.0f);
        boost::weak_ptr<_stMtnPlayWork> motion = m_model->fn_801602B4(0);

        if (boost::shared_ptr<_stMtnPlayWork> work = motion.lock()) {
            fn_80415DF8(work.get(), &angles);
        }

        UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

        effect.field_0x34 = 1.3f;
        effect.field_0x10.y = angles.y;
        effect.field_0x44 = clump;
        effect.field_0x54 = 1;
        effect.field_0x4C = 150;
        effect.field_0x40 = m_charInfo->m_position.y;
        effect.field_0x4A = 0;
        fn_8015D93C(&effect);
    }
}

float BAGunShip::Shoot() {
    if (m_muzzleFrame != 0) {
        RwMatrix *ltm = RwFrameGetLTM(m_muzzleFrame);
        Fn8040Vector3 muzzle(ltm->pos.x, ltm->pos.y, ltm->pos.z);
        Fn8040Vector3 target(0.0f, 0.0f, 0.0f);
        boost::weak_ptr<CharInfo> character = fn_800A8B74(fn_80074194(), lbl_805EE588);

        if (boost::shared_ptr<CharInfo> info = character.lock()) {
            target = info->m_position;
        }

        Fn8040Vector3 direction = target - muzzle;

        if (direction.y > 0.0f) {
            direction.y = 0.0f;
        }

        UnkParams_80051D10 params(boost::weak_ptr<CharInfo>(m_charInfo), 1, muzzle, direction,
                                  Fn8040Vector3(0.0f, 0.0f, 0.0f));

        fn_80051C34(0x1D, params);
        fn_801D54F4(fn_80053164(), 0x7930, &m_charInfo->m_position, 0, 1);
    }

    return m_params.interval;
}

} // namespace BAGunShip

extern "C" void fn_802FE2A8(void) {
    fn_8006B474(s_modelFile, 1);
}

extern "C" void fn_802FE2A4(void) {}

void BAGunShip::BAGunShip::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new BAGunShip(parent, owner);
}

static const char *s_attackLabels[2] = {"attack OFF", "attack ON"};

/* The label callback of the parameter "attack" (behaviour inferred). */
static const char *fn_802FE218(UnkParamCursor_800CBC20 *cursor) {
    if (cursor->m_index == 3) {
        if (cursor->m_value == 0) {
            return s_attackLabels[0];
        }

        return s_attackLabels[1];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "radius", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "height", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "second", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {1, "attack", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_802FE218, 0},
    {4, "N/A", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {1, "ComId", 0xFFFFFFFF, 0xFFFFFFFF, 0xFF, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {1, "num", 0, 0, 0x10000, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "delay", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "interval", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8055AA18 = {
    "BA GUNSHIP", fn_802FE2A8, fn_802FE2A4, BAGunShip::BAGunShip::Create, 0, 0, 0x17D4, 1, 0,
    10,           s_paramInfo,
};

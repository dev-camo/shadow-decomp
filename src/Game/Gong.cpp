/* Gong::Gong (class name from RTTI; the gadget's descriptor is "GONG"): a gadget with three models
 * of GONG.BIN. A character that hits model 2 is launched straight up; the first such hit also
 * gives it an item and starts a Gong::CSD::GongCSD task, which plays a scene of the CSD project
 * PR0302RINGANM. A character that hits model 1 is launched along the gadget's up direction with a
 * speed that depends on its own speed and on its distance from the gadget (behaviour inferred).
 * File name inferred from the class. Method and member names are inferred. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
/* The unit's .rodata starts with the type names that TNumEdit.h and CharaColliBaseCharInfo.h
 * produce, in that order (see those headers).
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/Gong.h"
#include "Game/CSD/csdmProject.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_8003F8D4.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_80136B5C.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

using Chao::CSD::CProject;
using Chao::CSD::CResourceBase;
using Chao::CSD::CScene;
using Chao::CSD::Project;
using Chao::CSD::RCPtr;

/* What the hit callbacks need to know about the gadget (placeholder name): the handle that
 * remembers the characters already dealt with, the gadget's character info, its parameters, the
 * point the characters are launched from, copies of its hit count and flags, and, as output,
 * whether anybody was hit. */
struct UnkHitContext_801373F4 {
    UnkHitContext_801373F4() : m_params(0), m_hitCount(0), m_flags(0), m_hit(false) {}

    /* 0x00 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x08 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x10 */ const Gong::GongParams *m_params;
    /* 0x14 */ Fn8040Vector3 m_position;
    /* 0x20 */ s32 m_hitCount;
    /* 0x24 */ u32 m_flags;
    /* 0x28 */ bool m_hit;
};
CHECK_SIZEOF(UnkHitContext_801373F4, 0x2C);

static const char *s_modelFileName = "GONG.BIN";

const char *Gong::Gong::s_name = "Gong";

/* The hit callback of model 1: takes a character that has not been dealt with out of control and
 * launches it from 5 above the context's point along the gadget's up direction. The speed is 560
 * once the model has hit before. Otherwise it is 670 times the ratio of the character's speed (the
 * length of its vector at 0x18) to the "speed" parameter, capped at 1, scaled by 1 within a
 * horizontal distance of 5 from the gadget, by 0.4 from a distance of 50 and by
 * 0.4 + (0.6 - distance / 50) in between (0.9 just beyond 5, falling towards 0 near 50), and at
 * least 560. */
static void fn_80138338(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_801373F4 *context = static_cast<UnkHitContext_801373F4 *>(arg);
    boost::shared_ptr<GadgetCharInfo> gongInfo = context->m_charInfo.lock();

    if (gongInfo.get() == 0) {
        return;
    }

    if (fn_801697CC(context->m_handle.get(), boost::weak_ptr<CharInfo>(hit))) {
        return;
    }

    hit->fn_801DC980(new Player::IgnoreInputCommand(1, 0));
    hit->fn_801DC980(new ClrVelocityCommand(4));
    hit->fn_801DC980(new Player::LaunchCommand(1, 4));

    float speed;

    if (context->m_hitCount > 0) {
        speed = 560.0f;
    } else {
        float ratio = fn_8040CF50(&hit->field_0x18) / context->m_params->speed;

        if (ratio >= 1.0f) {
            ratio = 1.0f;
        }

        Fn8040Vector3 hitPosition;
        Fn8040Vector3 gongPosition;
        Fn8040Vector3 difference;

        fn_800091EC(&hitPosition, &hit->m_position);
        hitPosition.y = 0.0f;
        fn_800091EC(&gongPosition, &gongInfo->m_position);
        gongPosition.y = 0.0f;
        fn_8040CD74(&difference, &hitPosition, &gongPosition);

        float distance = fn_8040CF50(&difference);
        float scale;

        if (distance <= 5.0f) {
            scale = 1.0f;
        } else if (distance >= 50.0f) {
            scale = 0.4f;
        } else {
            scale = 0.4f + (0.6f - distance / 50.0f);
        }

        speed = scale * (670.0f * ratio);

        if (speed < 560.0f) {
            speed = 560.0f;
        }
    }

    Fn8040Vector3 position;

    fn_800091EC(&position, &context->m_position);
    position.y += 5.0f;
    hit->fn_801DC980(new SetPositionCommand(&position, 4));

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B848(&matrix, gongInfo->m_rotation.y);
    fn_8040B884(&matrix, gongInfo->m_rotation.x);
    fn_8040B80C(&matrix, gongInfo->m_rotation.z);

    Fn8040Vector3 direction(0.0f, 1.0f, 0.0f);
    Fn8040Vector3 directionTemporary;

    fn_8040B67C(&directionTemporary, &matrix, &direction);
    fn_800091D0(&direction, &directionTemporary);
    fn_8040CE38(&direction, speed);
    hit->fn_801DC980(new SetVelocityCommand(&direction, 4));

    fn_801D560C(fn_80053164(), 0xE00F, 0, 0);
    hit->fn_801DC980(new Player::LaunchCommand(1, 4));

    fn_80169BB0(boost::weak_ptr<CharInfo>(hit), 5);
    context->m_hit = true;
}

/* The hit callback of model 2: takes the first character it is called with out of control and
 * launches it straight up from the context's point. The first time (bit 0 of the context's flags
 * clear) it also gives that character an item and starts a GongCSD task. */
static void fn_801380C0(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_801373F4 *context = static_cast<UnkHitContext_801373F4 *>(arg);

    if (context != 0 && !context->m_hit) {
        boost::shared_ptr<GadgetCharInfo> gongInfo = context->m_charInfo.lock();

        if (gongInfo.get() == 0) {
            return;
        }

        hit->fn_801DC980(new Player::IgnoreInputCommand(1, 0));
        hit->fn_801DC980(new ClrVelocityCommand(4));
        hit->fn_801DC980(new SetPositionCommand(&context->m_position, 4));

        Fn8040Vector3 velocity(0.0f, 400.0f, 0.0f);

        hit->fn_801DC980(new SetVelocityCommand(&velocity, 4));
        hit->fn_801DC980(new Player::LaunchCommand(1, 4));

        fn_801D560C(fn_80053164(), 0x743E, 0, 0);

        if ((context->m_flags & 1) == 0) {
            fn_8015F6EC(hit, 0, 50, 0);
            context->m_flags |= 1;
            fn_80136B5C(4);
        }

        fn_80169BB0(boost::weak_ptr<CharInfo>(hit), 1);
        context->m_hit = true;
    }
}

namespace Gong {

Gong::Gong(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x4A, 4, "Gong");
    ReadOwner(owner);

    m_model0 = fn_8015F800(1, s_modelFileName);
    m_model0->fn_80162A68(0, 0);
    m_model0->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    m_model2 = fn_8015F800(1, s_modelFileName);
    m_model2->fn_80162A68(2, 0);
    m_model2->fn_80161FC8(2, boost::weak_ptr<CharInfo>(m_charInfo));
    m_model2->m_collisionCallback =
        ColliDetectCallback(boost::bind(&Gong::OnModel2Collision, this, _1));
    m_model2->fn_80160C28(1);
    m_model2->fn_8015FF60();

    m_model1 = fn_8015F800(1, s_modelFileName);
    m_model1->fn_80162A68(1, 0);
    m_model1->fn_80161FC8(1, boost::weak_ptr<CharInfo>(m_charInfo));
    m_model1->m_collisionCallback =
        ColliDetectCallback(boost::bind(&Gong::OnModel1Collision, this, _1));
    m_model1->fn_80160C28(0);
    m_model1->fn_8015FF60();

    UpdateModelMatrix();
    m_handle = fn_801695B8();
}

Gong::~Gong() {
    fn_8016952C(m_handle);
}

void Gong::UpdateBody(float dt) {
    if (m_handle.get() != 0) {
        fn_801697F0(m_handle.get());
    }

    if (m_model0.get() != 0) {
        m_model0->fn_80162FB0(dt);
    }

    if (m_model1.get() != 0) {
        m_model1->fn_80162FB0(dt);
    }

    if (m_model2.get() != 0) {
        m_model2->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        m_state = 1;
        break;

    case 1:
        break;

    case 2:
        break;
    }

    UpdateModelMatrix();
}

void Gong::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model0.get() != 0) {
        m_model0->fn_8016108C();
    }

    if (m_model1.get() != 0) {
        m_model1->fn_80162EF4();
        m_model1->fn_8016108C();
    }

    if (m_model2.get() != 0) {
        m_model2->fn_80162EF4();
        m_model2->fn_8016108C();
    }
}

void Gong::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_state = 0;
    m_hitCount = 0;
    m_flags = 0;
}

void Gong::UpdateModelMatrix() {
    if (m_model0.get() != 0 && m_model1.get() != 0 && m_model2.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model0->fn_801633DC(&matrix);
        m_model1->fn_801633DC(&matrix);
        m_model2->fn_801633DC(&matrix);
    }
}

void Gong::OnModel1Collision(ColliDetectArgs &args) {
    UnkHitContext_801373F4 context;

    context.m_handle = m_handle;
    context.m_charInfo = m_charInfo;
    context.m_params = &m_params;
    fn_800091D0(&context.m_position, &m_charInfo->m_position);
    context.m_hitCount = m_hitCount;
    fn_8015AA84(&args, fn_80138338, &context);

    if (context.m_hit) {
        if (m_model1.get() != 0) {
            m_model1->fn_80160B18(0);
        }

        m_hitCount++;
    }
}

void Gong::OnModel2Collision(ColliDetectArgs &args) {
    UnkHitContext_801373F4 context;

    context.m_charInfo = m_charInfo;
    context.m_params = &m_params;
    context.m_flags = m_flags;
    fn_800091D0(&context.m_position, &m_charInfo->m_position);
    context.m_position.y += 300.0f;
    fn_8015B004(&args, fn_801380C0, &context);

    if (context.m_hit) {
        if (m_model2.get() != 0) {
            m_model2->fn_80161308(1.0f);
            m_model2->fn_80160B18(1);
        }

        if (context.m_flags & 1) {
            m_flags |= 1;
        }
    }
}

namespace CSD {

GongCSD::GongCSD(Task *parent, int motion) : Task(parent) {
    m_layer = fn_8004DFB0(fn_80046178()->field_0x38, "GongCSD");
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&GongCSD::OnDraw, this, _1));

    UnkResourceManager_800122D4 *resources = fn_800122D4();
    const CResourceBase<Project> *resource = static_cast<const CResourceBase<Project> *>(
        fn_80012700(resources, 1, fn_8004237C(fn_8003F8D4(), "PR0302RINGANM", 1)));

    m_project = new CProject(*resource);
    m_project->CreateScene("Scene_0000", motion, 0);
    m_soundTimer = 0.0f;

    if (motion == 0 || motion == 5) {
        m_soundFlags.Reset(0);
    } else {
        m_soundFlags.Set(0);
    }
}

GongCSD::~GongCSD() {
    if (m_project != 0) {
        m_project->RemoveAllScenes();
        delete m_project;
        m_project = 0;
    }
}

/* Advances the project. When the scene's motion has reached its end the task finishes and plays
 * sound 0x743A; until then, while bit 0 of the sound flags is set, sound 0xE00B plays every 0.2
 * seconds (behaviour inferred). */
void GongCSD::Update(float dt) {
    if (m_project != 0) {
        m_project->Update(dt);

        RCPtr<CScene> scene = m_project->GetScene("Scene_0000");

        if (scene->GetLastFrame() > scene->GetMotionEnd() - 0.01f) {
            m_flags |= 1;
            fn_801D560C(fn_80053164(), 0x743A, 0, 0);
        } else if (m_soundFlags.Test(0)) {
            m_soundTimer -= dt;

            if (m_soundTimer <= 0.0f) {
                m_soundTimer = 0.2f;
                fn_801D560C(fn_80053164(), 0xE00B, 0, 0);
            }
        }
    }
}

void GongCSD::OnDraw(RednerDrawArgs &args) {
    if (m_project != 0) {
        m_project->Draw();
    }
}

} // namespace CSD

} // namespace Gong

extern "C" void fn_80136B5C(int arg0) {
    if (arg0 != 5) {
        Task *parent = fn_801EB304(fn_8001C4F8(), 11);

        new Gong::CSD::GongCSD(parent, arg0);
    }
}

extern "C" void fn_80136B14(void) {
    fn_8006B474("GONG.BIN", 1);
    fn_8006B474(fn_8004237C(fn_8003F8D4(), "PR0302RINGANM", 1), 0);
}

extern "C" void fn_80136B10(void) {}

void Gong::Gong::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Gong(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "speed", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80528C94 = {
    "GONG", fn_80136B14, fn_80136B10, Gong::Gong::Create, 0, 0, 0xC81, 1, 0, 10, s_paramInfo,
};

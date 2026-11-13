/* TGiHPGaugeBoss (class name from RTTI): the task that shows a boss's gauge in the in-game
 * display, with its implementation class TGiHPGaugeBossImpl (name from the RTTI name of its draw
 * callback's binder). File name inferred from the class. Method and member names are inferred.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/TGiHPGaugeBoss.h"
#include "Game/CSD/csdmProject.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "Game/UnkObject_8003F8D4.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>

using Chao::CSD::CProject;
using Chao::CSD::CScene;
using Chao::CSD::RCPtr;

/* The gauge's settings, which fn_800BE2EC and fn_800BE2DC change. */
static bool s_enabled;
static float s_value;

/* The gauge itself: the CSD project BossGauge, drawn through a draw layer. */
class TGiHPGaugeBossImpl {
  public:
    TGiHPGaugeBossImpl();
    ~TGiHPGaugeBossImpl();

    /* While the gauge is enabled and the project has the scene "Scene_0000": sets the scene's
     * frame from the value and calls the project's fn_8036DAD4. */
    void Update();
    /* The draw layer's callback: draws the project while lbl_8057D6A0's byte at 0x148 is set and
     * the gauge is enabled. */
    void OnDraw(RednerDrawArgs &args) const;

    /* 0x0 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0x8 */ CProject *m_project;
};
CHECK_SIZEOF(TGiHPGaugeBossImpl, 0xC);

TGiHPGaugeBossImpl::TGiHPGaugeBossImpl() {
    s_enabled = false;
    s_value = 1.0f;
    m_layer = fn_8004E24C(fn_80046178()->field_0x38);
    m_layer->m_drawCallback =
        RenderDrawCallback(boost::bind(&TGiHPGaugeBossImpl::OnDraw, this, _1));
    m_project = new CProject;
    m_project->LoadFile(fn_800423B8(fn_8003F8D4(), "CsdFiles/BossGauge", 0));
    m_project->fn_8036D598(0);
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if a destructor called from checked_delete turns out to be inlined in the
 * original. */
#pragma auto_inline on
TGiHPGaugeBossImpl::~TGiHPGaugeBossImpl() {
    if (m_project != 0) {
        delete m_project;
    }
}
#pragma auto_inline reset

void TGiHPGaugeBossImpl::Update() {
    if (m_project != 0 && s_enabled) {
        RCPtr<CScene> scene = m_project->GetScene("Scene_0000");

        if (scene.Get() != 0) {
            float frame = scene->GetMotionEnd() - 1.0f;

            frame *= 1.0f - s_value;
            scene->SetFrame(frame);
            m_project->fn_8036DAD4();
        }
    }
}

void TGiHPGaugeBossImpl::OnDraw(RednerDrawArgs &args) const {
    if (lbl_8057D6A0.field_0x148 && s_enabled) {
        m_project->Draw();
    }
}

TGiHPGaugeBoss::TGiHPGaugeBoss(Task *parent) : Task(parent), m_impl(new TGiHPGaugeBossImpl) {
    m_name = "TGiHPGaugeBoss";
    m_flags |= 0x100;
}

TGiHPGaugeBoss::~TGiHPGaugeBoss() {}

void TGiHPGaugeBoss::Update(float dt) {
    m_impl->Update();
}

extern "C" bool fn_800BE2EC(bool value) {
    bool previous = s_enabled;

    s_enabled = value;
    return previous;
}

extern "C" float fn_800BE2DC(float value) {
    float previous = s_value;

    s_value = value;
    return previous;
}

/* TGiHdGage (class name from RTTI): the task that shows the hero and dark gauges of the in-game
 * display, two CSD projects drawn through a draw layer. File name inferred from the class. Method
 * and member names are inferred. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first). */
#include "Game/TGiHdGage.h"
#include "Game/CSD/csdmProject.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80079808.h"
#include "Game/UnkObject_8003F8D4.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using Chao::CSD::CProject;
using Chao::CSD::CScene;
using Chao::CSD::RCPtr;

/* The project and the scene of each gauge (name inferred); gauge N shows entry N of
 * UnkManager_80079808::field_0x70C. */
static const struct {
    const char *project;
    const char *scene;
} s_gaugeFiles[2] = {
    {"CsdFiles/HeroGauge", "Scene_0000"},
    {"CsdFiles/DarkGauge", "Scene_0000"},
};

TGiHdGage::TGiHdGage(Task *parent) : Task(parent) {
    m_name = "TGiHdGage";
    m_flags |= 0x100;
    LoadProjects();
    m_layer = fn_8004E24C(fn_80046178()->field_0x38);
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&TGiHdGage::OnDraw, this, _1));
}

/* Matching workaround: the destructor's cleanup loop is in an inline function that the
 * destructor expands. Its name and its form as a member (and not a free function) are inferred;
 * the original has no out-of-line copy of it. The original loads zero into the loop's counter and
 * copies the counter's register into the gauge offset before the loop. That fits a counter that
 * is a compiler temporary, which the local of an expanded inline function is. In all of the USA
 * .text 40 counted loops copy their zeroed counter's register like this (20 of them in functions
 * that test their first argument for null on entry, as destructors do), against 373 counted
 * loops that load zero into both registers (22 in such functions). With the loop written in the
 * destructor the build loads zero twice (2 of the destructor's 41 words differ). Revisit when the
 * class's original source is found, or another source form gives the copy. */
inline void TGiHdGage::ReleaseProjects() {
    for (int i = 0; i < 2; i++) {
        delete m_gauges[i].project;
    }
}

TGiHdGage::~TGiHdGage() {
    ReleaseProjects();
}

/* For each gauge whose project has the scene: the value becomes the entry's first word, or the
 * mean of the two (rounded towards zero) while the first word is above the value, and is then
 * capped at the entry's second word. The scene's frame is set to the motion's end frame
 * (truncated to an integer) times the value, divided by the second word as integers, and the
 * project's fn_8036DAD4 is called. */
void TGiHdGage::Update(float dt) {
    UnkState_8016F0CC *state = &fn_80079808()->field_0x70C;

    for (int i = 0; i < 2; i++) {
        RCPtr<CScene> scene = m_gauges[i].project->GetScene(s_gaugeFiles[i].scene);

        if (scene.Get() != lbl_805EE118.Get()) {
            int end = scene->GetMotionEnd();

            if (state->field_0x0[i][0] > m_gauges[i].value) {
                m_gauges[i].value = (m_gauges[i].value + state->field_0x0[i][0]) / 2;
            } else {
                m_gauges[i].value = state->field_0x0[i][0];
            }

            if (m_gauges[i].value > state->field_0x0[i][1]) {
                m_gauges[i].value = state->field_0x0[i][1];
            }

            scene->SetFrame(end * m_gauges[i].value / state->field_0x0[i][1]);
            m_gauges[i].project->fn_8036DAD4();
        }
    }
}

void TGiHdGage::LoadProjects() {
    for (int i = 0; i < 2; i++) {
        m_gauges[i].project = new CProject;
        m_gauges[i].project->LoadFile(fn_800423B8(fn_8003F8D4(), s_gaugeFiles[i].project, 0));
        m_gauges[i].project->CreateScene(s_gaugeFiles[i].scene, 0, 0);
    }
}

void TGiHdGage::OnDraw(RednerDrawArgs &args) const {
    if (lbl_8057D6A0.field_0x148) {
        for (int i = 0; i < 2; i++) {
            m_gauges[i].project->Draw();
        }
    }
}

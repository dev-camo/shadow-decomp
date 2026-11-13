/* TGiHissatu (class name from RTTI): the task that shows the hero and dark "Hissatu" displays of
 * the in-game display, two CSD projects held by TPjsCsd objects and drawn through a draw layer.
 * File name inferred from the class. Method and member names are inferred. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/TGiHissatu.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80079808.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include <cstring>

/* The project, the scene and the three motions of each display, and the value of the word at 0xC
 * of UnkEntry_80178B5C that opens it (name inferred). */
static const struct {
    const char *project;
    const char *scene;
    const char *openMotion;
    const char *shownMotion;
    const char *closeMotion;
    s32 value;
} s_displays[2] = {
    {"CsdFiles/Hissatu_Hero", "HissatuHero", "OpenHero", "Chaos_Control", "CloseHero", 2},
    {"CsdFiles/Hissatu_Dark", "HissatuDark", "OpenDark", "Chaos_Blast", "CloseDark", 1},
};

TGiHissatu::TGiHissatu(Task *parent) : Task(parent) {
    m_name = "TGiHissatu";
    LoadProjects();
    m_layer = fn_8004E24C(fn_80046178()->field_0x38);
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&TGiHissatu::OnDraw, this, _1));
}

/* Matching workaround: the destructor's cleanup loop is in an inline function that the
 * destructor expands. Its name and its form as a member (and not a free function) are inferred;
 * the original has no out-of-line copy of it. The original loads zero into the loop's counter and
 * copies the counter's register into the display offset before the loop. That fits a counter
 * that is a compiler temporary, which the local of an expanded inline function is (other compiler
 * transformations can produce such a temporary too). With the loop written in the destructor the
 * counter is an ordinary local and the build loads zero twice (2 of the destructor's 41 words
 * differ). Revisit when the class's original source is found, or another source form gives the
 * copy. */
inline void TGiHissatu::ReleaseProjects() {
    for (int i = 0; i < 2; i++) {
        m_displays[i].csd.fn_8003E490();
    }
}

TGiHissatu::~TGiHissatu() {
    ReleaseProjects();
}

/* Does nothing when the manager has no entry 0. Otherwise, for each display: when the entry's
 * word equals the display's value and the display is not open, it becomes open and its opening
 * motion and then its shown motion are queued (the first with the flag set, the second with it
 * clear). When the word differs and the display is open, the next queued motion is started, the
 * display stops being open and its shown motion and then its closing motion are queued, both with
 * the flag set. Each display is then updated (fn_8003FE2C). */
void TGiHissatu::Update(float dt) {
    UnkEntry_80178B5C *entry = fn_80178B5C(fn_80079808(), 0);

    if (entry != 0) {
        for (int i = 0; i < 2; i++) {
            if (s_displays[i].value == fn_8016EEC0(entry) && !m_displays[i].open) {
                m_displays[i].open = true;
                m_displays[i].csd.fn_8003E388(s_displays[i].openMotion, true);
                m_displays[i].csd.fn_8003E388(s_displays[i].shownMotion, false);
            } else if (s_displays[i].value != fn_8016EEC0(entry) && m_displays[i].open) {
                m_displays[i].csd.fn_8003FB10();
                m_displays[i].open = false;
                m_displays[i].csd.fn_8003E388(s_displays[i].shownMotion, true);
                m_displays[i].csd.fn_8003E388(s_displays[i].closeMotion, true);
            }

            m_displays[i].csd.fn_8003FE2C();
        }
    }
}

void TGiHissatu::LoadProjects() {
    TPjsCsd *csd;

    for (int i = 0; i < 2; i++) {
        csd = &m_displays[i].csd;
        csd->fn_8003FA08(s_displays[i].project);
        strcpy(csd->m_sceneName, s_displays[i].scene);
        csd->fn_8003E388(s_displays[i].closeMotion, true);
        csd->fn_8003FB10();
    }
}

void TGiHissatu::OnDraw(RednerDrawArgs &args) {
    if (lbl_8057D6A0.field_0x148) {
        for (int i = 0; i < 2; i++) {
            m_displays[i].csd.fn_8003FDF4();
        }
    }
}

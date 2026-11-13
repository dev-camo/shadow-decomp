/* TGiPause (class name from RTTI): the task that shows the pause screen of the in-game display, two
 * CSD projects with a texture dictionary and the folder objects that work on them, drawn through a
 * draw layer. File name inferred from the class; folderPause, folderStoryRoute and
 * folderTargetWindow are defined in files of their own. Method, member and nested type names are
 * inferred. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/TGiPause.h"
#include "Game/TGameInfomation.h"
#include "Game/UnkFileManager_800371D8.h"
#include "Game/UnkFilePath_80051810.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkObject_8003F8D4.h"
#include "Game/UnkObject_800B74B4.h"
#include "Game/UnkObject_800BADE8.h"
#include "Game/folderPause.h"
#include "Game/folderStoryRoute.h"
#include "Game/folderTargetWindow.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "rw/rwcore.h"
#include <cstdio>

TGiPause::TGiPause(Task *parent) : Task(parent) {
    m_name = "TGiPause";
    m_flags |= 0x100;
    m_texDict = 0;

    if (LoadProjects()) {
        m_layer = fn_8004E24C(fn_80046178()->field_0x38);
        m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&TGiPause::OnDraw, this, _1));
        m_pause = boost::shared_ptr<folderPause>(new folderPause(&m_projects.pause));
        m_storyRoute = boost::shared_ptr<folderStoryRoute>(new folderStoryRoute(&m_projects.pause));
        m_targetWindow =
            boost::shared_ptr<folderTargetWindow>(new folderTargetWindow(&m_projects.targetWindow));
    }
}

TGiPause::~TGiPause() {
    if (m_texDict != 0) {
        fn_8048CB38(m_texDict);
        m_texDict = 0;
    }
}

/* Without loaded projects the task sets bit 0x1 of m_flags (a finished task, inferred). Otherwise
 * it updates the three folders and, when the object that lbl_805EF9F4 points to exists, sets its
 * byte at 0x4D to 0 when the folderPause or the folderStoryRoute update returned true, otherwise
 * to 1. */
void TGiPause::Update(float dt) {
    if (m_projects.loaded) {
        int count = m_pause->fn_800B9D04();

        count += m_storyRoute->fn_800BADF0();
        m_targetWindow->fn_800BC0C0();

        if (lbl_805EF9F4 != 0) {
            if (count != 0) {
                lbl_805EF9F4->field_0x4D = 0;
            } else {
                lbl_805EF9F4->field_0x4D = 1;
            }
        }
    } else {
        m_flags |= 1;
    }
}

/* The project is "CsdFiles/pause/pauseBoss" when byte 0x9 of lbl_8057D6A0.field_0xA8 or
 * lbl_8057D6A0's byte at 0x125 is set, or when the record that fn_80176C3C returns for the number
 * in lbl_8057D6A0.field_0xA8 has bit 0x10 or bit 0x4 set; otherwise it is
 * "CsdFiles/pause/pause". */
const char *TGiPause::GetPauseProjectPath() {
    static char path[32];

    if (lbl_8057D6A0.field_0xA8.field_0x9 || lbl_8057D6A0.field_0x125 ||
        (fn_80176C3C(fn_800B74B4(), -1)->field_0x8 & 0x10) ||
        (fn_80176C3C(fn_800B74B4(), -1)->field_0x8 & 0x4)) {
        sprintf(path, "CsdFiles/pause/pauseBoss");
    } else {
        sprintf(path, "CsdFiles/pause/pause");
    }

    return fn_800423B8(fn_8003F8D4(), path, 0);
}

/* The texture dictionary is "CsdFiles/pause/pauseBoss" under the same conditions as the project
 * (GetPauseProjectPath), otherwise "CsdFiles/pause/pause" followed by the number in
 * lbl_8057D6A0.field_0xA8 in four digits, with the language suffix and ".txd" that fn_800C02CC
 * adds. */
bool TGiPause::LoadProjects() {
    bool loaded = false;

    if (m_projects.pause.LoadFile(GetPauseProjectPath()) &&
        m_projects.targetWindow.LoadFile(fn_800423B8(fn_8003F8D4(), "CsdFiles/TWindow", 0))) {
        loaded = true;
    }

    m_projects.loaded = loaded;

    if (m_projects.loaded) {
        char name[32];

        if (lbl_8057D6A0.field_0xA8.field_0x9 || lbl_8057D6A0.field_0x125 ||
            (fn_80176C3C(fn_800B74B4(), -1)->field_0x8 & 0x10) ||
            (fn_80176C3C(fn_800B74B4(), -1)->field_0x8 & 0x4)) {
            sprintf(name, "CsdFiles/pause/pauseBoss");
        } else {
            sprintf(name, "CsdFiles/pause/pause%04d", lbl_8057D6A0.field_0xA8.field_0x0);
        }

        if (LoadTextures(fn_800C02CC(name))) {
            /* Matching workaround: two statements stand for a call of a static member function
             * through the object (fn_8003F8D4()->fn_800421BC(...), inferred): the original calls
             * fn_8003F8D4, discards the result and passes no object to fn_800421BC. Its class is
             * only declared (layout and size unknown), so it cannot have the member here. Revisit
             * when UnkObject_8003F8D4 is defined. */
            fn_8003F8D4();
            fn_800421BC(&m_projects.pause, m_texDict);
            fn_8003F8D4();
            fn_800421BC(&m_projects.targetWindow, m_texDict);
        }
    }

    return m_projects.loaded != 0;
}

bool TGiPause::LoadTextures(const char *path) {
    char *checkedPath = fn_80051810(path);
    bool loaded = false;

    if (fn_800371D8()->field_0x4(checkedPath)) {
        u32 size;
        void *data = fn_8004509C(fn_800371D8(), path, &size);

        if (data != 0) {
            RwMemory memory;
            RwStream *stream;

            memory.start = static_cast<RwUInt8 *>(data);
            memory.length = size;
            stream = fn_80481368(rwSTREAMMEMORY, rwSTREAMREAD, &memory);

            if (stream != 0) {
                if (fn_8047CF50(stream, rwID_TEXDICTIONARY, 0, 0)) {
                    m_texDict = fn_80483FBC(stream);
                    loaded = true;
                }
                fn_8048124C(stream, &memory);
            }

            fn_80049B34(data);
        }
    }

    fn_80051830(checkedPath);
    return loaded;
}

void TGiPause::OnDraw(RednerDrawArgs &args) {
    if (lbl_8057D6A0.field_0x148) {
        m_projects.targetWindow.Draw();
        m_targetWindow->fn_800BBF9C();
        m_projects.pause.Draw();
        m_pause->fn_800B9B2C();
    }
}

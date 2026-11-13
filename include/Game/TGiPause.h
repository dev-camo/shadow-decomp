#ifndef GAME_TGIPAUSE_H
#define GAME_TGIPAUSE_H

#include "Game/CSD/csdmProject.h"
#include "Game/Task.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class folderPause;
class folderStoryRoute;
class folderTargetWindow;
struct RwTexDictionary;

/* TGiPause (class name from RTTI): the task of the in-game display that shows the pause screen. It
 * loads the CSD projects of the pause screen and of the target window with a texture dictionary
 * for both, and drives the folderPause, folderStoryRoute and folderTargetWindow objects that work
 * on them; a draw layer draws the projects and two of the folders. TGameInfomation's constructor
 * creates it. Member and method names are inferred. */
class TGiPause : public Task {
  public:
    TGiPause(Task *parent);
    virtual ~TGiPause();
    virtual void Update(float dt);

    /* Loads the two project files and, when both load, the texture dictionary of the pause screen,
     * which it gives to both projects. Returns whether both project files loaded. */
    bool LoadProjects();
    /* Returns the project file name of the pause screen (through fn_800423B8). */
    const char *GetPauseProjectPath();
    /* Reads the texture dictionary in the file at path into m_texDict when the file manager's
     * check accepts the path. Returns whether it read one. */
    bool LoadTextures(const char *path);
    /* The draw layer's callback: when lbl_8057D6A0's byte at 0x148 is set, draws the target
     * window project and calls the folderTargetWindow's fn_800BBF9C, then draws the pause project
     * and calls the folderPause's fn_800B9B2C. */
    void OnDraw(RednerDrawArgs &args);

    /* The struct's name and its nesting in TGiPause are inferred: the original has an out-of-line
     * constructor of a 0x9C-byte member at 0x28 that constructs the two projects. */
    struct Projects {
        /* 0x00 */ int loaded;                /* 1 when both project files loaded, otherwise 0 */
        /* 0x04 */ Chao::CSD::CProject pause; /* "CsdFiles/pause/pause..." */
        /* 0x50 */ Chao::CSD::CProject targetWindow; /* "CsdFiles/TWindow" */
    };

    /* 0x28 */ Projects m_projects;
    /* 0xC4 */ boost::shared_ptr<folderPause> m_pause;
    /* 0xCC */ boost::shared_ptr<folderStoryRoute> m_storyRoute;
    /* 0xD4 */ boost::shared_ptr<folderTargetWindow> m_targetWindow;
    /* 0xDC */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0xE4 */ RwTexDictionary *m_texDict;
};
CHECK_SIZEOF(TGiPause::Projects, 0x9C);
CHECK_SIZEOF(TGiPause, 0xE8);

#endif

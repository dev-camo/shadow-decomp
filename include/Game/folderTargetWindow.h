#ifndef GAME_FOLDERTARGETWINDOW_H
#define GAME_FOLDERTARGETWINDOW_H

#include <dolphin/types.h>
#include "layout_check.h"

namespace Chao {
namespace CSD {
class CProject;
} // namespace CSD
} // namespace Chao

/* folderTargetWindow (class name from RTTI: the type records of
 * boost::checked_deleter<folderTargetWindow> and of its counted-pointer record in TGiPause's file):
 * a part of the pause screen that works on TGiPause's target window project ("CsdFiles/TWindow").
 * TGiPause creates it, updates it every frame and calls it from its draw callback. Its functions
 * are between 0x800BBF9C and 0x800BC7BC in the USA release, in a file of its own that has no source
 * yet; only what TGiPause uses is declared, its layout is not mapped (the size is TGiPause's
 * allocation) and the method names are placeholders. The parameter and return types are inferred
 * from TGiPause's calls. */
class folderTargetWindow {
  public:
    folderTargetWindow(Chao::CSD::CProject *project);
    ~folderTargetWindow();

    /* TGiPause's update calls it every frame and ignores the result. */
    bool fn_800BC0C0();
    /* TGiPause's draw callback calls it after the target window project's Draw. */
    void fn_800BBF9C();

    /* 0x000 */ u8 field_0x0[0x51C];
};
CHECK_SIZEOF(folderTargetWindow, 0x51C);

#endif

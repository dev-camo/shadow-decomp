#ifndef GAME_FOLDERTARGETWINDOW_H
#define GAME_FOLDERTARGETWINDOW_H

#include <dolphin/types.h>
#include "Game/CSD/csdmProject.h"
#include "layout_check.h"

/* folderTargetWindow (class name from RTTI: the type records of
 * boost::checked_deleter<folderTargetWindow> and of its counted-pointer record in TGiPause's file):
 * a part of the pause screen that works on TGiPause's target window project ("CsdFiles/TWindow").
 * TGiPause creates it, updates it every frame and calls it from its draw callback. Its functions
 * are between 0x800BBF9C and 0x800BC7BC in the USA release, in a file of its own that has no source
 * yet; only what TGiPause uses is declared, only the member at 0x000 is mapped (the size is
 * TGiPause's allocation) and the method names are placeholders. The parameter types are inferred
 * from TGiPause's calls. */
class folderTargetWindow {
  public:
    folderTargetWindow(Chao::CSD::CProject *project);
    ~folderTargetWindow();

    /* TGiPause's update calls it every frame and ignores the result. The function adds the byte
     * results of its two fn_8003D914 calls and returns the sum normalised to 0 or 1
     * (0x800BC348-0x800BC34C in the USA release), hence the bool (inferred). */
    bool fn_800BC0C0();
    /* TGiPause's draw callback calls it after the target window project's Draw. */
    void fn_800BBF9C();

    /* The constructor constructs it with fn_800435FC and the destructor destroys it with
     * RCPtr<CSceneList>'s destructor (the call at 0x800BC668 in the USA release). */
    /* 0x000 */ Chao::CSD::RCPtr<Chao::CSD::CSceneList> field_0x0;
    /* 0x004 */ u8 field_0x4[0x51C - 0x4];

  private:
    /* Reconstruction safeguards: the native copy contract is unknown, so copying and assignment
     * are declared private and left undefined. */
    folderTargetWindow(const folderTargetWindow &);
    folderTargetWindow &operator=(const folderTargetWindow &);
};
CHECK_SIZEOF(folderTargetWindow, 0x51C);

#endif

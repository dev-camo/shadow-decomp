#ifndef GAME_FOLDERSTORYROUTE_H
#define GAME_FOLDERSTORYROUTE_H

#include <dolphin/types.h>
#include "Game/CSD/csdmProject.h"
#include "layout_check.h"

/* folderStoryRoute (class name from RTTI: the type records of
 * boost::checked_deleter<folderStoryRoute> and of its counted-pointer record in TGiPause's file): a
 * part of the pause screen that works on TGiPause's pause project. TGiPause creates it and updates
 * it every frame. Its functions are between 0x800BADF0 and 0x800BBF9C in the USA release, in a file
 * of its own that has no source yet; only what TGiPause uses is declared, only the member at 0x04
 * is mapped (the size is TGiPause's allocation) and the method name is a placeholder. The
 * parameter and return types are inferred from TGiPause's calls. */
class folderStoryRoute {
  public:
    folderStoryRoute(Chao::CSD::CProject *project);
    ~folderStoryRoute();

    /* TGiPause's update calls it every frame and counts a true result (meaning unknown). */
    bool fn_800BADF0();

    /* 0x000 */ u8 field_0x0[0x4];
    /* The constructor constructs it with fn_800435FC and the destructor destroys it with
     * RCPtr<CSceneList>'s destructor (the call at 0x800BBBD8 in the USA release). */
    /* 0x004 */ Chao::CSD::RCPtr<Chao::CSD::CSceneList> field_0x4;
    /* 0x008 */ u8 field_0x8[0x150 - 0x8];

  private:
    /* Reconstruction safeguards: the native copy contract is unknown, so copying and assignment
     * are declared private and left undefined. */
    folderStoryRoute(const folderStoryRoute &);
    folderStoryRoute &operator=(const folderStoryRoute &);
};
CHECK_SIZEOF(folderStoryRoute, 0x150);

#endif

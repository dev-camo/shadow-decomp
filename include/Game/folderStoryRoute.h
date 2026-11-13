#ifndef GAME_FOLDERSTORYROUTE_H
#define GAME_FOLDERSTORYROUTE_H

#include <dolphin/types.h>
#include "layout_check.h"

namespace Chao {
namespace CSD {
class CProject;
} // namespace CSD
} // namespace Chao

/* folderStoryRoute (class name from RTTI: the type records of
 * boost::checked_deleter<folderStoryRoute> and of its counted-pointer record in TGiPause's file): a
 * part of the pause screen that works on TGiPause's pause project. TGiPause creates it and updates
 * it every frame. Its functions are between 0x800BADF0 and 0x800BBF9C in the USA release, in a file
 * of its own that has no source yet; only what TGiPause uses is declared, its layout is not mapped
 * (the size is TGiPause's allocation) and the method name is a placeholder. The parameter and
 * return types are inferred from TGiPause's calls. */
class folderStoryRoute {
  public:
    folderStoryRoute(Chao::CSD::CProject *project);
    ~folderStoryRoute();

    /* TGiPause's update calls it every frame and counts a true result (meaning unknown). */
    bool fn_800BADF0();

    /* 0x000 */ u8 field_0x0[0x150];
};
CHECK_SIZEOF(folderStoryRoute, 0x150);

#endif

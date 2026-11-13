#ifndef GAME_FOLDERPAUSE_H
#define GAME_FOLDERPAUSE_H

#include <dolphin/types.h>
#include "Game/CSD/csdmProject.h"
#include "layout_check.h"

/* folderPause (class name from RTTI: the type records of boost::checked_deleter<folderPause> and of
 * its counted-pointer record in TGiPause's file): a part of the pause screen that works on
 * TGiPause's pause project. TGiPause creates it, updates it every frame and calls it from its draw
 * callback. Its functions are between 0x800B9B2C and 0x800BADE8 in the USA release, in a file of
 * its own that has no source yet; only what TGiPause uses is declared, only the member at 0x08 is
 * mapped (the size is TGiPause's allocation) and the method names are placeholders. The parameter
 * and return types are inferred from TGiPause's calls. */
class folderPause {
  public:
    folderPause(Chao::CSD::CProject *project);
    ~folderPause();

    /* TGiPause's update calls it every frame and counts a true result (meaning unknown). */
    bool fn_800B9D04();
    /* TGiPause's draw callback calls it after the pause project's Draw. */
    void fn_800B9B2C();

    /* 0x00 */ u8 field_0x0[0x8];
    /* The constructor constructs it with fn_800435FC and the destructor destroys it with
     * RCPtr<CSceneList>'s destructor (the call at 0x800BACC4 in the USA release). */
    /* 0x08 */ Chao::CSD::RCPtr<Chao::CSD::CSceneList> field_0x8;
    /* 0x0C */ u8 field_0xC[0xCC - 0xC];

  private:
    /* Reconstruction safeguards: the native copy contract is unknown, so copying and assignment
     * are declared private and left undefined. */
    folderPause(const folderPause &);
    folderPause &operator=(const folderPause &);
};
CHECK_SIZEOF(folderPause, 0xCC);

#endif

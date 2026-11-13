#ifndef GAME_TGIHISSATU_H
#define GAME_TGIHISSATU_H

#include "Game/TPjsCsd.h"
#include "Game/Task.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* TGiHissatu (class name from RTTI): the task of the in-game display that shows the CSD projects
 * Hissatu_Hero and Hissatu_Dark through a draw layer. Each update compares the word at 0xC of
 * entry 0 of UnkManager_80079808's table with each display's value (2 for the hero display, 1
 * for the dark one) and plays the display's opening or closing motions when that starts or stops
 * matching. TGameInfomation's constructor creates it. Member and method names are inferred. */
class TGiHissatu : public Task {
  public:
    TGiHissatu(Task *parent);
    virtual ~TGiHissatu();
    virtual void Update(float dt);

    /* Loads each display's project, names its scene and plays its closing motion. */
    void LoadProjects();
    /* Releases each display's project. Matching workaround: see its definition. */
    void ReleaseProjects();
    /* The draw layer's callback: while lbl_8057D6A0's byte at 0x148 is set, calls each display's
     * draw routine (fn_8003FDF4), which has its own checks: it draws only when the display has a
     * project and the byte at 0x805E3780 (USA) is set. */
    void OnDraw(RednerDrawArgs &args);

    struct Display {
        /* 0x00 */ TPjsCsd csd;
        /* 0x54 */ bool open; /* set while the entry's word equals the display's value */
    };

    /* 0x28 */ Display m_displays[2];
    /* 0xD8 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
};
CHECK_SIZEOF(TGiHissatu, 0xE0);

#endif

#ifndef GAME_TPJSEVENTACTIONFADE_H
#define GAME_TPJSEVENTACTIONFADE_H

#include "Game/Task.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "boost/shared_ptr.hpp"
#include "rw/rwcore.h"
#include "layout_check.h"

/* TPjsEventActionFade (class name from RTTI): a task that the event code creates through
 * fn_80328DDC. What it does depends on the kind its constructor receives (behaviour inferred):
 * - kind 1: the constructor passes 1/6 to fn_802051F8 and the colour (255, 255, 255, 255) to
 *   fn_80204874 and calls fn_80173878(1.0f, 0, 1). Update waits until fn_80044E10 returns 1, then
 *   calls fn_80173834(0.0f, 1); at a later update, once fn_80044E10 returns 0, it marks the task
 *   finished.
 * - kind 2: the constructor makes the same calls with 1/12 and the colour (0, 0, 0, 0); the first
 *   Update marks the task finished.
 * - kind 3: the constructor creates a draw layer whose callback, OnDraw, fills the rectangle from
 *   (0, 0) with the size at 0x44 and 0x48 of lbl_8057D6A0 with the colour (255, 128, 0, 64); Update
 *   does nothing.
 * - any other kind: the constructor does nothing more and the first Update marks the task finished.
 * While the file's stop flag is set (fn_80328618), Update marks a task of any kind but 3 finished
 * before anything else. The destructor calls fn_803285EC for every kind but 3. Member and method
 * names are inferred. */
class TPjsEventActionFade : public Task {
  public:
    TPjsEventActionFade(Task *parent, int kind);
    virtual ~TPjsEventActionFade();
    virtual void Update(float dt);

    /* The draw layer's callback: when the kind is 3, fills m_rect with the colour
     * (255, 128, 0, 64) through fn_800396D0, without a texture (name inferred). */
    void OnDraw(RednerDrawArgs &args);

    /* 0x28 */ int m_kind;
    /* The value that kinds 1 and 2 pass to fn_802051F8; the other kinds leave it unset. */
    /* 0x2C */ float field_0x2C;
    /* Set by the constructor. Kind 1's Update clears it at the update in which fn_80044E10
     * returns 1 and it calls fn_80173834. */
    /* 0x30 */ bool field_0x30;
    /* Set only by kind 3. */
    /* 0x34 */ RwRect m_rect;
    /* Kind 3's draw layer; null for the other kinds. */
    /* 0x44 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* Unclassified storage in the original 0x50-byte allocation; no accesses are identified. */
    /* 0x4C */ unsigned char field_0x4C[4];
};
CHECK_SIZEOF(TPjsEventActionFade, 0x50);

extern "C" {
/* Unless the stop flag is set, creates a TPjsEventActionFade of the given kind under the event
 * action manager, when there is one (behaviour inferred from its body). */
void fn_80328DDC(int kind);
/* Sets the stop flag, which makes fn_80328DDC do nothing and Update finish the tasks of every kind
 * but 3, then calls fn_803285EC (behaviour inferred from its body). */
void fn_80328618(void);
/* Clears the stop flag. */
void fn_80328640(void);
/* Passes 1.0f to fn_802051F8 and the colour (0, 0, 0, 0) to fn_80204874. */
void fn_803285EC(void);
}

#endif

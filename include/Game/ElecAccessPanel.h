#ifndef GAME_ELECACCESSPANEL_H
#define GAME_ELECACCESSPANEL_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_802B4254.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* ElecAccessPanelData (class name from RTTI, derived from SetAdapterState): what the owner keeps
 * of an ElecAccessPanel between its creations. The panel's constructor creates it with 0 and then
 * stores the state the panel starts in (0 or 3), and Activate stores 3; when the next constructor
 * finds 3, it sets the state of the com id to 1 (behaviour inferred). */
class ElecAccessPanelData : public SetAdapterState {
  public:
    /* 0x04 */ s32 m_state;
};
CHECK_SIZEOF(ElecAccessPanelData, 0x8);

/* ElecAccessPanel (class name from RTTI; the gadget's descriptor is "ELEC_ACCESS"). Member names
 * and the names of the non-virtual methods are inferred from the code. */
class ElecAccessPanel : public Gadget {
  public:
    ElecAccessPanel(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecAccessPanel();
    virtual void UpdateBody(float dt);
    /* Calls UpdateModelMatrix, then the model's fn_80162EF4 and fn_8016108C (purpose unknown). */
    virtual void fn_800CBC1C();
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and com id from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position and y rotation, and the object 34 units
     * above that position (name inferred). */
    void UpdateModelMatrix();
    /* The model's collision callback: passes the hits to fn_8015AA84 with the hit callback and
     * the address of a pointer to the panel (name inferred). */
    void OnCollision(ColliDetectArgs &args);
    /* Moves the panel to state 1 with m_timer 0: calls the part callback with 1 for the model's
     * parts, sets the colour of the "mat" atomic state to 0, stores 3 in the owner's
     * ElecAccessPanelData and passes the owner to fn_800C985C (name and behaviour inferred). */
    void Activate();

    static const char *s_name;

    /* 0x30 */ GadgetModelInfo *m_model;
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ u8 m_comId;         /* the owner's com id (fn_800C9E60) */
    /* 0x40 */ u8 field_0x40[0x4]; /* in the allocation size (0x58); no code here touches it */
    /* State 0 waits for a hit, state 1 calls the model's fn_80161880 and moves to state 2, state
     * 2 fades the "mat" colour over half a second of m_timer and state 3 passes sound 0x7249 to
     * fn_801D53C4 every frame (inferred). */
    /* 0x44 */ s32 m_state;
    /* 0x48 */ float m_timer;
    /* 0x4C */ boost::shared_ptr<UnkObject_802B4254> m_object;
    /* 0x54 */ u8 field_0x54[0x4]; /* in the allocation size (0x58); no code here touches it */
};
CHECK_SIZEOF(ElecAccessPanel, 0x58);

#endif

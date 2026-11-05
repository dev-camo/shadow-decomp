#ifndef GAME_DASHPANEL_H
#define GAME_DASHPANEL_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_801695B8.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace DashPanel {

/* The gadget parameters in the order of the descriptor's table. */
struct DashPanelParams {
    DashPanelParams() : level(0.0f), noControlTime(0.0f) {}

    /* 0x0 */ float level;         /* 0 to 15: selects the speed and the push factor */
    /* 0x4 */ float noControlTime; /* seconds, passed with the push command */
};
CHECK_SIZEOF(DashPanelParams, 0x8);

/* DashPanel::DashPanel (class name from RTTI; the gadget's descriptor is "DASHPANEL"): a panel
 * that, when a character touches the "col" node of its model, sends the character commands that
 * cancel its behaviour, clear its velocity, set it 5 units above the panel, turn it along the
 * panel and launch it along the panel's direction with a speed and a push that the level selects
 * (behaviour inferred). Member and method names are inferred. */
class DashPanel : public Gadget {
  public:
    DashPanel(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~DashPanel();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and parameters from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info with its rotation about the y, x and z axes (name
     * inferred). */
    void UpdateModelMatrix();
    /* The model's collision callback: passes the hits to fn_8015B004 with the hit callback and a
     * hit context (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x48 */ DashPanelParams m_params;
};
CHECK_SIZEOF(DashPanel, 0x50);

} // namespace DashPanel

#endif

#ifndef GAME_FLEETHOLDER_H
#define GAME_FLEETHOLDER_H

#include "Game/Gadget.h"
#include "boost/shared_ptr.hpp"

class EggFleet;

/* Owner parameter block; member names inferred from its original descriptor labels. */
struct FleetHolderParams {
    /* 0x00 */ u32 kind;
    /* 0x04 */ float rangeR;
    /* 0x08 */ float rangeHeight;
    /* 0x0C */ float moveLength;
    /* 0x10 */ float moveSpeed;
    /* 0x14 */ float fireTiming;
};
CHECK_SIZEOF(FleetHolderParams, 0x18);

/* Class name from RTTI; member and operation names are inferred. */
class FleetHolder : public Gadget {
  public:
    FleetHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~FleetHolder();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    void fn_800CBD58(UnkGadgetOwner_800C9CD0 *owner);
    void fn_800CBC38(UnkGadgetOwner_800C9CD0 *owner);
    static void Create(UnkGadgetOwner_800C9CD0 *owner);
    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<EggFleet> field_0x30;
    /* 0x38 */ FleetHolderParams m_params;
};
CHECK_SIZEOF(FleetHolder, 0x50);

#endif

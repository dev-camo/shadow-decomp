#ifndef GAME_ENERGYCORE_H
#define GAME_ENERGYCORE_H

#include "Game/Gadget.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/scoped_ptr.hpp"
#include "layout_check.h"

namespace EnergyCore {

class EnergyCore;

/* EnergyCore::EnergyCoreTask (class name from RTTI): an energy core created at run time by the
 * caller at 0x8033E95C through Create. It ends itself once its core has been touched, or once
 * nothing fn_80169BE4 checks is closer to m_position than the constructor's range (it receives
 * m_rangeSquared; behaviour inferred). Member and method names are inferred. */
class EnergyCoreTask : public Task {
  public:
    EnergyCoreTask(int type, Task *parent, Fn8040Vector3 *position, UnkRotation3 *rotation,
                   float range);
    virtual ~EnergyCoreTask();
    virtual void Update(float dt);

    /* Creates the task under task layer 11 (name inferred). */
    static void Create(int type, Fn8040Vector3 *position, UnkRotation3 *rotation, float range);

    static const char *s_name;

    /* 0x28 */ boost::scoped_ptr<EnergyCore> m_core;
    /* 0x2C */ Fn8040Vector3 m_position;
    /* 0x38 */ UnkRotation3 m_rotation;
    /* 0x44 */ float m_rangeSquared; /* the square of the constructor's range */
};
CHECK_SIZEOF(EnergyCoreTask, 0x48);

/* EnergyCore::SetEnergyCoreTask (class name from RTTI; the gadget's descriptor is "ENERGY_CORE"):
 * the placed energy core. It reads its position, rotation and parameter from its owner and, once its
 * core has been touched, plays effect 0x2A1 or 0x2A5 (by type) at its position and finishes
 * (behaviour inferred). Member and method names are inferred. */
class SetEnergyCoreTask : public Gadget {
  public:
    SetEnergyCoreTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetEnergyCoreTask();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameter, when there is an owner (name
     * inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
        if (owner != 0) {
            fn_800C9F14(owner, &m_position);
            fn_800C9ED4(owner, &m_rotation);
            fn_800C9DBC(owner, &m_type, sizeof(m_type));
        }
    }
    /* Passes the position and rotation to the core (name inferred). */
    void UpdateTransform();

    static const char *s_name;

    /* 0x30 */ boost::scoped_ptr<EnergyCore> m_core;
    /* 0x34 */ Fn8040Vector3 m_position;
    /* 0x40 */ UnkRotation3 m_rotation;
    /* 0x4C */ s32 m_type; /* the parameter "type": selects the core's effect and item */
};
CHECK_SIZEOF(SetEnergyCoreTask, 0x50);

} // namespace EnergyCore

#endif

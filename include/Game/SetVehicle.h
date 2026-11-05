#ifndef GAME_SETVEHICLE_H
#define GAME_SETVEHICLE_H

#include "Game/Gadget.h"
#include "Game/UnkIdList_8003A154.h"
#include "Game/VehicleTask.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace SetVehicle {

/* SetVehicle::SetVehicleTask (class name from RTTI; the gadget's descriptor is "SET_VEHICLE"): the
 * gadget that places a vehicle of the type its "Vehicle" parameter selects. Member names and the
 * names of the non-virtual methods are inferred from the code. */
class SetVehicleTask : public Gadget {
  public:
    SetVehicleTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetVehicleTask();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation, vehicle type and com id from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
        fn_800C9F14(owner, &m_position);
        fn_800C9ED4(owner, &m_rotation);
        fn_800C9DBC(owner, &m_vehicleType, sizeof(m_vehicleType));
        fn_800C9E60(owner, &m_comId);
    }
    /* Forgets the vehicle task when it has finished, or when both of the vehicle's queries
     * fn_801B5AA4 and fn_801B5AD4 are true; once there is no vehicle, tells the owner with
     * fn_800C9AD4 (name and behaviour inferred). */
    void CheckVehicle() {
        if (m_vehicle != 0) {
            if (!fn_8004EAC0(m_vehicle)) {
                m_vehicle = 0;
            } else if (m_vehicle->fn_801B5AA4() && m_vehicle->fn_801B5AD4()) {
                m_vehicle = 0;
            }

            if (m_vehicle == 0 && m_owner != 0) {
                fn_800C9AD4(m_owner);
            }
        }
    }
    /* Marks the vehicle task finished and forgets it (name inferred). */
    void ReleaseVehicle();
    /* Creates the vehicle unless the com id check fails or a vehicle of the type exists, gives it
     * the owner's range and sets m_vehicleCreated (name inferred). */
    void CreateVehicle();
    /* Returns false when the com id is set and its state is not 1; the vehicle is then released
     * and the owner told with fn_800C9AD4 (name and behaviour inferred). */
    bool CheckComId();

    /* 0x30 */ Vehicle::VehicleTask *m_vehicle;
    /* 0x34 */ s32 m_vehicleType; /* the "Vehicle" parameter */
    /* 0x38 */ Fn8040Vector3 m_position;
    /* 0x44 */ Fn8040Vector3 m_rotation;
    /* Built for m_position; the vehicle is not created while fn_80039EDC is true for it. */
    /* 0x50 */ boost::shared_ptr<UnkIdList_8003A154> m_idList;
    /* Set once a vehicle of an available type has been created, or when the constructor's com id
     * check fails; from then on UpdateBody only watches the vehicle (inferred). */
    /* 0x58 */ u8 m_vehicleCreated;
    /* 0x59 */ u8 m_comId;
    /* 0x5C */ u32 field_0x5C; /* never accessed; Create allocates 0x60 bytes */
};
CHECK_SIZEOF(SetVehicle::SetVehicleTask, 0x60);

} // namespace SetVehicle

#endif

#ifndef GAME_VEHICLETASK_H
#define GAME_VEHICLETASK_H

#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace Vehicle {

class TVehicleBase;

/* Vehicle::VehicleTask (class name from RTTI, with Task as its only base): the task that runs a
 * vehicle. The layout comes from the constructor at 0x801B5DF8; only the methods that the
 * SET_VEHICLE gadget calls are declared (placeholder names, behaviour inferred from their
 * bodies). */
class VehicleTask : public Task {
  public:
    virtual ~VehicleTask();
    virtual void Update(float dt);

    /* Returns the vehicle's type (fn_801B6938 of the vehicle). */
    int fn_801B5B08();
    /* Stores value in field_0x30. */
    void fn_801B5D1C(float value);
    /* Passes position and rotation to a virtual function of the vehicle. */
    void fn_801B5D24(const Fn8040Vector3 *position, const Fn8040Vector3 *rotation);
    /* Returns the result of another virtual function of the vehicle. */
    bool fn_801B5AA4();
    /* Returns true unless the weak pointer at 0x80 of the vehicle has expired. */
    bool fn_801B5AD4();

    /* 0x28 */ boost::shared_ptr<TVehicleBase> m_vehicle; /* name inferred */
    /* 0x30 */ float field_0x30;
    /* 0x34 */ u32 field_0x34; /* not seen used; the creator allocates 0x38 bytes */
};
CHECK_SIZEOF(Vehicle::VehicleTask, 0x38);

} // namespace Vehicle

/* The vehicle types are looked up in an eight-byte table (the first word points to the type's
 * record, the byte at 0x4 is a flag); the original function names are unknown (placeholder names,
 * behaviour inferred from their bodies). */
extern "C" {
/* Returns the name at the start of the type's record, or an empty string when the type has none. */
const char *fn_801B4B34(int type);
/* Returns the type's flag byte; fn_801B48D4 creates nothing when it is 0. */
bool fn_801B4B5C(int type);
/* Creates a vehicle of the type with the record's factory and a VehicleTask for it under task layer
 * 12, and returns the task, or null when the type is not available. */
Vehicle::VehicleTask *fn_801B48D4(int type);
}

#endif

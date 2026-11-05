/* SetVehicle::SetVehicleTask (class name from RTTI; the gadget's descriptor is "SET_VEHICLE"): a
 * gadget that creates a vehicle of the type its "Vehicle" parameter selects at its position, unless
 * its com id is set and the state of that com id is not 1, and finishes once the vehicle is gone
 * (behaviour inferred). File name inferred from the class's namespace. Method names marked
 * "(name inferred)" are not original. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first). */
#include <cstring>
#include "Game/SetVehicle.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/VehicleTask.h"

using Vehicle::VehicleTask;

namespace SetVehicle {

SetVehicleTask::SetVehicleTask(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_vehicle(0), m_rotation(0.0f, 0.0f, 0.0f), m_vehicleCreated(0) {
    m_name = "SetVehicle";
    ReadOwner(owner);
    m_idList = fn_8003A154(&m_position);

    if (!CheckComId()) {
        m_vehicleCreated = 1;
    }
}

SetVehicleTask::~SetVehicleTask() {
    ReleaseVehicle();
}

void SetVehicleTask::UpdateBody(float dt) {
    if (m_vehicleCreated == 0) {
        if (m_idList.get() == 0 || !fn_80039EDC(m_idList)) {
            CreateVehicle();
            if (m_vehicle != 0) {
                m_vehicle->fn_801B5D24(&m_position, &m_rotation);
            }
        }
    } else {
        CheckVehicle();
        if (m_vehicle == 0) {
            m_flags |= 1;
        }
    }
}

void SetVehicleTask::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    CreateVehicle();
    if (m_vehicle != 0) {
        m_vehicle->fn_801B5D24(&m_position, &m_rotation);
    }
    CheckVehicle();
}

bool SetVehicleTask::CheckComId() {
    u8 comId = m_comId;

    if (comId != 0 && fn_8016904C(fn_800CB69C(), comId) != 1) {
        ReleaseVehicle();
        if (m_owner != 0) {
            fn_800C9AD4(m_owner);
        }
        return false;
    }

    return true;
}

void SetVehicleTask::CreateVehicle() {
    if (!CheckComId()) {
        return;
    }

    if (m_vehicle != 0) {
        if (m_vehicle->fn_801B5B08() == m_vehicleType) {
            return;
        }
        ReleaseVehicle();
    }

    int type = m_vehicleType;

    if (fn_801B4B5C(type)) {
        m_vehicle = fn_801B48D4(type);
        if (m_vehicle != 0) {
            m_vehicle->fn_801B5D1C(fn_800C9938(m_owner));
        }
        m_vehicleCreated = 1;
    }
}

void SetVehicleTask::ReleaseVehicle() {
    if (m_vehicle != 0) {
        m_vehicle->m_flags |= 1;
        m_vehicle = 0;
    }
}

} // namespace SetVehicle

extern "C" void fn_8011E178(void) {}

extern "C" void fn_8011E174(void) {}

void SetVehicle::SetVehicleTask::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new SetVehicleTask(fn_801EB304(fn_8001C4F8(), 11), owner);
}

/* The label callback of the "Vehicle" parameter: the name of the vehicle type the value selects,
 * or "Not Available" when that name is empty (behaviour inferred). */
static const char *fn_8011E0A0(UnkParamCursor_800CBC20 *cursor) {
    static const char *s_notAvailable = "Not Available";

    if (cursor->m_index == 0) {
        const char *name = fn_801B4B34(cursor->m_value);

        if (strcmp(name, "") == 0) {
            return s_notAvailable;
        }
        return name;
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "Vehicle", 1, 1, 13, 0.0f, 0.0f, 0.0f, fn_8011E0A0, 0},
    {0},
};

GadgetTypeInfo lbl_80527670 = {
    "SET_VEHICLE",
    fn_8011E178,
    fn_8011E174,
    SetVehicle::SetVehicleTask::Create,
    0,
    0,
    0x4F,
    1,
    0,
    10,
    s_paramInfo,
};

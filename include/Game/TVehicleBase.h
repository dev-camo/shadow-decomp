#ifndef GAME_TVEHICLEBASE_H
#define GAME_TVEHICLEBASE_H

#include "Game/CharInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/scoped_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommand;

namespace Vehicle {

/* The 0x2C-byte object that TVehicleBase's constructor allocates and constructs with the vehicle
 * type (0x801B7338) and keeps at 0x98 (placeholder name). fn_801B6504 stores a float at its 0x8,
 * fn_801B6254 clears the float at its 0x28, and fn_801B6958 stores the vehicle at its 0xC. Its
 * layout is not mapped. */
class UnkVehicleState_801B7338;

/* Vehicle::TVehicleBase (class name from RTTI, with CharInfo as its only base): the 0x9C-byte base
 * class of the vehicles. The layout comes from its constructor (0x801B6C4C) and destructor
 * (0x801B6B50); only the members that the reconstructed vehicles use are named.
 *
 * Its vtable (0x8053B87C) overrides CharInfo's fn_801DC980 and the five bool slots from
 * fn_80078B2C on, and adds twenty slots. The added slots are declared in vtable order and named
 * after TVehicleBase's implementations, except where the behaviour gives an inferred name; two of
 * them are pure. The return types of the slots that return a constant (0, 1 or a float) and of
 * GetPosition are inferred, and so are the parameter spellings. */
class TVehicleBase : public CharInfo {
  public:
    /* Passes 0x33 to CharInfo's constructor and type to the constructor of the object at 0x98,
     * and sets the word at 0x5C to 6 and the word at 0x30 (the rotation order, inferred) to 2. */
    explicit TVehicleBase(int type);
    virtual ~TVehicleBase();
    /* Passes the command to the object at 0x98 (0x801B6B2C). */
    virtual void fn_801DC980(CharCommand *command);
    /* 0x801B6AB8, 0x801B6A44, 0x801B6A3C, 0x801B6A34 and 0x801B69C0: the first two and the last
     * each call their own slot of the character info at 0x80 and return the result, or false when
     * it has expired; the other two return false. */
    virtual bool fn_80078B2C();
    virtual bool fn_80078B34();
    virtual bool fn_80078B3C();
    virtual bool fn_80078B44();
    virtual bool fn_80078B4C();

    /* Stores the position and the rotation in m_position and m_rotation (0x801B663C; name
     * inferred). Vehicle::VehicleTask::fn_801B5D24 passes them on to this slot. */
    virtual void SetPosition(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* Copies m_position and m_rotation to the non-null arguments (0x801B657C; name inferred). */
    virtual void GetPosition(Fn8040Vector3 *position, UnkRotation3 *rotation);
    /* Empty here (0x801B634C). fn_801B6958 calls it after storing m_self. */
    virtual void fn_801B634C();
    /* Pure (name inferred). 0x801B6EDC calls it on the vehicle that the object at 0x98 keeps at
     * its 0xC, passing its own second argument; GunLift's implementation handles commands. */
    virtual void OnCommand(CharCommand *command) = 0;
    /* Empty here (0x801B6348). */
    virtual void fn_801B6348();
    /* Pure. Vehicle::VehicleTask's update (0x801B5B2C) passes the float that the vehicle's
     * field_0x60 points to (name inferred). */
    virtual void Update(float dt) = 0;
    /* Sets bit 0 and resets bit 3 of field_0x70, then, unless the character info at 0x80 has
     * expired, passes its word at 0x54 to this vehicle's fn_8007A4CC (0x801B62C4). */
    virtual void fn_801B62C4();
    /* Resets bit 0 and sets bits 2 and 3 of field_0x70, clears the float at 0x28 of the object at
     * 0x98 and passes -1 to this vehicle's fn_8007A4CC (0x801B6254). */
    virtual void fn_801B6254();
    virtual bool fn_801B624C();
    virtual bool fn_801B6244();
    virtual bool fn_801B623C();
    virtual bool fn_801B6234();
    virtual bool fn_801B622C();
    virtual float fn_801B6224();
    virtual void fn_801B6220();
    virtual void fn_801B621C();
    virtual bool fn_801B6214();
    virtual void fn_801B6210();
    virtual void fn_801B620C();
    virtual void fn_801B6208();

    /* Passes the resource, with this vehicle, to the object at 0x98 (0x801B641C; the parameter
     * type is inferred). */
    void fn_801B641C(void *resource);
    /* Stores value at 0x8 of the object at 0x98 (0x801B6504). */
    void fn_801B6504(float value);
    /* Return the weak pointers at 0x88 and 0x80 (0x801B66EC and 0x801B67C4; the const reference
     * spelling is inferred). */
    const boost::weak_ptr<CharInfo> &fn_801B66EC() const;
    const boost::weak_ptr<CharInfo> &fn_801B67C4() const;
    /* Returns a copy of m_self (name inferred). The game calls a copy that the linker kept from
     * another unit (0x800E8CD4). */
    boost::weak_ptr<TVehicleBase> GetSelf() const {
        return m_self;
    }

    /* 0x64 */ u8 field_0x64[0x70 - 0x64];
    /* Bit 0 is set by fn_801B62C4 and reset by fn_801B6254. */
    /* 0x70 */ UnkBitFlags_80014E04 field_0x70;
    /* 0x74 */ u8 field_0x74[0x80 - 0x74];
    /* 0x80 */ boost::weak_ptr<CharInfo> field_0x80;
    /* Passed as the owner of the shots by GunLift::Update (through fn_801B66EC). */
    /* 0x88 */ boost::weak_ptr<CharInfo> field_0x88;
    /* A weak pointer to this vehicle: Vehicle::VehicleTask's constructor (0x801B5DF8) passes one
     * made from its shared pointer to fn_801B6958, which stores it here (name inferred). */
    /* 0x90 */ boost::weak_ptr<TVehicleBase> m_self;
    /* 0x98 */ boost::scoped_ptr<UnkVehicleState_801B7338> field_0x98;
};
CHECK_SIZEOF(Vehicle::TVehicleBase, 0x9C);

} // namespace Vehicle

#endif

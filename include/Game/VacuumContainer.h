#ifndef GAME_VACUUMCONTAINER_H
#define GAME_VACUUMCONTAINER_H

#include "Game/VacuumObject.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class GadgetModelInfo;
class VacuumCommand;

namespace VacuumContainer {

/* VacuumContainer::Container (class name from RTTI, with VacuumObject as its only base): the
 * CONTAINER gadget's model while a character pulls it in (inferred from the names). It keeps the
 * container's model and its "model", "select" and "value" parameters. While it is pulled in it
 * turns its model from its rotation towards the one that fn_8040F364 builds from m_forward and m_up
 * (see Update) and spins it about its x axis. Member names and the names of the methods that are
 * not virtual are inferred from the code. */
class Container : public VacuumObject {
  public:
    Container(VacuumCommand *command, const boost::shared_ptr<GadgetModelInfo> &model,
              int modelIndex, int select, int value, int eventId);
    virtual ~Container();
    /* Passes rotation on to VacuumObject's implementation and keeps a copy in m_rotation. */
    virtual void fn_80238268(const UnkRotation3 *rotation);
    /* Advances the base object by dt, then sets the object's rotation (the virtual fn_80238268) to
     * a rotation that depends on t, fn_8023800C's result: zero, replaced by m_rotation when t <= 0;
     * then the rotation that fn_8040F364 builds from m_forward and m_up turned by the matrix of
     * fn_80238000 (fn_8040B618) when t >= 1, otherwise the interpolation from m_rotation to that
     * rotation by t (fn_8040F16C, order 2). Adds dt * lbl_805EFEE8 * c to m_spin, where c is
     * fn_8023800C's result clamped to [0, 1] (fn_80070460), and wraps m_spin with fn_8040E9D8.
     * Last it places the model (UpdateModelMatrix) and, when the model is not null, advances it by
     * dt. */
    virtual void Update(float dt);
    /* Sets bit 1 of the flags that fn_802380B8 returns and passes fn_80238058's character info,
     * the model, the parameters, the object's position (fn_802382B0), the angles of its rotation
     * (ApplyRotation applied to the identity matrix, read back with fn_800494F8), target,
     * lbl_805EFEE8, s_scale and m_eventId to fn_8027B3E8 (behaviour inferred). */
    virtual void fn_80238184(const Fn8040Vector3 *target);
    /* Returns VacuumObject's matrix (fn_80237F10) moved by (0, 0, 5) in the matrix's own frame:
     * fn_8040B59C turns the offset by the matrix's rotation before adding it to the translation.
     * When both weak pointers that fn_80237F10 reads have expired, it returns without writing its
     * result (0x80237F90); this override still copies that unwritten matrix and translates it. */
    virtual Fn8040Matrix34 fn_80237EE4();

    /* When the model is not null, places it at the object's position (fn_802382B0), turned by
     * ApplyRotation and scaled by s_scale on all three axes (name inferred). */
    void UpdateModelMatrix();
    /* Multiplies matrix by the rotations of the object's rotation (fn_80238248: about y, then x,
     * then z), then by the rotation about x by m_spin and the rotation about y by s_yaw (name
     * inferred). */
    void ApplyRotation(Fn8040Matrix34 *matrix);
    /* Takes the object's position minus the translation of the matrix that fn_80238000 returns,
     * turns it by the negated angles of m_rotation (about y, then x, then z) and replaces
     * m_forward with the axis along which that vector's component has the largest magnitude, as
     * a unit vector with the component's sign (+1 when the component is 0 or more, -1 otherwise);
     * m_up gets m_forward's components in the order y, z, x (name inferred). Equal magnitudes
     * select y over x and z, and x over z. */
    void ChooseAxes();

    /* 0x08 */ Fn8040Vector3 m_forward;
    /* 0x14 */ Fn8040Vector3 m_up;
    /* The rotation that fn_80238268 was last given; the constructor zeroes it. */
    /* 0x20 */ UnkRotation3 m_rotation;
    /* 0x2C */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* The container's "model", "select" and "value" parameters (see Container.h). */
    /* 0x34 */ s32 m_modelIndex;
    /* 0x38 */ s32 m_select;
    /* 0x3C */ s32 m_value;
    /* The angle about the x axis that Update advances (0 after construction). */
    /* 0x40 */ float m_spin;
    /* 0x44 */ s32 m_eventId;
};
CHECK_SIZEOF(Container, 0x48);

} // namespace VacuumContainer

/* Builds a VacuumContainer::Container from command, model, modelIndex, select, value and eventId
 * and keeps it in a shared_ptr<VacuumObject> (when the allocation fails, the pointer is empty and
 * the calls below are still made through it); passes position and rotation to its virtual
 * fn_802382E0 and fn_80238268, passes 0x252 to its fn_802380C4, calls its ChooseAxes and creates
 * a VacuumObjectTask for it, which is constructed only when the task's own allocation succeeds
 * (behaviour inferred from the body; the original name is unknown). The CONTAINER gadget's
 * container calls it for a VacuumCommand, with its "model", "select" and "value" parameters, its
 * own position and rotation and an id.
 *
 * The signature is inferred: the bytes give the eight integer register arguments, but not the
 * parameter types, the pointer or reference spellings or the const qualifiers. */
extern "C" void fn_80254384(VacuumCommand *command, const boost::shared_ptr<GadgetModelInfo> &model,
                            int modelIndex, int select, int value, const Fn8040Vector3 *position,
                            const UnkRotation3 *rotation, int eventId);

#endif

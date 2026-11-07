/* VacuumContainer::Container, the CONTAINER gadget's model while a character pulls it in (class
 * described in Game/VacuumContainer.h), and fn_80254384, which creates it. File name inferred from
 * the class's namespace. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first). */
#include "Game/VacuumContainer.h"
#include "Game/AngleConversion.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8027B3E8.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"

/* The scale of the model (name inferred), set at startup by this file's static initialiser. */
static const float s_scale = lbl_805F72B0 / 10.0f;
/* The last rotation about the y axis that ApplyRotation applies (name inferred). */
static const float s_yaw = DegreesToRadians(45.0f);

namespace VacuumContainer {

Container::Container(VacuumCommand *command, const boost::shared_ptr<GadgetModelInfo> &model,
                     int modelIndex, int select, int value, int eventId)
    : VacuumObject(fn_80237E34(command), command), m_rotation(0.0f, 0.0f, 0.0f),
      m_modelIndex(modelIndex), m_select(select), m_value(value), m_spin(0.0f), m_eventId(eventId) {
    m_model = model;
}

Container::~Container() {}

void Container::fn_80238268(const UnkRotation3 *rotation) {
    VacuumObject::fn_80238268(rotation);
    m_rotation = *rotation;
}

void Container::Update(float dt) {
    VacuumObject::Update(dt);

    float progress = fn_8023800C();
    Fn8040Matrix34 *matrix = fn_80238000();
    UnkRotation3 targetRotation =
        fn_8040F364(fn_8040B618(*matrix, m_forward), fn_8040B618(*matrix, m_up));
    UnkRotation3 rotation(0.0f, 0.0f, 0.0f);

    /* As in the original, the first test is not chained to the second, so the rotation it
     * stores is always replaced. */
    if (progress <= 0.0f) {
        rotation = m_rotation;
    }
    if (progress >= 1.0f) {
        rotation = targetRotation;
    } else {
        rotation = fn_8040F16C(&m_rotation, &targetRotation, 2, progress);
    }
    fn_80238268(&rotation);

    float speed = fn_8023800C();

    speed = fn_80070460(speed, 0.0f, 1.0f);
    m_spin += dt * (lbl_805EFEE8 * speed);
    m_spin = fn_8040E9D8(m_spin);

    UpdateModelMatrix();
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
}

void Container::fn_80238184(const Fn8040Vector3 *target) {
    fn_802380B8()->Set(1);

    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    ApplyRotation(&matrix);

    UnkRotation3 angles(0.0f, 0.0f, 0.0f);

    fn_800494F8(&angles, &matrix);

    fn_8027B3E8(fn_80238058(), m_model, m_modelIndex, m_select, m_value, fn_802382B0(), &angles,
                target, m_eventId, lbl_805EFEE8, s_scale);
}

Fn8040Matrix34 Container::fn_80237EE4() {
    Fn8040Matrix34 matrix = fn_80237F10();
    Fn8040Vector3 offset(0.0f, 0.0f, 5.0f);

    fn_8040B59C(&matrix, &offset);
    return matrix;
}

void Container::ChooseAxes() {
    Fn8040Matrix34 *matrix = fn_80238000();

    m_forward = fn_802382B0() - fn_8040FB08(matrix);
    m_forward = fn_8040D0AC(m_forward, -m_rotation.y);
    m_forward = fn_8040D118(m_forward, -m_rotation.x);
    m_forward = fn_8040D044(m_forward, -m_rotation.z);

    float x = fn_8000A980(m_forward.x);
    float y = fn_8000A980(m_forward.y);
    float z = fn_8000A980(m_forward.z);

    if (x > y) {
        if (z > x) {
            m_forward = Fn8040Vector3(0.0f, 0.0f, m_forward.z >= 0.0f ? 1.0f : -1.0f);
        } else {
            m_forward = Fn8040Vector3(m_forward.x >= 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
        }
    } else if (z > y) {
        m_forward = Fn8040Vector3(0.0f, 0.0f, m_forward.z >= 0.0f ? 1.0f : -1.0f);
    } else {
        m_forward = Fn8040Vector3(0.0f, m_forward.y >= 0.0f ? 1.0f : -1.0f, 0.0f);
    }

    m_up.x = m_forward.y;
    m_up.y = m_forward.z;
    m_up.z = m_forward.x;
}

void Container::ApplyRotation(Fn8040Matrix34 *matrix) {
    UnkRotation3 rotation = fn_80238248();

    fn_8040B848(matrix, rotation.y);
    fn_8040B884(matrix, rotation.x);
    fn_8040B80C(matrix, rotation.z);
    fn_8040B884(matrix, m_spin);
    fn_8040B848(matrix, s_yaw);
}

void Container::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Vector3 position = fn_802382B0();
        /* The original fetches the rotation here as well and leaves it unused (0x8025462C);
         * ApplyRotation fetches it again. */
        UnkRotation3 rotation = fn_80238248();
        /* Matching workaround: matrix is declared before translation. The original keeps
         * translation at the lower stack offset (0x2C, matrix at 0x5C), which this compiler gives
         * the later of two locals; declared the other way round, the offsets swap (6 words).
         * Revisit if translation turns out to be the result of a function of its own. */
        Fn8040Matrix34 matrix;
        Fn8040Matrix34 translation;

        fn_8040B034(&translation, &position);
        fn_8040C148(&matrix, &translation);
        ApplyRotation(&matrix);
        fn_8040B720(&matrix, s_scale, s_scale, s_scale);
        m_model->fn_801633DC(&matrix);
    }
}

} // namespace VacuumContainer

void fn_80254384(VacuumCommand *command, const boost::shared_ptr<GadgetModelInfo> &model,
                 int modelIndex, int select, int value, const Fn8040Vector3 *position,
                 const UnkRotation3 *rotation, int eventId) {
    VacuumContainer::Container *container =
        new VacuumContainer::Container(command, model, modelIndex, select, value, eventId);
    boost::shared_ptr<VacuumObject> object = boost::shared_ptr<VacuumObject>(container);

    object->fn_802382E0(position);
    object->fn_80238268(rotation);
    object->fn_802380C4(0x252);
    container->ChooseAxes();
    new VacuumObjectTask(object);
}

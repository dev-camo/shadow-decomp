/* ThrowContainer::Container (class name from RTTI, with ThrowObject as its only direct base):
 * the CONTAINER gadget's container after it is thrown, and the two functions that create it
 * (described in Game/ThrowContainer.h). File name inferred from the class's namespace. Method
 * and member names are inferred. Definitions are in reverse address order (deferred inlining
 * emits functions last-defined first). */
#include "Game/ThrowContainer.h"
#include "Game/CharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/ThrowObject.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8033E644.h"
#include "layout_check.h"

/* The radius of the container's swept sphere (5), and the range of a container that
 * fn_8027B3E8 creates (600); names inferred from what the code does with them.
 * Matching workaround: the original sets these two floats with the file's start-up function in
 * .ctors (constant initialisers would be emitted as initialised .sdata data instead).
 * UnkStartupFloat_800E2570 stands in for the original initialiser expressions, which are unknown,
 * and gives those stores. Revisit when the original initialisers are found. */
static const float s_radius = UnkStartupFloat_800E2570(5.0f).value;
static const float s_vacuumedRange = UnkStartupFloat_800E2570(600.0f).value;
/* The acceleration of a container that fn_8027B3E8 creates. */
static const Fn8040Vector3 s_noAcceleration(0.0f, 0.0f, 0.0f);

namespace ThrowContainer {

class Container : public ThrowObject {
  public:
    /* The parameters are those of fn_8027B338, which passes them on unchanged. Applies the y, x
     * and z angles, in that order, to m_rotation, which starts as the identity. */
    Container(const boost::weak_ptr<CharInfo> &owner,
              const boost::shared_ptr<GadgetModelInfo> &model, int modelIndex, int select,
              int value, const Fn8040Vector3 *position, const UnkRotation3 *angles,
              const Fn8040Vector3 *velocity, const Fn8040Vector3 &acceleration, int eventId,
              float spin, float scale, float range);
    virtual ~Container();
    /* ThrowObject's update; then turns m_rotation about its x axis by m_spin * dt, places the
     * model (UpdateModel) and, when there is a model, passes dt to its fn_80162FB0. */
    virtual void Update(float dt);
    /* When the owner still exists, passes it and m_eventId to fn_8020642C. */
    virtual void OnRangeOver();
    /* Passes the model and m_modelIndex to fn_8033E644 and m_select and m_value to fn_8033E828,
     * both with the position and the angles of m_rotation; then does what OnRangeOver does. */
    virtual void OnHit();

    /* When there is a model, passes it the product of the translation to the position and
     * m_rotation, with its three rotation columns scaled by m_scale. */
    void UpdateModel();

  private:
    /* 0x30 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x38 */ int m_modelIndex;
    /* 0x3C */ int m_select;
    /* 0x40 */ int m_value;
    /* 0x44 */ Fn8040Matrix34 m_rotation;
    /* 0x74 */ float m_spin;
    /* 0x78 */ float m_scale;
    /* 0x7C */ int m_eventId;
};
CHECK_SIZEOF(Container, 0x80);

Container::Container(const boost::weak_ptr<CharInfo> &owner,
                     const boost::shared_ptr<GadgetModelInfo> &model, int modelIndex, int select,
                     int value, const Fn8040Vector3 *position, const UnkRotation3 *angles,
                     const Fn8040Vector3 *velocity, const Fn8040Vector3 &acceleration, int eventId,
                     float spin, float scale, float range)
    : ThrowObject(owner, *position, *velocity, acceleration, s_radius, range),
      m_rotation(lbl_805E0720) {
    m_model = model;
    m_modelIndex = modelIndex;
    m_select = select;
    m_value = value;

    fn_8040B848(&m_rotation, angles->y);
    fn_8040B884(&m_rotation, angles->x);
    fn_8040B80C(&m_rotation, angles->z);

    m_spin = spin;
    m_scale = scale;
    m_eventId = eventId;
}

Container::~Container() {}

void Container::Update(float dt) {
    ThrowObject::Update(dt);

    Fn8040Matrix34 turn = lbl_805E0720;

    fn_8040B884(&turn, m_spin * dt);

    Fn8040Matrix34 turned;

    fn_8001E044(&turned, &m_rotation, &turn);
    m_rotation = turned;

    UpdateModel();

    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
}

void Container::OnRangeOver() {
    if (boost::shared_ptr<CharInfo> owner = GetOwner().lock()) {
        fn_8020642C(owner, m_eventId);
    }
}

void Container::OnHit() {
    Fn8040Vector3 position = GetPosition();
    UnkRotation3 angles(0.0f, 0.0f, 0.0f);

    fn_800494F8(&angles, &m_rotation);

    fn_8033E644(m_model, m_modelIndex, &position, &angles);
    fn_8033E828(m_select, m_value, &position, &angles);

    if (boost::shared_ptr<CharInfo> owner = GetOwner().lock()) {
        fn_8020642C(owner, m_eventId);
    }
}

void Container::UpdateModel() {
    if (m_model.get() != 0) {
        Fn8040Vector3 position = GetPosition();
        /* Matching workaround: matrix is declared before translation. The original keeps
         * translation at the lower stack offset (0x50, matrix at 0x80), which this compiler gives
         * the later of two locals; declared the other way round, the offsets swap (7 words).
         * Revisit if translation turns out to be the result of a function of its own. */
        Fn8040Matrix34 matrix;
        Fn8040Matrix34 translation;

        fn_8040B034(&translation, &position);
        matrix = translation;

        Fn8040Matrix34 product;

        fn_8001E044(&product, &matrix, &m_rotation);
        matrix = product;
        fn_8040B720(&matrix, m_scale, m_scale, m_scale);

        m_model->fn_801633DC(&matrix);
    }
}

} // namespace ThrowContainer

void fn_8027B3E8(const boost::weak_ptr<CharInfo> &owner,
                 const boost::shared_ptr<GadgetModelInfo> &model, int modelIndex, int select,
                 int value, const Fn8040Vector3 &position, const UnkRotation3 *angles,
                 const Fn8040Vector3 *velocity, int eventId, float spin, float scale) {
    ThrowContainer::Container *container = new ThrowContainer::Container(
        owner, model, modelIndex, select, value, &position, angles, velocity, s_noAcceleration,
        eventId, spin, scale, s_vacuumedRange);

    container->StartEffect(0x254);
}

void fn_8027B338(const boost::weak_ptr<CharInfo> &owner,
                 const boost::shared_ptr<GadgetModelInfo> &model, int modelIndex, int select,
                 int value, const Fn8040Vector3 *position, const UnkRotation3 *angles,
                 const Fn8040Vector3 *velocity, const Fn8040Vector3 &acceleration, int eventId,
                 float spin, float scale, float range) {
    new ThrowContainer::Container(owner, model, modelIndex, select, value, position, angles,
                                  velocity, acceleration, eventId, spin, scale, range);
}

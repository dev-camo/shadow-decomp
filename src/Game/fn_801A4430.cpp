/* Animation control, matrix map and event callback; original class and filename unknown.
 * NonMatching: several owner methods and a template instance retain instruction differences. */
#include "Game/fn_801A4430.h"

Unk801A4430Callback::Unk801A4430Callback() {}

Unk801A4430::Unk801A4430()
    : m_control(0), m_resourceId(0), m_firstMotionId(0), m_enabled(true), m_matrices(),
      m_callback() {}

Unk801A4430Callback::~Unk801A4430Callback() {}

Unk801A4430::~Unk801A4430() {
    if (m_control != 0) {
        fn_8041B884(m_control);
        m_control = 0;
    }
}

void Unk801A4430::fn_801A4B30(unsigned int resourceId, unsigned int firstMotionId, int arg) {
    m_resourceId = resourceId;
    m_firstMotionId = firstMotionId;
    if (m_control == 0) {
        m_control = fn_8041B908(resourceId, arg);
        stMtnControlWorkFlag(m_control, 0, 11, 0);
    }
}

void Unk801A4430::fn_801A4AF0() {
    if (m_control != 0) {
        fn_8041B884(m_control);
        m_control = 0;
    }
}

bool Unk801A4430Callback::fn_801A4A74() const {
    return m_function.empty();
}

void Unk801A4430Callback::fn_801A4A48(const Unk801A4430Event &event) {
    if (!m_function.empty()) {
        m_function(event);
    }
}

void Unk801A4430::fn_801A4934() {
    if (m_control != 0 && m_enabled) {
        fn_80415EA4(m_control);
        for (MatrixMap::iterator it = m_matrices.begin(); it != m_matrices.end(); ++it) {
            MotionTransform *transform = fn_80415E70(m_control, it->first);
            if (transform != 0) {
                fn_80016370(&m_matrices[it->first], &transform->matrix);
            }
        }
        if (!m_callback.fn_801A4A74()) {
            unsigned short eventId = 0;
            void *payload = 0;
            _stMtnPlayWork *control = m_control;
            while (fn_80415854(control, 0, &eventId, &payload, 0)) {
                control = 0;
                m_callback.fn_801A4A48(Unk801A4430Event(eventId, payload));
            }
        }
    }
}

void Unk801A4430::fn_801A4898(unsigned short motionId, int target, float blendFrames) {
    if (blendFrames > 0.0f) {
        stMtnPrepareBlendWithTarget(m_control, target, -1, -1);
        stMtnRequestBlendFrame(m_control, 0, blendFrames);
    } else {
        stMtnAbortBlend(m_control, 0);
    }
    fn_80417568(m_control, static_cast<unsigned short>(motionId + m_firstMotionId - 1));
}

void Unk801A4430::fn_801A4848(float speed) {
    stMtnRequestFrameSpeed(m_control, 0, speed);
    stMtnRequestBlendSpeed(m_control, 0, speed);
}

bool Unk801A4430::fn_801A4810() {
    return stMtnControlWorkFlag(m_control, 0, 3, 1) != 0;
}

Fn8040Matrix34 *Unk801A4430::fn_801A46E4(unsigned int nodeId) {
    MotionTransform *transform = fn_80415E70(m_control, nodeId);
    if (transform == 0) {
        return 0;
    }
    if (m_matrices.find(nodeId) == m_matrices.end()) {
        fn_80016370(&m_matrices[nodeId], &transform->matrix);
    }
    return &m_matrices[nodeId];
}

void Unk801A4430::fn_801A46C0(Fn8040Vector3 *vector) {
    fn_80415E34(m_control, vector);
}

void Unk801A4430::fn_801A469C(Fn8040Vector3 *vector) {
    fn_80415DBC(m_control, vector);
}

void Unk801A4430::fn_801A4640(const Fn8040Vector3 &position, const Fn8040Vector3 &rotation) {
    Fn8040Matrix34 matrix = fn_801A4430(rotation);
    fn_8040FA20(&matrix, &position);
    fn_801A4604(matrix);
}

void Unk801A4430::fn_801A4604(const Fn8040Matrix34 &matrix) {
    const Fn8040Matrix34 &converted = fn_800163F4(&matrix);
    fn_804172D8(m_control, &converted);
}

void Unk801A4430::fn_801A45E0(const Fn8040Vector3 *offset) {
    fn_80415BDC(m_control, offset);
}

void Unk801A4430::fn_801A4584(const Fn8040Vector3 &rotation) {
    Fn8040Matrix34 matrix = fn_801A4430(rotation);
    const Fn8040Matrix34 &converted = fn_800163F4(&matrix);
    fn_8041734C(m_control, &converted);
}

void Unk801A4430::fn_801A4548(bool enabled) {
    stMtnControlWorkFlag(m_control, 0, 1, enabled ? 0 : 2);
}

void Unk801A4430::fn_801A44DC(bool enabled) {
    stMtnControlWorkFlag(m_control, 0, 0, enabled ? 0 : 2);
    stMtnControlWorkFlag(m_control, 1, 0, enabled ? 0 : 2);
}

unsigned short Unk801A4430::fn_801A4494() {
    return static_cast<unsigned short>(fn_80415CDC(m_control, 0) - m_firstMotionId + 1);
}

Fn8040Matrix34 Unk801A4430::fn_801A4430(const Fn8040Vector3 &rotation) {
    Fn8040Matrix34 matrix;
    fn_8040B6F8(&matrix);
    fn_8040B848(&matrix, rotation.y);
    fn_8040B884(&matrix, rotation.x);
    fn_8040B80C(&matrix, rotation.z);
    return matrix;
}

/* ElecCube (class name from RTTI): a gadget of the electric circuit area that shows the
 * ROLLBLOCK.BIN model scaled by its "Scale" parameter and keeps turning it towards new random
 * rotations. File name inferred from the class. Method names marked "(name inferred)" are not
 * original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first).
 *
 * Not yet matching: the code is identical, but the original pools its .sdata2 float literals in
 * another order, starting with those of ApplyScale and PickNextRotation (70.0, 1.0, 0.1, 0.6, 0.0)
 * as if a function using both had been compiled first. No such function is known. */
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SharedConstants.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_random.h"
#include "layout_check.h"

class ElecCube : public Gadget {
  public:
    ElecCube(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecCube();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the cube under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and scale into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's translation and rotation, scaled (name inferred). */
    void UpdateModelMatrix();
    /* Moves the character info's rotation from m_rotationFrom towards m_rotationTo and picks a
     * new target when the turn is over (name inferred). */
    void UpdateRotation(float dt);
    /* Passes the "Scale" parameter to the model (name inferred). */
    void ApplyScale();
    /* Picks a random new target rotation near m_rotationFrom, turned further about one random axis
     * (name inferred). */
    void PickNextRotation();

    static const char *s_name;

    /* 0x30 */ float m_scale; /* the "Scale" parameter */
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ GadgetModelInfo *m_model;
    /* 0x40 */ float m_timer;                /* time into the current turn (inferred) */
    /* 0x44 */ Fn8040Vector3 m_rotationFrom; /* names inferred */
    /* 0x50 */ Fn8040Vector3 m_rotationTo;
    /* 0x5C */ u32 field_0x5C;
};
CHECK_SIZEOF(ElecCube, 0x60);

const char *ElecCube::s_name = "ElecCube";

inline void ElecCube::ApplyScale() {
    m_model->fn_801617D0(0, -2, 70.0f * m_scale, 1.0f, 1.0f);
}

inline void ElecCube::PickNextRotation() {
    m_rotationTo.x = fn_8040EC80(0.1f * -lbl_805FAD38, 0.1f * lbl_805FAD38);
    m_rotationTo.y = fn_8040EC80(0.1f * -lbl_805FAD38, 0.1f * lbl_805FAD38);
    m_rotationTo.z = fn_8040EC80(0.1f * -lbl_805FAD38, 0.1f * lbl_805FAD38);
    switch (fn_8040EA90(3)) {
    case 0:
        m_rotationTo.x += m_rotationTo.x < 0.0f ? 0.6f * -lbl_805FAD38 : 0.6f * lbl_805FAD38;
        break;
    case 1:
        m_rotationTo.y += m_rotationTo.y < 0.0f ? 0.6f * -lbl_805FAD38 : 0.6f * lbl_805FAD38;
        break;
    case 2:
        m_rotationTo.z += m_rotationTo.z < 0.0f ? 0.6f * -lbl_805FAD38 : 0.6f * lbl_805FAD38;
        break;
    }
    m_rotationTo.x += m_rotationFrom.x;
    m_rotationTo.y += m_rotationFrom.y;
    m_rotationTo.z += m_rotationFrom.z;
}

ElecCube::ElecCube(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_rotationFrom.x = 0.0f;
    m_rotationFrom.y = 0.0f;
    m_rotationFrom.z = 0.0f;
    m_rotationTo.x = 0.0f;
    m_rotationTo.y = 0.0f;
    m_rotationTo.z = 0.0f;
    m_name = s_name;
    fn_800C9DBC(owner, &m_scale, sizeof(m_scale));
    m_charInfo = fn_8015CB88(0x58, 4, "ElecCube");
    m_model = new GadgetModelInfo(1, "ROLLBLOCK.BIN");
    m_model->fn_80162A68(0, 0);
    CopyFromOwner(m_owner);
    m_model->fn_80161F08("col", m_charInfo);
    ApplyScale();

    switch (fn_8040EA90(3)) {
    case 0:
        m_charInfo->field_0x30 = 0;
        break;
    case 1:
        m_charInfo->field_0x30 = 1;
        break;
    case 2:
        m_charInfo->field_0x30 = 2;
        break;
    }

    m_timer = 0.0f;
    m_rotationFrom.x = fn_8040EC80(-lbl_805FAD38, lbl_805FAD38);
    m_rotationFrom.y = fn_8040EC80(-lbl_805FAD38, lbl_805FAD38);
    m_rotationFrom.z = fn_8040EC80(-lbl_805FAD38, lbl_805FAD38);

    PickNextRotation();
    UpdateModelMatrix();
}

ElecCube::~ElecCube() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

void ElecCube::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }
    UpdateRotation(dt);
    UpdateModelMatrix();
}

void ElecCube::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    if (m_model != 0) {
        m_model->fn_80162EF4();
        ApplyScale();
        m_model->fn_8016108C();
    }
}

void ElecCube::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_scale, sizeof(m_scale));
}

void ElecCube::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8001E070(&matrix, &m_charInfo->m_rotation, m_charInfo->field_0x30);
        fn_8040B720(&matrix, m_scale, m_scale, m_scale);
        m_model->fn_801633DC(&matrix);
    }
}

void ElecCube::UpdateRotation(float dt) {
    float t;

    m_timer += dt;
    if (m_timer < 0.3f) {
        t = 0.7f * (m_timer / 0.3f);
    } else {
        t = 0.3f * ((m_timer - 0.3f) / 1.2f) + 0.7f;
        if (t > 1.0f) {
            t = 1.0f;
        }
    }

    /* The original divides each angle by t + (1 - t) rather than simplifying it to 1; it stays
     * written out per angle, since hoisting it into a local changes the register allocation. */
    float deltaX = m_rotationTo.x - m_rotationFrom.x;
    m_charInfo->m_rotation.x = m_rotationFrom.x + deltaX * t / (t + (1.0f - t));
    float deltaY = m_rotationTo.y - m_rotationFrom.y;
    m_charInfo->m_rotation.y = m_rotationFrom.y + deltaY * t / (t + (1.0f - t));
    float deltaZ = m_rotationTo.z - m_rotationFrom.z;
    m_charInfo->m_rotation.z = m_rotationFrom.z + deltaZ * t / (t + (1.0f - t));

    if (m_timer > 1.5f) {
        m_timer = 0.0f;
        m_rotationFrom.x = m_rotationTo.x;
        m_rotationFrom.y = m_rotationTo.y;
        m_rotationFrom.z = m_rotationTo.z;

        PickNextRotation();
    }
}

extern "C" void fn_8021BD48(void) {
    fn_8006B474("ROLLBLOCK.BIN", 1);
}

extern "C" void fn_8021BD44(void) {}

void ElecCube::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecCube(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo lbl_805458F0[] = {
    {4, "Scale", 0, 0, 0, 1.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80545940 = {
    "ELEC_CUBE", fn_8021BD48, fn_8021BD44, ElecCube::Create, 0, 0, 0x7E2, 1, 0, 10, lbl_805458F0,
};

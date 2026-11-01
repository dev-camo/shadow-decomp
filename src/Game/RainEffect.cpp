/* RainEffect (class name from RTTI): a gadget with a box probe whose size the parameters
 * COLLI_WIDTH m, COLLI_HEIGHT m and COLLI_DEPTH m give. The probe is tested against a sphere of
 * radius 0 at the position of entry 0 of the manager that fn_80009548 returns. When the probe
 * starts touching it, the gadget starts the model RAINEFFECT.BIN and sound 0x733E; while it
 * touches, the gadget plays 0x7340 or 0x7341 every 3 to 6 seconds and places the model relative to
 * the entry; when it stops touching, the gadget stops the model and sound 0x733E (behaviour
 * inferred). File name inferred from the class. Method names marked "(name inferred)"
 * are not original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkProbeTarget_800587A0.h"
#include "Game/UnkSingleton_8000DCC0.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkTaskGroup_801D0A78.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

/* The sounds that the gadget plays at random while the probe touches the entry's position. */
static const u16 s_sounds[2] = {0x7340, 0x7341};

/* The gadget parameters in the order of the descriptor's table below. ReadOwner scales the three
 * sizes by 10 and halves them after reading them, so from then on they hold the half extents of
 * the box. */
struct RainEffectParams {
    /* 0x0 */ float width;  /* COLLI_WIDTH m */
    /* 0x4 */ float height; /* COLLI_HEIGHT m */
    /* 0x8 */ float depth;  /* COLLI_DEPTH m */
};
CHECK_SIZEOF(RainEffectParams, 0xC);

class RainEffect : public Gadget {
  public:
    RainEffect(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~RainEffect();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under the second task of the task group (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads position, rotation and parameters from the owner; the box size is turned into half
     * extents (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Gives the probe the gadget's position, rotation and half extents (name inferred). */
    void UpdateProbe();
    /* Places the model relative to entry 0 of the manager (name inferred). */
    void UpdateModelMatrix();

    static const char *s_name;

    /* 0x30 */ Fn8040Vector3 m_position;
    /* 0x3C */ Fn8040Vector3 m_rotation;
    /* 0x48 */ boost::shared_ptr<UnkProbeTarget_800587A0> m_probe;
    /* 0x50 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* The names of the members below are inferred. */
    /* 0x58 */ s32 m_state; /* 1 while the probe touches the entry's position, otherwise 0 */
    /* 0x5C */ RainEffectParams m_params;
    /* 0x68 */ float m_timer; /* seconds until the next sound (inferred) */
    /* 0x6C */ UnkSoundHandle_801D3DC8 m_sound;
};
CHECK_SIZEOF(RainEffect, 0x88);

const char *RainEffect::s_name = "RainEffect";

RainEffect::RainEffect(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_rotation(0.0f, 0.0f, 0.0f), m_state(0) {
    m_name = s_name;
    m_model = fn_8015F800(1, "RAINEFFECT.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80162790();
    ReadOwner(owner);

    Fn8040Vector3 halfExtents(m_params.width, m_params.height, m_params.depth);

    m_probe = fn_800C7D04(fn_80021B08(), &m_position, halfExtents, &m_rotation, 0, 0, 0xE, 0,
                          boost::weak_ptr<CharInfo>(lbl_805EE698), &lbl_805E07A0, &lbl_805E07B8, 0);
}

RainEffect::~RainEffect() {
    m_sound.fn_801D455C();
}

extern "C" void fn_800D525C(void) {
    fn_8006B474("RAINEFFECT.BIN", 1);
}

extern "C" void fn_800D5258(void) {}

/* The task group, constructed on first use and cached in a static pointer (name inferred).
 * Matching workaround: a unit-local copy of the original's accessor. The original at 0x800D51EC
 * is inferred to be a weak function with weak statics: seven functions in other units call this
 * one copy, and its statics lie outside the unit's sections. Here an inline definition is
 * expanded into Create (Create grows to 0x98 bytes), and a global non-inline one keeps the
 * statics local, so neither reproduces the original object; this copy has the original's
 * instructions, but the unit cannot replace the original object. Revisit when the shared header
 * of this accessor and the condition that keeps it out of line are found, for example when one of
 * the other seven callers is sourced. */
static UnkTaskGroup_801D0A78 *GetTaskGroup() {
    static UnkTaskGroup_801D0A78 *s_cache;

    if (s_cache == 0) {
        static UnkTaskGroup_801D0A78 s_group;
        s_cache = &s_group;
    }

    return s_cache;
}

void RainEffect::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new RainEffect(GetTaskGroup()->field_0x4, owner);
}

void RainEffect::UpdateBody(float dt) {
    UnkSphere_80021C04 sphere(fn_80010244(fn_80009548(), 0), 0.0f);

    if (fn_800C5878(fn_80021B08(), &sphere, boost::weak_ptr<UnkProbeTarget_800587A0>(m_probe), 0)) {
        if (m_state == 0) {
            m_model->fn_8016271C();
            m_sound = fn_801D3EF0(0x733E, this);
            m_timer = fn_8040EC80(3.0f, 6.0f);
            m_state = 1;
        } else {
            m_timer -= dt;

            if (m_timer < 0.0f) {
                fn_801D560C(fn_80053164(), s_sounds[fn_8040EA90(2)], 0, 0);
                m_timer = fn_8040EC80(3.0f, 6.0f);
            }
        }
    } else if (m_state == 1) {
        m_model->fn_80162790();
        m_sound.fn_801D455C();
        m_state = 0;
    }

    if (m_state == 1) {
        UpdateModelMatrix();
    }

    m_model->fn_80162FB0(dt);
}

void RainEffect::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateProbe();
    UpdateModelMatrix();
    m_probe->UnkSlot26();
    m_model->fn_80162EF4();
}

void RainEffect::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_position);
    fn_800C9ED4(owner, &m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_params.width = 0.5f * (10.0f * m_params.width);
    m_params.height = 0.5f * (10.0f * m_params.height);
    m_params.depth = 0.5f * (10.0f * m_params.depth);
}

void RainEffect::UpdateProbe() {
    m_probe->SetPosition(&m_position);
    m_probe->SetRotation(&m_rotation);
    m_probe->SetSize(m_params.width, -2, m_params.height, m_params.depth);
}

void RainEffect::UpdateModelMatrix() {
    /* The model takes the matrix of entry 0, moved by this distance from the matrix's translation
     * towards the entry's vector at 0x14. Its x and y axes are scaled by twice the two floats of
     * the fn_8000DCC0 record times the distance, so each float is a half extent per unit of
     * distance (behaviour inferred). */
    float distance = 1.1f;
    Fn8040Matrix34 matrix;
    Fn8040Vector3 matrixPosition;
    Fn8040Vector3 direction;
    Fn8040Vector3 toEntryVector;
    Fn8040Vector3 modelPosition;

    fn_8040C148(&matrix, GetEntryMatrix(fn_80009548(), 0));
    fn_8040FAA0(&matrixPosition, &matrix);
    fn_8040CD74(&toEntryVector, fn_800101F8(fn_80009548(), 0), &matrixPosition);
    fn_800091EC(&direction, &toEntryVector);
    fn_8040CE74(&direction);
    fn_8040CE38(&direction, distance);
    fn_8040CDD0(&modelPosition, &matrixPosition, &direction);
    fn_8040FA20(&matrix, &modelPosition);

    UnkRecord_8000DCC0 *record = fn_8000DCC0()->field_0x0;

    fn_8040B720(&matrix, 2.0f * (record->field_0x68 * distance),
                2.0f * (record->field_0x6C * distance), 1.0f);
    m_model->fn_801633DC(&matrix);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "COLLI_WIDTH m", 0, 0, 0, 20.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "COLLI_HEIGHT m", 0, 0, 0, 20.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "COLLI_DEPTH m", 0, 0, 0, 20.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805232D8 = {
    "RainEffect", fn_800D525C, fn_800D5258, RainEffect::Create, 0, 0, 0x106D, 1, 0, 10, s_paramInfo,
};

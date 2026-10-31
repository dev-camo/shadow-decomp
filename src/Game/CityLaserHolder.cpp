/* CityLaserHolder (class name from RTTI): the gadget of a laser in the city stage. It holds a
 * CityLaser (named after the boost::checked_deleter<CityLaser> RTTI string and the CITYLASER.BIN
 * model file) that, as far as the code shows (behaviour inferred), waits until a character comes
 * within the range, counts down a timer, then shows the beam and creates a CommonBomb task at its
 * own position when the beam starts and at each new target that the beam's probe reports. File
 * name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are ordered so that deferred inlining emits the functions at their original
 * addresses.
 *
 * NonMatching: SetState differs in five words (the order of the stack argument of fn_800C7A84 and
 * the moves around it), Update in five (the stack slot of the result of fn_8040FC3C), the
 * checked_deleter call operator inlines checked_delete, which the original keeps out of line,
 * five std::vector instances of weak pointers differ (insert, reserve, swap, the vector
 * destructor and the __vector_obj constructor) and 16 small helpers of the vector are not
 * reproduced; see the notes in configure.py. */
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkProbeTarget_800587A0.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"
#include <algorithm>
#include <vector>

class CityLaser;
class CityLaserHolder;

extern "C" {
/* Creates a 0x50-byte CommonBomb task (class name from the string its constructor fn_801DDBBC
 * stores as the task's name) under task layer 11 at position (behaviour inferred). arg1 is stored
 * at 0x48 of the task and is a type index into the table of 0xC-byte entries at 0x8053D570; arg2 is
 * stored at 0x4C and is a bit mask that the task tests against the hit character's word at 0x5C
 * (bit 2 for 4, bit 1 for 3, bit 0 for 1 or 6). The laser passes 1 and its config's mask. arg3 is
 * passed on to fn_80054DD0 (meaning unknown). */
void fn_801DD0BC(const Fn8040Vector3 *position, int arg1, int arg2, float arg3);
/* Degrees to radians. */
float fn_8000AB90(float degrees);
}

/* The height of the segment that goes up from the laser, and the radius around it that the bombs
 * are placed in: each bomb goes to the target's position, moved towards the nearest point of the
 * segment until it is at most s_hitRadius away from it (names inferred). */
static UnkStartupFloat_800E2570 s_hitHeight(1000.0f);
static UnkStartupFloat_800E2570 s_hitRadius(10.0f);
/* The length and radius of the beam's probe (names inferred). */
static UnkStartupFloat_800E2570 s_beamLength(500.0f);
static UnkStartupFloat_800E2570 s_beamRadius(10.0f);

/* The gadget parameters in the order of the descriptor's table below. */
struct CityLaserParams {
    /* 0x0 */ s32 mode;          /* 0 NORMAL or 1 RENDITION */
    /* 0x4 */ float rangeLength; /* RANGE_LENGTH (m); the code scales it by 10 */
    /* 0x8 */ float timer;       /* TIMER (sec) */
};
CHECK_SIZEOF(CityLaserParams, 0xC);

/* Two words per mode (names inferred): the mask that the laser passes to fn_801DD0BC for each
 * bomb, and whether state 3 creates an UnkTarget_80166AF4 object. */
struct CityLaserConfig {
    /* (name inferred): a bit mask that the CommonBomb task tests against the hit character's word
     * at 0x5C */
    /* 0x0 */ s32 bombFlags;
    /* 0x4 */ u8 hasTarget;
};
CHECK_SIZEOF(CityLaserConfig, 0x8);

static const CityLaserConfig s_configs[2] = {{7, 1}, {6, 0}};

class CityLaser {
  public:
    CityLaser(CityLaserHolder *holder, UnkGadgetOwner_800C9CD0 *owner);
    ~CityLaser();

    /* Runs the state machine for one frame (name inferred). */
    void Update(float dt);
    /* Changes the state and sets up what the new one shows (name inferred). */
    void SetState(int state);
    /* Re-reads the owner's position and byte and redraws the shapes (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position (name inferred). */
    void UpdateModelMatrix();
    /* Draws two debug spheres (the range and a fixed radius of 50) and two debug cylinders (the
     * beam's probe and the bomb placement radius) (name inferred). */
    void DrawRange();

    /* Creates a CommonBomb task with this laser's mask at position (name inferred). Through this
     * inline function a character info's position is read through the out-of-line operator->, as
     * in the original. */
    void SpawnBomb(const Fn8040Vector3 *position) {
        fn_801DD0BC(position, 1, m_config->bombFlags, 1.0f);
    }

    /* 0x00 */ CityLaserHolder *m_holder;
    /* 0x04 */ const CityLaserConfig *m_config;
    /* 0x08 */ s32 m_state;
    /* 0x0C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x14 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x1C */ boost::shared_ptr<UnkProbeTarget_800587A0> m_probe;
    /* 0x24 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x2C */ std::vector<boost::weak_ptr<UnkProbeTarget_800587A0> > m_hits;
    /* 0x38 */ float m_timer;
    /* 0x3C */ u8 m_tag; /* copied from the owner; passed to fn_801690D0 (name inferred) */
};
CHECK_SIZEOF(CityLaser, 0x40);

class CityLaserHolder : public Gadget {
  public:
    CityLaserHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~CityLaserHolder();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 and its laser (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the parameters from the owner (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Builds the laser (name inferred). */
    void CreateLaser(UnkGadgetOwner_800C9CD0 *owner);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<CityLaser> m_laser;
    /* 0x38 */ CityLaserParams m_params;
    /* 0x44 */ u32 field_0x44; /* not accessed here; the object is 0x48 bytes */
};
CHECK_SIZEOF(CityLaserHolder, 0x48);

const char *CityLaserHolder::s_name = "CityLaserHolder";

/* The predicate that remove_if uses to drop the targets that are gone (name inferred). */
static bool IsExpired(boost::weak_ptr<UnkProbeTarget_800587A0> target) {
    return target.expired();
}

CityLaser::CityLaser(CityLaserHolder *holder, UnkGadgetOwner_800C9CD0 *owner)
    : m_holder(holder), m_config(&s_configs[m_holder->m_params.mode]), m_state(0) {
    m_timer = 0.0f;
    m_probe.reset();
    m_hits.clear();
    m_hits.reserve(8);

    m_charInfo = fn_8015CD90(0x51, 4);
    fn_8040E970(&m_charInfo->m_rotation);
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9E60(owner, &m_tag);

    m_model = fn_8015F800(1, "CITYLASER.BIN");
}

CityLaser::~CityLaser() {}

void CityLaser::Update(float dt) {
    Fn8040Vector3 distance;
    Fn8040Vector3 point;
    Fn8040Vector3 clamped;
    Fn8040Vector3 difference;
    Fn8040Vector3 top;
    Fn8040Vector3 fromAxis;

    switch (m_state) {
    case 0: {
        boost::shared_ptr<CharInfo> character = fn_800A8B74(fn_80074194(), lbl_805EE588).lock();

        if (character.get() != 0) {
            fn_8040CD74(&difference, &character->m_position, &m_charInfo->m_position);
            fn_800091EC(&distance, &difference);
            if (fn_8040CF50(&distance) < m_holder->m_params.rangeLength) {
                SetState(1);
            }
        }
        break;
    }
    case 1:
        m_timer -= dt;
        if (m_timer < 0.0f) {
            if (m_holder->m_params.mode == 0) {
                SetState(2);
            } else {
                SetState(3);
            }
        }
        break;
    case 2: {
        boost::shared_ptr<UnkMotionControl_801602B4> motion = m_model->fn_801602B4(0).lock();

        if (motion.get() != 0) {
            if (stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                SetState(3);
            }
        }
        break;
    }
    case 3: {
        boost::shared_ptr<UnkMotionControl_801602B4> motion = m_model->fn_801602B4(1).lock();

        if (motion.get() != 0) {
            if (stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                SetState(4);
            } else if (m_probe.get() != 0) {
                UnkProbeList_80021FE4 *hits = m_probe->GetHits();

                if (hits != 0) {
                    /* Drop the targets that are gone from the list of the ones that already got
                     * a bomb. */
                    m_hits.erase(std::remove_if(m_hits.begin(), m_hits.end(), IsExpired),
                                 m_hits.end());

                    const Fn8040Vector3 &position = m_charInfo->m_position;
                    top.Set(position.x, s_hitHeight.value + position.y, position.z);
                    UnkSegment_80020A20 segment(top, position);

                    for (UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
                         iterator.node != hits->fn_8001EDEC(); fn_80027374(&iterator, 0)) {
                        boost::shared_ptr<UnkProbeTarget_800587A0> target =
                            fn_800587A0(&iterator.node->target).lock();

                        if (target.get() != 0) {
                            std::vector<boost::weak_ptr<UnkProbeTarget_800587A0> >::iterator known;

                            for (known = m_hits.begin(); known != m_hits.end(); ++known) {
                                boost::shared_ptr<UnkProbeTarget_800587A0> listed = known->lock();

                                if (listed.get() != 0 && listed == target) {
                                    break;
                                }
                            }
                            if (known == m_hits.end()) {
                                m_hits.push_back(boost::weak_ptr<UnkProbeTarget_800587A0>(target));

                                /* Set when the target's owner has the word at 0x50 equal to 1
                                 * (meaning unknown; a different field from the word at 0x5C that
                                 * the bomb tests); only mode 1 checks it. */
                                bool skip = false;

                                if (m_holder->m_params.mode == 1) {
                                    boost::shared_ptr<CharInfo> owner =
                                        fn_800C4568(
                                            fn_80021B08(),
                                            boost::weak_ptr<UnkProbeTarget_800587A0>(target))
                                            .lock();

                                    if (owner.get() != 0 && owner->field_0x50 == 1) {
                                        skip = true;
                                    }
                                }
                                if (!skip) {
                                    const Fn8040Vector3 *position = target->GetPosition();
                                    const Fn8040Vector3 &nearest = fn_8040FC3C(&segment, position);
                                    fn_800091EC(&point, &nearest);
                                    fn_8040CD74(&fromAxis, position, &point);
                                    fn_800091EC(&clamped, &fromAxis);
                                    if (fn_8040CF50(&clamped) > s_hitRadius.value) {
                                        fn_8040CE74(&clamped);
                                        fn_8040CE38(&clamped, s_hitRadius.value);
                                    }
                                    fn_8040CFF8(&point, &clamped);
                                    SpawnBomb(&point);
                                }
                            }
                        }
                    }
                }
            }
        }
        break;
    }
    }

    m_model->fn_80162FB0(dt);
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void CityLaser::SetState(int state) {
    switch (m_state) {
    case 2:
        if (m_target.get() != 0) {
            m_target->fn_80166F88();
            m_target.reset();
        }
        m_model->fn_80160670(0);
        m_model->fn_80162804();
        break;
    case 3:
        if (m_target.get() != 0) {
            m_target->fn_80166F88();
            m_target.reset();
        }
        m_model->fn_80160670(1);
        m_model->fn_80162804();
        fn_800C78B0(fn_80021B08(), &m_probe);
        m_hits.clear();
        break;
    }

    switch (state) {
    case 1:
        m_timer = m_holder->m_params.timer;
        break;
    case 2:
        m_model->fn_80162A68(0, 0);
        m_model->fn_80160C28(0);
        m_model->fn_80160B18(0);
        UpdateModelMatrix();
        {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
            m_target->fn_80167010(-1);
        }
        fn_801D54F4(fn_80053164(), 0x7131, &m_charInfo->m_position, 0, 1);
        fn_801D54F4(fn_80053164(), 0x7132, &m_charInfo->m_position, 0, 1);
        break;
    case 3: {
        m_model->fn_80162A68(1, 0);
        m_model->fn_80160C28(1);
        m_model->fn_80160B18(1);

        float halfLength = 0.5f * s_beamLength.value;
        Fn8040Vector3 offset(0.0f, halfLength, 0.0f);
        Fn8040Vector3 rotation(fn_8000AB90(90.0f), 0.0f, 0.0f);

        m_probe = fn_800C7A84(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), halfLength,
                              s_beamRadius.value, 0x40000, 15, 0, &offset, &rotation, 0);
        fn_801690D0(fn_800CB69C(), m_tag, 1);
        SpawnBomb(&m_charInfo->m_position);
        UpdateModelMatrix();
        if (m_config->hasTarget) {
            m_target = fn_80166AF4();

            UnkWeakHolder_800CC6C8 weakTarget;
            weakTarget.ptr = m_target;
            m_model->fn_8015FA78(fn_80169D00, &weakTarget);
            m_target->fn_80167010(-1);
        }
        fn_8000D664(fn_80009548(), 0, 0.3f, 1.24f, 10.0f);
        fn_801D54F4(fn_80053164(), 0x7133, &m_charInfo->m_position, 0, 1);
        fn_801D54F4(fn_80053164(), 0x7134, &m_charInfo->m_position, 0, 1);
        break;
    }
    }

    m_state = state;
}

void CityLaser::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9E60(owner, &m_tag);
    UpdateModelMatrix();
    DrawRange();
    m_model->fn_80162EF4();
}

void CityLaser::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    m_model->fn_801633DC(&matrix);
}

void CityLaser::DrawRange() {
    Fn8040Vector3 up;
    Fn8040Vector3 rotation;
    Fn8040Vector3 hitUp;
    Fn8040Vector3 hitRotation;
    Fn8040Vector3 center;
    Fn8040Vector3 hitCenter;
    float halfLength;

    fn_802E0E84(&m_charInfo->m_position, lbl_805F06D8, m_holder->m_params.rangeLength);
    fn_802E0E84(&m_charInfo->m_position, UnkDebugColor_802E0E84(0xC0, 0xFF, 0xFF, 0), 50.0f);

    halfLength = 0.5f * s_beamLength.value;
    up.x = 0.0f;
    up.y = halfLength;
    up.z = 0.0f;
    rotation.x = fn_8000AB90(90.0f);
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    fn_8040CDD0(&center, &m_charInfo->m_position, &up);
    fn_802E0B7C(&center, &rotation, UnkDebugColor_802E0E84(0xC0, 0xFF, 0xFF, 0), 0, halfLength,
                s_beamRadius.value);

    halfLength = 0.5f * s_hitHeight.value;
    hitUp.x = 0.0f;
    hitUp.y = halfLength;
    hitUp.z = 0.0f;
    hitRotation.x = fn_8000AB90(90.0f);
    hitRotation.y = 0.0f;
    hitRotation.z = 0.0f;
    fn_8040CDD0(&hitCenter, &m_charInfo->m_position, &hitUp);
    fn_802E0B7C(&hitCenter, &hitRotation, UnkDebugColor_802E0E84(0x80, 0, 0, 0xFF), 0, halfLength,
                s_hitRadius.value);
}

void CityLaserHolder::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_params.rangeLength = 10.0f * m_params.rangeLength;
}

void CityLaserHolder::CreateLaser(UnkGadgetOwner_800C9CD0 *owner) {
    m_laser = boost::shared_ptr<CityLaser>(new CityLaser(this, owner));
}

CityLaserHolder::CityLaserHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    CopyFromOwner(owner);
}

CityLaserHolder::~CityLaserHolder() {}

void CityLaserHolder::UpdateBody(float dt) {
    if (m_laser.get() != 0) {
        m_laser->Update(dt);
        if (m_laser->m_state == 4) {
            fn_8015A794(this);
            m_flags |= 1;
        }
    }
}

void CityLaserHolder::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CityLaserParams previous = m_params;

    CopyFromOwner(owner);
    if (previous.mode != m_params.mode) {
        CreateLaser(owner);
    }
    if (m_laser.get() != 0) {
        m_laser->ReadOwner(owner);
    }
}

extern "C" void fn_800E0984(void) {
    fn_8006B474("CITYLASER.BIN", 1);
}

extern "C" void fn_800E0980(void) {}

void CityLaserHolder::Create(UnkGadgetOwner_800C9CD0 *owner) {
    CityLaserHolder *holder = new CityLaserHolder(fn_801EB304(fn_8001C4F8(), 11), owner);

    holder->CreateLaser(owner);
}

static GadgetParamInfo lbl_80524000[] = {
    {1, "0:NORMAL 1:RENDITION", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "RANGE_LENGTH (m)", 0, 0, 0, 20.0f, 0.1f, 200.0f, fn_800CBC20, 0},
    {4, "TIMER (sec)", 0, 0, 0, 2.0f, 0.0f, 20.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805240A0 = {
    "CityLaser", fn_800E0984, fn_800E0980,  CityLaserHolder::Create, 0, 0, 0x3EA, 1,
    0,           10,          lbl_80524000,
};

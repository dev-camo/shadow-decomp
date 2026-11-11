/* CityLaserHolder (class name from RTTI): the gadget of a laser in the city stage. It holds a
 * CityLaser (named after the boost::checked_deleter<CityLaser> RTTI string and the CITYLASER.BIN
 * model file) that, as far as the code shows (behaviour inferred), waits until a character comes
 * within the range, counts down a timer, then shows the beam and creates a CommonBomb task at its
 * own position when the beam starts and at each new target that the beam's probe reports. File
 * name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are ordered so that deferred inlining emits the functions at their original
 * addresses.
 *
 * NonMatching: checked_delete<CityLaser> is a local symbol where the original's is weak, and some
 * functions are not in the original's order; see the notes in configure.py. */
#include "Game/AngleConversion.h"
#include "Game/CharInfo.h"
#include "Game/CommonBomb.h"
#include "Game/DebugDisp.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkWorld_80021B08.h"
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

using SonicteamUSA::System::CharaColli::CharaColliBase;
using SonicteamUSA::System::Collision::CollisionResultList;

class CityLaser;
class CityLaserHolder;

/* The height of the segment that goes up from the laser, and the radius around it that the bombs
 * are placed in: each bomb goes to the target's position, moved towards the nearest point of the
 * segment until it is at most s_hitRadius away from it (names inferred).
 * Matching workaround: the original sets these two floats and the two below with the unit's
 * start-up function in .ctors (0x800E2570), which shows only that their initialisers are not
 * constant expressions. UnkStartupFloat_800E2570 stands in for those initialisers, which are
 * unknown. The const qualifier (inferred) is measured on s_beamRadius: with it SetState sets up
 * the arguments of the fn_800C7A84 call in the original's order; as an UnkStartupFloat_800E2570
 * object the load of s_beamRadius comes two instructions later and 5 of SetState's 221 words
 * differ. On the other three the qualifier changes no instruction; they have it so that the file
 * uses one form. Revisit when the original initialisers are known. */
static const float s_hitHeight = UnkStartupFloat_800E2570(1000.0f).value;
static const float s_hitRadius = UnkStartupFloat_800E2570(10.0f).value;
/* The length and radius of the beam's probe (names inferred). */
static const float s_beamLength = UnkStartupFloat_800E2570(500.0f).value;
static const float s_beamRadius = UnkStartupFloat_800E2570(10.0f).value;

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

    /* Creates a CommonBomb task with this laser's mask at position (name inferred).
     *
     * Matching workaround: SpawnBomb exists only as an inline member so that the caller's character
     * info position is read through the out-of-line operator->, as in the original. Revisit if
     * the original's helper is found. */
    void SpawnBomb(const Fn8040Vector3 *position) {
        fn_801DD0BC(position, 1, m_config->bombFlags, 1.0f);
    }

    /* 0x00 */ CityLaserHolder *m_holder;
    /* 0x04 */ const CityLaserConfig *m_config;
    /* 0x08 */ s32 m_state;
    /* 0x0C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x14 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x1C */ boost::shared_ptr<CharaColliBase> m_probe;
    /* 0x24 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x2C */ std::vector<boost::weak_ptr<CharaColliBase> > m_hits;
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
static bool IsExpired(boost::weak_ptr<CharaColliBase> target) {
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

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if another compiler build reproduces this without the pragma, or if an original
 * checked_delete keeps out of line a destructor whose body calls only non-inline functions (the
 * pragma would inline that destructor into it). A destructor found inlined into checked_delete in
 * the original would instead mean that the original used auto_inline here. */
#pragma auto_inline on
CityLaser::~CityLaser() {}
#pragma auto_inline reset

void CityLaser::Update(float dt) {
    switch (m_state) {
    case 0: {
        boost::shared_ptr<CharInfo> character = fn_800A8B74(fn_80074194(), lbl_805EE588).lock();

        if (character.get() != 0) {
            Fn8040Vector3 distance = character->m_position - m_charInfo->m_position;
            if (fn_8040CF50(distance) < m_holder->m_params.rangeLength) {
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
        boost::shared_ptr<_stMtnPlayWork> motion = m_model->fn_801602B4(0).lock();

        if (motion.get() != 0) {
            if (stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                SetState(3);
            }
        }
        break;
    }
    case 3: {
        boost::shared_ptr<_stMtnPlayWork> motion = m_model->fn_801602B4(1).lock();

        if (motion.get() != 0) {
            if (stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                SetState(4);
            } else if (m_probe.get() != 0) {
                CollisionResultList *hits = m_probe->GetHits();

                if (hits != 0) {
                    /* Drop the targets that are gone from the list of the ones that already got
                     * a bomb. */
                    m_hits.erase(std::remove_if(m_hits.begin(), m_hits.end(), IsExpired),
                                 m_hits.end());

                    const Fn8040Vector3 &position = m_charInfo->m_position;
                    UnkSegment_80020A20 segment(
                        Fn8040Vector3(position.x, s_hitHeight + position.y, position.z), position);

                    for (UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
                         iterator.node != hits->fn_8001EDEC(); fn_80027374(&iterator, 0)) {
                        boost::shared_ptr<CharaColliBase> target =
                            fn_800587A0(&iterator.node->target).lock();

                        if (target.get() != 0) {
                            std::vector<boost::weak_ptr<CharaColliBase> >::iterator known;

                            for (known = m_hits.begin(); known != m_hits.end(); ++known) {
                                boost::shared_ptr<CharaColliBase> listed = known->lock();

                                if (listed.get() != 0 && listed == target) {
                                    break;
                                }
                            }
                            if (known == m_hits.end()) {
                                m_hits.push_back(boost::weak_ptr<CharaColliBase>(target));

                                /* Set when the target's owner has the word at 0x50 equal to 1
                                 * (meaning unknown; a different field from the word at 0x5C that
                                 * the bomb tests); only mode 1 checks it. */
                                bool skip = false;

                                if (m_holder->m_params.mode == 1) {
                                    boost::shared_ptr<CharInfo> owner =
                                        fn_800C4568(fn_80021B08(),
                                                    boost::weak_ptr<CharaColliBase>(target))
                                            .lock();

                                    if (owner.get() != 0 && owner->field_0x50 == 1) {
                                        skip = true;
                                    }
                                }
                                if (!skip) {
                                    const Fn8040Vector3 *position = target->GetPosition();
                                    Fn8040Vector3 point = fn_8040FC3C(&segment, position);
                                    Fn8040Vector3 clamped = *position - point;
                                    if (fn_8040CF50(clamped) > s_hitRadius) {
                                        fn_8040CE74(&clamped);
                                        fn_8040CE38(&clamped, s_hitRadius);
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

        float halfLength = 0.5f * s_beamLength;
        Fn8040Vector3 offset(0.0f, halfLength, 0.0f);
        UnkRotation3 rotation(fn_8000AB90(90.0f), 0.0f, 0.0f);

        m_probe = fn_800C7A84(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), halfLength,
                              s_beamRadius, 0x40000, 15, 0, &offset, &rotation, 0);
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
    fn_8040B59C(&matrix, m_charInfo->m_position);
    m_model->fn_801633DC(&matrix);
}

void CityLaser::DrawRange() {
    Fn8040Vector3 up;
    UnkRotation3 rotation;
    Fn8040Vector3 hitUp;
    UnkRotation3 hitRotation;
    float halfLength;

    fn_802E0E84(&m_charInfo->m_position, lbl_805F06D8, m_holder->m_params.rangeLength);
    fn_802E0E84(&m_charInfo->m_position, UnkDebugColor_802E0E84(0xC0, 0xFF, 0xFF, 0), 50.0f);

    halfLength = 0.5f * s_beamLength;
    up.x = 0.0f;
    up.y = halfLength;
    up.z = 0.0f;
    rotation.x = fn_8000AB90(90.0f);
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    const Fn8040Vector3 &center = m_charInfo->m_position + up;
    fn_802E0B7C(&center, &rotation, UnkDebugColor_802E0E84(0xC0, 0xFF, 0xFF, 0), 0, halfLength,
                s_beamRadius);

    halfLength = 0.5f * s_hitHeight;
    hitUp.x = 0.0f;
    hitUp.y = halfLength;
    hitUp.z = 0.0f;
    hitRotation.x = fn_8000AB90(90.0f);
    hitRotation.y = 0.0f;
    hitRotation.z = 0.0f;
    const Fn8040Vector3 &hitCenter = m_charInfo->m_position + hitUp;
    fn_802E0B7C(&hitCenter, &hitRotation, UnkDebugColor_802E0E84(0x80, 0, 0, 0xFF), 0, halfLength,
                s_hitRadius);
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

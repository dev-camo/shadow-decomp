/* Coaster (class name from RTTI; the gadget's descriptor is "Coaster"): a ride along a path. As far
 * as the code shows (behaviour inferred), a character that touches it while it waits is sent a
 * command and carried along the path of its path control, and where the path ends the
 * character is launched with the gadget's speed and angle after it lost its control for a time.
 * The five rows of the table of stage settings give the number of paths and some switches for the
 * stage. File name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * NonMatching: 79 of the 85 functions have identical instructions. Five differ in a few words
 * (counted as words of the original that have no equal in this object): the preload hook (5, the
 * registers of the table pointer and the stage number), SetupPath (2, the order of two argument
 * moves), ResetFromOwner (7, the original calls the smart pointer's operator-> out of line for
 * the character info's yaw), the camera event handler (2: the original tests the event argument
 * for null; the source form of that test is unknown) and UpdateBody (14: the stack slots of the
 * path start and direction vectors and of the launch vectors are exchanged, one argument move is
 * scheduled earlier, and the stage pointer is loaded into another register). The sixth, the
 * conversion fn_80131654, has no function of that name here. The destructor, UpdateBody and the
 * camera event handler pass a copy of the follower pointer to fn_801CE9F4 or fn_801CEA54: the
 * original builds that copy with fn_80131654 and destroys it with fn_80009844, so the parameter
 * of those two functions is a shared pointer of another, unknown type, and each copy differs in
 * two relocation targets (this object calls the copy constructor and the destructor of the
 * member's type; the copy constructor it emits has the bytes of fn_80131654). The RTTI and vtable
 * records of the boost and std templates differ in layout from the original's, and the object has
 * additional weak template instances and the RTTI records of the headers' casts. */
/* TNumEdit.h comes before CharaColliBase.h, in the order their names appear in the unit's .rodata
 * (alphabetical order would reverse them). */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBase.h"
#include "Game/CharCommand.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PathCtrl.h"
#include "Game/PlayerCommands.h"
#include "Game/SharedConstants.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_801E72DC.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkProbeTarget_800587A0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"
#include <cmath>

typedef boost::function1<void, CharCommand &> PathCallback;

/* The path follower (placeholder name; the original class is unknown). The coaster keeps one in a
 * shared pointer, sets the start and end of its path and registers it with the manager that
 * fn_80099954 returns. The callback at 0xC4 is called with a CharCommand. */
class UnkPathFollower_800096F4 {
  public:
    /* 0x00 */ u8 field_0x0[0xC4];
    /* 0xC4 */ PathCallback m_callback;
};

/* A value that the constructor at 0x8000B2C0 builds from a float for the registration of the
 * follower (placeholder name; the original class is unknown). */
class UnkFollowerParams_8000B2C0 {
  public:
    UnkFollowerParams_8000B2C0(float value);
    ~UnkFollowerParams_8000B2C0();

    /* 0x00 */ u8 field_0x0[0x30];
};
CHECK_SIZEOF(UnkFollowerParams_8000B2C0, 0x30);

class UnkFollowerManager_80099954;

extern "C" {
/* Float helpers (descriptions inferred from the code that calls them). */
/* Moves current towards target by at most maxStep (behaviour inferred). */
float fn_8012773C(float target, float current, float maxStep);
/* Returns the address of whichever of value, low and high lies in between (a clamp; behaviour
 * inferred). */
const float *fn_80070460(const float *value, const float *low, const float *high);

/* Path control functions (behaviour inferred). */
/* The position where the path starts. */
Fn8040Vector3 fn_80018634(SonicteamUSA::System::PathCtrl *path);
/* The rotation angles of the path at its start (the numbers are not understood). */
Fn8040Vector3 fn_80018434(SonicteamUSA::System::PathCtrl *path, int arg1, int arg2);

/* The follower manager (behaviour inferred). */
UnkFollowerManager_80099954 *fn_80099954(void);
/* The parameter type of these two is known to be wrong: the original builds the argument with
 * the conversion at 0x80131654 and destroys it with fn_80009844, not with the copy constructor and
 * destructor of shared_ptr<UnkPathFollower_800096F4>, so it is a shared pointer of another type,
 * which is not known. */
/* Releases the follower from the manager. */
void fn_801CE9F4(UnkFollowerManager_80099954 *manager, boost::shared_ptr<UnkPathFollower_800096F4>);
/* Registers the follower with the manager (the numbers are not understood). */
void fn_801CEA54(UnkFollowerManager_80099954 *manager, boost::shared_ptr<UnkPathFollower_800096F4>,
                 int arg2, const UnkFollowerParams_8000B2C0 &params, int arg4, int arg5);

/* Model effect helpers (behaviour inferred). */
void fn_8015FD10(GadgetModelInfo *model);
void fn_8015FDD8(GadgetModelInfo *model, float fraction);
void fn_8016082C(GadgetModelInfo *model, int arg1);
/* Owner queries (behaviour inferred): the owner's flag word, its range and a call that finishes
 * it. */
void fn_800C9E70(UnkGadgetOwner_800C9CD0 *owner, u32 *flags);
float fn_800C9938(UnkGadgetOwner_800C9CD0 *owner);
/* Creates a path follower (behaviour inferred; the numbers are not understood). */
boost::shared_ptr<UnkPathFollower_800096F4> fn_80009704(int arg0, int arg1);
/* Sets the start and end of the path that the follower moves along, and its up direction. */
void fn_8000A5D4(UnkPathFollower_800096F4 *follower, const Fn8040Vector3 *start,
                 const Fn8040Vector3 *end);
void fn_8000ACE4(UnkPathFollower_800096F4 *follower, const Fn8040Vector3 *up);
/* Creates the object of the gadget (the number is not understood). */
boost::shared_ptr<UnkObject_801E72DC> fn_801E7128(boost::weak_ptr<CharInfo> info, int arg1);
}

/* Converts degrees to radians (name inferred). */
static inline float DegreesToRadians(float degrees) {
    return degrees * (lbl_805FAD38 / 180.0f);
}

/* One row of the table of stage settings (name inferred). The meaning of most members is not
 * known; the comments say where the code uses them. */
struct CoasterStage {
    /* 0x00 */ s32 stage;        /* compared with the word at 0xAC of lbl_8057D6A0 */
    /* 0x04 */ float field_0x4;  /* the y of the position and of both ends of the follower path */
    /* 0x08 */ u32 pathCount;    /* the number of paths */
    /* 0x0C */ const s32 *paths; /* the path number of each path */
    /* 0x10 */ u8 field_0x10;    /* when set, the matrix keeps only the y angle of its rotation */
    /* 0x11 */ u8 field_0x11;    /* the last argument of the path move in state 2 */
    /* 0x12 */ u8 field_0x12;    /* gates calls of the model's functions */
    /* 0x13 */ u8 field_0x13;    /* gates the roll that the camera event adds */
    /* 0x14 */ u8 field_0x14;    /* selects how a ride ends */
    /* 0x18 */ float field_0x18; /* the y of an offset of the start of the path callback */
    /* 0x1C */ float field_0x1C; /* the z of that offset */
    /* 0x20 */ float field_0x20; /* added to the height of the end of the path callback */
    /* 0x24 */ float field_0x24; /* the argument of the follower parameters */
    /* 0x28 */ const char *effectFile; /* a file for the effect at the end of a ride, or null */
};
CHECK_SIZEOF(CoasterStage, 0x2C);

/* The radius of the probe sphere (name inferred). */
static UnkStartupFloat_800E2570 s_probeRadius(10.0f);

/* The settings row that the preload hook selected for the current stage, or null. */
static const CoasterStage *s_stage;

/* The gadget parameters in the order of the descriptor's table below. */
struct CoasterParams {
    /* 0x00 */ u32 pathNo;
    /* 0x04 */ float moveSpeed;     /* MOVE_SPPED m/sec; the code scales it by 10 */
    /* 0x08 */ float launchSpeed;   /* LAUNCH_SPD m/sec; scaled by 10 */
    /* 0x0C */ float launchAngle;   /* LAUNCH_ANG deg; converted to radians */
    /* 0x10 */ float noControlTime; /* LAUNCH_NO_CTRL_TIME sec */
};
CHECK_SIZEOF(CoasterParams, 0x14);

class Coaster : public Gadget {
  public:
    Coaster(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Coaster();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    virtual bool IsFinished();

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the parameters and flags from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the matrix and the object at the character info (name inferred). */
    void UpdateModelMatrix();
    /* Builds the two matrices from the rotation and the roll, and the position of the ride
     * (name inferred). */
    void BuildMatrices();
    /* Creates the path control for the parameter's path and places the character info at its
     * start (name inferred). */
    void SetupPath();
    /* Sends the carried character the command to follow the ride (name inferred). */
    void SendCommand(boost::shared_ptr<CharInfo> hit);
    /* The callbacks (names inferred). */
    void OnPathCallback(CharCommand &command);
    void OnCameraEvent(CameraUpdateEventArgs &args);
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<SonicteamUSA::System::PathCtrl> m_path;
    /* 0x38 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x40 */ boost::shared_ptr<UnkObject_801E72DC> m_object;
    /* 0x48 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x50 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0x58 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* States 0 and 3 wait at the start of the path (3 after a ride that was reversed), 1 moves
     * along the path and commands the character, 2 moves along it without (names inferred). */
    /* 0x60 */ s32 m_state;
    /* 0x64 */ CoasterParams m_params;
    /* 0x78 */ s32 m_reversed; /* 1 when the ride goes the other way (inferred) */
    /* The rotation angles of the path, set from fn_80018434 and by the path move. */
    /* 0x7C */ Fn8040Vector3 m_rotation;
    /* 0x88 */ Fn8040Matrix34 m_matrix;
    /* 0xB8 */ Fn8040Matrix34 m_matrix2; /* the matrix turned by pi when reversed */
    /* 0xE8 */ Fn8040Vector3 m_position;
    /* 0xF4 */ Fn8040Vector3 m_angles; /* the angles of m_matrix2 (inferred) */
    /* 0x100 */ float m_roll;          /* an angle about the z axis that returns to 0 (inferred) */
    /* 0x104 */ float m_timer; /* set to 0.4 when a ride starts and counted down (inferred) */
    /* 0x108 */ UnkSoundHandle_801D3DC8 m_sound;
    /* 0x124 */ u8 m_fading; /* set by an owner flag; fades the model in over 0.6 s (inferred) */
    /* 0x128 */ float m_fadeTime;
    /* 0x12C */ boost::shared_ptr<UnkPathFollower_800096F4> m_follower;
    /* 0x134 */ u32 field_0x134; /* not accessed here; the object is 0x138 bytes */
};
CHECK_SIZEOF(Coaster, 0x138);

/* The path numbers of the stage rows (names inferred). */
static const s32 s_paths9[4] = {0x1E, 0x1F, 0x20, 0x21};
static const s32 s_paths20[3] = {1, 2, 7};
static const s32 s_paths25[5] = {1, 2, 3, 4, 5};
static const s32 s_paths22[3] = {0x1E, 0x1F, 0x20};
static const s32 s_paths11[2] = {5, 6};

/* The settings of the stages the coaster is in (name inferred). */
static const CoasterStage s_stages[5] = {
    {9, -30.0f, 4, s_paths9, 1, 1, 0, 1, 0, 20.0f, 100.0f, 10.0f, 2.0f, 0},
    {11, -23.0f, 2, s_paths11, 0, 1, 1, 1, 0, 10.0f, 110.0f, 20.0f, 2.0f, 0},
    {20, -19.0f, 3, s_paths20, 0, 1, 0, 1, 1, 0.0f, 60.0f, -10.0f, 0.5f, "A10503COASTER_BRK.DFF"},
    {25, -19.0f, 5, s_paths25, 0, 1, 0, 1, 1, -20.0f, 50.0f, 5.0f, 0.3f, "A10603COASTER_BRK.DFF"},
    {22, -30.0f, 3, s_paths22, 0, 1, 1, 1, 1, 10.0f, 80.0f, 10.0f, 2.0f, 0},
};

const char *Coaster::s_name = "Coaster";

Coaster::Coaster(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_state(0), m_reversed(0), m_rotation(0.0f, 0.0f, 0.0f),
      m_angles(0.0f, 0.0f, 0.0f) {
    m_name = s_name;
    m_follower = fn_80009704(0, 0);
    m_follower->m_callback = PathCallback(boost::bind(&Coaster::OnPathCallback, this, _1));

    m_charInfo = fn_8015CD90(0x43, 4);
    m_charInfo->field_0x30 = 1;
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_cameraCallback =
        CameraUpdateCallback(boost::bind(&Coaster::OnCameraEvent, this, _1));
    ReadOwner(owner);

    m_model = fn_8015F800(1, "COASTER.BIN");
    m_model->fn_80162A68(0, m_fading);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));
    SetupPath();
    BuildMatrices();

    {
        boost::shared_ptr<UnkProbeTarget_800587A0> probe = fn_800C7EA8(
            fn_80021B08(), &m_position, 0x100, 10, 0, boost::weak_ptr<CharInfo>(lbl_805EE698),
            &lbl_805E07A0, s_probeRadius.value);

        m_handle = fn_8015B8C4(probe);
        m_handle->field_0x0.fn_800F5980(
            ColliDetectCallback(boost::bind(&Coaster::OnCollision, this, _1)));
    }
    m_object = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x2000);
    UpdateModelMatrix();

    m_target = fn_80166AF4();

    UnkWeakHolder_800CC6C8 weakTarget;
    weakTarget.ptr = m_target;
    m_model->fn_8015FA78(fn_80169D00, &weakTarget);
}

Coaster::~Coaster() {
    fn_801CE9F4(fn_80099954(), m_follower);
    m_sound.fn_801D455C();
}

/* Loads the model and selects the row of the table of stage settings for the current stage. */
extern "C" void fn_80132BC8(void) {
    fn_8006B474("COASTER.BIN", 1);
    for (int i = 0; i < 5; i++) {
        if (lbl_8057D6A0.field_0xAC == s_stages[i].stage) {
            s_stage = &s_stages[i];
            break;
        }
    }
}

extern "C" void fn_80132BBC(void) {
    s_stage = 0;
}

void Coaster::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new Coaster(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void Coaster::UpdateBody(float dt) {
    if (m_path.get() == 0) {
        return;
    }
    if (!fn_800ABDCC(m_path.get())) {
        return;
    }

    boost::shared_ptr<CharInfo> character = fn_800A8B74(fn_80074194(), lbl_805EE588).lock();

    if (character.get() == 0) {
        return;
    }

    if (m_fading) {
        m_fadeTime += dt;
        if (m_fadeTime > 0.6f) {
            fn_8015FD10(m_model.get());
            m_fading = 0;
        } else {
            fn_8015FDD8(m_model.get(), m_fadeTime / 0.6f);
        }
    }

    if (!fn_8000B044(m_roll)) {
        m_roll = fn_8012773C(0.0f, m_roll, DegreesToRadians(240.0f * dt));

        /* The roll stays within 90 degrees to either side. Declared first and assigned high
         * first: initialising the two at their declarations changes the code. */
        float low;
        float high;

        high = DegreesToRadians(90.0f);
        low = DegreesToRadians(-90.0f);

        m_roll = *fn_80070460(&m_roll, &low, &high);
    }

    switch (m_state) {
    case 0:
    case 3: {
        /* Waiting: stays at the start of the path. */
        GadgetCharInfo *info = m_charInfo.get();
        const Fn8040Vector3 &start = fn_80018634(m_path.get());

        fn_800091D0(&info->m_position, &start);
        m_rotation = fn_80018434(m_path.get(), 1, 0);
        BuildMatrices();
        break;
    }

    case 1: {
        bool arrived = fn_8001C91C(fn_8001B494(), m_path.get(), m_reversed, &m_charInfo->m_position,
                                   m_params.moveSpeed * dt, &m_rotation, 1);

        BuildMatrices();
        if (arrived) {
            fn_801CE9F4(fn_80099954(), m_follower);

            Fn8040Vector3 launch;
            Fn8040Vector3 launchTemporary;
            Fn8040Vector3 turnedTemporary;

            launch.Set(0.0f, 0.0f, -1.0f);

            float pitch = m_params.launchAngle;
            float angle = m_rotation.y;

            if (m_reversed == 1) {
                angle += DegreesToRadians(180.0f);
            }

            fn_8040D118(&launchTemporary, &launch, pitch);
            fn_800091D0(&launch, &launchTemporary);
            fn_8040D0AC(&turnedTemporary, &launch, angle);
            fn_800091D0(&launch, &turnedTemporary);
            fn_8040CE38(&launch, m_params.launchSpeed);

            character->fn_801DC980(new Player::IgnoreInputCommand(0, m_params.noControlTime));
            character->fn_801DC980(new ClrVelocityCommand(4));
            character->fn_801DC980(new SetPositionCommand(&m_position, 4));
            character->fn_801DC980(new SetVelocityCommand(&launch, 4));
            character->fn_801DC980(new Player::LaunchCommand(0, 4));

            if (m_target.get() != 0) {
                m_target->fn_80166F88();
            }

            UnkResourceRecord_80208848 *record = fn_80208848(
                static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, "COASTER.BIN")),
                1);

            if (record != 0) {
                fn_801D54F4(fn_80053164(), record->field_0x8, &m_charInfo->m_position, 0, 1);
            }
            m_sound.fn_801D455C();

            if (!s_stage->field_0x14) {
                if (m_reversed == 0) {
                    m_state = 3;
                } else {
                    m_state = 0;
                }
                m_handle->field_0x18.ClearFlags(2);
                m_handle->SuspendProbe(0.4f);
                if (s_stage->field_0x12) {
                    m_model->fn_801604FC();
                }
            } else {
                m_model->fn_80161880();
                m_model->fn_80162790();
                if (s_stage->effectFile != 0) {
                    UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

                    effect.field_0x44 =
                        static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, s_stage->effectFile));
                    effect.field_0x40 = m_charInfo->m_position.y;
                    effect.field_0x34 = 1.0f;
                    effect.field_0x4A = 0;
                    effect.field_0x54 = 1;
                    fn_8015D93C(&effect);
                }
                fn_800C9AD4(m_owner);
                m_flags |= 1;
                return;
            }
        } else {
            if (m_timer > 0.0f) {
                m_timer -= dt;
            }
            SendCommand(character);
        }
        break;
    }

    case 2: {
        bool arrived = fn_8001C91C(fn_8001B494(), m_path.get(), m_reversed, &m_charInfo->m_position,
                                   m_params.moveSpeed * dt, &m_rotation, s_stage->field_0x11);

        BuildMatrices();
        if (arrived) {
            if (m_target.get() != 0) {
                m_target->fn_80166F88();
            }

            UnkResourceRecord_80208848 *record = fn_80208848(
                static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, "COASTER.BIN")),
                1);

            if (record != 0) {
                fn_801D54F4(fn_80053164(), record->field_0x8, &m_charInfo->m_position, 0, 1);
            }
            m_sound.fn_801D455C();

            if (!s_stage->field_0x14) {
                m_handle->field_0x18.ClearFlags(2);
                if (s_stage->field_0x12) {
                    m_model->fn_801604FC();
                }
                if (m_reversed == 0) {
                    m_state = 3;
                } else {
                    m_state = 0;
                }
            } else {
                m_model->fn_80161880();
                m_model->fn_80162790();
                if (s_stage->effectFile != 0) {
                    UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

                    effect.field_0x44 =
                        static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, s_stage->effectFile));
                    effect.field_0x40 = m_charInfo->m_position.y;
                    effect.field_0x34 = 1.0f;
                    effect.field_0x4A = 0;
                    effect.field_0x54 = 1;
                    fn_8015D93C(&effect);
                }
                fn_800C9AD4(m_owner);
                m_flags |= 1;
                return;
            }
        }
        break;
    }
    }

    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
        m_handle->field_0x18.m_probe->SetPosition(&m_position);
    }
}

void Coaster::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CoasterParams previous = m_params;

    ReadOwner(owner);
    if (previous.pathNo != m_params.pathNo) {
        m_state = 0;
        SetupPath();
        m_handle->field_0x18.ClearFlags(2);
    }
    BuildMatrices();
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
    if (m_handle.get() != 0) {
        fn_8015BE64(m_handle.get());
    }

    /* Draws a debug line from the position along the launch direction (behaviour inferred). */
    Fn8040Vector3 direction(0.0f, 0.0f, -1.0f);
    float yaw = m_charInfo->m_rotation.y;
    Fn8040Vector3 directionTemporary;
    Fn8040Vector3 rotatedTemporary;
    Fn8040Vector3 end;

    fn_8040D118(&directionTemporary, &direction, m_params.launchAngle);
    fn_800091D0(&direction, &directionTemporary);
    fn_8040D0AC(&rotatedTemporary, &direction, yaw);
    fn_800091D0(&direction, &rotatedTemporary);
    fn_8040CE38(&direction, 50.0f);
    fn_8040CDD0(&end, &m_position, &direction);
    fn_802E0B58(UnkSegment_80020A20(m_position, end), lbl_805F1370);
}

bool Coaster::IsFinished() {
    if (m_owner != 0) {
        if (fn_800C9A64(m_owner)) {
            m_flags |= 1;
            return true;
        }

        if (m_state == 2 || m_state == 3) {
            boost::shared_ptr<CharInfo> character = fn_800A8B74(fn_80074194(), lbl_805EE588).lock();

            if (character.get() != 0) {
                Fn8040Vector3 distance;
                Fn8040Vector3 difference;

                fn_8040CD74(&difference, &character->m_position, &m_charInfo->m_position);
                fn_800091EC(&distance, &difference);
                if (fn_8040CF50(&distance) > fn_800C9938(m_owner)) {
                    m_flags |= 1;
                    return true;
                }
            }
        } else if (m_state == 0) {
            if (fn_800C996C(m_owner)) {
                m_flags |= 1;
                return true;
            }
        }
    }

    return false;
}

void Coaster::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    u32 ownerFlags;

    fn_800C9E70(owner, &ownerFlags);
    if (ownerFlags & 0x400000) {
        m_fading = 1;
        m_fadeTime = 0.0f;
    } else {
        m_fading = 0;
        m_fadeTime = 0.0f;
    }

    if (m_params.pathNo >= s_stage->pathCount) {
        m_params.pathNo = s_stage->pathCount - 1;
        fn_800C9D68(owner, &m_params, sizeof(m_params));
    }

    m_params.moveSpeed = 10.0f * m_params.moveSpeed;
    m_params.launchSpeed = 10.0f * m_params.launchSpeed;
    m_params.launchAngle = DegreesToRadians(m_params.launchAngle);
}

void Coaster::SetupPath() {
    m_path.reset();
    m_path = boost::shared_ptr<SonicteamUSA::System::PathCtrl>(new SonicteamUSA::System::PathCtrl);
    if (!fn_8001C7B4(fn_8001B494(), m_path.get(), s_stage->paths[m_params.pathNo])) {
        m_path.reset();
    }

    m_reversed = 0;
    m_roll = 0.0f;
    if (m_path.get() != 0) {
        fn_80018304(m_path.get());

        GadgetCharInfo *info = m_charInfo.get();
        const Fn8040Vector3 &start = fn_80018634(m_path.get());

        fn_800091D0(&info->m_position, &start);

        m_rotation = fn_80018434(m_path.get(), 1, 0);
    }
    if (s_stage->field_0x12) {
        m_model->fn_801604FC();
    }
}

void Coaster::BuildMatrices() {
    Fn8040Vector3 angles;
    Fn8040Vector3 axis;
    Fn8040Vector3 origin;
    Fn8040Vector3 axisTemporary;
    Fn8040Vector3 originTemporary;
    Fn8040Matrix34 rotation;
    Fn8040Matrix34 translation;
    Fn8040Matrix34 rolled;
    Fn8040Matrix34 rolled2;
    Fn8040Matrix34 moved;
    Fn8040Matrix34 moved2;

    fn_8040C148(&m_matrix, &lbl_805E0720);
    fn_8040B80C(&m_matrix, m_rotation.z);
    fn_8040B884(&m_matrix, m_rotation.x);
    fn_8040B848(&m_matrix, m_rotation.y);
    if (s_stage->field_0x10) {
        angles.Set(0.0f, 0.0f, 0.0f);
        fn_800494F8(&angles, &m_matrix);
        fn_8040C148(&m_matrix, &lbl_805E0720);
        fn_8040B848(&m_matrix, angles.y);
    }

    fn_8040C148(&m_matrix2, &m_matrix);
    if (m_reversed == 1) {
        fn_8040B848(&m_matrix2, DegreesToRadians(180.0f));
    }

    if (!fn_8000B044(m_roll)) {
        axis.Set(0.0f, 0.0f, 1.0f);
        fn_8040B67C(&axisTemporary, &m_matrix, &axis);
        fn_800091D0(&axis, &axisTemporary);
        fn_800498A0(&rotation, &axis, m_roll);
        fn_8001E044(&rolled, &rotation, &m_matrix);
        fn_8040C148(&m_matrix, &rolled);
        fn_8001E044(&rolled2, &rotation, &m_matrix2);
        fn_8040C148(&m_matrix2, &rolled2);
    }

    fn_800493C4(&m_charInfo->m_rotation, &m_matrix);

    fn_8040C148(&translation, &lbl_805E0720);
    fn_8040B59C(&translation, &m_charInfo->m_position);
    fn_8001E044(&moved, &translation, &m_matrix);
    fn_8040C148(&m_matrix, &moved);
    fn_8001E044(&moved2, &translation, &m_matrix2);
    fn_8040C148(&m_matrix2, &moved2);

    origin.Set(0.0f, s_stage->field_0x4, 0.0f);
    fn_8040B67C(&originTemporary, &m_matrix, &origin);
    fn_800091D0(&m_position, &originTemporary);
}

void Coaster::UpdateModelMatrix() {
    m_model->fn_801633DC(&m_matrix);
    fn_800CCE38(m_object.get(), &m_charInfo->m_position);
    if (m_handle.get() != 0) {
        m_handle->field_0x18.m_probe->SetPosition(&m_position);
    }
}

void Coaster::OnPathCallback(CharCommand &command) {
    Fn8040Matrix34 matrix;
    Fn8040Vector3 angles;
    Fn8040Vector3 start;
    Fn8040Vector3 end;
    Fn8040Vector3 up;
    Fn8040Vector3 startOffset;
    Fn8040Vector3 startMoved;
    Fn8040Vector3 startTemporary;
    Fn8040Vector3 endMoved;
    Fn8040Vector3 endTemporary;
    Fn8040Vector3 upTemporary;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B80C(&matrix, m_rotation.z);
    fn_8040B884(&matrix, m_rotation.x);
    fn_8040B848(&matrix, m_rotation.y);
    if (s_stage->field_0x10) {
        angles.Set(0.0f, 0.0f, 0.0f);
        fn_800494F8(&angles, &matrix);
        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B848(&matrix, angles.y);
    }
    if (m_reversed == 1) {
        fn_8040B848(&matrix, DegreesToRadians(180.0f));
    }

    start.Set(0.0f, s_stage->field_0x4, 0.0f);
    fn_80008E1C(&startOffset, 0.0f, s_stage->field_0x18, s_stage->field_0x1C);
    fn_8040CFF8(&start, &startOffset);

    GadgetCharInfo *startInfo = m_charInfo.get();

    fn_8040B67C(&startTemporary, &matrix, &start);
    fn_8040CDD0(&startMoved, &startTemporary, &startInfo->m_position);
    fn_800091D0(&start, &startMoved);

    end.Set(0.0f, s_stage->field_0x4, 0.0f);
    end.y += s_stage->field_0x20;
    /* A direct m_charInfo->m_position changes the code. */
    GadgetCharInfo *endInfo = m_charInfo.get();

    fn_8040B67C(&endTemporary, &matrix, &end);
    fn_8040CDD0(&endMoved, &endTemporary, &endInfo->m_position);
    fn_800091D0(&end, &endMoved);
    fn_8000A5D4(m_follower.get(), &start, &end);

    up.Set(0.0f, 1.0f, 0.0f);
    fn_8040B67C(&upTemporary, &matrix, &up);
    fn_800091D0(&up, &upTemporary);
    fn_8000ACE4(m_follower.get(), &up);
}

void Coaster::SendCommand(boost::shared_ptr<CharInfo> hit) {
    fn_800494F8(&m_angles, &m_matrix2);
    hit->fn_801DC980(new Player::TouchHangCommand(boost::weak_ptr<CharInfo>(m_charInfo),
                                                  &m_position, &m_angles, 3, 4));
}

void Coaster::OnCollision(ColliDetectArgs &args) {
    if (m_state == 0 || m_state == 3) {
        if (fn_8015D2EC(args.m_hits)) {
            boost::shared_ptr<CharInfo> character = fn_800A8B74(fn_80074194(), lbl_805EE588).lock();

            if (character.get() != 0) {
                SendCommand(character);
            }
        }
    }
}

void Coaster::OnCameraEvent(CameraUpdateEventArgs &args) {
    switch (args.m_type) {
    case 0xB:
        if (m_state == 0 || m_state == 3) {
            m_handle->field_0x18.SetFlags(2);
            if (s_stage->field_0x12) {
                m_model->fn_80160C28(0);
                fn_8016082C(m_model.get(), 0);
                m_model->fn_80160B18(0);
            }

            if (m_state == 0) {
                m_reversed = 0;
                if (m_target.get() != 0) {
                    m_target->fn_80167010(1);
                }
            } else {
                m_reversed = 1;
                if (m_target.get() != 0) {
                    m_target->fn_80167010(2);
                }
            }
            m_timer = 0.4f;

            {
                UnkFollowerParams_8000B2C0 params(s_stage->field_0x24);

                fn_801CEA54(fn_80099954(), m_follower, 2, params, 0, 1);
            }

            UnkResource_80012700 *resource =
                static_cast<UnkResource_80012700 *>(fn_80012700(fn_800122D4(), 1, "COASTER.BIN"));
            UnkResourceRecord_80208848 *record = fn_80208848(resource, 0);

            if (record != 0) {
                fn_801D54F4(fn_80053164(), record->field_0x8, &m_charInfo->m_position, 0, 1);
            }

            record = fn_80208848(resource, 2);
            if (record != 0) {
                m_sound = fn_801D3DC8(record->field_0x8, &m_charInfo->m_position, this);
            }
            m_state = 1;
        }
        break;

    case 0xC:
        fn_801CE9F4(fn_80099954(), m_follower);
        m_state = 2;
        break;

    case 1:
        if (s_stage->field_0x13 && m_timer <= 0.0f) {
            Fn8040Matrix34 matrix;
            Fn8040Vector3 angles;
            Fn8040Vector3 direction;
            Fn8040Vector3 directionTemporary;
            /* Built in three statements: a single expression changes the code. */
            float strength = 540.0f;

            strength *= lbl_8057D6A0.field_0x28;
            strength *= fn_8040CF50(&args.m_vector);

            fn_8040C148(&matrix, &lbl_805E0720);
            fn_8040B80C(&matrix, m_rotation.z);
            fn_8040B884(&matrix, m_rotation.x);
            fn_8040B848(&matrix, m_rotation.y);
            if (s_stage->field_0x10) {
                angles.Set(0.0f, 0.0f, 0.0f);
                fn_800494F8(&angles, &matrix);
                fn_8040C148(&matrix, &lbl_805E0720);
                fn_8040B848(&matrix, angles.y);
            }
            fn_8040BCD4(&matrix);

            fn_8040B67C(&directionTemporary, &matrix, &args.m_vector);
            fn_800091EC(&direction, &directionTemporary);
            direction.y = 0.0f;
            fn_8040CE74(&direction);

            float side = 0.0f;
            float limit = std::cos(DegreesToRadians(60.0f));

            if (fn_8000A980(direction.x) > limit) {
                side = direction.x;
            }
            m_roll += DegreesToRadians(strength * side);
        }
        break;
    }

    args.m_flags |= 2;
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "PATH_NO", 0, 0, 100, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "MOVE_SPPED m/sec", 0, 0, 0, 50.0f, 0.01f, 65536.0f, fn_800CBC20, 0},
    {4, "LAUNCH_SPD m/sec", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "LAUNCH_ANG deg", 0, 0, 0, 0.0f, -90.0f, 90.0f, fn_800CBC20, 0},
    {4, "LAUNCH_NO_CTRL_TIME sec", 0, 0, 0, 0.0f, 0.0f, 100.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805289AC = {
    "Coaster", fn_80132BC8, fn_80132BBC, Coaster::Create, 0, 0, 0xC88, 1, 0, 10, s_paramInfo,
};

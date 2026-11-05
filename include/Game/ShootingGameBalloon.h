#ifndef GAME_SHOOTINGGAMEBALLOON_H
#define GAME_SHOOTINGGAMEBALLOON_H

#include "Game/CharaColliAttack.h"
#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <cstring>
#include "layout_check.h"

namespace ShootingGame {

namespace Kind {

/* The balloon kinds (ShootingGame::Kind::__Kind from the RTTI names of the RandomSampling
 * instances). The enumerator names are inferred: they are the letters of the fields of the "Rate"
 * parameter's label, "B:N:E:G", whose bytes give the numbers of balloons of each kind. A burst
 * balloon adds 1 (B), -2 (E) or 2 (N) to the count of its shooting game entry, or nothing (G). */
enum __Kind { B = 0, E = 1, N = 2, G = 3 };

} // namespace Kind

/* ShootingGame::RandomSampling (class name from the RTTI names of its shared_ptr deleters): a
 * random sample of a fixed size from the values added to it (Add keeps each of the values seen so
 * far with the same chance), which is then shuffled and taken one by one. Method names are
 * inferred. The members are defined below the class. */
template <class T> class RandomSampling {
  public:
    RandomSampling(int size);
    ~RandomSampling();

    /* Offers value to the sample. */
    void Add(T value);
    /* Removes the last sampled value and returns it. */
    T Pop();
    /* Puts the sampled values in random order. */
    void Shuffle();

    /* 0x0 */ T *m_values;
    /* 0x4 */ int m_size;  /* the sample size, lowered by Pop */
    /* 0x8 */ int m_count; /* the number of values offered */
};

/* Only the first value is cleared (sizeof(T) bytes), as in the original. */
template <class T> RandomSampling<T>::RandomSampling(int size) {
    m_values = new T[size];
    memset(m_values, 0, sizeof(T));
    m_size = size;
    m_count = 0;
}

template <class T> RandomSampling<T>::~RandomSampling() {
    delete[] m_values;
    m_values = 0;
    m_size = 0;
    m_count = 0;
}

template <class T> void RandomSampling<T>::Add(T value) {
    if (++m_count <= m_size) {
        m_values[m_count - 1] = value;
    } else {
        int index = m_count * fn_8040EE90();

        if (index < m_size) {
            m_values[index] = value;
        }
    }
}

template <class T> T RandomSampling<T>::Pop() {
    return m_values[--m_size];
}

template <class T> void RandomSampling<T>::Shuffle() {
    for (int i = 0; i < m_size; i++) {
        int other = m_size * fn_8040EE90();
        T value = m_values[i];

        m_values[i] = m_values[other];
        m_values[other] = value;
    }
}

} // namespace ShootingGame

/* What a balloon is created with (placeholder name; the binary does not name it). The
 * generator fills one per balloon; the balloon keeps a copy. */
struct UnkBalloonParam_8013B2C0 {
    UnkBalloonParam_8013B2C0()
        : m_speed(0.0f), m_height(0.0f), m_kind(ShootingGame::Kind::B), m_id(-1) {}

    /* Copies the members one by one, the position with fn_800091D0. */
    UnkBalloonParam_8013B2C0 &operator=(const UnkBalloonParam_8013B2C0 &other) {
        fn_800091D0(&m_position, &other.m_position);
        m_speed = other.m_speed;
        m_height = other.m_height;
        m_kind = other.m_kind;
        m_id = other.m_id;
        m_generator = other.m_generator;
        return *this;
    }

    /* 0x00 */ Fn8040Vector3 m_position;
    /* 0x0C */ float m_speed;  /* rise per unit of dt, scaled by 30 */
    /* 0x10 */ float m_height; /* the balloon is finished once it has risen this far */
    /* 0x14 */ ShootingGame::Kind::__Kind m_kind;
    /* 0x18 */ s32 m_id; /* the shooting game entry, or -1 */
    /* The character info of the generator, which receives a DeadCommand when the balloon bursts. */
    /* 0x1C */ boost::weak_ptr<GadgetCharInfo> m_generator;
};
CHECK_SIZEOF(UnkBalloonParam_8013B2C0, 0x24);

namespace ShootingGame {

/* ShootingGame::Balloon (class name from RTTI): one balloon of the PARK_BALLOON gadget. It rises
 * from its start position until it has risen m_param.m_height, or bursts when an attack hits it:
 * then it changes the count of its shooting game entry by its kind, sends a DeadCommand to the
 * generator, starts the kind's effect and plays the kind's sounds. Method names marked
 * "(name inferred)" are not original. */
class Balloon : public Task {
  public:
    Balloon(Task *parent, const UnkBalloonParam_8013B2C0 &param);
    virtual ~Balloon();
    /* 0 new, 1 rising, 2 burst by an attack, 3 finished (inferred). */
    virtual void Update(float dt);

    /* Creates a balloon under task layer 11 (name inferred). */
    static void Create(const UnkBalloonParam_8013B2C0 &param);

    /* Places the model at the character info and the target 10 units above it (name inferred). */
    void UpdateModelMatrix();
    /* The attack object's callback: a rising balloon bursts unless bit 1 of the attack's word at
     * 0x40 is set (name inferred). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x28 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x30 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x38 */ boost::shared_ptr<Target> m_target;
    /* 0x40 */ s32 m_state;
    /* 0x44 */ UnkBalloonParam_8013B2C0 m_param;
};
CHECK_SIZEOF(ShootingGame::Balloon, 0x68);

/* The gadget parameters in the order of the descriptor's table. The constructor clears them. */
struct BalloonGeneratorParams {
    BalloonGeneratorParams()
        : height(0.0f), radius(0.0f), minSpeed(0.0f), maxSpeed(0.0f), num(0), maxNum(0), rate(0),
          sec(0.0f), id(0) {}

    /* 0x00 */ float height;   /* Height */
    /* 0x04 */ float radius;   /* Radius */
    /* 0x08 */ float minSpeed; /* MinSpeed */
    /* 0x0C */ float maxSpeed; /* MaxSpeed */
    /* 0x10 */ s32 num;        /* Num: balloons per round */
    /* 0x14 */ s32 maxNum;     /* MaxNum */
    /* Rate: four byte counts, from the most significant byte B, N, E and G (see Kind). */
    /* 0x18 */ u32 rate;
    /* 0x1C */ float sec; /* Sec: the time between two rounds */
    /* 0x20 */ s32 id;    /* Id: the shooting game entry, or 0 for none */
};
CHECK_SIZEOF(ShootingGame::BalloonGeneratorParams, 0x24);

/* ShootingGame::BalloonGenerator (class name from RTTI; the gadget's descriptor is
 * "PARK_BALLOON"): every Sec units of dt it creates a round of Num balloons of random kinds at
 * random angles on a circle of the given radius around itself. With an Id it starts once the
 * shooting game entry is started and finishes when it is finished; without one it finishes after
 * MaxNum of its balloons have burst. Method names marked "(name inferred)" are not original. */
class BalloonGenerator : public Gadget {
  public:
    BalloonGenerator(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~BalloonGenerator();
    virtual void UpdateBody(float dt);
    /* Reads the owner again and queues a debug cylinder (fn_802E0C60) of the gadget's height and
     * radius, centred half the height above the gadget and turned 90 degrees about x. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameters and resets the state and the counters
     * (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Creates count balloons: their kinds are a random sample of the Rate counts and their angles
     * a random sample of count + 6 angles spaced evenly around the circle, turned by 13 degrees
     * more in each round (name inferred). */
    void Generate(int count);
    /* The character info's command handler: counts the commands with id 8 (DeadCommand), which
     * the balloons send when they burst (name inferred). */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0 initial, 1 waiting for the shooting game entry to start, 2 generating, 3 finished
     * (inferred). */
    /* 0x30 */ s32 m_state;
    /* 0x34 */ BalloonGeneratorParams m_params;
    /* 0x58 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x60 */ float m_timer;    /* counts down from Sec to the next round */
    /* 0x64 */ s32 m_burstCount; /* the DeadCommands received */
    /* 0x68 */ s32 m_roundCount; /* the rounds generated */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(ShootingGame::BalloonGenerator, 0x70);

} // namespace ShootingGame

#endif

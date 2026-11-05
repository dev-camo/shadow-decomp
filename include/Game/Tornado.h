#ifndef GAME_TORNADO_H
#define GAME_TORNADO_H

/* Matching workaround: <vector> comes before the other includes. The original file emits the
 * boost::detail::sp_counted_base_impl members that its vtable needs before the out-of-line vector
 * functions at its end, which MWCC does when the vector template is defined first; included after
 * the boost headers, the vector functions come first. Revisit if the original include order of the
 * file is found. */
#include <vector>
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Task.h"
#include "Game/TornadoCharInfo.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class TornadoTowing;

/* The gadget parameters in the order of the TORNADO descriptor's table, whose entry names give
 * the member names. Tornado::ReadOwner converts the lengths and speeds from metres to the game's
 * units (times 10), halves the cylinder height and converts the launch angles to radians. */
struct TornadoParams {
    /* 0x00 */ u32 turn;              /* "TURN 0:L 1:R" */
    /* 0x04 */ float turnSpeed;       /* "TURN_SPEED m/s" */
    /* 0x08 */ float upSpeed;         /* "UP_SPEED m/s" */
    /* 0x0C */ float cylinderRadius;  /* "CYLINDER_R m" */
    /* 0x10 */ float cylinderY;       /* "CYLINDER_Y m"; half the height after ReadOwner */
    /* 0x14 */ float cylinderOffsetY; /* "CYLINDER_OFFSET_Y m" */
    /* 0x18 */ float launchX;         /* "LAUNCH_X deg" */
    /* 0x1C */ float launchY;         /* "LAUNCH_Y deg" */
    /* 0x20 */ float launchSpeed;     /* "LAUNCH_SPEED m/s" */
    /* 0x24 */ float noControl;       /* "NO_CONTROL sec" */
};
CHECK_SIZEOF(TornadoParams, 0x28);

/* Tornado (class name from RTTI; the gadget's descriptor is "TORNADO"): the TORNADO.BIN model,
 * turned towards the point of fn_80010244, with a character info (TornadoCharInfo) that hands it
 * the characters the TornadoCollision tasks catch. For each character it creates a TornadoTowing
 * task, which carries the character to the cylinder of the parameters, up and round it, and
 * launches it (behaviour inferred). Method and member names are inferred. */
class Tornado : public Gadget {
  public:
    Tornado(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~Tornado();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11. */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, com id and parameters, converts the parameters and raises the
     * character info by the cylinder's offset. */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position, lowered by the cylinder's offset, and
     * turns it about y to face the point that fn_80010244 returns for index 0 (BDHologram measures
     * its distance to the same point). */
    void UpdateModelMatrix();
    /* Queues a debug cylinder of the cylinder parameters and a debug line along the launch
     * direction. */
    void DrawRange();
    /* Creates a TornadoTowing task for charInfo unless one of the towing tasks already carries it;
     * returns whether it did. */
    bool AddTowing(boost::shared_ptr<CharInfo> charInfo);
    /* Removes towing from the list of towing tasks. */
    void RemoveTowing(TornadoTowing *towing);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x38 */ boost::shared_ptr<TornadoCharInfo> m_charInfo;
    /* 0x40 */ std::vector<TornadoTowing *> m_towings;
    /* 0x4C */ TornadoParams m_params;
    /* 0x74 */ u8 m_tag; /* the owner's com id (fn_800C9E60); TornadoCollision compares its own */
    /* 0x78 */ UnkSoundHandle_801D3DC8 m_sound;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(Tornado, 0x98);

/* TornadoTowing (class name from RTTI, derived from Task): the task that carries one character
 * info for a Tornado. It takes the character out of control, moves it to the bottom of the
 * cylinder (along a tangent when it starts outside it), raises it round the cylinder's axis to
 * the top, circles it inwards there and launches it with the launch parameters (behaviour
 * inferred). Method and member names are inferred. */
class TornadoTowing : public Task {
  public:
    TornadoTowing(Task *parent, Tornado *tornado, boost::shared_ptr<CharInfo> charInfo);
    virtual ~TornadoTowing();
    virtual void Update(float dt);

    /* Moves the character along m_direction by the turn speed until it reaches m_start; returns
     * whether it did (state 1). */
    bool MoveToCylinder(boost::shared_ptr<CharInfo> charInfo, float dt);
    /* Turns the character about the cylinder's axis and raises it by the up speed; returns whether
     * it reached the top, where m_top is set (state 2). */
    bool Rise(boost::shared_ptr<CharInfo> charInfo, float dt);
    /* Turns the character about m_top while the up speed shortens its distance from it; returns
     * true once the remaining distance is no more than a turn step (state 3). */
    bool Circle(boost::shared_ptr<CharInfo> charInfo, float dt);
    /* Sends IgnoreInputCommand(3, 0) and InvincibleCommand(false), as the destructor does for a
     * character it lets go, then makes the character ignore input for the "NO_CONTROL sec" time,
     * places it at m_top and sends it off along the launch angles at the launch speed. */
    void Launch(boost::shared_ptr<CharInfo> charInfo);

    static const char *s_name;

    /* 0x28 */ Tornado *m_tornado;
    /* 1 moving to the cylinder, 2 rising, 3 circling, 4 launched (0 during construction). */
    /* 0x2C */ s32 m_state;
    /* 0x30 */ boost::weak_ptr<CharInfo> m_charInfo;
    /* 0x38 */ Fn8040Vector3 m_position;
    /* 0x44 */ Fn8040Vector3 m_start;     /* the point at the bottom of the cylinder */
    /* 0x50 */ Fn8040Vector3 m_direction; /* from the first position to m_start, normalised */
    /* 0x5C */ Fn8040Vector3 m_top;       /* the top of the cylinder's axis */
    /* 0x68 */ u8 field_0x68[0x4];        /* no code here touches it */
    /* 0x6C */ boost::weak_ptr<UnkObject_80169BB0> m_effect; /* fn_8034DEE4's object */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(TornadoTowing, 0x78);

#endif

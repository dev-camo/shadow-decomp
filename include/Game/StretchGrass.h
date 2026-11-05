#ifndef GAME_STRETCHGRASS_H
#define GAME_STRETCHGRASS_H

#include "Game/CharCommand.h"
#include "Game/CharaColliBase.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/Target.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

struct RpClump;

namespace StretchGrass {

/* The gadget's parameters, in the order of the descriptor's table (names from the table's
 * strings). The constructor clears them before reading them from the owner. */
struct StretchGrassParams {
    StretchGrassParams() : level(0.0f), angle(0.0f), sec(0.0f), length(0.0f), charge(0.0f) {}

    /* 0x00 */ float level;  /* selects the launch speed */
    /* 0x04 */ float angle;  /* degrees: the launch direction is turned by it about the x axis */
    /* 0x08 */ float sec;    /* passed to the IgnoreInputCommand of the launch */
    /* 0x0C */ float length; /* the stretch at which the grass stops */
    /* 0x10 */ float charge; /* how long the grass stays stretched before it springs back */
};
CHECK_SIZEOF(StretchGrassParams, 0x14);

/* The moving part of the gadget (name inferred): the hit callback and the draw callback reach it
 * through pointers. Its constructor (0x8014D2EC) clears every member. */
struct StretchGrassState {
    StretchGrassState()
        : m_mode(MODE_NEW), m_stretch(0.0f), m_speed(0.0f), m_position(0.0f, 0.0f, 0.0f),
          m_rotation(0.0f, 0.0f, 0.0f), m_timer(0.0f), field_0x28(0) {}

    /* The values of m_mode (names inferred from the code that sets each value): MODE_NEW after
     * construction, MODE_WAITING for a character, MODE_STRETCHING with a character on it,
     * MODE_SPRINGING_BACK, MODE_SWINGING back and forth about its rest length and MODE_HOLDING at
     * full length. */
    enum Mode {
        MODE_NEW,
        MODE_WAITING,
        MODE_STRETCHING,
        MODE_SPRINGING_BACK,
        MODE_SWINGING,
        MODE_HOLDING
    };

    /* 0x00 */ s32 m_mode;      /* a Mode */
    /* 0x04 */ float m_stretch; /* scales the offset of the tip (by -50 along z) */
    /* 0x08 */ float m_speed;   /* the stretching speed */
    /* 0x0C */ Fn8040Vector3 m_position;
    /* 0x18 */ UnkRotation3 m_rotation; /* radians */
    /* 0x24 */ float m_timer;           /* counts the "charge" time down while the grass is held */
    /* Cleared when a character starts the stretch, set when the grass launches it or receives
     * command id 0xC; while it is set the grass stops holding and launches nobody else. */
    /* 0x28 */ u8 field_0x28;
};
CHECK_SIZEOF(StretchGrassState, 0x2C);

/* StretchGrass::SetStretchGrass (class name from RTTI; the gadget's descriptor is "NOBI_KUSA"): a
 * stretching grass. A character in its probe sphere is taken out of control and carried while the
 * grass stretches up to its "length"; the grass then holds for its "charge" time and springs back,
 * launching the character at a speed that the parameter "level" selects. The grass is drawn
 * through a draw layer: with the delta morph clump of JG0404STRETCH.DFF when it is the grass
 * selected globally (the one that last began carrying a character) and its draw stretch is
 * nonzero, otherwise with the clump of JG0404STRETCHONE.DFF (behaviour inferred). Method names marked
 * "(name inferred)" and the member names are inferred. */
class SetStretchGrass : public Gadget {
  public:
    SetStretchGrass(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetStretchGrass();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, the rotation and the parameters from the owner and copies the position
     * and the rotation to the character info (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Moves the target object to the character info's position (name inferred). */
    void UpdateTarget();
    /* The draw layer's callback: draws the grass when its bounding sphere is in view (name
     * inferred). */
    void OnDraw(RednerDrawArgs &args);
    /* The character info's command handler: command id 0xC sets the state's byte at 0x28 (name
     * inferred). */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ StretchGrassParams m_params;
    /* 0x44 */ StretchGrassState m_state;
    /* 0x70 */ boost::shared_ptr<GadgetCharInfo> m_charInfo; /* follows the tip of the grass */
    /* 0x78 */ RpClump *m_clump;                             /* JG0404STRETCH.DFF */
    /* 0x7C */ RpClump *m_clumpOne;                          /* JG0404STRETCHONE.DFF */
    /* 0x80 */ void *m_morphData;   /* JG0404STRETCH.DMA, handed to m_clump's atomics */
    /* 0x84 */ float m_drawStretch; /* the stretch the draw callback uses as morph weight */
    /* 0x88 */ StretchGrassState *m_drawState; /* &m_state */
    /* 0x8C */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_probe;
    /* 0x94 */ boost::shared_ptr<Target> m_target;
    /* 0x9C */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* The point and the angles the character is carried at: 5 beyond the tip, facing back. */
    /* 0xA4 */ Fn8040Vector3 m_launchPosition;
    /* 0xB0 */ UnkRotation3 m_launchAngles;
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(SetStretchGrass, 0xC0);

} // namespace StretchGrass

#endif

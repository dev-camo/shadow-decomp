/* GravitySignBoard (class name from RTTI): a signboard that shows the model GRAVITYSIGNBOARD.BIN
 * turned to the stage's current gravity direction and, when that direction changes, turns it to
 * the new one over half a second (behaviour inferred). It plays a sound while it exists. File name
 * inferred from the class. Method names marked "(name inferred)" are not original. Definitions
 * are in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkGravity_80293928.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class GravitySignBoard : public Gadget {
  public:
    GravitySignBoard(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~GravitySignBoard();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info, stops any turn in
     * progress (m_state = 0), notes the current gravity direction and sets the turn to it (name
     * inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Takes the owner's rotation, applies the turn after it, stores the angles of the product in
     * the character info and places the model at the character info's position with that
     * rotation (name inferred). */
    void UpdateModelMatrix();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* The names of the members below are inferred. */
    /* 0 at rest, 1 while turning (inferred). */
    /* 0x40 */ s32 m_state;
    /* Bit 0 is set when the gravity direction differs from m_gravityIndex (inferred). */
    /* 0x44 */ UnkBitFlags_80014E04 m_flags;
    /* 0x48 */ s32 m_gravityIndex; /* the gravity direction that the model is turned to */
    /* 0x4C */ UnkSoundHandle_801D3DC8 m_sound;
    /* 0x68 */ float m_timer;             /* seconds into the current turn */
    /* 0x6C */ Fn8040Matrix34 m_turn;     /* the turn from lbl_80584360 that the model shows */
    /* 0x9C */ Fn8040Matrix34 m_turnFrom; /* the turn when the current turn started */
    /* 0xCC */ Fn8040Matrix34 m_turnTo;   /* the turn to the new gravity direction */
    /* 0xFC */ u32 field_0xFC;            /* never accessed; Create allocates 0x100 bytes */
};
CHECK_SIZEOF(GravitySignBoard, 0x100);

const char *GravitySignBoard::s_name = "GravitySignBoard";

/* Builds the matrix that turns lbl_80584360 onto direction: the identity when they are the same,
 * otherwise the rotation about their cross product by the angle between them (behaviour inferred;
 * name inferred). */
static void BuildTurnMatrix(Fn8040Matrix34 *matrix, const Fn8040Vector3 *direction) {
    if (!fn_8000AD08(direction, &lbl_80584360)) {
        Fn8040Matrix34 rotation;
        Fn8040Vector3 axis;
        Fn8040Vector3 cross;

        fn_8040CFB8(&cross, &lbl_80584360, direction);
        fn_800091EC(&axis, &cross);
        fn_8040FD8C(&rotation, &axis, fn_8040F8FC(&lbl_80584360, direction));
        fn_8040C148(matrix, &rotation);
    } else {
        fn_8040C148(matrix, &lbl_805E0720);
    }
}

GravitySignBoard::GravitySignBoard(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_flags(0), m_timer(0.0f) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x69, 4);
    ReadOwner(owner);

    m_model = fn_8015F800(1, "GRAVITYSIGNBOARD.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    UpdateModelMatrix();
    m_sound = fn_801D3E58(0x7747, &m_charInfo->m_position, this);
}

GravitySignBoard::~GravitySignBoard() {
    m_sound.fn_801D455C();
}

extern "C" void fn_802590F0(void) {
    fn_80293B1C();
    fn_8006B474("GRAVITYSIGNBOARD.BIN", 1);
}

extern "C" void fn_802590D0(void) {
    fn_80293AE4();
}

void GravitySignBoard::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new GravitySignBoard(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void GravitySignBoard::UpdateBody(float dt) {
    if (m_gravityIndex != fn_80293AC8()) {
        m_flags.Set(0);
    }

    /* The gravity direction changed: start turning from the current turn to the new direction. */
    if (m_flags.Test(0)) {
        fn_8040C148(&m_turnFrom, &m_turn);
        m_gravityIndex = fn_80293AC8();
        BuildTurnMatrix(&m_turnTo, fn_80293B60(m_gravityIndex));
        m_timer = 0.0f;
        m_state = 1;
        m_flags.Reset(0);
    }

    switch (m_state) {
    case 1:
        m_timer += dt;
        if (m_timer >= 0.5f) {
            BuildTurnMatrix(&m_turn, fn_80293B60(m_gravityIndex));
            m_state = 0;
        } else {
            Fn8040Matrix34 turn;

            fn_8040EFA4(&turn, &m_turnFrom, &m_turnTo, m_timer / 0.5f);
            fn_8040C148(&m_turn, &turn);
        }
        UpdateModelMatrix();
        break;
    }

    m_model->fn_80162FB0(dt);
}

void GravitySignBoard::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();

    /* Draws a debug line from the character info's position to position - 50 * the current
     * gravity direction vector. */
    Fn8040Vector3 direction;
    Fn8040Vector3 end;

    fn_800091EC(&direction, fn_80293B60(m_gravityIndex));
    fn_8040CE38(&direction, -50.0f);
    fn_8040CDD0(&end, &m_charInfo->m_position, &direction);

    UnkSegment_80020A20 segment(m_charInfo->m_position, end);

    fn_802E0AA0(segment, UnkDebugColor_802E0E84(0xC0, 0xFF, 0xFF, 0), 1.0f);
}

void GravitySignBoard::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    m_state = 0;
    m_gravityIndex = fn_80293AC8();
    BuildTurnMatrix(&m_turn, fn_80293B60(m_gravityIndex));
}

void GravitySignBoard::UpdateModelMatrix() {
    Fn8040Matrix34 rotation;
    Fn8040Matrix34 placed;
    Fn8040Matrix34 turned;
    Fn8040Matrix34 product;

    fn_8040C148(&rotation, &lbl_805E0720);
    fn_800C9ED4(m_owner, &m_charInfo->m_rotation);
    fn_8040B848(&rotation, m_charInfo->m_rotation.y);
    fn_8040B884(&rotation, m_charInfo->m_rotation.x);
    fn_8040B80C(&rotation, m_charInfo->m_rotation.z);
    fn_8001E044(&turned, &m_turn, &rotation);
    fn_8040C148(&rotation, &turned);
    fn_800494F8(&m_charInfo->m_rotation, &rotation);

    fn_8040C148(&placed, &lbl_805E0720);
    fn_8040B59C(&placed, &m_charInfo->m_position);
    fn_8001E044(&product, &placed, &rotation);
    fn_8040C148(&placed, &product);
    m_model->fn_801633DC(&placed);
}

GadgetTypeInfo lbl_80549F40 = {
    "GravitySignBoard",
    fn_802590F0,
    fn_802590D0,
    GravitySignBoard::Create,
    0,
    0,
    0x14B7,
    1,
    0,
    10,
    0,
};

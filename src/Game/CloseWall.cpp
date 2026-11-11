/* CloseWall::SetCloseWall (class name from RTTI; the gadget's descriptor is "CLOSE WALL"): two
 * instances of CLOSEWALL.BIN, each on a character info of its own, that stand half the parameter
 * "width" away from the gadget's position on either side, along its x axis turned by its y
 * rotation. When either character info receives command id 11 while the gadget waits, the walls
 * move towards the position by "speed" times dt at each update, requesting sound 0x7849, until
 * they are s_closedOffset away from it (behaviour inferred). File name inferred from the class.
 * Method names marked "(name inferred)" are not original. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first). */
/* TNumEdit.h comes before CharaColliBaseCharInfo.h, in the order their names appear in the unit's
 * .rodata (alphabetical order would reverse them). */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/CloseWall.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

/* The distance from the position at which the walls stop closing (name inferred).
 * Matching workaround: the original sets this float with the unit's start-up function in .ctors,
 * which shows only that its initialiser is not a constant expression. UnkStartupFloat_800E2570
 * stands in for that initialiser, which is unknown. The const qualifier (inferred) lets UpdateBody
 * load the value before the store to m_offset, as the original does; as an
 * UnkStartupFloat_800E2570 object it is loaded after that store (6 of UpdateBody's 91 words
 * differ). Revisit when the original initialiser is known. */
static const float s_closedOffset = UnkStartupFloat_800E2570(40.0f).value;

namespace CloseWall {

const char *SetCloseWall::s_name = "SetCloseWall";

SetCloseWall::SetCloseWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_rotation(0.0f, 0.0f, 0.0f) {
    m_name = s_name;

    m_charInfo[0] = fn_8015CD90(0x7B, 4);
    m_charInfo[1] = fn_8015CD90(0x7B, 4);

    fn_8015CF7C(m_charInfo[0].get());
    m_charInfo[0]->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&SetCloseWall::OnCommand, this, _1));
    fn_8015CF7C(m_charInfo[1].get());
    m_charInfo[1]->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&SetCloseWall::OnCommand, this, _1));

    ReadOwner(owner);

    m_model[0] = fn_8015F800(1, "CLOSEWALL.BIN");
    m_model[0]->fn_80162A68(0, 0);
    m_model[1] = fn_8015F800(1, "CLOSEWALL.BIN");
    m_model[1]->fn_80162A68(0, 0);

    m_model[0]->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo[0]));
    m_model[1]->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo[1]));

    UpdateModelMatrices();
}

SetCloseWall::~SetCloseWall() {}

void SetCloseWall::UpdateBody(float dt) {
    if (m_model[0].get() != 0) {
        m_model[0]->fn_80162FB0(dt);
    }
    if (m_model[1].get() != 0) {
        m_model[1]->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0:
        m_state = 1;
        break;

    case 1:
        break;

    case 2: {
        m_offset -= m_params.speed * dt;
        if (m_offset <= s_closedOffset) {
            m_offset = s_closedOffset;
            m_state = 3;
        }

        /* The same placement as in ReadOwner. The original has these calls in place in both
         * functions; an inline member function holding them is not expanded at -inline level=1
         * (the constructors and operator-> calls inside it are at the inline depth limit). */
        Fn8040Matrix34 rotation;

        fn_8040B6F8(&rotation);
        fn_8040B848(&rotation, m_rotation.y);

        Fn8040Vector3 axis(1.0f, 0.0f, 0.0f);

        axis = rotation * axis;
        fn_8040CE38(&axis, m_offset);

        m_charInfo[0]->m_position = m_position - axis;
        m_charInfo[1]->m_position = m_position + axis;

        fn_801D53C4(fn_80053164(), 0x7849, 0, this, 2, 0, 2);
        break;
    }

    case 3:
        break;
    }

    UpdateModelMatrices();
}

void SetCloseWall::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrices();
}

void SetCloseWall::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_position);
    fn_800C9ED4(owner, &m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_state = 0;
    m_offset = 0.5f * m_params.width;

    Fn8040Matrix34 rotation;

    fn_8040B6F8(&rotation);
    fn_8040B848(&rotation, m_rotation.y);

    Fn8040Vector3 axis(1.0f, 0.0f, 0.0f);

    axis = rotation * axis;
    fn_8040CE38(&axis, m_offset);

    m_charInfo[0]->m_position = m_position - axis;
    m_charInfo[1]->m_position = m_position + axis;

    m_charInfo[0]->m_rotation = m_rotation;
    m_charInfo[1]->m_rotation = m_rotation;
}

void SetCloseWall::UpdateModelMatrices() {
    Fn8040Matrix34 matrix;
    Fn8040Matrix34 matrix2;

    if (m_model[0].get() != 0) {
        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo[0]->m_position);
        fn_8040B848(&matrix, m_charInfo[0]->m_rotation.y);
        m_model[0]->fn_801633DC(&matrix);
    }
    if (m_model[1].get() != 0) {
        fn_8040C148(&matrix2, &lbl_805E0720);
        fn_8040B59C(&matrix2, m_charInfo[1]->m_position);
        fn_8040B848(&matrix2, m_charInfo[1]->m_rotation.y);
        m_model[1]->fn_801633DC(&matrix2);
    }
}

void SetCloseWall::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 11:
        if (m_state == 1) {
            m_state = 2;
        }
        break;
    }
}

} // namespace CloseWall

extern "C" void fn_80228A34(void) {
    fn_8006B474("CLOSEWALL.BIN", 1);
}

extern "C" void fn_80228A30(void) {}

void CloseWall::SetCloseWall::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SetCloseWall(parent, owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "width", 0, 0, 0, 40.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "speed", 0, 0, 0, 4.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805468C8 = {
    "CLOSE WALL", fn_80228A34, fn_80228A30, CloseWall::SetCloseWall::Create, 0, 0, 0x1773, 1, 0,
    10,           s_paramInfo,
};

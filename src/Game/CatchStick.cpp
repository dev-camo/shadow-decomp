/* The CATCHSTICK gadget (class described in Game/CatchStick.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to the character info's command unit and to the probe's handle are
 * boost::function objects built with boost::bind(&CatchStick::method, this, _1); their managers,
 * invokers and copy functions come from the boost headers. */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/CatchStick.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerBase.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkWorld_80021B08.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

/* What the probe callback passes to fn_8015AE44 (placeholder name). */
struct UnkHitContext_800F4B60 {
    /* 0x0 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x8 */ boost::weak_ptr<UnkSegment_80020A20> m_line;
};
CHECK_SIZEOF(UnkHitContext_800F4B60, 0x10);

/* Matching workaround: this constructor and UnkCallbackSlot_800F5980::fn_800F5980 below are
 * defined here, out of line, as strong globals. The original has one copy of each, in this file's
 * code, and the other units that use them call these copies. Each copy sits among the out-of-line
 * functions that its first caller here emits after itself, in the reverse order of that caller's
 * calls (this constructor between the hit callback and its weak_ptr<CharInfo> conversion,
 * fn_800F5980 among the constructor's), which is where MWCC puts an inline function's out-of-line
 * copy. Made inline in PlayerCommands.h, this constructor takes that place; as strong globals,
 * this constructor follows the conversion instead of preceding it, and fn_800F5980 with the four
 * function1 members it calls (0x188 bytes) follows the constructor's other out-of-line functions
 * instead of sitting among them. With both defined inline in their headers (PlayerCommands.h and
 * UnkHandleBTarget_8015BE64.h), this file's function order is the original's and the USA
 * (GUPE8P) DOL still links identically, but more than a hundred other objects change: most only
 * renumber their @N labels, and some also emit weak copies of fn_800F5980, of function1 members
 * or of this constructor, which the linker discards.
 * Revisit with a change of its own that defines both inline in those headers. */
Player::TouchLineCommand::TouchLineCommand(boost::weak_ptr<CharInfo> charInfo,
                                           boost::weak_ptr<UnkSegment_80020A20> line, int arg2)
    : PlayerCommand(0x111, arg2), m_charInfo(charInfo), m_line(line) {}

/* The hit callback that OnCollision passes to fn_8015AE44. When there is a character info and a
 * context and the character info is a Player::PlayerBase, it passes the character info's
 * fn_801DC980 a new Player::TouchLineCommand (a request; the pointer is null when the allocation
 * fails, and it is passed all the same) with the gadget's character info and line, unless the
 * player's segment is too far from the line: the segment runs between the vectors at 0xC of
 * fn_80078CEC's results for 0 and 1, or is the first point alone when the length of their
 * difference is < 0.1 (a NaN length takes the segment), and the test is a squared distance > 144
 * (more than 12; a NaN distance sends the command). When the line has expired, the distance test
 * is skipped and the command is still sent (behaviour inferred). */
static void fn_800F5FAC(boost::shared_ptr<CharInfo> hit, void *arg) {
    UnkHitContext_800F4B60 *context = static_cast<UnkHitContext_800F4B60 *>(arg);

    if (hit.get() != 0 && context != 0) {
        Player::PlayerBase *player = dynamic_cast<Player::PlayerBase *>(hit.get());

        if (player != 0) {
            boost::shared_ptr<UnkSegment_80020A20> line = context->m_line.lock();

            if (line.get() != 0) {
                float squaredDistance;

                Fn8040Vector3 first = fn_80078CEC(player, 0)->field_0xC;
                Fn8040Vector3 second = fn_80078CEC(player, 1)->field_0xC;
                Fn8040Vector3 span = first - second;
                if (fn_8040CF50(&span) < 0.1f) {
                    squaredDistance = fn_8003C660(&first, line.get(), 0);
                } else {
                    UnkSegment_80020A20 segment(first, second);

                    squaredDistance = fn_8003C4C4(&segment, line.get(), 0, 0);
                }

                if (squaredDistance > 144.0f) {
                    return;
                }
            }

            hit->fn_801DC980(new Player::TouchLineCommand(
                boost::weak_ptr<CharInfo>(context->m_charInfo), context->m_line, 4));
        }
    }
}

UnkCallbackSlot_800F5980 &
UnkCallbackSlot_800F5980::fn_800F5980(const GadgetColliDetectCallback &callback) {
    m_callback = callback;
    return *this;
}

/* Matching workaround: s_name is defined after the hit callback. The original's .sdata has the
 * RTTI records of CharInfo, Player::PlayerObject and Player::PlayerBase, which the hit callback's
 * dynamic_cast creates, before s_name, and its .rodata has their names before "CatchStick";
 * defined before the hit callback, s_name and its string come first. Revisit when the original
 * order of definitions in this file is known. */
const char *CatchStick::s_name = "CatchStick";

CatchStick::CatchStick(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x38, 4);
    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&CatchStick::OnCommand, this, _1));
    ReadOwner(m_owner);

    boost::shared_ptr<CharaColliBase> probe =
        fn_800C78D4(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 0x40000, 10, 0,
                    &lbl_805E07A0, lbl_805E07B8, 0, 0.5f * m_length, 2.0f);

    m_handle = fn_8015B8C4(probe);
    m_handle->field_0x0.fn_800F5980(
        GadgetColliDetectCallback(boost::bind(&CatchStick::OnCollision, this, _1)));
    m_line = boost::shared_ptr<UnkSegment_80020A20>(new UnkSegment_80020A20);
    UpdateLine();
}

CatchStick::~CatchStick() {}

void CatchStick::UpdateBody(float dt) {
    fn_8015BE9C(m_handle.get(), dt);
}

void CatchStick::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateLine();
    if (m_handle.get() != 0) {
        m_handle->m_probe->SetSize(2.0f, -2, 0.0f, 0.5f * m_length);
        fn_8015BE64(m_handle.get());
        fn_802E0B58(*m_line, lbl_805F06D8);
    }
}

void CatchStick::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_length, 4);
}

void CatchStick::UpdateLine() {
    Fn8040Vector3 front(0.0f, 0.0f, 1.0f);
    Fn8040Vector3 back(0.0f, 0.0f, -1.0f);
    Fn8040Matrix34 matrix(lbl_805E0720);

    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    fn_8040B720(&matrix, 0.0f, 0.0f, 0.5f * m_length);
    m_line->start = matrix * front;
    m_line->end = matrix * back;
}

void CatchStick::OnCommand(CharCommand &command) {}

void CatchStick::OnCollision(GadgetColliDetectArgs &args) {
    UnkHitContext_800F4B60 context;

    context.m_charInfo = m_charInfo;
    context.m_line = m_line;
    fn_8015AE44(&args, fn_800F5FAC, &context);
}

extern "C" void fn_800F4B5C(void) {}

extern "C" void fn_800F4B58(void) {}

void CatchStick::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new CatchStick(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "length", 0, 0, 0, 10.0f, 1.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80525278 = {
    "CATCHSTICK", fn_800F4B5C, fn_800F4B58, CatchStick::Create, 0, 0, 0x5A, 1, 0, 10, s_paramInfo,
};

/* The BK_TANK_COLI gadget: a CommandCollision whose callback sends a command to the character info
 * of the BK_TANK gadget's controller. File name inferred from the gadget's name. Definitions are
 * in reverse address order (deferred inlining emits functions last-defined first).
 *
 * NonMatching: the eight functions match; the order of the unit's .data does not (see
 * configure.py). */
/* The RTTI data of TNumEdit.h comes before that of CharaColliBaseCharInfo.h at the start of the
 * unit's .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBaseCharInfo.h.
 * Matching workaround: the headers' stand-in inline functions (see TNumEdit.h) emit that RTTI
 * data; revisit when the original inline functions or their real callers are recovered. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/CharInfo.h"
#include "Game/CommandCollision.h"
#include "Game/GadgetCommand.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkCharInfoTable_800DDF20.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/shared_ptr.hpp"

/* The labels of the parameter "type", and those of the two parameters after it, which depend on
 * the type. */
static const char *s_typeLabels[4] = {"type : Move", "type : Stop", "type : Speed",
                                      "type : SetPos"};
static const char *s_timeLabels[3] = {"no timer", "sec", "route"};
static const char *s_valueLabels[3] = {"Not in Use", "speed", "rate(0.0 - 1.0)"};

/* The label callback of the descriptor's last three parameters: the labels of the two floats
 * follow the value of the last "type" label request (behaviour inferred). The fifth is "route"
 * for the type 3 and otherwise "sec" when its value is above 0 and "no timer" when it is not; the
 * sixth is "speed" for the type 2, "rate(0.0 - 1.0)" for the type 3 and otherwise "Not in Use". */
static const char *fn_803212B4(UnkParamCursor_800CBC20 *cursor) {
    static s32 s_type = 0;

    if (cursor->m_index == 3) {
        s_type = cursor->m_value;
        return s_typeLabels[cursor->m_value];
    } else if (cursor->m_index == 4) {
        if (s_type == 3) {
            return s_timeLabels[2];
        }
        if (cursor->m_floatValue > 0.0f) {
            return s_timeLabels[1];
        }
        return s_timeLabels[0];
    } else if (cursor->m_index == 5) {
        if (s_type == 2) {
            return s_valueLabels[1];
        }
        if (s_type == 3) {
            return s_valueLabels[2];
        }
        return s_valueLabels[0];
    }

    return 0;
}

extern "C" void fn_803212B0(void) {}

extern "C" void fn_803212AC(void) {}

/* The callback of the gadget's CommandCollision (name of the function unknown). When the
 * collision has a hit that fn_8015AC84 accepts, or one of a character info whose word at 0x5C is 4
 * and whose word at 0x50 is 0x91 (fn_8015B544; the BK_TANK gadget's SetBKTank creates its
 * character info with 0x91), it looks up the first character info of the table of fn_800DDF20
 * whose word at 0x50 is 0x92 (the BKTankController::Controller's). When there is such an entry
 * and its character info still exists, it sends that one a command chosen by the parameter
 * "type": 0 a GadgetCommand::MoveStartCommand and 1 a GadgetCommand::MoveEndCommand with the
 * fifth parameter, 2 an AccelerateCommand with the vector (sixth parameter, 0, fifth parameter)
 * and 3 a SetPositionCommand with the vector (fifth parameter, sixth parameter, 0); other types
 * send nothing. Whether or not a command was sent, it then sets bit 0 of the CommandCollision's
 * flags, and the CommandCollision does not call it while that bit is set. Without such a hit it
 * does nothing. */
static void fn_803210B8(CommandCollision::Args &args) {
    ColliDetectArgs *detectArgs = args.m_detectArgs;
    CommandCollision::Params *params = args.m_params;
    UnkBitFlags_80014E04 *flags = args.m_flags;

    /* The second test is made only when the first fails, and one comparison then tests the result
     * of whichever call was last (0x803210D8-0x803210F8). */
    bool hit = fn_8015AC84(*detectArgs);

    if (!hit) {
        hit = fn_8015B544(*detectArgs, 4, 0x91);
    }
    if (hit) {
        UnkCharInfoNode_801DCB14 *node = fn_801DCB14(fn_800DDF20(), 0x92);

        if (node != 0) {
            if (boost::shared_ptr<CharInfo> charInfo = node->charInfo.lock()) {
                switch (params->m_type) {
                case 0:
                    charInfo->fn_801DC980(
                        new GadgetCommand::MoveStartCommand(params->field_0x10, 4));
                    break;

                case 1:
                    charInfo->fn_801DC980(new GadgetCommand::MoveEndCommand(params->field_0x10, 4));
                    break;

                case 2:
                    charInfo->fn_801DC980(new AccelerateCommand(
                        Fn8040Vector3(params->field_0x14, 0.0f, params->field_0x10), 4));
                    break;

                case 3: {
                    /* Stored member by member: the original stores x before it reads the sixth
                     * parameter (0x80321234-0x8032124C). */
                    Fn8040Vector3 position;

                    position.x = params->field_0x10;
                    position.y = params->field_0x14;
                    position.z = 0.0f;

                    charInfo->fn_801DC980(new SetPositionCommand(position, 4));
                    break;
                }
                }
            }
        }
        flags->Set(0);
    }
}

/* The descriptor's create function: creates a CommandCollision under task layer 11 and installs
 * the callback above. */
extern "C" void fn_80320EE4(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);
    CommandCollision::CommandCollision *collision =
        new CommandCollision::CommandCollision(parent, owner, 7);

    collision->m_callback.fn_8025E1BC(CommandCollision::Callback(boost::bind(fn_803210B8, _1)));
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "X", 0, 0, 0, 40.0f, 1.0f, 10000.0f, fn_800CBC20, 0},
    {4, "Y", 0, 0, 0, 40.0f, 1.0f, 10000.0f, fn_800CBC20, 0},
    {4, "Z", 0, 0, 0, 40.0f, 1.0f, 10000.0f, fn_800CBC20, 0},
    {1, "type", 0, 0, 3, 0.0f, 0.0f, 0.0f, fn_803212B4, 0},
    {4, "sec", 0, 0, 0, 1.0f, 0.0f, 10000.0f, fn_803212B4, 0},
    {4, "Not in Use", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_803212B4, 0},
    {0},
};

GadgetTypeInfo lbl_8055C7A0 = {
    "BK_TANK_COLI", fn_803212B0, fn_803212AC, fn_80320EE4, 0, 0x800, 0x899, 1, 0, 10, s_paramInfo,
};

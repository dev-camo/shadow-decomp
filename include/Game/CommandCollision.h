#ifndef GAME_COMMANDCOLLISION_H
#define GAME_COMMANDCOLLISION_H

#include "Game/Gadget.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class GadgetCharInfo;

namespace CommandCollision {

/* The parameter block of a CommandCollision gadget (name inferred): the 0x18 bytes that the
 * constructor and ResetFromOwner copy from the owner (fn_800C9DBC). The E_P_COLI, E_B_COLI and
 * BK_TANK_COLI gadgets, which create CommandCollision objects, describe six parameters: "X", "Y",
 * "Z", "type" and two floats whose labels depend on the type. CommandCollision passes the first
 * three to its box probe as the size; the gadgets' callbacks read the others (member names
 * inferred from the labels, placeholders for the two floats). */
struct Params {
    /* 0x00 */ float m_x;
    /* 0x04 */ float m_y;
    /* 0x08 */ float m_z;
    /* 0x0C */ s32 m_type;
    /* 0x10 */ float field_0x10;
    /* 0x14 */ float field_0x14;
};
CHECK_SIZEOF(Params, 0x18);

/* What CommandCollision passes to its callback (class name from the RTTI name of the bound
 * callback, "void (*)(CommandCollision::Args &)"): the arguments of its own collision callback,
 * its parameter block and its flags (0x80260078-0x80260090). The three members are addresses;
 * the bytes do not show whether they are pointers or references, and the member names are
 * inferred. */
struct Args {
    /* 0x0 */ ColliDetectArgs *m_detectArgs;
    /* 0x4 */ Params *m_params;
    /* 0x8 */ UnkBitFlags_80014E04 *m_flags;
};
CHECK_SIZEOF(Args, 0xC);

/* The callback that the gadget file installs. */
typedef boost::function1<void, Args &> Callback;
CHECK_SIZEOF(Callback, 0xC);

/* The class of CommandCollision's callback member (placeholder name: the binary does not name it).
 * Like UnkCallbackSlot_800F5980, it has a function1 as its one member, at offset 0: its
 * constructor (0x80260A5C) only runs function1's default constructor (0x80260A8C) and its
 * destructor (0x80260338) only destroys the function1 (0x8025E344), passing -1 as for a member. */
class UnkCallbackSlot_8025E1BC {
  public:
    UnkCallbackSlot_8025E1BC();
    ~UnkCallbackSlot_8025E1BC();

    /* Assigns callback to the member and returns the slot (behaviour inferred: it calls function1's
     * assignment with the slot as this). */
    UnkCallbackSlot_8025E1BC &fn_8025E1BC(const Callback &callback);

    /* 0x00 */ Callback m_callback;
};
CHECK_SIZEOF(UnkCallbackSlot_8025E1BC, 0xC);

/* CommandCollision::CommandCollision (class name from RTTI, with Gadget as its base): a gadget
 * with a box probe of the size in its parameters. The collision callback that it installs on the
 * probe's handle target (0x8026003C) calls m_callback with an Args when m_callback is not empty
 * and bit 0 of m_flags is clear. Its file (vtable 0x8054BA3C) is not recovered: only what the
 * gadget files that create it use is declared, and the layout comes from its constructor
 * (0x80260668), which constructs or sets every member below except m_state and m_comId, from
 * the function it calls last (0x802600E8), which sets those two, and from its destructor
 * (0x802602B8). Member names are inferred. */
class CommandCollision : public Gadget {
  public:
    /* arg2 is passed on to fn_800C7C34, which creates the box probe (the E_P_COLI gadget passes
     * 8, the E_B_COLI and BK_TANK_COLI gadgets 7; meaning unknown). */
    CommandCollision(Task *parent, UnkGadgetOwner_800C9CD0 *owner, int arg2);
    virtual ~CommandCollision();
    virtual void UpdateBody(float dt);                           /* 0x802601EC */
    virtual void fn_800CBC1C(float dt);                          /* 0x802601C0 */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner); /* 0x8026015C */

    /* 0x30 */ UnkCallbackSlot_8025E1BC m_callback;
    /* 0x3C */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x44 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle; /* of the box probe */
    /* 0x4C */ Params m_params;
    /* 0x64 */ s32 m_state;
    /* The callback receives its address; the gadget files' callbacks set bit 0. */
    /* 0x68 */ UnkBitFlags_80014E04 m_flags;
    /* 0x6C */ s32 m_comId; /* the owner's com id (fn_800C9E50) */

  private:
    /* Copying is unrecovered: an implicit copy would duplicate the task links (declared only, as
     * in C++03). */
    CommandCollision(const CommandCollision &);
    CommandCollision &operator=(const CommandCollision &);
};
CHECK_OFFSETOF(CommandCollision, m_callback, 0x30);
CHECK_OFFSETOF(CommandCollision, m_params, 0x4C);
CHECK_SIZEOF(CommandCollision, 0x70);

} // namespace CommandCollision

#endif

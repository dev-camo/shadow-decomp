#ifndef GAME_DIABLONPARTSBASE_H
#define GAME_DIABLONPARTSBASE_H

#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Boss {
namespace Diablon {

class Diablon;

namespace Parts {

/* Boss::Diablon::Parts::Action (class name from RTTI, with no bases): the message that 0x802D5F24
 * passes to the OnAction of every part below the part it is given. Derived classes such as
 * Parts::ActionMove (constructor 0x802D571C, kind 1) store their kind in m_type (name inferred).
 * The destructor is at 0x802D4944. */
class Action {
  public:
    virtual ~Action();

    /* 0x4 */ s32 m_type;
};
CHECK_SIZEOF(Boss::Diablon::Parts::Action, 0x8);

/* Boss::Diablon::Parts::PartsBase (class name from RTTI, with no bases): a part of the boss in a
 * tree of parts. The constructor (0x802D6180) appends the part to the end of the parent's child
 * list when parent is not null. The destructor (0x802D60B4) deletes the children, each of which
 * unlinks itself on destruction, then unlinks the part from its parent when it has one. Update
 * (0x802D5F1C) and OnAction (0x802D5F20) do nothing here; 0x802D5FB0 and 0x802D5F24 call them
 * for every part below the part they are given. Method and member names are inferred, and so are
 * the parameters' pointer, reference and const spellings. */
class PartsBase {
  public:
    PartsBase(PartsBase *parent, const char *name, const boost::shared_ptr<Diablon> &diablon);
    virtual ~PartsBase();
    virtual void Update(float dt);
    virtual void OnAction(const Action &action);

    /* 0x04 */ boost::shared_ptr<Diablon> m_diablon;
    /* 0x0C */ const char *m_name; /* the derived class's name, for example "Bomb" */
    /* 0x10 */ PartsBase *m_parent;
    /* 0x14 */ PartsBase *m_child; /* the first child */
    /* 0x18 */ PartsBase *m_next;  /* the next sibling */
    /* 0x1C */ u32 field_0x1C;     /* cleared by the constructor */
    /* Cleared by the constructor; Parts::Bomb stores 1 here in its last state. */
    /* 0x20 */ u8 field_0x20;

  private:
    /* Copying is disabled in this reconstructed interface because the copy contract is
     * unrecovered: the destructor deletes the children and unlinks the part from its parent, so a
     * memberwise copy would share the children and leave a part that its parent does not list.
     * Declared and not defined. */
    PartsBase(const PartsBase &);
    PartsBase &operator=(const PartsBase &);
};
CHECK_SIZEOF(Boss::Diablon::Parts::PartsBase, 0x24);

} // namespace Parts
} // namespace Diablon
} // namespace Boss

#endif

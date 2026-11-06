#ifndef GAME_VACUUMOBJECT_H
#define GAME_VACUUMOBJECT_H

#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;
class VacuumCommand;
class UnkVacuumState_802389BC;

/* VacuumObject (class name from RTTI, with no bases): by the RTTI strings, the base class of
 * SetItem::VacuumItemBox, VacuumBomb::Bomb, VacuumContainer::Container, Balloon::VacuumBalloon,
 * VacuumThrowBomb, VacuumCityBombSmall and the file-local classes Ring (VacuumRing.cpp) and
 * CEnemyVacuumObject (EnemyVacuumObject.cpp); SetItem::VacuumItemBox's vtable reuses its
 * implementations of the slots it does not override. Its constructor takes a VacuumCommand (objects
 * that a character pulls in, inferred from the names). Its file (from 0x802379B8) is not recovered:
 * only what SetItem::VacuumItemBox calls is declared. The constructor (0x80238424) creates the
 * 0x84-byte object at 0x4 with arg0 (constructor 0x802389BC), copies the command's members at 0x1C,
 * 0x24 and 0x2C into it and copies the matrix that the virtual fn_80237EE4 returns (possibly left
 * unwritten; see fn_80237EE4) into it at 0x18. The destructor (0x80238304), when the effect that
 * object keeps at 0x7C has not expired, sets bit 4 of the effect's word at 0x18 and clears its
 * words at 0x48-0x50, then destroys the member at 0x4. The functions that are not virtual pass the
 * object at 0x4 on to functions of their own. Names and signatures are inferred: the virtual
 * functions are named after their implementations in VacuumObject's vtable (0x80547A24), in slot
 * order. */
class VacuumObject {
  public:
    /* arg0 comes from a function of the command (fn_80237EDC for SetItem::VacuumItemBox). */
    VacuumObject(int arg0, VacuumCommand *command);
    virtual ~VacuumObject();
    /* Passes position on to fn_802388B8 with the object at 0x4 (behaviour inferred). */
    virtual void fn_802382E0(const Fn8040Vector3 *position);
    /* Copies the rotation into the object at 0x4 (0x6C-0x74), where fn_80238248 reads it
     * (behaviour inferred). */
    virtual void fn_80238268(const UnkRotation3 *rotation);
    /* Advances the object by dt (name inferred): copies the matrix of the virtual fn_80237EE4
     * (possibly left unwritten; see fn_80237EE4) into the object at 0x4, passes that object and dt
     * to fn_80238654 and, when the object's effect has not expired, passes the effect and the
     * position (fn_802382B0) to fn_80425E4C. */
    virtual void Update(float dt);
    /* VacuumObject's two following implementations are empty, so their arguments are unknown;
     * SetItem::VacuumItemBox's override of the second uses none. */
    virtual void fn_80238184();
    virtual void fn_80238180();
    /* Returns fn_80238600 of the object at 0x4: true while the weak pointer at the object's 0x4
     * has not expired, otherwise whether the weak_ptr<CharInfo> at its 0xC has not expired. The
     * bool return is inferred: fn_80238600 returns 0 or 1, and VacuumObjectTask (0x802379E8) and
     * a direct caller (0x80209100) test the low byte; VacuumObjectTask sets bit 0 of its flags
     * when it is false. */
    virtual bool fn_80238088();
    /* Returns the object's matrix through fn_80237F10: while the weak pointer at 0x4 of the
     * object at 0x4 has not expired, the matrix that the pointee's virtual function at vtable
     * offset 0x44 returns; otherwise, while the object's weak_ptr<CharInfo> at 0xC has not
     * expired, a matrix built from that character info (fn_8040B034, fn_8040FFBC); when both have
     * expired it returns without writing the result (0x80237F90). The return type is inferred
     * from the constructor, which copies the result with fn_8040C148. */
    virtual Fn8040Matrix34 fn_80237EE4();

    /* Returns what fn_8023851C returns for the object at 0x4 (the type is inferred from
     * SetItem::VacuumItemBox, which locks the result as a weak_ptr<CharInfo>). */
    boost::weak_ptr<CharInfo> fn_80238058();
    /* Returns the object's position (fn_80238880; behaviour inferred). */
    Fn8040Vector3 fn_802382B0();
    /* Returns the rotation that fn_80238268 stores (behaviour inferred from its body). */
    UnkRotation3 fn_80238248();
    /* Returns 1 minus the length of the vector at 0x48 of the object at 0x4 divided by that
     * object's float at 0x68 (behaviour from its body; the meaning of both values is unknown; the
     * member form is inferred from its use of the object at 0x4, the float result from the
     * caller's use of f1). Balloon::VacuumBalloon clamps it to [0, 1] to shrink its model. */
    float fn_8023800C();

    /* The constructor's 0x84-byte object. A scoped_ptr: VacuumObject's destructor destroys it with
     * 0x802383B4, which passes the pointer to boost's checked_delete (0x80238400), and some of
     * VacuumObject's functions read it through an out-of-line operator-> (0x80238240). */
    /* 0x04 */ boost::scoped_ptr<UnkVacuumState_802389BC> field_0x4;
};
CHECK_SIZEOF(VacuumObject, 0x8);

/* VacuumObjectTask (class name from RTTI, with Task as its only base): the task that keeps a
 * VacuumObject. Its constructor (0x80237C90) creates it under task layer 13 and keeps a copy of
 * object at 0x28; its destructor is at 0x80237B24. Nothing else is declared because the derived
 * classes' files only create it. */
class VacuumObjectTask : public Task {
  public:
    VacuumObjectTask(const boost::shared_ptr<VacuumObject> &object);
    virtual ~VacuumObjectTask();

    /* 0x28 */ boost::shared_ptr<VacuumObject> m_object;
};
CHECK_SIZEOF(VacuumObjectTask, 0x30);

extern "C" {
/* Returns 1 (the body ignores command; purpose unknown). SetItem::VacuumItemBox passes its command
 * and hands the result to VacuumObject's constructor. */
int fn_80237EDC(VacuumCommand *command);
}

/* The float 3.5 at the start of the .sdata2 of VacuumObject's file (purpose unknown). The BALLOON
 * gadget divides it by 15 for the scale its VacuumBalloon shrinks to; the static initialisers at
 * 0x80254E64, 0x802F6320 and 0x8033FB8C (.ctors entries) read it too. */
extern const float lbl_805F72B0;

#endif

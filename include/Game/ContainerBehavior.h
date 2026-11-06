#ifndef GAME_CONTAINERBEHAVIOR_H
#define GAME_CONTAINERBEHAVIOR_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class GadgetModelInfo;
class UnkHandleBTarget_8015BE64;
class UnkIdList_8003A154;

/* ContainerBehavior (class name from RTTI, with no base class): a 0x44-byte object with a position
 * and a rotation. The CONTAINER gadget's container keeps one in a shared pointer; copies of that
 * shared pointer's functions and its control block's constructor are also in the code before the
 * CONTAINER gadget's file (the first at USA (GUPE8P) 0x800EED84). Its out-of-line functions
 * (0x800F8D2C-0x800F95C0), vtable (0x80525560) and RTTI follow the CONTAINER gadget's file in a
 * file of their own.
 *
 * The layout comes from the constructor (0x800F9354) and the virtual destructor (0x800F9290).
 * Method names are inferred from what the bodies do, and so are the parameters' pointer and const
 * spellings. */
class ContainerBehavior {
  public:
    /* Copies *position to m_position, keeps model in m_model and fills field_0xC and field_0x14
     * (0x800F9354; the parameter types are inferred: the CONTAINER gadget passes its character
     * info's position and a copy of its model pointer that it destroys after the call). */
    ContainerBehavior(const Fn8040Vector3 *position, boost::shared_ptr<GadgetModelInfo> model);
    /* Defined elsewhere (0x800F9290). */
    virtual ~ContainerBehavior();

    /* Copies *position to m_position and *rotation to m_rotation (0x800F8FC4 calls it; the
     * original's out-of-line copy is at 0x800EF474). */
    void SetTransform(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
        m_position = *position;
        m_rotation = *rotation;
    }
    /* Copies m_position to *position. The original calls its out-of-line copy (0x800EF448) from
     * all six callers (0x800EF1E8, 0x800F79E0, 0x800FAA64, 0x800FC0FC, 0x800FDCF8 and
     * 0x80278CF0). */
    void GetPosition(Fn8040Vector3 *position) {
        *position = m_position;
    }
    /* SetTransform with position and rotation, then sets field_0x40 and clears m_flags
     * (0x800F8FC4, behaviour from its body). */
    void Reset(const Fn8040Vector3 *position, const UnkRotation3 *rotation);
    /* Updates m_position and m_flags (0x800F9000; it takes no dt; the behaviour is not recovered
     * here). */
    void Update();

    /* 0x04 */ boost::weak_ptr<GadgetModelInfo> m_model;
    /* 0x0C */ boost::shared_ptr<UnkIdList_8003A154> field_0xC;
    /* 0x14 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> field_0x14;
    /* The constructor zeroes the eight words at 0x1C-0x3C through the out-of-line function at
     * 0x800F9570 before it sets them, so they may form one member of an unrecovered type. */
    /* 0x1C */ Fn8040Vector3 m_position;
    /* 0x28 */ UnkRotation3 m_rotation;
    /* 0x34 */ float field_0x34; /* the constructor stores -1.2 */
    /* 0x38 */ float field_0x38; /* the constructor stores 0 */
    /* 0x3C */ u32 m_flags;      /* cleared by the constructor and Reset; Update sets bit 0 */
    /* 0x40 */ u8 field_0x40;    /* set by the constructor and Reset; Update clears it */
};
CHECK_SIZEOF(ContainerBehavior, 0x44);

#endif

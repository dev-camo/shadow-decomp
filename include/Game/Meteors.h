#ifndef GAME_METEORS_H
#define GAME_METEORS_H

#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* Meteors (class name from RTTI): the gadget of the file that follows Meteor's. Only what Meteor's
 * code uses is declared here; the classes are not mapped yet. */
class Meteors;

/* The 0x24-byte settings that MeteorsTask hands on to its Meteors (placeholder name; the type has
 * no RTTI). The constructor at 0x8021FCE4 stores arg0 and arg1 at 0x0 and 0x4, copies direction
 * and rotateSpeed to 0x8 and 0x14 and stores arg4 at 0x20 (behaviour inferred from its body; the
 * meanings of the floats are unknown). */
struct UnkMeteorsParams_8021FCE4 {
    UnkMeteorsParams_8021FCE4(float arg0, float arg1, const Fn8040Vector3 &direction,
                              Fn8040Vector3 rotateSpeed, float arg4);

    /* 0x00 */ float field_0x0;
    /* 0x04 */ float field_0x4;
    /* 0x08 */ Fn8040Vector3 m_direction;
    /* 0x14 */ Fn8040Vector3 m_rotateSpeed;
    /* 0x20 */ float field_0x20;
};
CHECK_SIZEOF(UnkMeteorsParams_8021FCE4, 0x24);

/* MeteorsTask (class name from RTTI): a task that owns a Meteors through a shared pointer. Its
 * constructor (0x8021E334) creates the Meteors from its arguments; the meaning of arg4 is unknown
 * (Meteors stores it at 0x4). */
class MeteorsTask : public Task {
  public:
    MeteorsTask(Task *parent, const Fn8040Vector3 &position, const Fn8040Vector3 &rotation,
                const UnkMeteorsParams_8021FCE4 &params, int arg4);
    virtual ~MeteorsTask();
    virtual void Update(float dt);

    /* 0x28 */ boost::shared_ptr<Meteors> m_meteors;
};
CHECK_SIZEOF(MeteorsTask, 0x30);

#endif

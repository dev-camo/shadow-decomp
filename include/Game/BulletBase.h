#ifndef GAME_BULLETBASE_H
#define GAME_BULLETBASE_H

#include "Game/UnkParams_80051D10.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;

/* The 0x24-byte record at 0x4 of every Weapon::Bullet::BulletBase (placeholder name; the original
 * type is unknown). Its default constructor (0x80052390) sets only rotation (to 0), the
 * constructor at 0x80053878 copies its three arguments, the copy constructor is at 0x80052914 and
 * the assignment at 0x8005246C. Member names are inferred: fn_8005A228 sets velocity to a
 * direction scaled by a speed and rotation to a pitch and yaw with a zero roll, and
 * Weapon::Bullet::ChaosSpear::ChaosSpear moves position by velocity * dt. The type of rotation is
 * inferred: the copy constructor copies position and velocity through Fn8040Vector3's out-of-line
 * copy constructor and rotation member by member, as UnkRotation3 is copied. */
class UnkBulletMotion_80052390 {
  public:
    UnkBulletMotion_80052390();
    UnkBulletMotion_80052390(const Fn8040Vector3 &position, const Fn8040Vector3 &velocity,
                             const UnkRotation3 &rotation);
    UnkBulletMotion_80052390(const UnkBulletMotion_80052390 &other);
    UnkBulletMotion_80052390 &operator=(const UnkBulletMotion_80052390 &other);

    /* 0x00 */ Fn8040Vector3 position;
    /* 0x0C */ Fn8040Vector3 velocity;
    /* 0x18 */ UnkRotation3 rotation;
};
CHECK_SIZEOF(UnkBulletMotion_80052390, 0x24);

extern "C" {
/* Builds a motion from the bullet parameters: position is field_0x10 of params; velocity is
 * field_0x1C normalised and scaled by speed plus the dot product of field_0x28 with that unit
 * vector when the product is above 0; rotation is the pitch and yaw that fn_8040F760 gives for
 * the negated field_0x1C, with a zero roll (behaviour from its body; the name of the function is
 * unknown, the reference and the by-value result are inferred). */
UnkBulletMotion_80052390 fn_8005A228(const UnkParams_80051D10 &params, float speed);
}

namespace Weapon {
namespace Bullet {

/* Weapon::Bullet::BulletBase (class name from RTTI): the base of the bullets. Its constructor
 * (0x8005230C) and destructor (0x80052240) are out of line; the constructor copies field_0x34 of
 * the parameters to field_0x28 and the parameters' owner to m_owner, sets field_0x34 to 3 when
 * field_0x28 is 0 and to 6 otherwise, sets m_distance to 0 and passes the bullet to fn_80058B80
 * with the object that fn_800522A8 returns; the destructor passes it to fn_80058B5C with the same
 * object. Slot 1 of its vtable (0x8051D840) is null (a pure virtual function). Member names are
 * inferred where they are not placeholders; the reference parameter of the constructor is
 * inferred. */
class BulletBase {
  public:
    BulletBase(const UnkParams_80051D10 &params);
    virtual ~BulletBase();
    /* Weapon::Bullet::ChaosSpear::ChaosSpear moves its motion by dt (name inferred). */
    virtual void Update(float dt) = 0;

    /* 0x04 */ UnkBulletMotion_80052390 m_motion;
    /* 0x28 */ int field_0x28;
    /* 0x2C */ boost::weak_ptr<CharInfo> m_owner;
    /* Passed as the first argument of fn_8005AFE4 by Weapon::Bullet::ChaosSpear::ChaosSpear. */
    /* 0x34 */ int field_0x34;
    /* Weapon::Bullet::ChaosSpear::ChaosSpear adds the length of each step to it. */
    /* 0x38 */ float m_distance;
};
CHECK_SIZEOF(Weapon::Bullet::BulletBase, 0x3C);

} // namespace Bullet
} // namespace Weapon

#endif

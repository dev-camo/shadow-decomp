#ifndef GAME_LAMBORGHINICAMERA_H
#define GAME_LAMBORGHINICAMERA_H

#include "Game/ExternalCameraRequest.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/scoped_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class CharInfo;

namespace Boss {
namespace Camera {

class LamborghiniImpl;

/* Boss::Camera::Lamborghini (name inferred from its implementation class,
 * Boss::Camera::LamborghiniImpl, named by RTTI): a camera driven by two characters, created with
 * new by the code that uses it. The implementation that it owns does the work (see
 * Game/LamborghiniCamera.cpp); each member forwards to the implementation's member of the same
 * name. Method and parameter names and the pointer and reference spellings of the parameters are
 * inferred. */
class Lamborghini {
  public:
    /* The implementation keeps copies of the two weak pointers. */
    Lamborghini(const boost::weak_ptr<CharInfo> &first, const boost::weak_ptr<CharInfo> &second);
    ~Lamborghini();

    /* Asks for a camera request when none is held (params may be null). */
    void Start(const UnkCameraParams_8000B31C *params);
    /* Passes the camera request to fn_801CEAD4 (inferred: withdraws it). */
    void Stop();
    void Update(float dt);
    /* The offset added to the first character's position (default the zero vector). */
    void SetFirstOffset(const Fn8040Vector3 &offset);
    /* The distance (x) and height (y) by which the follow point may differ from the first
     * character's position plus its offset (defaults 10 and 20). */
    void SetFirstLimits(const Fn8040Vector2 &limits);
    /* The offset added to the second character's position (default the zero vector). */
    void SetSecondOffset(const Fn8040Vector3 &offset);
    /* The distance (x) and height (y) by which the look point may differ from the second
     * character's position plus its offset (defaults 80 and 20). */
    void SetSecondLimits(const Fn8040Vector2 &limits);
    /* The lowest and highest pitch, in radians, of the look point seen from the follow point
     * (defaults -45 and 15 degrees). */
    void SetPitchLimits(float low, float high);

  private:
    /* 0x0 */ boost::scoped_ptr<LamborghiniImpl> m_impl;
};
CHECK_SIZEOF(Lamborghini, 0x4);

} // namespace Camera
} // namespace Boss

#endif

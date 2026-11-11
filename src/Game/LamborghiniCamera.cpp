/* Boss::Camera::Lamborghini (described in Game/LamborghiniCamera.h) and its implementation class
 * Boss::Camera::LamborghiniImpl (class name from RTTI). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to the camera mode are boost::function objects built with
 * boost::bind(&LamborghiniImpl::method, this, _1); their manager, invoker and copy functions come
 * from the boost headers. */
/* The CharaColliBase RTTI data and the base list that name it start the unit's .rodata, .sdata and
 * .data, so CharaColliBaseCharInfo.h is included first.
 * Matching workaround: the header's stand-in inline cast (see CharaColliBaseCharInfo.h) emits that
 * data; revisit when the original inline function or its real caller is recovered. */
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/LamborghiniCamera.h"
#include "Game/AngleConversion.h"
#include "Game/CameraModeUser.h"
#include "Game/CharInfo.h"
#include "Game/ExternalCameraRequest.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"
#include <cmath>

using SonicteamUSA::System::Camera::CameraModeUser;

namespace Boss {
namespace Camera {

/* Boss::Camera::LamborghiniImpl (class name from RTTI): the state of the camera. Two points follow
 * the two characters: the follow point stays near the first character and the look point near the
 * second. OnCameraUpdate gives the camera mode a start point beyond the follow point, on the side
 * away from the look point, and the look point as the end (inferred: where the camera is and the
 * point it looks at). Method and member names are inferred, as are the pointer and reference
 * spellings of the parameters, the by-value results and the type of m_turning. */
class LamborghiniImpl {
  public:
    /* Creates the camera mode, binds its three callbacks, sets the defaults and puts the follow
     * point and the look point at the two characters' positions (the locks are not checked). */
    LamborghiniImpl(const boost::weak_ptr<CharInfo> &first,
                    const boost::weak_ptr<CharInfo> &second);
    ~LamborghiniImpl();

    /* Only when the weak pointer to the request has expired (no request is held): asks
     * fn_801CED24 for a camera request for the mode, with 0, 4 and params when params is not null
     * and with 0, 0 and null otherwise, and keeps the weak pointer it returns (empty when no
     * request was registered). */
    void Start(const UnkCameraParams_8000B31C *params);
    /* The offset setters copy the offset into the offset that GetPoint uses and into field_0x44
     * or field_0x5C, which this file does not read otherwise. */
    void SetFirstOffset(const Fn8040Vector3 *offset);
    void SetFirstLimits(const Fn8040Vector2 *limits);
    void SetSecondOffset(const Fn8040Vector3 *offset);
    void SetSecondLimits(const Fn8040Vector2 *limits);
    /* Returns point with its y clamped to target's y +- limits.y (fn_80070460), then, when it is
     * more than limits.x from target, moved to target plus limits.x times the unit direction from
     * target to it. */
    Fn8040Vector3 ClampPoint(const Fn8040Vector3 &point, const Fn8040Vector3 &target,
                             const Fn8040Vector2 &limits);
    /* Returns point unchanged when the character has expired, otherwise ClampPoint of point
     * towards the character's position plus offset (the lock is not checked). */
    Fn8040Vector3 GetPoint(const Fn8040Vector3 &point, const boost::weak_ptr<CharInfo> &character,
                           const Fn8040Vector3 &offset, const Fn8040Vector2 &limits);
    /* When either character has expired, calls Stop and returns. Otherwise moves the follow point
     * after the first character (GetPoint), moves the look point by the same amount and then
     * after the second character, and limits the pitch of that new look point from the new follow
     * point (see the body). When the horizontal direction from the follow point to the look point
     * then turns by more than dt * 300 degrees, the old look point's offset from the old follow
     * point is turned by that much and added to the new follow point instead: about the y axis
     * (in the sense of m_turnSign) when the turn is above 45 degrees, otherwise about the
     * normalised cross product of the old and new directions. Otherwise the look point takes its
     * new value and m_turning is cleared. The follow point takes its new value in both cases. */
    void UpdatePoints(float dt);
    /* The camera mode's switch-on callback: puts the follow point at the first character's
     * position and the look point at the second's, each only when that character has not expired,
     * clears m_turning and updates with dt 0. */
    void OnSwitchOn(CameraModeUser::SwitchEventArgs &args);
    /* The camera mode's switch-off callback; it does nothing. */
    void OnSwitchOff(CameraModeUser::SwitchEventArgs &args);
    /* The camera mode's update callback: passes the follow point plus m_distance times the unit
     * direction away from the look point (no distance when the points are too close to
     * normalise), plus m_cameraOffset, as the start and the look point as the end
     * (fn_8000A5D4). */
    void OnCameraUpdate(CameraUpdateEventArgs &args);

    void SetPitchLimits(float low, float high) {
        m_pitchLow = low;
        m_pitchHigh = high;
    }

    /* Passes the camera request to fn_801CEAD4 (inferred: withdraws it). */
    void Stop() {
        fn_801CEAD4(m_request);
    }

    /* Matching workaround: Lamborghini::Update calls the scoped pointer's operator-> out of line
     * (USA (GUPE8P) 0x8031A144), as happens where the implementation's member is an inline
     * function; calling UpdatePoints directly there expands the operator in place (0x24 bytes
     * instead of 0x34). The inline member the original called is unknown. Revisit when it is
     * identified. */
    void Update(float dt) {
        UpdatePoints(dt);
    }

    /* 0x00 */ boost::weak_ptr<PJSCamera::ExternalCameraRequest> m_request;
    /* 0x08 */ boost::shared_ptr<CameraModeUser> m_cameraMode;
    /* 0x10 */ boost::weak_ptr<CharInfo> m_first;
    /* 0x18 */ boost::weak_ptr<CharInfo> m_second;
    /* 0x20 */ Fn8040Vector3 m_lookPoint;
    /* 0x2C */ Fn8040Vector3 m_followPoint;
    /* Added to the characters' positions to give the points' targets. */
    /* 0x38 */ Fn8040Vector3 m_firstOffset;
    /* 0x44 */ Fn8040Vector3 field_0x44;
    /* 0x50 */ Fn8040Vector3 m_secondOffset;
    /* 0x5C */ Fn8040Vector3 field_0x5C;
    /* Set by the first update that turns the look point; cleared by the constructor, by an update
     * that gives both points their new values and by OnSwitchOn. */
    /* 0x68 */ bool m_turning;
    /* 1 or -1, chosen by the update that sets m_turning: from the y of the cross product of the
     * new horizontal direction and the first character's forward direction ((0, 0, -1) turned by
     * its rotation with fn_80410098; the lock is not checked) when the turn is above 90 degrees,
     * otherwise from the y of the cross product of the old and the new horizontal directions; 1
     * when that y is >= 0. */
    /* 0x6C */ float m_turnSign;
    /* ClampPoint's limits: x is the distance, y the height. */
    /* 0x70 */ Fn8040Vector2 m_firstLimits;
    /* 0x78 */ Fn8040Vector2 m_secondLimits;
    /* The pitch limits in radians. */
    /* 0x80 */ float m_pitchLow;
    /* 0x84 */ float m_pitchHigh;
    /* 0x88 */ float m_distance;
    /* 0x8C */ Fn8040Vector3 m_cameraOffset;

  private:
    /* Copying is disabled in this reconstructed interface because the copy contract is
     * unrecovered. The camera mode's callbacks are bound to this object, so a memberwise copy
     * would share the mode and the request while the callbacks still update the original. */
    LamborghiniImpl(const LamborghiniImpl &);
    LamborghiniImpl &operator=(const LamborghiniImpl &);
};
CHECK_SIZEOF(LamborghiniImpl, 0x98);

LamborghiniImpl::LamborghiniImpl(const boost::weak_ptr<CharInfo> &first,
                                 const boost::weak_ptr<CharInfo> &second)
    : m_first(first), m_second(second) {
    m_cameraMode = fn_80009704(0, 0);
    m_cameraMode->field_0xAC =
        CameraSwitchCallback(boost::bind(&LamborghiniImpl::OnSwitchOn, this, _1));
    m_cameraMode->m_callback =
        CameraUpdateCallback(boost::bind(&LamborghiniImpl::OnCameraUpdate, this, _1));
    m_cameraMode->field_0xB8 =
        CameraSwitchCallback(boost::bind(&LamborghiniImpl::OnSwitchOff, this, _1));

    fn_8040CE60(&m_firstOffset);
    fn_8040CE60(&field_0x44);
    fn_8040CE60(&m_secondOffset);
    fn_8040CE60(&field_0x5C);
    m_firstLimits.x = 10.0f;
    m_firstLimits.y = 20.0f;
    m_secondLimits.x = 80.0f;
    m_secondLimits.y = 20.0f;
    m_pitchLow = DegreesToRadians(-45.0f);
    m_pitchHigh = DegreesToRadians(15.0f);
    m_distance = 70.0f;
    m_cameraOffset = Fn8040Vector3(0.0f, 15.0f, 0.0f);

    m_followPoint = first.lock()->m_position;
    m_lookPoint = second.lock()->m_position;
    m_turning = false;
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if another compiler build reproduces this without the pragma, or if an original
 * checked_delete keeps out of line a destructor whose body calls only non-inline functions (the
 * pragma would inline that destructor into it). A destructor found inlined into checked_delete in
 * the original would instead mean that the original used auto_inline here.
 * The destructor is defined out of line, as a global symbol: an inline definition would be
 * emitted at its first use, after checked_delete, away from the original's position. */
#pragma auto_inline on
LamborghiniImpl::~LamborghiniImpl() {}
#pragma auto_inline reset

void LamborghiniImpl::Start(const UnkCameraParams_8000B31C *params) {
    if (m_request.expired()) {
        if (params != 0) {
            m_request = fn_801CED24(m_cameraMode, 0, 4, params, 0, 0, 0);
        } else {
            m_request = fn_801CED24(m_cameraMode, 0, 0, 0, 0, 0, 0);
        }
    }
}

void LamborghiniImpl::SetFirstOffset(const Fn8040Vector3 *offset) {
    field_0x44 = *offset;
    m_firstOffset = *offset;
}

void LamborghiniImpl::SetFirstLimits(const Fn8040Vector2 *limits) {
    m_firstLimits = *limits;
}

void LamborghiniImpl::SetSecondOffset(const Fn8040Vector3 *offset) {
    field_0x5C = *offset;
    m_secondOffset = *offset;
}

void LamborghiniImpl::SetSecondLimits(const Fn8040Vector2 *limits) {
    m_secondLimits = *limits;
}

Fn8040Vector3 LamborghiniImpl::ClampPoint(const Fn8040Vector3 &point, const Fn8040Vector3 &target,
                                          const Fn8040Vector2 &limits) {
    Fn8040Vector3 result(point);

    result.y = fn_80070460(result.y, target.y - limits.y, target.y + limits.y);

    Fn8040Vector3 offset = result - target;
    if (fn_8040CF50(offset) > limits.x) {
        result = target + fn_8040CEE8(offset) * limits.x;
    }
    return result;
}

Fn8040Vector3 LamborghiniImpl::GetPoint(const Fn8040Vector3 &point,
                                        const boost::weak_ptr<CharInfo> &character,
                                        const Fn8040Vector3 &offset, const Fn8040Vector2 &limits) {
    if (character.expired()) {
        return point;
    }

    boost::shared_ptr<CharInfo> charInfo(character.lock());
    Fn8040Vector3 target = charInfo->m_position + offset;
    return ClampPoint(point, target, limits);
}

void LamborghiniImpl::UpdatePoints(float dt) {
    if (m_second.expired() || m_first.expired()) {
        Stop();
        return;
    }

    Fn8040Vector3 followPoint = GetPoint(m_followPoint, m_first, m_firstOffset, m_firstLimits);
    Fn8040Vector3 move = followPoint - m_followPoint;
    Fn8040Vector3 lookPoint = m_lookPoint + move;
    lookPoint = GetPoint(lookPoint, m_second, m_secondOffset, m_secondLimits);
    Fn8040Vector3 oldDirection = m_lookPoint - m_followPoint;
    Fn8040Vector3 newDirection = lookPoint - followPoint;
    Fn8040Vector3 oldFlat(oldDirection.x, 0.0f, oldDirection.z);
    Fn8040Vector3 newFlat(newDirection.x, 0.0f, newDirection.z);

    /* Unless the horizontal distance is 0 as fn_8000A950 judges it: when the height is >= 0 and
     * the slope is above tan(m_pitchHigh), or the height is below 0 (or not a number) and the slope
     * is below tan(m_pitchLow), the height becomes the horizontal distance times that tangent. */
    float length = fn_8040CF50(newFlat);
    float height = newDirection.y;
    if (!fn_8000A950(length)) {
        float slope = height / length;
        if (height >= 0.0f) {
            float limit = std::tan(m_pitchHigh);
            if (slope > limit) {
                newDirection.y = length * limit;
                lookPoint.y = newDirection.y + followPoint.y;
            }
        } else {
            float limit = std::tan(m_pitchLow);
            if (slope < limit) {
                newDirection.y = length * limit;
                lookPoint.y = newDirection.y + followPoint.y;
            }
        }
    }

    fn_8040CE74(&oldFlat);
    fn_8040CE74(&newFlat);
    float angle = fn_8040F8FC(&oldFlat, &newFlat);
    float maxTurn = dt * DegreesToRadians(300.0f);

    if (angle > maxTurn) {
        if (!m_turning) {
            m_turning = true;
            if (angle > DegreesToRadians(90.0f)) {
                boost::shared_ptr<CharInfo> charInfo(m_first.lock());
                /* The original builds (0, 0, -1) twice, in forward and in the argument of
                 * fn_80410098 (USA (GUPE8P) 0x8031A678-0x8031A694). */
                Fn8040Vector3 forward(0.0f, 0.0f, -1.0f);
                forward = fn_80410098(Fn8040Vector3(0.0f, 0.0f, -1.0f), &charInfo->m_rotation,
                                      charInfo->field_0x30);
                if (fn_8040CFB8(newFlat, forward).y >= 0.0f) {
                    m_turnSign = 1.0f;
                } else {
                    m_turnSign = -1.0f;
                }
            } else {
                if (fn_8040CFB8(oldFlat, newFlat).y >= 0.0f) {
                    m_turnSign = 1.0f;
                } else {
                    m_turnSign = -1.0f;
                }
            }
        }

        if (angle > DegreesToRadians(45.0f)) {
            Fn8040Vector3 offset = m_lookPoint - m_followPoint;
            offset = fn_8040D0AC(offset, maxTurn * m_turnSign);
            m_lookPoint = offset + followPoint;
        } else {
            Fn8040Vector3 axis = fn_8040CEE8(fn_8040CFB8(oldDirection, newDirection));
            Fn8040Matrix34 turn = fn_8040FD8C(&axis, maxTurn);
            Fn8040Vector3 offset = m_lookPoint - m_followPoint;
            offset = turn * offset;
            m_lookPoint = offset + followPoint;
        }
        m_followPoint = followPoint;
    } else {
        m_turning = false;
        m_lookPoint = lookPoint;
        m_followPoint = followPoint;
    }
}

void LamborghiniImpl::OnSwitchOn(CameraModeUser::SwitchEventArgs &args) {
    if (!m_first.expired()) {
        m_followPoint = m_first.lock()->m_position;
    }
    if (!m_second.expired()) {
        m_lookPoint = m_second.lock()->m_position;
    }
    m_turning = false;
    UpdatePoints(0.0f);
}

void LamborghiniImpl::OnSwitchOff(CameraModeUser::SwitchEventArgs &args) {}

void LamborghiniImpl::OnCameraUpdate(CameraUpdateEventArgs &args) {
    /* The original copies the look point first (USA 0x8031A2DC) and assigns position after the
     * sums (0x8031A33C). */
    Fn8040Vector3 lookPoint(m_lookPoint);
    Fn8040Vector3 position;
    Fn8040Vector3 direction = m_followPoint - m_lookPoint;
    fn_8040CE74(&direction);
    position = m_followPoint + direction * m_distance + m_cameraOffset;
    m_cameraMode->SetPoints(position, lookPoint);
}

Lamborghini::Lamborghini(const boost::weak_ptr<CharInfo> &first,
                         const boost::weak_ptr<CharInfo> &second)
    : m_impl(new LamborghiniImpl(first, second)) {}

Lamborghini::~Lamborghini() {}

void Lamborghini::Start(const UnkCameraParams_8000B31C *params) {
    m_impl->Start(params);
}

void Lamborghini::Stop() {
    m_impl->Stop();
}

void Lamborghini::Update(float dt) {
    m_impl->Update(dt);
}

void Lamborghini::SetFirstOffset(const Fn8040Vector3 *offset) {
    m_impl->SetFirstOffset(offset);
}

void Lamborghini::SetFirstLimits(const Fn8040Vector2 *limits) {
    m_impl->SetFirstLimits(limits);
}

void Lamborghini::SetSecondOffset(const Fn8040Vector3 *offset) {
    m_impl->SetSecondOffset(offset);
}

void Lamborghini::SetSecondLimits(const Fn8040Vector2 *limits) {
    m_impl->SetSecondLimits(limits);
}

void Lamborghini::SetPitchLimits(float low, float high) {
    m_impl->SetPitchLimits(low, high);
}

} // namespace Camera
} // namespace Boss

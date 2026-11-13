/* The parachute of the ROCKET gadget's rider (class described in Game/Parachute.h). File name
 * inferred from the namespace. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first).
 *
 * The character info's command handler is a boost::function object built with
 * boost::bind(&Parachute::OnCommand, this, _1); its manager, invoker and copy functions come from
 * the boost headers. */
#include "Game/Parachute.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/CharaColliBase.h"
#include "Game/PlayerCommands.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_801BFC28.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

namespace Parachute {

/* The model file, which is the ROCKET gadget's (name inferred). */
static const char *s_modelFile = "ROCKET.BIN";

const float Parachute::s_driftRadius = 200.0f;

/* How far below the parachute UpdateMotion looks for the ground (name inferred).
 * Matching workaround: the original sets this float with the unit's start-up function in .ctors
 * (USA (GUPE8P) 0x80293904 stores 20 to .sbss 0x805F032C), which shows only that its initialiser
 * is not a constant expression. UnkStartupFloat_800E2570 stands in for that initialiser, which is
 * unknown. The const qualifier follows the other files that use this form; here the code is the
 * same without it (measured). Revisit when the original initialiser is known. */
static const float s_groundDistance = UnkStartupFloat_800E2570(20.0f).value;
/* Two degrees in radians. No code reads it; the start-up function stores it (0x805F0330). */
static const float s_unusedAngle = DegreesToRadians(2.0f);

/* The predicate of UpdateMotion's probe (placeholder name): rejects the records whose target has
 * 1, 2 or 13 in the halfword at 0x4 (what those values stand for is unknown) and accepts every
 * other record, including those without a target. */
static bool fn_802937E4(const UnkProbeFilterRecord *record) {
    const CharaColliBase *target = record->target;

    if (target != 0) {
        u16 kind = target->field_0x4;

        if (kind == 1 || kind == 2 || kind == 13) {
            return false;
        }
    }
    return true;
}

/* Matching workaround: angles is not const. The original loads each of its components only after
 * storing the previous one (USA (GUPE8P) 0x802934F8-0x8029350C), as this compiler does when the
 * stores may change the source; with a const pointer the y and z loads move ahead of the x and y
 * stores (4 words differ, measured). Rocket::SetTransform has the same form. Revisit with the
 * shared Fn8040Vector3/UnkRotation3 parameter conventions. */
Parachute::Parachute(Task *parent, const boost::weak_ptr<CharInfo> &rider,
                     const Fn8040Vector3 *position, UnkRotation3 *angles)
    : Task(parent), m_rider(rider), m_state(STATE_OPENING) {
    m_charInfo = fn_8015CB88(0x89, 4, "Parachute");
    fn_8015CF7C(m_charInfo.get());
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&Parachute::OnCommand, this, _1));

    m_charInfo->m_position = *position;
    m_charInfo->m_rotation = *angles;
    m_anchor = m_previousPosition = m_charInfo->m_position;

    m_model = fn_8015F800(0, s_modelFile);
    m_model->fn_80162A68(2, 0);
    m_model->fn_80160C28(2);
    m_model->fn_80160B18(2);

    if (boost::shared_ptr<CharInfo> info = m_rider.lock()) {
        info->fn_801DC980(new Player::TouchHangCommand(boost::weak_ptr<CharInfo>(m_charInfo),
                                                       &m_charInfo->m_position,
                                                       &m_charInfo->m_rotation, 3, 4));
    }

    UpdateModelMatrix();

    m_stateFlags.Clear();
    m_stateFlags.Set(FLAG_MOVING);
    m_sweepFlags.Set(1);
    fn_801D560C(fn_80053164(), 0x100C, 0, 0);
}

Parachute::~Parachute() {}

void Parachute::Update(float dt) {
    if (m_stateFlags.Test(FLAG_MOVING)) {
        UpdateMotion(dt);
    }

    UpdateModelMatrix();
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    if (!m_stateFlags.Test(FLAG_RELEASED)) {
        if (boost::shared_ptr<CharInfo> rider = m_rider.lock()) {
            rider->fn_801DC980(new Player::TouchHangCommand(boost::weak_ptr<CharInfo>(m_charInfo),
                                                            &m_charInfo->m_position,
                                                            &m_charInfo->m_rotation, 3, 4));
        }
    }

    switch (m_state) {
    case STATE_OPENING:
        if (m_model->fn_80160040(2)) {
            m_stateFlags.Set(FLAG_MOVING);
            m_state = STATE_DESCENDING;
        }
        break;

    case STATE_RELEASE:
        if (boost::shared_ptr<CharInfo> rider = m_rider.lock()) {
            rider->fn_801DC980(new Player::CancelBehaviorCommand(4));
        }
        m_state = STATE_FINISHED;
        break;

    case STATE_FINISHED:
        m_flags |= 1;
        break;
    }
}

void Parachute::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
}

void Parachute::UpdateMotion(float dt) {
    fn_804103E8(&m_charInfo->m_position, &m_charInfo->field_0x18, &m_charInfo->field_0x24, 1.0f,
                dt);
    m_charInfo->m_position.y -= 66.0f * dt;

    Fn8040Vector3 movement = m_charInfo->m_position - m_previousPosition;
    Fn8040Vector3 position = m_previousPosition;

    /* Matching workaround: the world getter is called on its own and its result is unused. The
     * original calls it at this point and discards r3 (USA (GUPE8P) 0x80292BC8, before 0 is
     * loaded as fn_801BFC28's first argument), as the other callers of fn_801BFC28 that pass 0
     * there do (see fn_801BFC28.h); the source form behind the call, probably a shared inline
     * wrapper, is unknown. Revisit when the wrapper is recovered. */
    fn_80021B08();
    if (fn_801BFC28(0, &position, &movement, &m_sweepFlags, 0, 10.0f)) {
        m_charInfo->m_position = position;
        m_charInfo->field_0x18 = movement * (1.0f / dt);
        m_charInfo->field_0x18.y = 0.0f;
    }

    m_previousPosition = m_charInfo->m_position;
    fn_8040CE60(&m_charInfo->field_0x24);

    Fn8040Vector3 start = m_charInfo->m_position;
    Fn8040Vector3 end = start;

    end.y -= s_groundDistance;

    UnkSegment_80020A20 segment(start, end);
    UnkProbeNode_80021FE4 *hit = fn_80021E24(fn_80020D40(), &segment, 3, 8, fn_802937E4, 0);

    if (hit != 0) {
        if (hit->hit != 0) {
            m_stateFlags.Set(FLAG_RELEASED);
            m_stateFlags.Reset(FLAG_MOVING);
            m_state = STATE_RELEASE;
        }
        delete hit;
    }

    Fn8040Vector3 drift = m_charInfo->m_position - m_anchor;

    drift.y = 0.0f;

    float distance = fn_8040CF50(drift);

    if (distance > s_driftRadius) {
        distance = s_driftRadius;
    }
    drift = fn_8040CEE8(drift) * distance;
    m_charInfo->m_position.x = m_anchor.x + drift.x;
    m_charInfo->m_position.z = m_anchor.z + drift.z;

    fn_801D53C4(fn_80053164(), 0x100D, 0, this, 2, 0, 2);
}

void Parachute::OnCommand(CharCommand &command) {
    switch (command.m_type) {
    case 12:
        m_stateFlags.Set(FLAG_RELEASED);
        m_state = STATE_RELEASE;
        break;

    case 1: {
        AccelerateInputCommand *accelerate = static_cast<AccelerateInputCommand *>(&command);

        m_charInfo->field_0x24 = accelerate->m_vector * 80.0f;
        m_charInfo->field_0x24.y = 0.0f;
        break;
    }
    }
}

} // namespace Parachute

extern "C" void fn_80292A64(const boost::weak_ptr<CharInfo> &rider, const Fn8040Vector3 *position,
                            UnkRotation3 *angles) {
    /* The parent has a variable of its own: the original keeps the pointer in a scratch register
     * across the load of the allocation size (USA (GUPE8P) 0x80292A8C-0x80292A94); written as the
     * constructor's argument, the function is one instruction shorter (measured). */
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new Parachute::Parachute(parent, rider, position, angles);
}

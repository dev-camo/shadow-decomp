/* The ElecSearchLight gadget (behaviour in the class comment). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The character info's command handler, the model's collision callback and the draw layer's
 * callback are boost::function objects built with boost::bind(&ElecSearchLight::OnCommand, this,
 * _1) and its siblings; their managers, invokers and copy functions come from the boost headers. */
#include <cmath>
/* TNumEdit.h and CharaColliBaseCharInfo.h are included only for the RTTI records that their
 * never-called stand-in functions emit (see the Matching workaround notes there); no code in this
 * file uses them. TNumEdit.h comes first, in the order their names appear in the unit's .rodata
 * (alphabetical order would reverse them). */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/ElecSearchLight.h"
#include "Game/AngleConversion.h"
#include "Game/CharCommandUnit.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PlayerCommands.h"
#include "Game/SharedConstants.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkModelAtomicState.h"
#include "Game/UnkParamCursor_800CBC20.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_80038E84.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8012773C.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

/* The most the beam turns per second towards its target, in radians (inferred). */
static const float s_turnSpeed = 2.0f * lbl_805FAD38;
/* The colours that FadeIn blends the model's materials through: (0, 0, 0, 0), (0xFF, 0xFF, 0x33, 0)
 * and (0xFF, 0xFF, 0xFF, 0xFF) (names inferred). */
static UnkDebugColor_802E0E84 s_fadeColor0(0x00, 0x00, 0x00, 0x00);
static UnkDebugColor_802E0E84 s_fadeColor1(0xFF, 0xFF, 0x33, 0x00);
static UnkDebugColor_802E0E84 s_fadeColor2(0xFF, 0xFF, 0xFF, 0xFF);

/* What OnCollision passes with the hits (placeholder name): the gadget's character info and its
 * parameters. The callback below reads neither. */
struct UnkHitContext_801543A8 {
    UnkHitContext_801543A8() : m_params(0) {}

    /* 0x0 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x8 */ const ElecSearchLightParams *m_params;
};
CHECK_SIZEOF(UnkHitContext_801543A8, 0xC);

/* The callback that fn_8015AA84 calls for each character info in the collision results
 * (placeholder name): it does nothing. */
static void fn_80156018(boost::shared_ptr<CharInfo> hit, void *context) {}

const char *ElecSearchLight::s_name = "ElecSearchLight";

/* The part callback (placeholder name; ElecBomb's has the same body): for a part whose bit (1 << its
 * word at 0x8) is set in the mask passed as the user data, calls fn_80038F94 on its clump when bit
 * 0x8000 of the mask is set and fn_80038FC0 otherwise. Those set and clear the render flag (0x4) of
 * the clump's atomics, so the comments at the calls say "shows" and "hides". */
static UnkModelPart_8015FA78 *fn_80154DA0(UnkModelPart_8015FA78 *part, void *userData);

ElecSearchLight::ElecSearchLight(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_charInfo = fn_8015CB88(5, 4, "ElecSearchLight");
    m_charInfo->m_commandUnit = boost::shared_ptr<CharCommandUnit>(new CharCommandUnit);
    m_charInfo->m_commandUnit->m_handler =
        CharCommandHandler(boost::bind(&ElecSearchLight::OnCommand, this, _1));

    m_model = new GadgetModelInfo(1, "LIGHT.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_8016239C("mat1");
    m_model->fn_8016239C("mat2");
    m_model->fn_8016239C("mat3");
    m_model->fn_8016239C("mat4");
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&ElecSearchLight::OnCollision, this, _1));

    CopyFromOwner(m_owner);
    m_timer = 0.0f;
    m_state = 0;
    m_pitch = 0.0f;
    m_yaw = 0.0f;
    m_switchState = fn_8016904C(fn_800CB69C(), m_comId);

    /* Shows parts 0 to 2. */
    m_model->fn_8015FA78(fn_80154DA0, reinterpret_cast<void *>(0x8007));
    m_brightness = 1.0f;
    UpdateModelMatrix();

    m_layer = fn_8004E24C(fn_80046178()->field_0x28);
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&ElecSearchLight::OnDraw, this, _1));
}

ElecSearchLight::~ElecSearchLight() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }
}

/* Matching workaround: the return inside the if repeats the final one. With one trailing return
 * the shift constant 1 goes to r3 instead of r5 and the prologue and epilogue are scheduled
 * differently (9 of the 23 words differ); an early return when the bit is clear makes the function
 * 0x60 bytes instead of 0x5C. Revisit if the original form of this callback (ElecBomb's
 * fn_80263F38 has the same body) is found. */
static UnkModelPart_8015FA78 *fn_80154DA0(UnkModelPart_8015FA78 *part, void *userData) {
    u32 mask = reinterpret_cast<u32>(userData);

    if ((1 << part->field_0x8) & mask) {
        if (mask & 0x8000) {
            fn_80038F94(part->clump);
        } else {
            fn_80038FC0(part->clump);
        }
        return part;
    }
    return part;
}

extern "C" void fn_80154C6C(UnkModelAtomicState *state, float t, const UnkDebugColor_802E0E84 &from,
                            const UnkDebugColor_802E0E84 &to, float t0, float t1) {
    float ratio = (t - t0) / (t1 - t0);

    state->color.field_0x0 = from.field_0x0 + ratio * (to.field_0x0 - from.field_0x0);
    state->color.field_0x1 = from.field_0x1 + ratio * (to.field_0x1 - from.field_0x1);
    state->color.field_0x2 = from.field_0x2 + ratio * (to.field_0x2 - from.field_0x2);
    state->color.field_0x3 = from.field_0x3 + ratio * (to.field_0x3 - from.field_0x3);
}

void ElecSearchLight::FadeIn(float dt) {
    UnkModelAtomicState *material1 = m_model->fn_801621F0("mat1");
    UnkModelAtomicState *material2 = m_model->fn_801621F0("mat2");
    UnkModelAtomicState *material3 = m_model->fn_801621F0("mat3");
    UnkModelAtomicState *material4 = m_model->fn_801621F0("mat4");
    float previous = m_timer;

    m_timer = previous + dt;
    if (previous < 0.666667f && 0.666667f <= m_timer) {
        /* Shows part 0. */
        m_model->fn_8015FA78(fn_80154DA0, reinterpret_cast<void *>(0x8001));
    } else if (previous < 1.0f && 1.0f <= m_timer) {
        /* Shows parts 1 and 2 and hides part 3. */
        m_model->fn_8015FA78(fn_80154DA0, reinterpret_cast<void *>(0x8006));
        m_model->fn_8015FA78(fn_80154DA0, reinterpret_cast<void *>(0x8));
    }

    if (m_timer < 0.666667f) {
        fn_80154C6C(material4, m_timer, s_fadeColor0, s_fadeColor1, 0.0f, 0.666667f);
    } else if (m_timer < 1.0f) {
        fn_80154C6C(material4, m_timer, s_fadeColor1, s_fadeColor0, 0.666667f, 1.0f);
        fn_80154C6C(material1, m_timer, s_fadeColor0, s_fadeColor2, 0.666667f, 1.0f);
    } else if (m_timer < 1.5f) {
        fn_80154C6C(material2, m_timer, s_fadeColor0, s_fadeColor2, 1.0f, 1.5f);
        fn_80154C6C(material3, m_timer, s_fadeColor0, s_fadeColor2, 1.0f, 1.5f);
        m_brightness = 1.0f - (1.5f - m_timer) / 0.5f;
    } else {
        m_state = 0;
        m_brightness = 1.0f;
        m_timer = 0.0f;
    }
}

/* The two switches repeat the cases 2 to 5: the original has a separate copy of each in both. */
void ElecSearchLight::UpdateBody(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_params.type) {
    case 0:
        switch (m_state) {
        case 0:
            Sweep();
            Search();
            break;

        case 1:
            break;

        case 2:
            PullTarget();
            break;

        case 3:
            /* Hides parts 0 to 2. */
            m_model->fn_8015FA78(fn_80154DA0, reinterpret_cast<void *>(0x7));
            m_brightness = 0.0f;
            m_state = 4;
            m_timer = 0.0f;
            StartBreakEffect();
            m_model->fn_80161230(0, -1);
            break;

        case 4:
            m_timer += dt;
            if (m_timer >= 3.0f) {
                boost::shared_ptr<CharInfo> nearest(
                    fn_800A8A7C(fn_80074194(), &m_charInfo->m_position));

                if (nearest.get() != 0) {
                    Fn8040Vector3 offset = m_charInfo->m_position - nearest->m_position;

                    if (fn_8040CF2C(&offset) > 10000.0f) {
                        m_state = 5;
                        m_timer = 0.0f;
                        m_yaw = 0.0f;
                        /* Shows part 3. */
                        m_model->fn_8015FA78(fn_80154DA0, reinterpret_cast<void *>(0x8008));
                    }
                }
            }
            break;

        case 5:
            FadeIn(dt);
            m_model->fn_8016111C(0, -1);
            break;
        }
        break;

    case 1:
        switch (m_state) {
        case 0:
            Sweep();
            if (fn_8016904C(fn_800CB69C(), m_comId) == 1 && m_switchState != 1) {
                boost::shared_ptr<CharInfo> nearest(fn_800A8810(
                    fn_80074194(), UnkSegment_80020A20(m_charInfo->m_position, m_beamEnd)));

                if (nearest.get() != 0) {
                    m_target = nearest;
                    m_state = 1;
                }
            }
            m_switchState = fn_8016904C(fn_800CB69C(), m_comId);
            break;

        case 1:
            TurnToTarget(dt);
            break;

        case 2:
            PullTarget();
            break;

        case 3:
            /* Hides parts 0 to 2. */
            m_model->fn_8015FA78(fn_80154DA0, reinterpret_cast<void *>(0x7));
            m_brightness = 0.0f;
            m_state = 4;
            m_timer = 0.0f;
            StartBreakEffect();
            m_model->fn_80161230(0, -1);
            break;

        case 4:
            m_timer += dt;
            if (m_timer >= 3.0f) {
                boost::shared_ptr<CharInfo> nearest(
                    fn_800A8A7C(fn_80074194(), &m_charInfo->m_position));

                if (nearest.get() != 0) {
                    Fn8040Vector3 offset = m_charInfo->m_position - nearest->m_position;

                    if (fn_8040CF2C(&offset) > 10000.0f) {
                        m_state = 5;
                        m_timer = 0.0f;
                        m_yaw = 0.0f;
                        /* Shows part 3. */
                        m_model->fn_8015FA78(fn_80154DA0, reinterpret_cast<void *>(0x8008));
                    }
                }
            }
            break;

        case 5:
            FadeIn(dt);
            m_model->fn_8016111C(0, -1);
            break;
        }
        break;
    }

    UpdateModelMatrix();
}

void ElecSearchLight::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    Sweep();
    UpdateModelMatrix();

    if (m_model != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void ElecSearchLight::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    fn_800C9E60(owner, &m_comId);
}

void ElecSearchLight::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix(lbl_805E0720);

        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        fn_8040B848(&matrix, m_yaw);
        fn_8040B884(&matrix, m_pitch);

        m_beamEnd = matrix * Fn8040Vector3(0.0f, 0.0f, 10.0f * -m_params.lightLength);
        m_model->fn_801633DC(&matrix);
        UpdateCone(&matrix);
    }
}

void ElecSearchLight::OnCollision(ColliDetectArgs &args) {
    UnkHitContext_801543A8 context;

    context.m_charInfo = m_charInfo;
    context.m_params = &m_params;
    fn_8015AA84(&args, fn_80156018, &context);
}

void ElecSearchLight::OnCommand(CharCommand &command) {}

float ElecSearchLight::GetSegmentDistance(const Fn8040Vector3 *point,
                                          const UnkSegment_80020A20 *segment,
                                          Fn8040Vector3 *closest, float *fraction) {
    Fn8040Vector3 direction = segment->end - segment->start;
    Fn8040Vector3 offset = *point - segment->start;

    float projection = fn_8040CF90(&offset, &direction);

    if (projection > 0.0f) {
        float lengthSquared = fn_8040CF90(&direction, &direction);

        if (projection >= lengthSquared) {
            if (closest != 0) {
                *closest = segment->end;
            }
            if (fraction != 0) {
                *fraction = 0.0f;
            }
            return fn_8040D180(point, &segment->end);
        }

        projection /= lengthSquared;

        Fn8040Vector3 nearest = segment->start + direction * projection;
        if (closest != 0) {
            *closest = nearest;
        }
        if (fraction != 0) {
            *fraction = projection;
        }
        return fn_8040D180(point, &nearest);
    }

    if (closest != 0) {
        *closest = segment->start;
    }
    if (fraction != 0) {
        *fraction = 1.0f;
    }
    return fn_8040CF2C(&offset);
}

void ElecSearchLight::Search() {
    UnkSegment_80020A20 beam(m_charInfo->m_position, m_beamEnd);
    boost::shared_ptr<CharInfo> nearest(fn_800A8810(fn_80074194(), beam));

    if (nearest.get() == 0) {
        return;
    }

    Fn8040Vector3 closest;
    float fraction;
    float distanceSquared = GetSegmentDistance(&nearest->m_position, &beam, &closest, &fraction);

    if (fraction < 0.0f || 1.0f < fraction) {
        return;
    }

    float radius = 25.0f * (1.0f - fraction) + 50.0f * fraction;

    if (distanceSquared < radius * radius) {
        m_target = nearest;
        Catch(&closest);
    }
}

void ElecSearchLight::Catch(const Fn8040Vector3 *position) {
    UnkRotation3 angles(0.0f, 0.0f, 0.0f);

    GetBeamAngles(&angles);
    m_target->fn_801DC980(new ClrVelocityCommand(4));
    m_target->fn_801DC980(new Player::StartExternalControlCommand(0, 2));
    m_target->fn_801DC980(new Player::SetMotionCommand(0x26, 0.0f, 2));
    m_target->fn_801DC980(new SetPositionCommand(position, 4));
    m_target->fn_801DC980(new SetAngleCommand(angles, 2, 4));
    m_state = 2;
}

void ElecSearchLight::PullTarget() {
    Fn8040Vector3 toLight = m_charInfo->m_position - m_target->m_position;
    Fn8040Vector3 position = m_target->m_position + fn_8040CEE8(toLight) * 20.0f;
    m_target->fn_801DC980(new SetPositionCommand(&position, 4));
    fn_801D53C4(fn_80053164(), 0x7230, 0, this, 2, 0, 5);

    if (fn_8040CF2C(&toLight) < 2500.0f) {
        m_target->fn_801DC980(new Player::EndExternalControlCommand(4));
        m_target->fn_801DC980(new ClrVelocityCommand(4));
        m_target->fn_801DC980(new Player::HomingJumpCommand(Fn8040Vector3(0.0f, 1.5f, 0.0f), 4));
        m_state = 3;
    }
}

void ElecSearchLight::GetBeamAngles(UnkRotation3 *angles) {
    Fn8040Matrix34 matrix(lbl_805E0720);

    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    fn_8040B848(&matrix, m_yaw);
    fn_8040B884(&matrix, m_pitch);
    fn_8040B848(&matrix, lbl_805FAD38);
    fn_800494F8(angles, &matrix);
}

void ElecSearchLight::Sweep() {
    m_timer += lbl_8057D6A0.field_0x28;

    m_yaw = std::sin(DegreesToRadians(m_params.rotateSpeed) * m_timer) *
            DegreesToRadians(m_params.rotateRange);
}

void ElecSearchLight::StartBreakEffect() {
    if (m_model != 0) {
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(4);

        if (entry != 0) {
            UnkResourceManager_800122D4 *resources = fn_800122D4();
            UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);

            parameters.field_0x34 = 1.3f;
            parameters.field_0x44 = static_cast<RpClump *>(fn_80012700(resources, 1, entry->name));
            parameters.field_0x54 = 0;
            parameters.field_0x4C = 150;
            fn_8015D93C(&parameters);
        }
    }

    fn_801D560C(fn_80053164(), 0x7231, 0, 0);
}

void ElecSearchLight::TurnToTarget(float dt) {
    Fn8040Vector3 direction = m_charInfo->m_position - m_target->m_position;
    UnkRotation3 angles = lbl_805E07B8;
    fn_8003B9E8(&direction, &angles.x, &angles.y);

    Fn8040Matrix34 matrix(lbl_805E0720);

    fn_8040B80C(&matrix, -m_charInfo->m_rotation.z);
    fn_8040B884(&matrix, -m_charInfo->m_rotation.x);
    fn_8040B848(&matrix, -m_charInfo->m_rotation.y);
    fn_8040B848(&matrix, angles.y);
    fn_8040B884(&matrix, angles.x);
    fn_800494F8(&angles, &matrix);

    m_pitch = fn_8012773C(angles.x, m_pitch, s_turnSpeed * dt);
    m_yaw = fn_8012773C(angles.y, m_yaw, s_turnSpeed * dt);
    if (m_pitch == angles.x && m_yaw == angles.y) {
        Catch(&m_target->m_position);
    }
}

void ElecSearchLight::UpdateCone(const Fn8040Matrix34 *matrix) {
    float angleStep = 2.0f * lbl_805FAD38 / 12.0f;
    Fn8040Matrix34 rotation(*matrix);

    for (int i = 0; i < 12; i++) {
        Fn8040Vector3 point;
        Fn8040Vector3 normal;
        point = rotation * Fn8040Vector3(0.0f, 25.0f, -40.0f);
        RwIm3DVertexSetPos(&m_cone[i * 2], point.x, point.y, point.z);

        point = rotation * Fn8040Vector3(0.0f, 50.0f, 10.0f * -m_params.lightLength);
        RwIm3DVertexSetPos(&m_cone[i * 2 + 1], point.x, point.y, point.z);

        normal = fn_8040B618(rotation, Fn8040Vector3(0.0f, 1.0f, 0.0f));
        RwIm3DVertexSetNormal(&m_cone[i * 2], normal.x, normal.y, normal.z);
        RwIm3DVertexSetNormal(&m_cone[i * 2 + 1], normal.x, normal.y, normal.z);

        /* The last pair repeats the first one and closes the strip. */
        if (i == 0) {
            point = rotation * Fn8040Vector3(0.0f, 25.0f, -40.0f);
            RwIm3DVertexSetPos(&m_cone[24], point.x, point.y, point.z);

            point = rotation * Fn8040Vector3(0.0f, 50.0f, 10.0f * -m_params.lightLength);
            RwIm3DVertexSetPos(&m_cone[25], point.x, point.y, point.z);

            RwIm3DVertexSetNormal(&m_cone[24], normal.x, normal.y, normal.z);
            RwIm3DVertexSetNormal(&m_cone[25], normal.x, normal.y, normal.z);
        }

        fn_8040B80C(&rotation, angleStep);
    }

    RwUInt8 nearRed = 128.0f * m_brightness;
    RwUInt8 nearGreen = 96.0f * m_brightness;
    RwUInt8 nearBlue = 64.0f * m_brightness;
    RwUInt8 nearAlpha = 32.0f * m_brightness;
    RwUInt8 farRed = 96.0f * m_brightness;
    RwUInt8 farGreen = 64.0f * m_brightness;
    RwUInt8 farBlue = 48.0f * m_brightness;
    RwUInt8 farAlpha = 48.0f * m_brightness;

    for (int i = 0; i < 12; i++) {
        RwIm3DVertexSetRGBA(&m_cone[i * 2], nearRed, nearGreen, nearBlue, nearAlpha);
        RwIm3DVertexSetRGBA(&m_cone[i * 2 + 1], farRed, farGreen, farBlue, farAlpha);

        if (i == 0) {
            RwIm3DVertexSetRGBA(&m_cone[24], nearRed, nearGreen, nearBlue, nearAlpha);
            RwIm3DVertexSetRGBA(&m_cone[25], farRed, farGreen, farBlue, farAlpha);
        }
    }
}

void ElecSearchLight::OnDraw(RednerDrawArgs &args) {
    RwBool zWrite;
    RwBool fog;
    RwInt32 cullMode;

    RwRenderStateGet(rwRENDERSTATEZWRITEENABLE, &zWrite);
    RwRenderStateGet(rwRENDERSTATEFOGENABLE, &fog);
    RwRenderStateGet(rwRENDERSTATECULLMODE, &cullMode);
    RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);

    RwMatrix matrix;

    RwMatrixSetIdentity(&matrix);
    if (fn_8049A82C(m_cone, 26, &matrix, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA)) {
        fn_8049AAF8(rwPRIMTYPETRISTRIP);
        fn_8049A90C();
    }

    RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)cullMode);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)zWrite);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)fog);
}

extern "C" void fn_801534FC(void) {
    fn_8006B474("LIGHT.BIN", 1);
}

extern "C" void fn_801534F8(void) {}

void ElecSearchLight::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecSearchLight(fn_801EB304(fn_8001C4F8(), 11), owner);
}

/* The names of the values of the "Type" parameter. */
static const char *s_typeLabels[2] = {"NORMAL", "ENEMY_ID"};

/* The label callback of the descriptor's "Type" entry (behaviour inferred). */
static const char *fn_80153478(UnkParamCursor_800CBC20 *cursor) {
    switch (cursor->m_index) {
    case 0:
        return s_typeLabels[cursor->m_value];
    }

    return 0;
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "Type", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_80153478, 0},
    {4, "RotateRange", 0, 0, 0, 0.0f, -65536.0f, 65536.0f, fn_800CBC20, 0},
    {4, "RotateSpeed", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "LightLength", 0, 0, 0, 100.0f, 40.0f, 1000.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8052A8BC = {
    "ELEC_LIGHT", fn_801534FC, fn_801534F8, ElecSearchLight::Create, 0, 0, 0x7D1, 1, 0,
    10,           s_paramInfo,
};

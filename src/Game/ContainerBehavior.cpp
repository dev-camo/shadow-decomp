/* ContainerBehavior (class described in Game/ContainerBehavior.h). File name inferred from the
 * class. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
#include "Game/ContainerBehavior.h"
#include "Game/CharInfo.h"
#include "Game/CharaColliBase.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkIdList_8003A154.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

/* The half extents of the box probe that the constructor creates (name inferred from that call). */
static Fn8040Vector3 s_probeSize(10.0f, 10.0f, 10.0f);

ContainerBehavior::ContainerBehavior(const Fn8040Vector3 *position,
                                     boost::shared_ptr<GadgetModelInfo> model) {
    m_state.m_position = *position;
    m_state.field_0x18 = -1.2f;
    m_state.field_0x1C = 0.0f;
    m_model = model;
    field_0xC = fn_8003A154(&m_state.m_position);

    {
        /* The original copies the static size into a local before the call and releases the
         * probe handle before the stores below. */
        Fn8040Vector3 probeSize = s_probeSize;
        boost::shared_ptr<CharaColliBase> probe = fn_800C7D04(
            fn_80021B08(), &m_state.m_position, probeSize, m_state.m_rotation, 2, 0x80002, 10, 0,
            boost::weak_ptr<CharInfo>(lbl_805EE698), lbl_805E07A0, &lbl_805E07B8, 0);
        field_0x14 = fn_8015B8C4(probe);
    }

    field_0x40 = 1;
    m_flags = 0;
}

ContainerBehavior::~ContainerBehavior() {}

void ContainerBehavior::Update() {
    if (field_0xC.get() != 0 && fn_80039EDC(field_0xC)) {
        return;
    }

    if (field_0x40 != 0) {
        m_state.m_position.y += m_state.field_0x18;

        /* The segment runs from 5 above the new position to 100 below it. */
        Fn8040Vector3 start(m_state.m_position);
        Fn8040Vector3 end(start);
        start.y += 5.0f;
        end.y -= 100.0f;

        int result = ProbeGround(start, end, &m_state.field_0x1C);
        if (result == 1) {
            if (m_state.m_position.y < m_state.field_0x1C) {
                m_state.m_position.y = m_state.field_0x1C;
            }
        } else if (result == 2) {
            if (m_state.m_position.y - m_state.field_0x1C < 5.0f) {
                m_flags |= 1;
            }
            if (m_state.m_position.y < m_state.field_0x1C) {
                m_state.m_position.y = m_state.field_0x1C;
            }
            field_0x40 = 0;
        }
    } else {
        m_state.m_position.y += m_state.field_0x18;
        if (m_state.m_position.y < m_state.field_0x1C) {
            m_state.m_position.y = m_state.field_0x1C;
            if ((m_flags & 1) == 0) {
                m_flags |= 1;
                fn_801D54F4(fn_80053164(), 0x1003, &m_state.m_position, 0, 1);
            }
        }
    }

    if (field_0x14.get() != 0) {
        field_0x14->m_probe->SetPosition(&m_state.m_position);
        field_0x14->m_probe->SetRotation(m_state.m_rotation);

        /* Half the height above the ground. */
        float halfHeight = 0.5f * (m_state.m_position.y - m_state.field_0x1C);
        if (halfHeight > 0.0f) {
            field_0x14->m_probe->SetSize(10.0f, -2, halfHeight, 10.0f);
        }

        Fn8040Vector3 offset(0.0f, -halfHeight, 0.0f);
        field_0x14->m_probe->UnkSlot17(&offset, -2);
    }
}

void ContainerBehavior::Reset(const Fn8040Vector3 *position, const UnkRotation3 *rotation) {
    SetTransform(position, rotation);
    field_0x40 = 1;
    m_flags = 0;
}

/* The probe predicate of ProbeGround: skips the collision object that the probe passes as the
 * context, the model's (name inferred). It has no null test, so a null target is skipped too
 * when the context is null. */
static bool IsNotModelColli(const UnkProbeFilterRecord *record) {
    return record->target != record->context;
}

int ContainerBehavior::ProbeGround(const Fn8040Vector3 &start, const Fn8040Vector3 &end,
                                   float *groundY) {
    boost::shared_ptr<GadgetModelInfo> model = m_model.lock();

    if (model.get() != 0) {
        UnkSegment_80020A20 segment(start, end);
        UnkProbeNode_80021FE4 *node = fn_80021E24(fn_80020D40(), &segment, 3, 8, IsNotModelColli,
                                                  model->fn_80161614(0).lock().get());

        if (node != 0) {
            UnkProbeHit_8001E9BC *hit = node->hit;

            if (hit != 0) {
                bool flag = node->fn_800F8E88();
                Fn8040Vector3 position = *fn_8001E9BC(hit);

                delete node;
                *groundY = position.y;

                if (flag) {
                    return 2;
                }
                return 1;
            }
        }
    }
    return 0;
}

/* EffectObjectRW3 methods; class identity comes from RTTI, original filename unknown. */
#include "Game/EffectObjectRW3.h"
#include "Game/EffectLaunchInfoRW3.h"
#include "rw/rpworld.h"

namespace Effect {
namespace RW3 {

EffectObjectRW3::EffectObjectRW3(System::UnkEffectElementData4 *definition,
                                 System::EffectLaunchInfo *info, System::EffectManager *manager)
    : System::EffectObject(definition, info, manager), m_clump(0), field_0x128(0), m_world(0),
      m_inWorld(0) {
    m_size -= sizeof(System::EffectObject);
    m_size += sizeof(EffectObjectRW3);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    EffectLaunchInfoRW3 *rwInfo = dynamic_cast<EffectLaunchInfoRW3 *>(info);
    if (rwInfo != 0) {
        if (rwInfo->field_0xC != 0) {
            m_clump = fn_80457090(rwInfo->field_0xC);
        }
        if (m_clump == 0) {
            m_flags &= ~System::ELEMENT_ACTIVE;
            return;
        }
    }
}

EffectObjectRW3::~EffectObjectRW3() {
    if (m_clump != 0) {
        RpWorld *world = fn_8046256C(m_clump);
        if (world != 0) {
            fn_804623EC(world, m_clump);
        }
        fn_80457468(m_clump);
        m_clump = 0;
    }
}

void EffectObjectRW3::Update(float deltaTime) {
    System::EffectObject::Update(deltaTime);
    if ((m_flags & System::ELEMENT_ACTIVE) != 0 && (m_flags & System::ELEMENT_FLAG_4) == 0) {
        if (m_clump == 0) {
            m_flags &= ~System::ELEMENT_ACTIVE;
            return;
        }
        RwFrame *frame = static_cast<RwFrame *>(m_clump->object.parent);
        if (frame != 0) {
            RwMatrix *matrix = &frame->modelling;
            /* The original also tests this member address after checking the frame. */
            if (matrix != 0) {
                matrix->right.x = m_transform.values[0][0];
                matrix->right.y = m_transform.values[1][0];
                matrix->right.z = m_transform.values[2][0];
                matrix->up.x = m_transform.values[0][1];
                matrix->up.y = m_transform.values[1][1];
                matrix->up.z = m_transform.values[2][1];
                matrix->at.x = m_transform.values[0][2];
                matrix->at.y = m_transform.values[1][2];
                matrix->at.z = m_transform.values[2][2];
                matrix->pos.x = m_transform.values[0][3];
                matrix->pos.y = m_transform.values[1][3];
                matrix->pos.z = m_transform.values[2][3];
                matrix->flags &= ~(rwMATRIXTYPEORTHONORMAL | rwMATRIXINTERNALIDENTITY);
                fn_8048722C(frame);
            }
        }
    }
}

void EffectObjectRW3::Render() {
    if ((m_flags & System::ELEMENT_ACTIVE) == 0 || (m_flags & System::ELEMENT_FLAG_8) != 0) {
        if (m_inWorld != 0) {
            RpWorld *world = fn_8046256C(m_clump);
            if (world != 0) {
                fn_804623EC(world, m_clump);
            }
            m_inWorld = 0;
        }
    } else {
        if (m_world != 0 && m_inWorld == 0) {
            fn_804622FC(m_world, m_clump);
            m_inWorld = 1;
        }
        if (m_inWorld == 0 && m_clump != 0) {
            fn_80456BC4(m_clump);
        }
    }
}

} // namespace RW3
} // namespace Effect

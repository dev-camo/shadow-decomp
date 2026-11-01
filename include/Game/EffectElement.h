#ifndef GAME_EFFECTELEMENT_H
#define GAME_EFFECTELEMENT_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

namespace Effect {
namespace System {

class EffectManager;
class EffectLaunchInfo;
struct UnkEffectElementData;

/* Flag names are inferred from the update and stop paths. */
enum {
    ELEMENT_ACTIVE = 1,
    ELEMENT_FLAG_2 = 2,
    ELEMENT_FLAG_4 = 4,
    ELEMENT_FLAG_8 = 8,
    ELEMENT_STOP_REQUESTED = 0x10
};

/* Class name from RTTI. Method and member names are inferred where not placeholders. */
class EffectElement {
  public:
    virtual ~EffectElement();
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;

    /* Matching workaround: inline accessors keep shared_ptr::operator-> out of line, as
     * in the original. Direct field access expands it in place and removes those calls.
     * Accessor names are inferred; revisit when the original interface is identified. */
    bool HasFlags(u32 mask) const {
        return (m_flags & mask) != 0;
    }
    void SetFlags(u32 mask) {
        m_flags |= mask;
    }
    void ClearFlags(u32 mask) {
        m_flags &= ~mask;
    }
    void RequestStop() {
        m_flags |= ELEMENT_STOP_REQUESTED;
        m_matrixSource = 0;
        m_translationSource = 0;
        m_rotationSource = 0;
    }
    void SetTimeScale(float scale) {
        m_timeScale = scale;
    }
    void SetMatrixSource(const Fn8040Matrix34 *matrix) {
        m_matrixSource = matrix;
    }

    /* 0x04 */ unsigned char field_0x4;
    /* 0x05 */ unsigned char field_0x5;
    /* 0x08 */ unsigned int m_size;
    /* 0x0C */ float m_elapsed;
    /* 0x10 */ float m_timeScale;
    /* 0x14 */ const float *m_timeScaleSource;
    /* 0x18 */ u32 m_flags;
    /* 0x1C */ EffectManager *m_manager;
    /* 0x20 */ boost::shared_ptr<EffectElement> m_updatePrev;
    /* 0x28 */ boost::shared_ptr<EffectElement> m_updateNext;
    /* 0x30 */ int field_0x30;
    /* 0x34 */ boost::shared_ptr<EffectElement> m_renderPrev;
    /* 0x3C */ boost::shared_ptr<EffectElement> m_renderNext;
    /* 0x44 */ const UnkEffectElementData *m_definition;
    /* 0x48 */ const Fn8040Matrix34 *m_matrixSource;
    /* 0x4C */ const Fn8040Vector3 *m_translationSource;
    /* 0x50 */ const Fn8040Vector3 *m_rotationSource;
    /* 0x54 */ Fn8040Matrix34 m_transform;
    /* 0x84 */ Fn8040Matrix34 m_baseTransform;
    /* 0xB4 */ Fn8040Vector3 m_previousTranslation;
    /* 0xC0 */ Fn8040Vector3 m_translationVelocity;
    /* 0xCC */ Fn8040Vector3 m_translation;
    /* 0xD8 */ Fn8040Vector3 m_rotation;
    /* 0xE4 */ Fn8040Vector3 m_scale;
    /* 0xF0 */ Fn8040Vector3 m_velocity;
};
CHECK_SIZEOF(EffectElement, 0xFC);

} // namespace System
} // namespace Effect

extern "C" {
/* Creates an effect at position (the other arguments are not understood). */
boost::weak_ptr<Effect::System::EffectElement> fn_80168C48(const Fn8040Vector3 *position,
                                                           const Fn8040Vector3 *arg1, int arg2);
/* These helpers write the base transform (names remain unknown). */
void fn_80425E4C(Effect::System::EffectElement *effect, const Fn8040Vector3 *position);
void fn_80425E68(Effect::System::EffectElement *effect, const Fn8040Matrix34 *matrix);
void fn_80425D94(Effect::System::EffectElement *effect, const Fn8040Vector3 *rotation);
}

#endif

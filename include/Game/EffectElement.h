#ifndef GAME_EFFECTELEMENT_H
#define GAME_EFFECTELEMENT_H

#include "Game/fn_8040_vector_types.h"
#include "Game/EffectSystem.h"
#include "Game/fn_8042A444.h"
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
    EffectElement(const UnkEffectElementData *definition, EffectLaunchInfo *info,
                  EffectManager *manager);
    virtual ~EffectElement();
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;

    /* Inferred: each new expression in EffectManagerRW3::CreateElement allocates through
     * fn_8042A5D0()->Allocate, and its cleanup uses EffectElement::operator delete. */
    static void *operator new(unsigned long size) {
        return fn_8042A5D0()->Allocate(size);
    }

    static void operator delete(void *block) {
        fn_8042A5D0()->Free(block);
    }

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
    void SetTranslationSource(const Fn8040Vector3 *translation) {
        m_translationSource = translation;
    }
    void SetRotationSource(const Fn8040Vector3 *rotation) {
        m_rotationSource = rotation;
    }
    void SetTimeScaleSource(const float *source) {
        m_timeScaleSource = source;
    }

    /* 0x04 */ unsigned char field_0x4;
    /* 0x05 */ signed char field_0x5;
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
void fn_80425C34(Effect::System::EffectElement *effect, float deltaTime);
/* Creates effect id from the global launch info at position, applies rotation derived
 * from the negated direction, and clears ELEMENT_FLAG_4 (behaviour inferred). */
boost::weak_ptr<Effect::System::EffectElement> fn_80168C48(const Fn8040Vector3 *position,
                                                           const Fn8040Vector3 *direction, int id);
/* Creates effect id from the global launch info with matrix as its base transform (fn_80425E68),
 * and clears ELEMENT_FLAG_4 when the effect exists (behaviour inferred). The const reference is
 * inferred: the BK_TANK gadget passes the matrix that BKTank::SetBKTank::GetNodeMatrix returns by
 * value in place at its eight calls (four in Update, four in OnAttack). A pointer parameter needs
 * a name for that result: a named matrix adds a copy of it at each call (three instructions more),
 * and a named reference to it sets up the two arguments in the other order (two words differ at
 * each call) (both measured). */
boost::weak_ptr<Effect::System::EffectElement> fn_80168A50(const Fn8040Matrix34 &matrix, int id);
/* Creates effect id from the global launch info at position with rotation, and clears
 * ELEMENT_FLAG_4 (behaviour inferred). */
boost::weak_ptr<Effect::System::EffectElement> fn_80168B38(const Fn8040Vector3 *position,
                                                           const UnkRotation3 *rotation, int id);
/* These helpers write the base transform (names remain unknown). The const reference of
 * fn_80425D94 is inferred from fn_8030536C: the vector it passes (0x80305448, USA (GUPE8P)) is at
 * r1+0x30, below the vector fn_80016284 returned at r1+0x3C. A temporary constructed in the call
 * gets that slot and a named local the slot above it (both measured). A pointer parameter cannot
 * take the temporary, and fn_80425C34 passes the stored rotation without a copy, which a by-value
 * parameter would make. The const reference of fn_80425E68 is inferred from BKTank::SetBKTank's
 * Update, which passes the matrix that GetNodeMatrix returns by value in place: the effect pointer
 * it tests is loaded before that call and reused as the first argument after it (0x8031FBE8-
 * 0x8031FC08). A pointer parameter needs a name for that result: a named matrix adds a copy of it
 * at each of Update's six calls (three instructions more), and a named reference to it reloads the
 * effect pointer after the call (three words differ at each call) (both measured). */
void fn_80425E4C(Effect::System::EffectElement *effect, const Fn8040Vector3 *position);
void fn_80425E68(Effect::System::EffectElement *effect, const Fn8040Matrix34 &matrix);
void fn_80425D94(Effect::System::EffectElement *effect, const Fn8040Vector3 &rotation);
/* Passes the word for arg0 of the table at 0x80581394 (8-byte entries) to fn_803054D0 with the
 * other arguments (behaviour inferred from its body; the GoalRing gadget creates its colour effect
 * with it, passing its position, its rotation and an effect id). */
boost::weak_ptr<Effect::System::EffectElement> fn_801D2B38(int arg0, const Fn8040Vector3 *position,
                                                           const UnkRotation3 *rotation, int id,
                                                           int arg4);
/* Passes the word for arg0 of the table at 0x80581394 (8-byte entries) to fn_80305900 with the
 * other arguments and returns the effect it creates (behaviour inferred from its body; timeScale
 * is spelled as fn_80305900 takes it). */
boost::weak_ptr<Effect::System::EffectElement> fn_801D2C28(int arg0, const Fn8040Matrix34 *matrix,
                                                           int id, const float *timeScale);
/* Creates an effect (behaviour inferred). EnemyDispCommon's fn_801A1F54 passes 0, a position, a
 * second vector, an id and the address 0x24 bytes into lbl_8057D6A0; the EggLastMech boss passes
 * the same address through its character info's field_0x60. */
boost::weak_ptr<Effect::System::EffectElement> fn_801D2AFC(int arg0, const Fn8040Vector3 *position,
                                                           const Fn8040Vector3 *vector, int id,
                                                           const float *arg4);
/* Passes position, direction and id (with 0 before position and after id) to fn_801D2AFC and
 * returns the effect it creates (behaviour inferred). */
boost::weak_ptr<Effect::System::EffectElement> fn_8015D7DC(const Fn8040Vector3 *position,
                                                           const Fn8040Vector3 *direction, int id);
/* Calls fn_8015D7DC with the direction (0, 1, 0) (behaviour inferred; the GoalRing gadget
 * stores the result as an effect). */
boost::weak_ptr<Effect::System::EffectElement> fn_8015D794(const Fn8040Vector3 *position, int id);
/* Passes 0, matrix, id and 0 to fn_801D2B74 and returns the effect it creates (behaviour and
 * spellings inferred from its body; the SECRETDOOR gadget passes a frame's world matrix). */
boost::weak_ptr<Effect::System::EffectElement> fn_8015D754(const Fn8040Matrix34 *matrix, int id);
}

#endif

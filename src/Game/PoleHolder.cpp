/* PoleHolder (class name from RTTI; the gadget's descriptor is "STONE_POLE"): a stone pole with
 * five stones that falls when the character nearest to it (in three dimensions) comes into its
 * range, or when an attack whose strength has a magnitude of at least 0.0001 hits it (behaviour
 * inferred). The pole is a StonePoleFlat, StonePoleSlope or StonePoleAir (StonePole), and its
 * stones are FallingStoneSoft or FallingStoneHard (FallingStone); all class names come from RTTI.
 * File name inferred from the gadget class. Method names marked "(name inferred)" in the header
 * are not original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
/* Matching workaround: TNumEdit.h comes before CharaColliBaseCharInfo.h, in the order their names
 * appear in the unit's .rodata (alphabetical order would reverse them). Revisit when the original
 * inline functions behind that data are found (see TNumEdit.h). */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/PoleHolder.h"
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SharedConstants.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/fn_804157E8_motion_range.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8000A950_near_zero.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;

/* The radius that FallingStone::UpdatePosition divides a slope stone's rolled distance by, and
 * the height whose half StonePole::UpdateModelMatrix puts the collision object above the pole's
 * position (names inferred from their uses).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of these
 * variables. The original's start-up function (0x800DC374) stores 30 to both of them (a constant
 * float initialiser would be emitted as initialised .sdata data instead), and the shared stand-in
 * reproduces those stores. Revisit when the original type is found. */
static UnkStartupFloat_800E2570 s_rollRadius(30.0f);
static UnkStartupFloat_800E2570 s_collisionHeight(30.0f);

/* Each is copied from a temporary, as the original's start-up function does. */
Fn8040Vector3 StonePoleFlat::s_rollVelocity = Fn8040Vector3(0.0f, 0.0f, 0.0f);
Fn8040Vector3 StonePoleSlope::s_rollVelocity = Fn8040Vector3(0.0f, -14.101201f, 68.6502f);
Fn8040Vector3 StonePoleAir::s_rollVelocity = Fn8040Vector3(0.0f, -707.59344f, 0.0f);

FallingStone::FallingStone(UnkGadgetOwner_800C9CD0 *owner, StonePole *pole, u32 index)
    : m_index(index), m_pole(pole), m_owner(owner), m_rolledDistance(0.0f) {
    m_charInfo = fn_8015CD90(0x2E, 4);
    m_charInfo->field_0x30 = 1;
    m_model = fn_8015F800(1, "STONEPOLE.BIN");
}

FallingStone::~FallingStone() {}

void FallingStone::Update(s32 poleState, float dt) {
    UpdatePosition(poleState, dt);
    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
}

void FallingStone::ResetFromOwner() {
    UpdatePosition(0, 0.0f);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    m_model->fn_8016108C();
}

void FallingStone::UpdatePosition(s32 poleState, float dt) {
    if (poleState != 3) {
        boost::shared_ptr<_stMtnPlayWork> motion = m_pole->m_motion.lock();

        if (motion.get() != 0) {
            Fn8040Matrix34 matrix;
            MotionTransform *transform = fn_80415E70(motion.get(), m_pole->GetStoneNode(m_index));

            fn_80016370(&matrix, &transform->matrix);
            fn_80049690(&m_charInfo->m_position, &matrix);
            fn_800493C4(&m_charInfo->m_rotation, &matrix);
        }
    } else if (m_pole->m_holder->GetParams()->type != 0) {
        Fn8040Vector3 move = *(m_pole->GetRollVelocity());
        fn_8040CE38(&move, dt);
        move = fn_8040D0AC(move, m_pole->m_charInfo->m_rotation.y);
        AddVector(&m_charInfo->m_position, move);

        float distance = fn_8040CF50(move);

        m_rolledDistance += distance;

        if (m_pole->m_holder->GetParams()->type == 1) {
            float angle = 2.0f * (distance / s_rollRadius.value);

            if (m_pole->m_holder->GetParams()->fall != 0) {
                m_charInfo->m_rotation.y += angle;
            } else {
                m_charInfo->m_rotation.y -= angle;
            }
        }

        if (m_rolledDistance > m_pole->m_holder->GetParams()->rollingDistance) {
            if (m_pole->m_holder->GetParams()->type == 1) {
                Break();
            }
            m_pole->QueueStoneRemoval(m_index);
        }
    }
}

void FallingStone::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, m_charInfo->m_position);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        m_model->fn_801633DC(&matrix);
        UpdateAttachments();
    }
}

FallingStoneHard::FallingStoneHard(UnkGadgetOwner_800C9CD0 *owner, StonePole *pole, u32 index)
    : FallingStone(owner, pole, index), m_health(2.0f) {
    m_model->fn_801629D8("StoneHard", 0);
    m_model->fn_80161F08("StoneColli", boost::weak_ptr<CharInfo>(m_charInfo));

    m_object = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x2800);
    m_object->ClearFlags(1);

    m_attack = m_model->fn_80161334("StoneColli");

    boost::shared_ptr<CharaColliAttack> attack(m_attack.lock());

    if (attack.get() != 0) {
        attack->field_0x54 = 0;
        attack->field_0x34 = 1.0f;
        attack->field_0x38 = 4;
        attack->field_0x40 = 1;
        attack->field_0x10.field_0x0 =
            AttackCallback(boost::bind(&FallingStoneHard::SetPushDirection, this, _1));
        attack->field_0x28.field_0x0 =
            AttackCallback(boost::bind(&FallingStoneHard::OnHit, this, _1));
        attack->field_0x55 = 1;
        attack->field_0x3C = 0;
        attack->field_0x1C.field_0x0 =
            AttackCallback(boost::bind(&FallingStoneHard::OnAttack, this, _1));
    }
}

FallingStoneHard::~FallingStoneHard() {
    m_sound.fn_801D455C();
}

void FallingStoneHard::UpdateAttachments() {
    if (m_object.get() != 0) {
        fn_800CCE38(m_object.get(), m_charInfo->m_position);
    }

    if (m_pole->m_holder->GetParams()->type == 1 && m_sound.field_0x0 != 0) {
        m_sound.fn_800CCE14(&m_charInfo->m_position);
    }
}

void FallingStoneHard::Break() {
    UnkEffectParams_8015F0B8 breakEffect(m_charInfo, 0);

    breakEffect.field_0x44 =
        static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "CN0000ROLLOBJECTHEADBRK.DFF"));
    breakEffect.field_0x34 = 1.0f;
    breakEffect.field_0x54 = 1;

    if (m_pole->m_holder->GetParams()->type == 0) {
        breakEffect.field_0x4A = 1;
        breakEffect.field_0x40 = m_pole->m_charInfo->m_position.y;
    } else {
        breakEffect.field_0x4A = 0;
    }

    fn_8015D93C(&breakEffect);
    fn_801D54F4(fn_80053164(), 0x7332, &m_charInfo->m_position, 0, 1);
}

void FallingStoneHard::OnFallStart() {
    boost::shared_ptr<CharaColliAttack> attack(m_attack.lock());

    if (attack.get() != 0) {
        attack->field_0x54 = 1;
    }

    if (m_object.get() != 0) {
        m_object->SetFlags(1);
    }
}

void FallingStoneHard::OnFallEnd() {
    if (m_pole->m_holder->GetParams()->type == 1) {
        m_sound = fn_801D3DC8(0x7334, &m_charInfo->m_position, this);
    }
}

void FallingStoneHard::OnStop() {
    boost::shared_ptr<CharaColliAttack> attack(m_attack.lock());

    if (attack.get() != 0) {
        attack->field_0x54 = 0;
    }
}

void FallingStoneHard::SetPushDirection(AttackCallbackParam &param) {
    param.field_0x34 = *(m_pole->GetRollVelocity());
    fn_8040CE38(&param.field_0x34, 1.0f);
    param.field_0x4C = fn_8040CF50(param.field_0x34);

    param.field_0x34 = fn_8040D0AC(param.field_0x34, m_pole->m_charInfo->m_rotation.y);
    fn_8040CE74(&param.field_0x34);
    param.field_0x50 = 1.0f;
}

void FallingStoneHard::OnHit(AttackCallbackParam &param) {
    if ((param.field_0x54 & 1) != 0 && !m_pole->IsStoneRemovalQueued(m_index)) {
        Break();
        m_pole->QueueStoneRemoval(m_index);
    }
}

void FallingStoneHard::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 1;

    if (m_pole->m_state == 0 && !fn_8000B044(param.field_0x50)) {
        m_pole->Fall();
    }

    m_health -= param.field_0x50;

    if (m_health < 0.0001f) {
        param.field_0x54 = 3;

        if (!m_pole->IsStoneRemovalQueued(m_index)) {
            /* The attacker's pointer is released before Break(), as in the original. */
            {
                boost::shared_ptr<CharInfo> attacker(fn_8015D570(&param));

                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x40);
                }
            }

            Break();
            m_pole->QueueStoneRemoval(m_index);
        }
    }
}

FallingStoneSoft::FallingStoneSoft(UnkGadgetOwner_800C9CD0 *owner, StonePole *pole, u32 index)
    : FallingStone(owner, pole, index) {
    m_model->fn_801629D8("StoneSoft", 0);
    m_model->fn_80161F08("StoneColli", boost::weak_ptr<CharInfo>(m_charInfo));
    m_attack = m_model->fn_80161334("StoneColli");

    boost::shared_ptr<CharaColliAttack> attack(m_attack.lock());

    if (attack.get() != 0) {
        attack->field_0x54 = 0;
        attack->field_0x55 = 1;
        attack->field_0x1C.field_0x0 =
            AttackCallback(boost::bind(&FallingStoneSoft::OnAttack, this, _1));
    }
}

FallingStoneSoft::~FallingStoneSoft() {}

void FallingStoneSoft::UpdateAttachments() {}

void FallingStoneSoft::Break() {
    UnkEffectParams_8015F0B8 breakEffect(m_charInfo, 0);

    breakEffect.field_0x44 =
        static_cast<RpClump *>(fn_80012700(fn_800122D4(), 1, "CN0000ROLLOBJECTNMLBRK.DFF"));
    breakEffect.field_0x34 = 1.0f;
    breakEffect.field_0x54 = 1;
    breakEffect.field_0x4A = 0;
    fn_8015D93C(&breakEffect);
}

void FallingStoneSoft::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 1;

    if (m_pole->m_state == 0 && !fn_8000B044(param.field_0x50)) {
        m_pole->Fall();
    }
}

boost::shared_ptr<FallingStone> FallingStone::Create(UnkGadgetOwner_800C9CD0 *owner,
                                                     StonePole *pole, u32 index, int kind) {
    if (kind == 1) {
        return boost::shared_ptr<FallingStone>(new FallingStoneHard(owner, pole, index));
    }
    return boost::shared_ptr<FallingStone>(new FallingStoneSoft(owner, pole, index));
}

StonePole::StonePole(PoleHolder *holder, UnkGadgetOwner_800C9CD0 *owner)
    : m_soundPlayed(false), m_holder(holder), m_owner(owner), m_state(0), m_softStonesBroken(false),
      m_motionFlagSet(false) {
    m_stones.clear();
    m_removalQueue.clear();
    m_removalQueue.reserve(5);

    m_charInfo = fn_8015CD90(0x2D, 4);
    m_charInfo->field_0x30 = 1;

    m_model = fn_8015F800(1, "STONEPOLE.BIN");
    m_model->fn_80162A68(m_holder->GetParams()->type, 0);
    CreateCollision();
    m_model->fn_80160C28(m_holder->GetParams()->type);
    m_model->fn_80160B18(m_holder->GetParams()->type);
    m_motion = m_model->fn_801602B4(m_holder->GetParams()->type);

    boost::shared_ptr<_stMtnPlayWork> motion = m_motion.lock();

    if (m_holder->GetParams()->fall != 0) {
        stMtnControlWorkFlag(motion.get(), 0, 7, 0);
    }
    stMtnRequestFrameSpeed(motion.get(), 0, 0.0f);

    ReadOwner(owner);
    UpdateModelMatrix();
    CreateStones();
}

StonePole::~StonePole() {
    if (m_state > 0) {
        fn_800C9B34(m_owner);
    }
}

bool StonePole::IsStoneRemovalQueued(u32 index) {
    if (!m_removalQueue.empty()) {
        for (u32 *it = m_removalQueue.begin(); it != m_removalQueue.end(); ++it) {
            if (*it == index) {
                return true;
            }
        }
    }
    return false;
}

void StonePole::QueueStoneRemoval(u32 index) {
    m_removalQueue.push_back(index);
}

FallingStone *StonePole::FindStone(u32 index) {
    for (std::list<boost::shared_ptr<FallingStone> >::iterator it = m_stones.begin();
         it != m_stones.end(); it++) {
        if ((*it)->m_index == index) {
            return it->get();
        }
    }
    return 0;
}

void StonePole::Update(float dt) {
    RemoveQueuedStones();
    m_model->fn_80162FB0(dt);

    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }

    if (m_state == 0) {
        if (!m_motionFlagSet) {
            m_model->fn_80160480(m_holder->GetParams()->type);
            m_motionFlagSet = true;
        }

        if (IsCharacterInRange()) {
            Fall();
        }
    } else {
        if (m_collision.get() != 0) {
            m_collision.reset();
        }

        if (!m_softStonesBroken) {
            MotionRange range;

            fn_80415D40(m_motion.lock().get(), 0, &range);

            if (range.currentFrame > 70.0f) {
                for (int i = 0;
                     i < sizeof(m_holder->m_params.steps) / sizeof(m_holder->m_params.steps[0]);
                     i++) {
                    if (m_holder->GetParams()->steps[i] == 0) {
                        FallingStone *stone = FindStone(i);

                        if (stone != 0) {
                            stone->Break();
                            QueueStoneRemoval(i);
                        }
                    }
                }

                fn_801D54F4(fn_80053164(), 0x7331, &m_charInfo->m_position, 0, 1);
                m_softStonesBroken = true;
            }
        }

        if (m_state == 1) {
            if (stMtnControlWorkFlag(m_motion.lock().get(), 0, 3, 1)) {
                m_state = 3;
                m_model->fn_80160480(m_holder->GetParams()->type);
                m_motionFlagSet = true;
                OnFallEnd();

                for (std::list<boost::shared_ptr<FallingStone> >::iterator it = m_stones.begin();
                     it != m_stones.end(); it++) {
                    (*it)->OnFallEnd();
                }
            }
        }
    }

    for (std::list<boost::shared_ptr<FallingStone> >::iterator it = m_stones.begin();
         it != m_stones.end(); it++) {
        (*it)->Update(m_state, dt);
    }

    UpdateSound(dt);
}

void StonePole::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    if (m_motionFlagSet) {
        m_model->fn_80160384(m_holder->GetParams()->type);
        m_motionFlagSet = false;
    }

    if (m_state > 0) {
        Reset();
        CreateStones();
    }

    ReadOwner(owner);
    UpdateModelMatrix();
    UnkSlot5();
    m_model->fn_80162EF4();

    for (std::list<boost::shared_ptr<FallingStone> >::iterator it = m_stones.begin();
         it != m_stones.end(); it++) {
        (*it)->ResetFromOwner();
    }

    DrawRange();
    m_model->fn_8016108C();

    if (m_collision.get() != 0) {
        m_collision->UnkSlot26();
    }

    UnkSlot4();
}

void StonePole::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void StonePole::Reset() {
    boost::shared_ptr<_stMtnPlayWork> motion = m_motion.lock();

    stMtnRequestFrameSpeed(motion.get(), 0, 0.0f);
    stMtnRequestCurrentFrame(motion.get(), 0, 0.0f);
    CreateCollision();
    UnkSlot6();
    m_state = 0;
    m_softStonesBroken = false;
    m_soundPlayed = false;
}

void StonePole::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    m_model->fn_801633DC(&matrix);

    if (m_collision.get() != 0) {
        Fn8040Vector3 center(0.0f, 0.5f * s_collisionHeight.value, 0.0f);

        AddVector(&center, m_charInfo->m_position);
        m_collision->SetPosition(&center);
        m_collision->SetRotation(m_charInfo->m_rotation);
    }
}

void StonePole::CreateStones() {
    m_stones.clear();

    /* The loop bound is unsigned (a sizeof quotient), here and in StonePole::Update: the original
     * compares unsigned and tests before the first iteration, which an int bound of 5 folds away. */
    for (int i = 0; i < sizeof(m_holder->m_params.steps) / sizeof(m_holder->m_params.steps[0]);
         i++) {
        m_stones.push_back(FallingStone::Create(m_owner, this, i, m_holder->GetParams()->steps[i]));
    }
}

void StonePole::RemoveQueuedStones() {
    if (!m_removalQueue.empty()) {
        for (u32 *it = m_removalQueue.begin(); it != m_removalQueue.end(); ++it) {
            RemoveStone(*it);
        }
        m_removalQueue.clear();
    }
}

void StonePole::RemoveStone(u32 index) {
    for (std::list<boost::shared_ptr<FallingStone> >::iterator it = m_stones.begin();
         it != m_stones.end(); it++) {
        if ((*it)->m_index == index) {
            m_stones.erase(it);
            return;
        }
    }
}

void StonePole::Fall() {
    for (std::list<boost::shared_ptr<FallingStone> >::iterator it = m_stones.begin();
         it != m_stones.end(); it++) {
        (*it)->OnFallStart();
    }

    UnkSlot7();
    fn_801D54F4(fn_80053164(), 0x7335, &m_charInfo->m_position, 0, 1);
    stMtnRequestFrameSpeed(m_motion.lock().get(), 0, 1.0f);
    m_model->fn_80160384(m_holder->GetParams()->type);
    m_motionFlagSet = false;

    m_target = fn_80166AF4();

    {
        UnkWeakHolder_800CC6C8 weakTarget;
        weakTarget.ptr = m_target;
        m_model->fn_8015FA78(fn_80169D00, &weakTarget);
        m_target->fn_80167448(-1);
    }

    m_state = 1;
}

void StonePole::CreateCollision() {
    if (m_collision.get() == 0) {
        void *collision = fn_80012700(fn_800122D4(), 1, "CN0000ROLLOBJECTNML.CCL");

        m_collision =
            fn_800C86A8(fn_80021B08(), collision, &m_charInfo->m_position, &m_charInfo->m_rotation,
                        1, 7, 0, boost::weak_ptr<CharInfo>(lbl_805EE698));

        boost::shared_ptr<CharaColliAttack> attack(fn_800C4374(fn_80021B08(), m_collision).lock());

        if (attack.get() != 0) {
            attack->field_0x54 = 0;
            attack->field_0x55 = 1;
            attack->field_0x1C.field_0x0 =
                AttackCallback(boost::bind(&StonePole::OnAttack, this, _1));
        }
    }
}

void StonePole::OnAttack(AttackCallbackParam &param) {
    param.field_0x54 = 1;

    if (m_state == 0 && !fn_8000B044(param.field_0x50)) {
        Fall();
    }
}

int StonePole::IsCharacterInRange() {
    boost::shared_ptr<CharInfo> character =
        fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();

    if (character.get() == 0) {
        return 0;
    }

    Fn8040Vector3 polePosition = m_charInfo->m_position;
    Fn8040Vector3 characterPosition = character->m_position;
    polePosition.y = 0.0f;
    characterPosition.y = 0.0f;
    Fn8040Vector3 offset = characterPosition - polePosition;

    if (fn_8040CF50(offset) > m_holder->GetParams()->rangeRadius) {
        return 0;
    }

    if (fn_8000A980(character->m_position.y - m_charInfo->m_position.y) >
        m_holder->GetParams()->rangeHeight) {
        return 0;
    }

    return 1;
}

void StonePole::DrawRange() {
    UnkRotation3 rotation(0.5f * lbl_805FAD38, 0.0f, 0.0f);

    fn_802E0C60(m_charInfo->m_position, rotation, 0, m_holder->GetParams()->rangeHeight,
                m_holder->GetParams()->rangeRadius);
}

StonePoleFlat::~StonePoleFlat() {}

void StonePoleFlat::UpdateSound(float dt) {
    if (!m_stones.empty() && !m_soundPlayed) {
        MotionRange range;

        fn_80415D40(m_motion.lock().get(), 0, &range);

        if (range.currentFrame > 65.0f) {
            fn_801D54F4(fn_80053164(), 0x7333, &m_charInfo->m_position, 0, 1);
            m_soundPlayed = true;
        }
    }
}

void StonePoleFlat::UnkSlot4() {}

void StonePoleFlat::UnkSlot5() {}

void StonePoleFlat::UnkSlot6() {}

void StonePoleFlat::UnkSlot7() {}

void StonePoleFlat::OnFallEnd() {
    for (std::list<boost::shared_ptr<FallingStone> >::iterator it = m_stones.begin();
         it != m_stones.end(); it++) {
        (*it)->OnStop();
    }

    m_model->fn_80162790();
}

StonePoleSlope::~StonePoleSlope() {}

void StonePoleSlope::UnkSlot5() {}

void StonePoleSlope::UpdateSound(float dt) {
    if (!m_stones.empty() && !m_soundPlayed) {
        MotionRange range;

        fn_80415D40(m_motion.lock().get(), 0, &range);

        if (range.currentFrame > 80.0f) {
            fn_801D54F4(fn_80053164(), 0x7333, &m_charInfo->m_position, 0, 1);
            m_soundPlayed = true;
        }
    }
}

void StonePoleSlope::OnFallEnd() {
    m_model->fn_80162790();
}

StonePoleAir::~StonePoleAir() {}

void StonePoleAir::UnkSlot5() {}

void StonePoleAir::OnFallEnd() {
    m_model->fn_80162790();
}

/* Matching workaround: the node tables and s_name are defined here, after the functions with bound
 * callbacks, so that the tables and the class name string follow the callbacks' RTTI names in
 * .rodata as in the original (defined at the top of the file, they come first). Revisit if the
 * original file order is found to differ. */
const u32 StonePoleFlat::s_stoneNodes[5] = {1020101, 1020102, 1020103, 1020104, 1020105};
const u32 StonePoleSlope::s_stoneNodes[5] = {1020113, 1020114, 1020115, 1020116, 1020117};
const u32 StonePoleAir::s_stoneNodes[5] = {1040201, 1040202, 1040203, 1040204, 1040205};
const char *PoleHolder::s_name = "PoleHolder";

PoleHolder::PoleHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    ReadParams(owner);
}

PoleHolder::~PoleHolder() {}

extern "C" void fn_800D7E88(void) {
    fn_8006B474("STONEPOLE.BIN", 1);
}

extern "C" void fn_800D7E84(void) {}

void PoleHolder::Create(UnkGadgetOwner_800C9CD0 *owner) {
    PoleHolder *holder = new PoleHolder(fn_801EB304(fn_8001C4F8(), 11), owner);

    holder->CreatePole(owner);
}

void PoleHolder::UpdateBody(float dt) {
    if (m_pole.get() != 0) {
        m_pole->Update(dt);

        if (m_pole->m_stones.empty()) {
            m_flags |= 1;
        }
    }
}

void PoleHolder::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    PoleHolderParams previous = m_params;

    ReadParams(owner);

    if (previous.type != m_params.type || previous.fall != m_params.fall) {
        CreatePole(owner);
    } else {
        for (int i = 0; i < 5; i++) {
            if (previous.steps[i] != m_params.steps[i]) {
                m_pole->CreateStones();
                break;
            }
        }
    }

    m_pole->ResetFromOwner(owner);
}

void PoleHolder::ReadParams(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    m_params.rangeRadius = 10.0f * m_params.rangeRadius;
    m_params.rangeHeight = 0.5f * (10.0f * m_params.rangeHeight);
    m_params.rollingSpeed = 10.0f * m_params.rollingSpeed;
    m_params.rollingDistance = 10.0f * m_params.rollingDistance;
}

void PoleHolder::CreatePole(UnkGadgetOwner_800C9CD0 *owner) {
    if (m_params.type == 0) {
        m_pole = boost::shared_ptr<StonePole>(new StonePoleFlat(this, owner));
    } else if (m_params.type == 1) {
        m_pole = boost::shared_ptr<StonePole>(new StonePoleSlope(this, owner));
    } else {
        m_pole = boost::shared_ptr<StonePole>(new StonePoleAir(this, owner));
    }
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "0:FLAT 1:SLOPE 2:AIR", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {2, "FALL L/R 0:L 1:R", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "RANGE_R m", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "RANGE_HEIGHT m", 0, 0, 0, 15.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "ROLLING_SPD m/sec", 0, 0, 0, 5.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "ROLLING_DISTANCE m", 0, 0, 0, 50.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {2, "STEP_2 0:SOFT 1:HARD", 1, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {2, "STEP_3 0:SOFT 1:HARD", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {2, "STEP_4 0:SOFT 1:HARD", 1, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {2, "STEP_5 0:SOFT 1:HARD", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {2, "STEP_6 0:SOFT 1:HARD", 1, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_8052383C = {
    "STONE_POLE", fn_800D7E88, fn_800D7E84, PoleHolder::Create, 0, 0, 0x837, 1, 0, 20, s_paramInfo,
};

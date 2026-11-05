/* ElecSecurityCode (behaviour in the class comment): the objects that the ELEC_SECURITY gadget
 * (ElecSecurityCodeGenerator.cpp) creates. File name inferred from the class. Definitions are in
 * reverse address order (deferred inlining emits functions last-defined first).
 *
 * The three callbacks of the model's attack object are boost::function objects built with
 * boost::bind(&ElecSecurityCode::..., this, _1); their manager, invoker and copy functions come
 * from the boost headers. */
/* Matching workaround: TNumEdit.h is included only for the RTTI records that its never-called
 * stand-in functions emit (see the note there), and it comes before CharaColliBaseCharInfo.h, in
 * the order their names appear in the unit's .rodata (alphabetical order would reverse them).
 * Revisit with the TNumEdit stand-ins. */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBaseCharInfo.h"
#include "Game/ElecSecurityCode.h"
#include "Game/CharInfo.h"
#include "Game/ElecSecurityCodeGenerator.h"
#include "Game/SharedConstants.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkPartFrameSearch.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8012773C.h"
#include "Game/fn_800E9CEC.h"
#include "Game/fn_8026EE9C.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "rw/rpworld.h"

using SonicteamUSA::System::CharaColli::AttackCallbackParam;
using SonicteamUSA::System::CharaColli::CharaColliAttack;
using SonicteamUSA::System::CharaColli::CharaColliBaseCharInfo;

/* The most a chasing code turns its pitch and its yaw per update, in radians (inferred: the step
 * is passed to fn_8012773C without dt). */
static const float s_turnStep = 0.33333f * lbl_805FAD38;
/* How fast a code spins its character info's yaw in state 0, in radians per second (inferred). */
static const float s_spinSpeed = 6.0f * lbl_805FAD38;

/* The height of the target above the character info for each type (name inferred). */
static const float s_targetHeights[3] = {15.0f, 10.0f, 6.5f};

/* The part callback (placeholder name; ElecFireWall's fn_801F7778 has the same body): stores the
 * frame of the clump of the part whose word at 0x8 equals the searched id and stops the walk once
 * a frame is found (behaviour inferred). */
static UnkModelPart_8015FA78 *fn_80157A6C(UnkModelPart_8015FA78 *part, void *userData) {
    UnkPartFrameSearch *search = static_cast<UnkPartFrameSearch *>(userData);

    if (search != 0 && part->clump != 0 && part->field_0x8 == search->id) {
        search->frame = static_cast<RwFrame *>(part->clump->object.parent);
        if (search->frame != 0) {
            return 0;
        }
    }
    return part;
}

ElecSecurityCode::ElecSecurityCode(s32 type, const Fn8040Vector3 &position,
                                   ElecSecurityCodeGenerator *generator,
                                   const Fn8040Vector2 &rotation, s32 splitSide)
    : m_type(type), m_rotation(rotation), m_splitSide(splitSide) {
    m_time = 0.0f;
    m_state = 0;
    m_scale.x = m_scale.z = 0.0f;
    m_scale.y = 3.0f;
    m_partAngles[0] = m_partAngles[1] = m_partAngles[2] = 0.0f;

    m_charInfo = fn_8015CB88(0x57, 4, "ElecSecurityCode");
    fn_800091D0(&m_charInfo->m_position, &position);
    m_charInfo->m_rotation = Fn8040Vector3(0.0f, 0.0f, 0.0f);

    m_model = new GadgetModelInfo(1, "SECURITY.BIN");
    m_model->fn_80162A68(m_type, 0);
    m_model->fn_80161FC8(m_type, boost::weak_ptr<CharInfo>(m_charInfo));
    m_target = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x10C00);
    m_attack = m_model->fn_80161428(m_type);

    if (boost::shared_ptr<CharaColliAttack> attack = m_attack.lock()) {
        attack->fn_80054DD0(
            1, 1, AttackCallback(boost::bind(&ElecSecurityCode::fn_80156344, this, _1)),
            AttackCallback(boost::bind(&ElecSecurityCode::fn_80156340, this, _1)), 1.0f);
        attack->fn_80096B64(1, AttackCallback(boost::bind(&ElecSecurityCode::OnAttack, this, _1)));
        attack->field_0x54 = 0;

        if (m_model != 0) {
            m_model->fn_80161230(m_type, -1);
        }
    }

    UpdateModelMatrix();
    m_generator = generator;
    fn_801D54F4(fn_80053164(), 0x723A, &m_charInfo->m_position, 0, 1);
}

ElecSecurityCode::~ElecSecurityCode() {
    if (m_model != 0) {
        delete m_model;
        m_model = 0;
    }

    fn_801E7108(m_target);
}

bool ElecSecurityCode::fn_80156C78(float dt) {
    if (m_model != 0) {
        m_model->fn_80162FB0(dt);
    }

    if (lbl_8057D6A0.field_0xC8 != 0) {
        return true;
    }

    switch (m_state) {
    case 0: {
        m_time += dt;

        float progress = m_time;

        if (progress >= 1.0f) {
            progress = 1.0f;

            if (boost::shared_ptr<CharaColliAttack> attack = m_attack.lock()) {
                attack->field_0x54 = 1;

                if (m_model != 0) {
                    m_model->fn_8016111C(m_type, -1);
                }
            }

            m_state = 1;
        }

        m_scale.x = m_scale.z = progress;
        m_scale.y = 2.0f * (1.0f - progress) + 1.0f;
        m_charInfo->m_rotation.y += s_spinSpeed * dt;

        if (m_splitSide != 0) {
            MoveApart(dt);
        }
        break;
    }

    case 1:
        Chase(dt);
        fn_801D53C4(fn_80053164(), 0x723B, &m_charInfo->m_position, this, 3, 0, 5);
        break;

    case 2:
        if (boost::shared_ptr<CharaColliAttack> attack = m_attack.lock()) {
            attack->field_0x54 = 0;

            if (m_model != 0) {
                m_model->fn_80161230(m_type, -1);
            }
        }

        fn_801D54F4(fn_80053164(), 0x7241, &m_charInfo->m_position, 0, 1);

        if (m_type == 0 || m_type == 1) {
            /* Creates two codes of the next type, 5 units from the code on either side along a
             * direction made from the one away from the nearest character info: (z, 0, x),
             * normalised and turned by pi / 8 about the x axis; (1, 0, 0.5) normalised
             * when that direction has no x or z, and (1, 0, 0) without a character info. */
            boost::shared_ptr<CharInfo> nearest =
                fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();
            Fn8040Vector3 direction;

            if (nearest.get() != 0) {
                Fn8040Vector3 away;

                fn_8040CD74(&away, &m_charInfo->m_position, &nearest->m_position);
                fn_800091D0(&direction, &away);

                /* Matching workaround: each result vector is declared before the vector it is
                 * computed from; declared the other way round, the two swap stack offsets.
                 * Revisit if the vector helpers return by value. */
                if (direction.x == 0.0f && direction.z == 0.0f) {
                    Fn8040Vector3 normalised;
                    Fn8040Vector3 fallback(1.0f, 0.0f, 0.5f);

                    fn_8040CEE8(&normalised, &fallback);
                    fn_800091D0(&direction, &normalised);
                } else {
                    Fn8040Vector3 normalised;
                    Fn8040Vector3 swapped(direction.z, 0.0f, direction.x);
                    Fn8040Vector3 tilted;

                    fn_8040CEE8(&normalised, &swapped);
                    fn_800091D0(&direction, &normalised);
                    fn_8040D118(&tilted, &direction, 0.125f * lbl_805FAD38);
                    fn_800091D0(&direction, &tilted);
                }
            } else {
                Fn8040Vector3 fallback(1.0f, 0.0f, 0.0f);

                fn_800091D0(&direction, &fallback);
            }

            /* Matching workaround: each code's offset and position are scoped to their own block.
             * The original builds the position as a temporary of the new-expression's argument
             * (it is computed after operator new returns), which the pointer-based vector helpers
             * cannot express; the blocks give the two vectors of each code their own stack slots,
             * as temporaries would have. Declared once for both codes, they shrink the original's
             * 272-byte stack frame by 16 bytes and move stack offsets throughout the function.
             * Revisit if the vector helpers return by value. */
            {
                Fn8040Vector3 offset;
                Fn8040Vector3 position;

                fn_8040CDA8(&offset, &direction, 5.0f);
                fn_8040CDD0(&position, &m_charInfo->m_position, &offset);
                m_generator->m_codes.push_back(new ElecSecurityCode(
                    m_type + 1, position, m_generator, fn_8040F760(&direction), 1));
            }

            {
                Fn8040Vector3 offset;
                Fn8040Vector3 position;

                fn_8040CDA8(&offset, &direction, 5.0f);
                fn_8040CD74(&position, &m_charInfo->m_position, &offset);
                m_generator->m_codes.push_back(new ElecSecurityCode(
                    m_type + 1, position, m_generator, fn_8040F760(&direction), 2));
            }

            m_generator->field_0x3C = nearest;
            return false;
        } else if (m_type == 2) {
            ShowBreakEffect();
            return false;
        }
        break;
    }

    UpdateModelMatrix();
    RotateParts(dt);
    return true;
}

/* The predicate that Move passes to fn_8026EE9C with the character info as the context
 * (placeholder name; behaviour inferred): collision objects that are not the character colliders
 * of a living character info are tested; a character collider is skipped when bit 0x100000 of the
 * record's flags is set, when its owner's word at 0x5C is 1, 2 or 6, or when its owner is the
 * code's own character info. */
static bool fn_80156BA8(const UnkProbeFilterRecord *record) {
    CharInfo *owner = 0;
    CharaColliBaseCharInfo *collider = dynamic_cast<CharaColliBaseCharInfo *>(record->target);

    /* owner is a raw pointer taken from a temporary shared_ptr, which is released before owner is
     * read below, and the collider holds only a weak pointer: the reads rely on an owner outside
     * this function keeping the character info alive (which object that is is not identified). */
    if (collider != 0) {
        owner = fn_800586EC(collider).lock().get();
    }

    if (owner == 0) {
        return true;
    }

    if ((record->flags & 0x100000) != 0 || owner->field_0x5C == 1 || owner->field_0x5C == 2 ||
        owner->field_0x5C == 6 || record->context == owner) {
        return false;
    }
    return true;
}

void ElecSecurityCode::Chase(float dt) {
    boost::shared_ptr<CharInfo> nearest =
        fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();
    /* Matching workaround: toNearest and forward are declared before difference, and forward is
     * set later with Set; declared at their first uses, the three vectors take other stack offsets
     * (13 words differ). Revisit if the vector helpers return by value. */
    Fn8040Vector3 toNearest;
    Fn8040Vector3 forward;
    Fn8040Vector3 difference;

    /* nearest is used without a null test, as in the original. */
    fn_8040CD74(&difference, &nearest->m_position, &m_charInfo->m_position);
    fn_800091EC(&toNearest, &difference);

    Fn8040Vector2 rotation = fn_8040F760(&toNearest);

    m_rotation.x = fn_8012773C(rotation.x, m_rotation.x, s_turnStep);
    m_rotation.y = fn_8012773C(rotation.y, m_rotation.y, s_turnStep);

    Fn8040Vector3 pitched;
    Fn8040Vector3 turned;

    forward.Set(0.0f, 0.0f, -1.0f);
    fn_8040D118(&pitched, &forward, m_rotation.x);
    fn_800091D0(&forward, &pitched);
    fn_8040D0AC(&turned, &forward, m_rotation.y);
    fn_800091D0(&forward, &turned);

    float distanceSquared = fn_8040CF2C(&toNearest);

    fn_800091D0(&m_charInfo->field_0x24, &nearest->field_0x24);
    if (distanceSquared > 0.0f) {
        Fn8040Vector3 acceleration;

        fn_800091A4(&acceleration, &forward, 50.0f);
        AddVector(&m_charInfo->field_0x24, acceleration);
    }

    Move(dt);
}

void ElecSecurityCode::MoveApart(float dt) {
    boost::shared_ptr<CharInfo> nearest =
        fn_800A8A7C(fn_80074194(), &m_charInfo->m_position).lock();
    /* Matching workaround: declaration order and Set as in Chase (13 words differ the same way
     * with the vectors declared at their first uses). Revisit with Chase, if the vector helpers
     * return by value. */
    Fn8040Vector3 toNearest;
    Fn8040Vector3 forward;
    Fn8040Vector3 difference;

    /* nearest is used without a null test, as in the original. */
    fn_8040CD74(&difference, &nearest->m_position, &m_charInfo->m_position);
    fn_800091EC(&toNearest, &difference);

    Fn8040Vector2 rotation = fn_8040F760(&toNearest);

    switch (m_splitSide) {
    case 1:
        rotation.y -= 0.85f * lbl_805FAD38;
        break;
    case 2:
        rotation.y += 0.85f * lbl_805FAD38;
        break;
    }
    m_rotation = rotation;

    Fn8040Vector3 pitched;
    Fn8040Vector3 turned;

    forward.Set(0.0f, 0.0f, -1.0f);
    fn_8040D118(&pitched, &forward, m_rotation.x);
    fn_800091D0(&forward, &pitched);
    fn_8040D0AC(&turned, &forward, m_rotation.y);
    fn_800091D0(&forward, &turned);

    float distanceSquared = fn_8040CF2C(&toNearest);

    fn_800091D0(&m_charInfo->field_0x24, &nearest->field_0x24);
    if (distanceSquared > 0.0f) {
        Fn8040Vector3 acceleration;

        fn_800091A4(&acceleration, &forward, 75.0f);
        AddVector(&m_charInfo->field_0x24, acceleration);
    }

    Move(dt);
}

void ElecSecurityCode::Move(float dt) {
    Fn8040Vector3 start;

    fn_800091EC(&start, &m_charInfo->m_position);
    fn_804103E8(&m_charInfo->m_position, &m_charInfo->field_0x18, &m_charInfo->field_0x24, 3.0f,
                dt);

    float radius = 0.0f;

    switch (m_type) {
    case 0:
        radius = 15.0f;
        break;
    case 1:
        radius = 10.0f;
        break;
    case 2:
        radius = 7.5f;
        break;
    }

    /* The sphere's centre is its radius above the position. */
    start.y += radius;
    m_charInfo->m_position.y += radius;
    fn_8026EE9C(&m_charInfo->m_position, &start, &m_charInfo->field_0x18, fn_80156BA8,
                m_charInfo.get(), radius);
    m_charInfo->m_position.y -= radius;
}

void ElecSecurityCode::UpdateModelMatrix() {
    if (m_model != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B720(&matrix, m_scale.x, m_scale.y, m_scale.z);
        m_model->fn_801633DC(&matrix);
    }

    if (m_target.get() != 0) {
        Fn8040Vector3 position;

        fn_800091EC(&position, &m_charInfo->m_position);
        position.y += s_targetHeights[m_type];
        fn_800CCE38(m_target.get(), &position);
    }
}

void ElecSecurityCode::RotateParts(float dt) {
    if (m_model != 0) {
        UnkPartFrameSearch search;
        int i;

        for (i = 0; i < 3; i++) {
            search.id = m_type * 3 + i;
            search.frame = 0;
            m_model->fn_8015FA78(fn_80157A6C, &search);
            m_partFrames[i] = search.frame;
            if (search.frame != 0) {
                m_partMatrices[i] = search.frame->modelling;
            }
        }

        if (m_partFrames[0] != 0) {
            m_partAngles[0] += lbl_805FAD38 * dt;
            fn_800E9CEC(m_partFrames[0], rwCOMBINEREPLACE, m_partAngles[0]);
            fn_804876E8(m_partFrames[0], &m_partMatrices[0], rwCOMBINEPOSTCONCAT);
        }

        if (m_partFrames[1] != 0) {
            m_partAngles[1] += 2.0f * lbl_805FAD38 * dt;
            fn_800E9CEC(m_partFrames[1], rwCOMBINEREPLACE, m_partAngles[1]);
            fn_804876E8(m_partFrames[1], &m_partMatrices[1], rwCOMBINEPOSTCONCAT);
        }

        if (m_partFrames[2] != 0) {
            m_partAngles[2] += -2.0f * lbl_805FAD38 * dt;
            fn_800E9CEC(m_partFrames[2], rwCOMBINEREPLACE, m_partAngles[2]);
            fn_804876E8(m_partFrames[2], &m_partMatrices[2], rwCOMBINEPOSTCONCAT);
        }
    }
}

void ElecSecurityCode::ShowBreakEffect() {
    if (m_model != 0) {
        UnkModelEntry_8015F7BC *entry = m_model->fn_8015F7BC(9);

        if (entry != 0) {
            UnkResourceManager_800122D4 *resources = fn_800122D4();
            UnkEffectParams_8015F0B8 parameters(m_charInfo, 0);

            parameters.field_0x34 = 1.3f;
            parameters.field_0x44 = static_cast<RpClump *>(fn_80012700(resources, 1, entry->name));
            parameters.field_0x54 = 1;
            parameters.field_0x4C = 150;
            fn_8015D93C(&parameters);
        }
    }
}

void ElecSecurityCode::fn_80156344(AttackCallbackParam &param) {
    fn_800091D0(&param.field_0x34, &m_charInfo->field_0x18);
    param.field_0x4C = fn_8040CF50(&param.field_0x34);
    fn_8040CE74(&param.field_0x34);

    if (param.field_0x4C == 0.0f) {
        Fn8040Vector3 up(0.0f, 1.0f, 0.0f);

        fn_800091D0(&param.field_0x34, &up);
        param.field_0x4C = 1.0f;
    }
}

void ElecSecurityCode::fn_80156340(AttackCallbackParam &param) {}

void ElecSecurityCode::OnAttack(AttackCallbackParam &param) {
    if (m_state == 1) {
        param.field_0x54 = 3;
        m_state = 2;
    } else {
        param.field_0x54 = 5;
    }
}

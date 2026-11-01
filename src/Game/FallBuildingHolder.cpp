/* FallBuildingHolder (class name from RTTI): the gadget of a building that falls down in the
 * stage (behaviour inferred). It holds a FallBuilding (named after the
 * boost::checked_deleter<FallBuilding> RTTI string and the FALLBUILDING.BIN model file) that shows
 * the model, gives its parts attack objects with a bound callback and starts the fall once the
 * state of its tag in the manager of fn_800CB69C reads 1 or once an attack object accepts a hit
 * (behaviour inferred). Its FallBuildingData (class name from RTTI, derived from SetAdapterState)
 * is remembered by the owner and tells whether the building has fallen already. File name
 * inferred from the class. Method names marked "(name inferred)" are not original. Definitions
 * are ordered so that deferred inlining emits the functions at their original addresses.
 *
 * NonMatching: every function and the data match the original once the weak copies that the
 * linker discards are removed, but checked_delete<FallBuilding> is a local symbol here where the
 * original's is weak. See the note in configure.py. */
/* TNumEdit.h comes before CharaColliBase.h, in the order their names appear in the unit's .rodata
 * (alphabetical order would reverse them). */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBase.h"
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/fn_804157E8_motion_control.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/UnkAttack_8005A840.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGadgetOwner_800C9CD0.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80009548.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkManager_80079808.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class FallBuilding;
class FallBuildingHolder;

/* The gadget parameter in the order of the descriptor's table below. */
struct FallBuildingParams {
    /* 0x0 */ s32 type; /* 0 ANT15, 1 ANT30 or 2 BLDG */
};
CHECK_SIZEOF(FallBuildingParams, 0x4);

/* One entry of the table that the building picks by the holder's type (names inferred from the
 * calls that use the values; -1 marks a part or sound that the type does not have). The part
 * numbers are passed to the model's methods and the sound numbers to the sound manager. */
struct FallBuildingConfig {
    /* 0x00 */ s32 field_0x0;  /* passed to fn_80162A68 when the building is set up */
    /* 0x04 */ s32 field_0x4;  /* passed to fn_80162A68 after the fall, or -1 */
    /* 0x08 */ s32 field_0x8;  /* passed to fn_80160C28, fn_80160B18 and fn_801602B4 */
    /* 0x0C */ s32 field_0xC;  /* a part that gets a character info and an attack object */
    /* 0x10 */ s32 field_0x10; /* a second such part, or -1 */
    /* 0x14 */ s32 field_0x14; /* a third such part, or -1 */
    /* 0x18 */ u8 field_0x18; /* when set, an accepted hit also calls fn_8016F0CC and fn_8016F5B8 */
    /* 0x19 */ u8 hasTarget;  /* the fall creates an UnkTarget_80166AF4 when set */
    /* 0x1C */ s32 fallSound; /* played when the fall starts, or -1 */
    /* Played when the fall motion ends while flag 1 is set (only state 1 sets it, for a building
     * that had fallen already), or -1. */
    /* 0x20 */ s32 endSound;
    /* 0x24 */ float field_0x24; /* the three floats passed to fn_8000D664 when the fall starts */
    /* 0x28 */ float field_0x28;
    /* 0x2C */ float field_0x2C;
    /* 0x30 */ s32 effectPart; /* the model entry whose name is the effect file, or -1 */
};
CHECK_SIZEOF(FallBuildingConfig, 0x34);

static const FallBuildingConfig s_configs[3] = {
    {0, 3, 0, 0, 1, 5, 1, 0, 0x7137, -1, 0.3f, 1.24f, 10.0f, 1000},
    {1, 4, 1, 2, 3, 5, 1, 0, 0x7137, -1, 0.3f, 1.24f, 10.0f, 1000},
    {2, -1, 2, 4, -1, -1, 0, 1, 0x712F, 0x7130, 5.0f, 1.0f, 10.0f, -1},
};

/* The state that the owner remembers between the creations of a FallBuilding (class name from
 * RTTI, derived from SetAdapterState): whether the building has fallen already (behaviour
 * inferred). */
class FallBuildingData : public SetAdapterState {
  public:
    FallBuildingData() {}

    virtual ~FallBuildingData() {}

    /* 0x04 */ s32 m_fallen;
};
CHECK_SIZEOF(FallBuildingData, 0x8);

class FallBuilding {
  public:
    FallBuilding(FallBuildingHolder *holder, UnkGadgetOwner_800C9CD0 *owner);
    ~FallBuilding();

    /* Re-reads the owner's position, rotation and byte and places the model (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position, turned about the y axis by its rotation
     * (name inferred). */
    void UpdateModelMatrix();
    /* Runs the state machine for one frame (name inferred). */
    void Update(float dt);
    /* The attack objects' callback (behaviour inferred): an accepted hit starts the fall. */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    /* 0x00 */ FallBuildingHolder *m_holder;
    /* 0x04 */ const FallBuildingConfig *m_config; /* the entry for the holder's type */
    /* 0x08 */ FallBuildingData *m_data;
    /* 0x0C */ s32 m_state; /* 0 standing, 1 fallen at creation, 2 falling, 3 done (inferred) */
    /* 0x10 */ UnkBitFlags_80014E04 m_flags;
    /* 0x14 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x1C */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x24 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x2C */ u8 m_tag; /* copied from the owner (name inferred) */
};
CHECK_SIZEOF(FallBuilding, 0x30);

class FallBuildingHolder : public Gadget {
  public:
    FallBuildingHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~FallBuildingHolder();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 and its building (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Builds the building (name inferred). */
    void CreateBuilding(UnkGadgetOwner_800C9CD0 *owner);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<FallBuilding> m_building;
    /* 0x38 */ FallBuildingParams m_params;
    /* 0x3C */ u32 field_0x3C; /* not accessed here; the object is 0x40 bytes */
};
CHECK_SIZEOF(FallBuildingHolder, 0x40);

/* The highest type that the load hook allows (name inferred). */
static s32 s_maxType;

FallBuilding::FallBuilding(FallBuildingHolder *holder, UnkGadgetOwner_800C9CD0 *owner)
    : m_holder(holder), m_config(&s_configs[m_holder->m_params.type]) {
    m_flags.Clear();

    m_charInfo = fn_8015CD90(0x50, 4);
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_tag);

    if (owner->m_adapterState == 0) {
        m_data = new FallBuildingData;
        m_data->m_fallen = 0;
        fn_800C9930(owner, m_data);
    } else {
        m_data = dynamic_cast<FallBuildingData *>(owner->m_adapterState);
    }

    if (m_data->m_fallen == 0) {
        m_state = 0;
    } else {
        m_state = 1;
    }

    m_model = fn_8015F800(1, "FALLBUILDING.BIN");

    if (m_data->m_fallen == 0) {
        m_model->fn_80162A68(m_config->field_0x0, 0);

        m_model->fn_80161FC8(m_config->field_0xC, boost::weak_ptr<CharInfo>(m_charInfo));
        {
            boost::shared_ptr<UnkAttack_8005A840> attack =
                m_model->fn_80161428(m_config->field_0xC).lock();

            if (attack.get() != 0) {
                attack->fn_80096B64(1,
                                    AttackCallback(boost::bind(&FallBuilding::OnAttack, this, _1)));
            }
        }

        if (m_config->field_0x10 != -1) {
            m_model->fn_80161FC8(m_config->field_0x10, boost::weak_ptr<CharInfo>(m_charInfo));

            boost::shared_ptr<UnkAttack_8005A840> attack =
                m_model->fn_80161428(m_config->field_0x10).lock();

            if (attack.get() != 0) {
                attack->fn_80096B64(1,
                                    AttackCallback(boost::bind(&FallBuilding::OnAttack, this, _1)));
            }
        }

        if (m_config->field_0x14 != -1) {
            m_model->fn_80161FC8(m_config->field_0x14, boost::weak_ptr<CharInfo>(m_charInfo));

            boost::shared_ptr<UnkAttack_8005A840> attack =
                m_model->fn_80161428(m_config->field_0x14).lock();

            if (attack.get() != 0) {
                attack->fn_80096B64(1,
                                    AttackCallback(boost::bind(&FallBuilding::OnAttack, this, _1)));
            }
        }
    } else {
        if (m_config->field_0x4 != -1) {
            m_model->fn_80162A68(m_config->field_0x4, 0);
        } else {
            m_model->fn_80162A68(m_config->field_0x0, 0);
        }
        m_model->fn_80160C28(m_config->field_0x8);
        m_model->fn_80160B18(m_config->field_0x8);
        m_model->fn_80162790();
    }

    UpdateModelMatrix();
}

/* Matching workaround: auto_inline around this destructor. The original keeps checked_delete<T>
 * out of line and emits this destructor at its definition; with the stock GC/2.7 compiler and
 * -inline noauto that needs auto_inline here. A tooling constraint, not a claim about the original
 * source; revisit if another compiler build reproduces this without the pragma, or if an original
 * checked_delete keeps out of line a destructor whose body calls only non-inline functions (the
 * pragma would inline that destructor into it). A destructor found inlined into checked_delete in
 * the original would instead mean that the original used auto_inline here. */
#pragma auto_inline on
FallBuilding::~FallBuilding() {}
#pragma auto_inline reset

void FallBuilding::Update(float dt) {
    switch (m_state) {
    case 0:
        if (!m_flags.Test(0)) {
            if (fn_8016904C(fn_800CB69C(), m_tag) == 1) {
                m_flags.Set(0);
            }
        }

        if (m_flags.Test(0)) {
            if (m_config->field_0x14 != -1) {
                boost::shared_ptr<UnkAttack_8005A840> attack =
                    m_model->fn_80161428(m_config->field_0x14).lock();

                if (attack.get() != 0) {
                    attack->field_0x55 = 0;
                }
            }

            m_model->fn_80161A24(m_config->field_0xC);
            if (m_config->field_0x10 != -1) {
                m_model->fn_80161A24(m_config->field_0x10);
            }

            if (m_config->field_0x4 != -1) {
                m_model->fn_80162804();
                m_model->fn_80162A68(m_config->field_0x4, 0);
            }
            m_model->fn_80160C28(m_config->field_0x8);

            m_model->fn_80161BD8(m_config->field_0xC, boost::weak_ptr<CharInfo>(m_charInfo),
                                 m_config->field_0x8);
            if (m_config->field_0x10 != -1) {
                m_model->fn_80161BD8(m_config->field_0x10, boost::weak_ptr<CharInfo>(m_charInfo),
                                     m_config->field_0x8);
            }
            m_model->fn_80160B18(m_config->field_0x8);
            UpdateModelMatrix();

            if (m_config->effectPart != -1) {
                UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

                effect.field_0x44 = static_cast<RpClump *>(fn_80012700(
                    fn_800122D4(), 1, m_model->fn_8015F7BC(m_config->effectPart)->name));
                effect.field_0x40 = m_charInfo->m_position.y;
                effect.field_0x34 = 1.0f;
                effect.field_0x4A = 0;
                effect.field_0x54 = 1;
                fn_8015D93C(&effect);
            }

            if (m_config->hasTarget) {
                m_target = fn_80166AF4();

                UnkWeakHolder_800CC6C8 weakTarget;
                weakTarget.ptr = m_target;
                m_model->fn_8015FA78(fn_80169D00, &weakTarget);
                m_target->fn_80167448(-1);
            }

            if (m_config->fallSound != -1) {
                fn_801D54F4(fn_80053164(), m_config->fallSound, &m_charInfo->m_position, 0, 1);
            }
            fn_8000D664(fn_80009548(), 0, m_config->field_0x24, m_config->field_0x28,
                        m_config->field_0x2C);

            m_state = 2;
            m_data->m_fallen = 1;
            m_flags.Reset(0);
        }
        break;

    case 1: {
        boost::shared_ptr<_stMtnPlayWork> motion = m_model->fn_801602B4(m_config->field_0x8).lock();

        if (motion.get() != 0) {
            if (m_flags.Test(1)) {
                MotionRange range;

                fn_80415D40(motion.get(), 0, &range);
                stMtnRequestCurrentFrame(motion.get(), 0, range.endFrame);
                m_model->fn_8016271C();

                m_model->fn_80161BD8(m_config->field_0xC, boost::weak_ptr<CharInfo>(m_charInfo),
                                     m_config->field_0x8);
                if (m_config->field_0x10 != -1) {
                    m_model->fn_80161BD8(m_config->field_0x10,
                                         boost::weak_ptr<CharInfo>(m_charInfo),
                                         m_config->field_0x8);
                }
                if (m_config->field_0x14 != -1) {
                    m_model->fn_80161FC8(m_config->field_0x14,
                                         boost::weak_ptr<CharInfo>(m_charInfo));
                }
                m_state = 2;
            } else {
                m_flags.Set(1);
            }
        }
        break;
    }

    case 2: {
        boost::shared_ptr<_stMtnPlayWork> motion = m_model->fn_801602B4(m_config->field_0x8).lock();

        if (motion.get() != 0) {
            if (stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                m_model->fn_80160480(m_config->field_0x8);
                if (m_flags.Test(1) && m_config->endSound != -1) {
                    fn_801D54F4(fn_80053164(), m_config->endSound, &m_charInfo->m_position, 0, 1);
                }
                m_state = 3;
            }
        }
        break;
    }

    /* Matching workaround: an explicit empty case for the finished state, the 3 stored above. The
     * original's compare tree tests 2 first, which the compiler builds for the case range 0 to 3;
     * with cases 0 to 2 only, or with default: break, it tests 1 first and adds a compare with 3
     * (366 words instead of 365). Revisit if the original's state values are found. */
    case 3:
        break;
    }

    m_model->fn_80162FB0(dt);
    if (m_target.get() != 0) {
        m_target->fn_8016761C(dt);
    }
}

void FallBuilding::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_tag);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
}

void FallBuilding::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    m_model->fn_801633DC(&matrix);
}

void FallBuilding::OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param) {
    if (m_state == 0 && !m_flags.Test(0) && fn_8015D4E4(&param)) {
        boost::shared_ptr<UnkAttack_8005A840> attack = param.field_0x0.lock();

        if (attack.get() != 0 && (attack->field_0x40 & 0x10) != 0) {
            param.field_0x54 = 1;

            if (m_config->field_0x18) {
                UnkManager_80079808 *manager = fn_80079808();

                fn_8016F0CC(&manager->field_0x70C, 1, 900);

                UnkEntry_80178B5C *entry = fn_80178B5C(fn_80079808(), 0);

                if (entry != 0) {
                    fn_8016F5B8(&entry->field_0x2C, 2, 200);
                }
            }

            m_flags.Set(0);
            return;
        }
    }

    param.field_0x54 = 5;
}

const char *FallBuildingHolder::s_name = "FallBuildingHolder";

FallBuildingHolder::FallBuildingHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

FallBuildingHolder::~FallBuildingHolder() {}

/* Chooses the highest allowed type from the word at 0xAC of the global at 0x8057D6A0 (1 when it
 * is 12, 2 when it is 5, otherwise 0) and loads the model. */
extern "C" void fn_800E292C(void) {
    if (lbl_8057D6A0.field_0xAC == 12) {
        s_maxType = 1;
    } else if (lbl_8057D6A0.field_0xAC == 5) {
        s_maxType = 2;
    } else {
        s_maxType = 0;
    }
    fn_8006B474("FALLBUILDING.BIN", 1);
}

extern "C" void fn_800E2928(void) {}

void FallBuildingHolder::Create(UnkGadgetOwner_800C9CD0 *owner) {
    FallBuildingHolder *holder = new FallBuildingHolder(fn_801EB304(fn_8001C4F8(), 11), owner);

    holder->CreateBuilding(owner);
}

void FallBuildingHolder::UpdateBody(float dt) {
    if (m_building.get() != 0) {
        m_building->Update(dt);
    }
}

void FallBuildingHolder::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    FallBuildingParams previous = m_params;

    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    if (m_params.type > s_maxType) {
        m_params.type = s_maxType;
        fn_800C9D68(owner, &m_params, sizeof(m_params));
    }
    if (previous.type != m_params.type) {
        CreateBuilding(owner);
    }
    if (m_building.get() != 0) {
        m_building->ReadOwner(owner);
    }
}

void FallBuildingHolder::CreateBuilding(UnkGadgetOwner_800C9CD0 *owner) {
    m_building = boost::shared_ptr<FallBuilding>(new FallBuilding(this, owner));
}

static GadgetParamInfo lbl_80524188[] = {
    {1, "0:ANT15 1:ANT30 2:BLDG", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805241D8 = {
    "FallBuilding", fn_800E292C, fn_800E2928, FallBuildingHolder::Create, 0, 0, 0x3E9, 1, 0, 10,
    lbl_80524188,
};

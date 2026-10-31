/* ThreatObjHolder (class name from RTTI): a gadget that keeps one ThreatObj, a breakable model
 * that receives attacks through a callback in the attack object of one of its model's parts, and
 * re-creates it when its "MODEL_TYPE" parameter changes. File name inferred from the class. Method
 * names marked "(name inferred)" are not original. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first).
 *
 * The callback is a boost::function object built with boost::bind(&ThreatObj::fn_8013F390, this,
 * _1); its manager, invoker and copy functions come from the boost headers.
 *
 * NonMatching: the ThreatObj constructor has an r6/r7 swap, and ThreatObj's destructor is emitted
 * in a different place. */
#include <algorithm>
#include <dolphin/types.h>
#include "Game/CharInfo.h"
#include "Game/DebugDisp.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkAttack_8005A840.h"
#include "Game/UnkEffectParams_8015F0B8.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_random.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class ThreatObjHolder;

extern "C" {
/* True when |value| < 0.0001 (behaviour inferred). */
bool fn_8000B044(float value);
boost::shared_ptr<CharInfo>
fn_8015D570(SonicteamUSA::System::CharaColli::AttackCallbackParam *param);
/* Returns 0 for arg1 0x27, otherwise fn_802063BC's result (return type inferred, ThreatObj
 * ignores it). */
int fn_8020642C(const boost::shared_ptr<CharInfo> &charInfo, int arg1);
int stMtnControlWorkFlag(UnkMotionControl_801602B4 *control, int arg1, int arg2, int arg3);
}

/* One model that a ThreatObj can use (name inferred). The first word is the second argument of the
 * model's fn_80160A84 when the object becomes active; the second word is that argument once
 * stMtnControlWorkFlag(motion, 0, 3, 1) is set (inferred: the motion has finished), or -1 to call
 * fn_80160480 instead. The file is the break model file (from its name) whose resource is passed
 * to fn_8015D93C when the object breaks. */
struct ThreatObjModel {
    /* 0x0 */ int field_0x0;
    /* 0x4 */ int field_0x4;
    /* 0x8 */ const char *breakFile;
};
CHECK_SIZEOF(ThreatObjModel, 0xC);

/* The models of one stage setting (name inferred). */
struct ThreatObjModelTable {
    /* 0x0 */ u32 count;
    /* 0x4 */ const ThreatObjModel *models;
};
CHECK_SIZEOF(ThreatObjModelTable, 0x8);

/* The two words that the gadget parameters "MODEL_TYPE" and "RANGE_RADIUS m" fill, in this
 * order. */
struct ThreatObjParams {
    /* 0x0 */ u32 modelType;
    /* 0x4 */ float radius;
};
CHECK_SIZEOF(ThreatObjParams, 0x8);

/* ThreatObj (class name from the RTTI name strings of its boost::bind callback): the breakable
 * model. It starts idle, becomes active when the character that fn_800A8B74 returns comes within
 * the holder's radius, and is finished once an attack has broken it. */
class ThreatObj {
  public:
    ThreatObj(ThreatObjHolder *holder, UnkGadgetOwner_800C9CD0 *owner);
    ~ThreatObj() {}

    /* Advances the object by dt (name inferred). */
    void Update(float dt);
    /* Re-reads the owner's position and rotation and shows the range (name inferred). */
    void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Copies the owner's position and rotation into the character info (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position and rotation (name inferred). */
    void UpdateModelMatrix();
    /* True when the character is closer than the holder's radius (name inferred). */
    bool IsCharacterInRange();
    /* The attack object's callback: an attack lowers the health; when the health falls
     * below zero or reaches (nearly) zero, the object breaks. */
    void fn_8013F390(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    /* 0x00 */ ThreatObjHolder *m_holder;
    /* 0x04 */ const ThreatObjModel *m_modelEntry;
    /* 0x08 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x10 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x18 */ int m_state; /* 0 idle, 1 active, 2 broken (inferred) */
    /* 0x1C */ float m_health;
    /* 0x20 */ int m_recordValue;   /* word 0x8 of THREATOBJ.BIN's record with id 2, or -1 */
    /* 0x24 */ float m_repeatTimer; /* time left until the next fn_801D54F4 call */
};
CHECK_SIZEOF(ThreatObj, 0x28);

class ThreatObjHolder : public Gadget {
  public:
    ThreatObjHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ThreatObjHolder();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the holder under task layer 11 and its first ThreatObj (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the parameters from the owner (name inferred). */
    void ReadParams(UnkGadgetOwner_800C9CD0 *owner);
    /* Replaces the ThreatObj with a new one (name inferred). */
    void CreateThreatObj(UnkGadgetOwner_800C9CD0 *owner);

    static const char *s_name;
    /* The setting in use, chosen when the gadget type loads (name inferred). */
    static const ThreatObjModelTable *s_modelTable;

    /* 0x30 */ boost::shared_ptr<ThreatObj> m_threatObj;
    /* 0x38 */ ThreatObjParams m_params;
};
CHECK_SIZEOF(ThreatObjHolder, 0x40);

/* The numbers of models of the two stage settings. */
static const unsigned kModelCountA = 1;
static const unsigned kModelCountB = 2;

/* The models of the two stage settings, and the table that lists them. */
static const ThreatObjModel s_modelsA[kModelCountA] = {
    {1, 2, "PR0300HORRORTHREAT_BRK.DFF"},
};

static const ThreatObjModel s_modelsB[kModelCountB] = {
    {1, -1, "PR0302CIRCUSTHREAT01BRK.DFF"},
    {2, 1, "PR0302CIRCUSTHREAT02PBRK.DFF"},
};

static const ThreatObjModelTable s_modelTables[] = {
    {kModelCountA, s_modelsA},
    {kModelCountB, s_modelsB},
};

/* The highest model count of the two settings; the "MODEL_TYPE" parameter's limit is one less. */
static unsigned s_maxModelCount = std::max(kModelCountA, kModelCountB);

ThreatObj::ThreatObj(ThreatObjHolder *holder, UnkGadgetOwner_800C9CD0 *owner) : m_holder(holder) {
    m_state = 0;
    m_health = 2.0f;
    m_recordValue = -1;
    m_modelEntry = &ThreatObjHolder::s_modelTable->models[m_holder->m_params.modelType];
    m_charInfo = fn_8015CD90(0x44, 4);
    m_model = fn_8015F800(1, "THREATOBJ.BIN");
    m_model->fn_80162A68(m_holder->m_params.modelType, 0);
    m_model->fn_80162790();

    UnkResourceRecord_80208848 *record =
        fn_80208848(fn_80012700(fn_800122D4(), 1, "THREATOBJ.BIN"), 2);

    if (record != 0) {
        m_recordValue = record->field_0x8;
        m_repeatTimer = fn_8040EC80(2.5f, 3.0f);
    }

    CopyFromOwner(owner);
    UpdateModelMatrix();
}

void ThreatObj::Update(float dt) {
    switch (m_state) {
    case 0:
        if (IsCharacterInRange()) {
            m_model->fn_8016271C();
            m_model->fn_80160C28(m_holder->m_params.modelType);
            m_model->fn_8015FF60();
            m_model->fn_80160A84(m_holder->m_params.modelType, m_modelEntry->field_0x0);
            m_model->fn_80161FC8(m_holder->m_params.modelType, m_charInfo);

            {
                boost::shared_ptr<UnkAttack_8005A840> attack =
                    m_model->fn_80161428(m_holder->m_params.modelType).lock();

                if (attack.get() != 0) {
                    attack->field_0x54 = 0;
                    attack->field_0x55 = 1;
                    attack->field_0x1C.field_0x0 =
                        AttackCallback(boost::bind(&ThreatObj::fn_8013F390, this, _1));
                }
            }

            m_state = 1;

            UnkResourceRecord_80208848 *record =
                fn_80208848(fn_80012700(fn_800122D4(), 1, "THREATOBJ.BIN"), 0);

            if (record != 0) {
                fn_801D54F4(fn_80053164(), record->field_0x8, &m_charInfo->m_position, 0, 1);
            }
        }
        break;

    case 1: {
        boost::shared_ptr<UnkMotionControl_801602B4> motion =
            m_model->fn_801602B4(m_holder->m_params.modelType).lock();

        if (motion.get() != 0) {
            if (stMtnControlWorkFlag(motion.get(), 0, 3, 1)) {
                if (m_modelEntry->field_0x4 != -1) {
                    m_model->fn_80160A84(m_holder->m_params.modelType, m_modelEntry->field_0x4);
                } else {
                    m_model->fn_80160480(m_holder->m_params.modelType);
                }
            }
        }

        if (m_recordValue != -1) {
            m_repeatTimer -= dt;

            if (m_repeatTimer < 0.0f) {
                fn_801D54F4(fn_80053164(), m_recordValue, &m_charInfo->m_position, 0, 1);
                m_repeatTimer = fn_8040EC80(2.5f, 3.0f);
            }
        }
        break;
    }
    }

    UpdateModelMatrix();
    m_model->fn_80162FB0(dt);
}

void ThreatObj::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
    fn_802E0E84(&m_charInfo->m_position, &lbl_805F06D8, m_holder->m_params.radius);
}

void ThreatObj::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
}

void ThreatObj::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    fn_8040B884(&matrix, m_charInfo->m_rotation.x);
    fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
    m_model->fn_801633DC(&matrix);
}

bool ThreatObj::IsCharacterInRange() {
    boost::shared_ptr<CharInfo> character = fn_800A8B74(fn_80074194(), lbl_805EE588).lock();

    if (character.get() != 0) {
        Fn8040Vector3 distance;
        Fn8040Vector3 difference;

        fn_8040CD74(&difference, &character->m_position, &m_charInfo->m_position);
        fn_800091EC(&distance, &difference);

        if (fn_8040CF50(&distance) < m_holder->m_params.radius) {
            return true;
        }
    }

    return false;
}

void ThreatObj::fn_8013F390(SonicteamUSA::System::CharaColli::AttackCallbackParam &param) {
    if (m_state == 1) {
        m_health -= param.field_0x50;

        if (m_health < 0.0f || fn_8000B044(m_health)) {
            UnkEffectParams_8015F0B8 effect(m_charInfo, 0);

            effect.field_0x44 = fn_80012700(fn_800122D4(), 1, m_modelEntry->breakFile);
            effect.field_0x40 = m_charInfo->m_position.y;
            effect.field_0x34 = 1.0f;
            effect.field_0x4A = 1;
            effect.field_0x54 = 1;
            fn_8015D93C(&effect);

            {
                boost::shared_ptr<CharInfo> attacker = fn_8015D570(&param);

                if (attacker.get() != 0) {
                    fn_8020642C(attacker, 0x23);
                }
            }

            param.field_0x54 = 3;

            UnkResourceRecord_80208848 *record =
                fn_80208848(fn_80012700(fn_800122D4(), 1, "THREATOBJ.BIN"), 1);

            if (record != 0) {
                fn_801D54F4(fn_80053164(), record->field_0x8, &m_charInfo->m_position, 0, 1);
            }

            m_state = 2;
        } else {
            param.field_0x54 = 1;
        }
    } else {
        param.field_0x54 = 8;
    }
}

const char *ThreatObjHolder::s_name = "ThreatObjHolder";
const ThreatObjModelTable *ThreatObjHolder::s_modelTable;

ThreatObjHolder::ThreatObjHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    ReadParams(owner);
}

ThreatObjHolder::~ThreatObjHolder() {}

extern "C" void fn_8013F274(void) {
    fn_8006B474("THREATOBJ.BIN", 1);
    ThreatObjHolder::s_modelTable = &s_modelTables[lbl_8057D6A0.field_0xAC != 9 ? 1 : 0];
}

extern "C" void fn_8013F268(void) {
    ThreatObjHolder::s_modelTable = 0;
}

void ThreatObjHolder::Create(UnkGadgetOwner_800C9CD0 *owner) {
    ThreatObjHolder *holder = new ThreatObjHolder(fn_801EB304(fn_8001C4F8(), 11), owner);

    holder->CreateThreatObj(owner);
}

void ThreatObjHolder::UpdateBody(float dt) {
    if (m_threatObj.get() != 0) {
        if (m_threatObj->m_state == 2) {
            m_threatObj.reset();
            fn_800C9B34(m_owner);
            m_flags |= 1;
        } else {
            m_threatObj->Update(dt);
        }
    }
}

void ThreatObjHolder::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ThreatObjParams previous = m_params;

    ReadParams(owner);

    if (previous.modelType != m_params.modelType) {
        CreateThreatObj(owner);
    }

    m_threatObj->ResetFromOwner(owner);
}

void ThreatObjHolder::ReadParams(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9DBC(owner, &m_params, sizeof(m_params));

    if (s_modelTable != 0 && m_params.modelType >= s_modelTable->count) {
        m_params.modelType = s_modelTable->count - 1;
        fn_800C9D68(owner, &m_params, sizeof(m_params));
    }

    m_params.radius = 10.0f * m_params.radius;
}

void ThreatObjHolder::CreateThreatObj(UnkGadgetOwner_800C9CD0 *owner) {
    m_threatObj = boost::shared_ptr<ThreatObj>(new ThreatObj(this, owner));
}

static GadgetParamInfo s_paramInfo[] = {
    {2, "MODEL_TYPE", 0, 0, s_maxModelCount - 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "RANGE_RADIUS m", 0, 0, 0, 5.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80529404 = {
    "ThreatObj", fn_8013F274, fn_8013F268, ThreatObjHolder::Create, 0, 0, 0xBBC, 1,
    0,           10,          s_paramInfo,
};

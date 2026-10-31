/* FallBuildingHolder (class name from RTTI): the gadget of a building that falls down in the
 * stage (behaviour inferred). It holds a FallBuilding (named after the
 * boost::checked_deleter<FallBuilding> RTTI string and the FALLBUILDING.BIN model file) that shows
 * the model. File name inferred from the class. Method names marked "(name inferred)" are not
 * original. Definitions are ordered so that deferred inlining emits the functions at their
 * original addresses. checked_delete<FallBuilding> is a local symbol where the original's is weak.
 *
 * Not reconstructed yet: FallBuilding's constructor (0x800E3608: it casts a pointer of the owner to
 * FallBuildingData with dynamic_cast, builds three attack callbacks with boost::bind and
 * boost::function from the 0x34-byte entry of the table at 0x804B8438 and shows the model), its
 * Update (0x800E2C48) and the attack callback at 0x800E2A6C, and the boost::function and
 * boost::bind instances of the callbacks (0x800E3324-0x800E3ED4). For these reasons the unit
 * stays NonMatching. */
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class FallBuilding;
class FallBuildingHolder;

/* The gadget parameter in the order of the descriptor's table below. */
struct FallBuildingParams {
    /* 0x0 */ s32 type; /* 0 ANT15, 1 ANT30 or 2 BLDG */
};
CHECK_SIZEOF(FallBuildingParams, 0x4);

class FallBuilding {
  public:
    FallBuilding(FallBuildingHolder *holder, UnkGadgetOwner_800C9CD0 *owner);
    ~FallBuilding();

    /* Re-reads the owner's position, rotation and byte and places the model (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position, turned about the y axis by its rotation
     * (name inferred). */
    void UpdateModelMatrix();
    /* Not reconstructed yet (name inferred). */
    void Update(float dt);

    /* 0x00 */ FallBuildingHolder *m_holder;
    /* 0x04 */ const u8 *m_config; /* the entry of the table at 0x804B8438 for the holder's type */
    /* 0x08 */ void *field_0x8;
    /* 0x0C */ s32 field_0xC;
    /* 0x10 */ u32 field_0x10;
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

const char *FallBuildingHolder::s_name = "FallBuildingHolder";

/* The highest type that the load hook allows (name inferred). */
static s32 s_maxType;

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

void FallBuildingHolder::CreateBuilding(UnkGadgetOwner_800C9CD0 *owner) {
    m_building = boost::shared_ptr<FallBuilding>(new FallBuilding(this, owner));
}

FallBuildingHolder::FallBuildingHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
}

FallBuildingHolder::~FallBuildingHolder() {}

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

static GadgetParamInfo lbl_80524188[] = {
    {1, "0:ANT15 1:ANT30 2:BLDG", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_805241D8 = {
    "FallBuilding", fn_800E292C, fn_800E2928, FallBuildingHolder::Create, 0, 0, 0x3E9, 1, 0, 10,
    lbl_80524188,
};

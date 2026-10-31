/* CircusPole (class name from RTTI; the gadget's descriptor is "CIRCUS_POLE"): a pole of the
 * circus stage. It shows the model CIRCUSPOLE.BIN, registers the clump of PR0302CIRCUSPOLETOP.DFF
 * under the name "CircusPole" and, through the callback it hands to a draw layer, renders that
 * clump at both ends of the pole when it is in view (behaviour inferred). The parameter "scale Y"
 * stretches the model. File name inferred from the class. Method names marked "(name inferred)"
 * are not original. Definitions are in reverse address order (deferred inlining emits functions
 * last-defined first). */
/* TNumEdit.h comes before CharaColliBase.h, in the order their names appear in the unit's .rodata
 * (alphabetical order would reverse them). */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBase.h"
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/RwClumpView.h"
#include "Game/UnkRegistry_801DE414.h"
#include "Game/UnkResource_80012700.h"
#include "Game/UnkString_8004F888.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class RednerDrawArgs;

typedef boost::function1<void, RednerDrawArgs &> RenderDrawCallback;
CHECK_SIZEOF(RenderDrawCallback, 0xC);

/* The 0x28-byte draw layer that fn_8004DFB0 creates (placeholder name; only the members the gadget
 * uses are mapped): its callback at 0x14 is called when the layer is drawn. */
class UnkLayer_8004DFB0 {
  public:
    /* 0x00 */ u8 field_0x0[0x14];
    /* 0x14 */ RenderDrawCallback m_drawCallback;
    /* 0x20 */ u8 field_0x20[0x8];
};
CHECK_SIZEOF(UnkLayer_8004DFB0, 0x28);

/* The manager that fn_80046178 returns (placeholder). Its word at 0x8 is what fn_8004DFB0 takes. */
class UnkLayerManager_80046178 {
  public:
    /* 0x0 */ u8 field_0x0[0x8];
    /* 0x8 */ void *field_0x8;
};

extern "C" {
UnkLayerManager_80046178 *fn_80046178(void);
boost::shared_ptr<UnkLayer_8004DFB0> fn_8004DFB0(void *arg0, const char *name);
/* Hands the resource to each atomic of the clump: calls fn_80037B54 for every atomic with the
 * resource as its argument (behaviour inferred from the body; purpose unknown). */
void fn_80037A80(UnkRwClump *clump, UnkResource_80012700 *resource);
/* Rotates the frame about the y axis by angle radians, with the combine operation combine
 * (behaviour inferred from the sin and cos it computes and the matrix rotation it calls). */
void fn_800E9CEC(UnkRwFrame *frame, int combine, float angle);
}

class CircusPole : public Gadget {
  public:
    CircusPole(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~CircusPole();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads position, rotation and the y scale from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info, stretched by the y scale (name inferred). */
    void UpdateModelMatrix();
    /* The draw layer's callback: draws the top clump at both ends of the pole when it is in view
     * (name inferred). */
    void OnDraw(RednerDrawArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ float m_scaleY;
    /* 0x44 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0x4C */ UnkRwClump *m_clump;
};
CHECK_SIZEOF(CircusPole, 0x50);

const char *CircusPole::s_name = "CircusPole";

CircusPole::CircusPole(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x42, 4, "CircusPole");
    ReadOwner(owner);
    m_model = fn_8015F800(1, "CIRCUSPOLE.BIN");
    m_model->fn_80162A68(0, 0);

    /* fn_80037A80 hands the .DMA resource to each atomic of the clump. The resource loader returns
     * the clump of the .DFF file as an opaque pointer (type inferred from its uses). */
    UnkResource_80012700 *dmaResource = fn_80012700(fn_800122D4(), 1, "PR0302CIRCUSPOLETOP.DMA");

    m_clump =
        reinterpret_cast<UnkRwClump *>(fn_80012700(fn_800122D4(), 1, "PR0302CIRCUSPOLETOP.DFF"));
    fn_80037A80(m_clump, dmaResource);
    fn_801DE0EC(fn_80130B2C(), UnkString_8004F888("CircusPole"), m_clump);
    m_layer = fn_8004DFB0(fn_80046178()->field_0x8, "CircusPole");
    m_layer->m_drawCallback = RenderDrawCallback(boost::bind(&CircusPole::OnDraw, this, _1));
    UpdateModelMatrix();
}

/* Builds the registry on the first call and caches its address. */
extern "C" UnkRegistry_801DE414 *fn_80130B2C(void) {
    static UnkRegistry_801DE414 *s_registry;

    if (s_registry == 0) {
        static UnkRegistry_801DE414 registry;

        s_registry = &registry;
    }

    return s_registry;
}

CircusPole::~CircusPole() {
    fn_801DDFDC(fn_80130B2C(), UnkString_8004F888("CircusPole"));
}

void CircusPole::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }
}

void CircusPole::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();
}

void CircusPole::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9DBC(owner, &m_scaleY, sizeof(m_scaleY));
}

void CircusPole::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B720(&matrix, 1.0f, m_scaleY, 1.0f);
        m_model->fn_801633DC(&matrix);
    }
}

void CircusPole::OnDraw(RednerDrawArgs &args) {
    UnkRwSphere sphere;

    sphere.radius = 20.0f;
    sphere.center.x = m_charInfo->m_position.x;
    sphere.center.y = m_charInfo->m_position.y + 25.0f * m_scaleY;
    sphere.center.z = m_charInfo->m_position.z;

    if (fn_804856C8(lbl_805F15B4->curCamera, &sphere)) {
        UnkRwFrame *frame = m_clump->frame;

        fn_800E9CEC(frame, rwCOMBINEREPLACE, m_charInfo->m_rotation.y);
        fn_804875B0(frame, &sphere.center, rwCOMBINEPOSTCONCAT);
        fn_80456BC4(m_clump);
    }

    sphere.center.x = m_charInfo->m_position.x;
    sphere.center.y = m_charInfo->m_position.y - 25.0f * m_scaleY - 10.0f;
    sphere.center.z = m_charInfo->m_position.z;

    if (fn_804856C8(lbl_805F15B4->curCamera, &sphere)) {
        UnkRwFrame *frame = m_clump->frame;

        fn_800E9CEC(frame, rwCOMBINEREPLACE, m_charInfo->m_rotation.y);
        fn_804875B0(frame, &sphere.center, rwCOMBINEPOSTCONCAT);
        fn_80456BC4(m_clump);
    }
}

extern "C" void fn_801307D8(void) {
    fn_8006B474("CIRCUSPOLE.BIN", 1);
    fn_8006B474("PR0302CIRCUSPOLETOP.DFF", 0);
    fn_8006B474("PR0302CIRCUSPOLETOP.DMA", 0);
}

extern "C" void fn_801307D4(void) {}

void CircusPole::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new CircusPole(fn_801EB304(fn_8001C4F8(), 11), owner);
}

static GadgetParamInfo s_paramInfo[] = {
    {4, "scale Y", 0, 0, 0, 1.0f, 0.1f, 1000.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80528804 = {
    "CIRCUS_POLE", fn_801307D8, fn_801307D4, CircusPole::Create, 0, 0, 0xC87, 1, 0, 10, s_paramInfo,
};

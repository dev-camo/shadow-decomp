/* ElecCircuitEntrance (class name from RTTI): the entrance gadget of the electric circuit puzzle.
 * It watches collisions of its character info and, when a character touches it, sends that
 * character a command and sets a circuit state (behaviour inferred).
 * File name inferred from the class. Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The callbacks it hands to the collision object are boost::function objects built with
 * boost::bind(&ElecCircuitEntrance::method, this, _1); their manager, invoker and copy functions
 * come from the boost headers. */
#include <cmath>
/* The RTTI data of TNumEdit.h comes before that of CharaColliBase.h at the start of the unit's
 * .data, .sdata and .rodata, so TNumEdit.h is included before CharaColliBase.h (alphabetical
 * order would reverse them). */
#include "Game/TNumEdit.h"
#include "Game/CharaColliBase.h"
#include "Game/CharCommand.h"
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/PathCtrl.h"
#include "Game/SharedConstants.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkProbeTarget_800587A0.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class ElecCircuitEntrance;

/* What a handle wraps (placeholder name). */
class UnkHandleTarget_800768D8;

/* The handle that a command holds (placeholder name): a shared pointer in a class of its own. The
 * compiler-generated copy constructor and destructor are the functions at 0x800811AC and
 * 0x800ABC9C. */
struct UnkHandle_800ABCE8 {
    /* Builds the handle for path and the character info target; result is the value
     * fn_8001CDA0 stored (behaviour inferred). */
    UnkHandle_800ABCE8(const boost::weak_ptr<CharInfo> &target,
                       SonicteamUSA::System::PathCtrl *path, int result);

    /* 0x0 */ boost::shared_ptr<UnkHandleTarget_800768D8> m_ptr;
};
CHECK_SIZEOF(UnkHandle_800ABCE8, 0x8);

namespace Player {

/* Player::ElectricCircuitCommand (class name from RTTI): what the entrance sends to the character
 * that touched it. The destructor is defined elsewhere, together with the vtable. */
class ElectricCircuitCommand : public PlayerCommand {
  public:
    /* The command id 0x131 is the value the original constructor loads; its meaning is unknown. */
    ElectricCircuitCommand(const UnkHandle_800ABCE8 &handle, int arg1, bool flag)
        : PlayerCommand(0x131, arg1), m_handle(handle), m_flag(flag) {}
    virtual ~ElectricCircuitCommand();

    /* 0x14 */ UnkHandle_800ABCE8 m_handle;
    /* 0x1C */ bool m_flag;
};
CHECK_SIZEOF(Player::ElectricCircuitCommand, 0x20);

} // namespace Player

extern "C" {
bool fn_800B78B0(const boost::shared_ptr<CharInfo> &charInfo);
}

/* What the contact callback receives: the gadget's character info and the gadget itself
 * (placeholder name). */
struct UnkHitContext_801F7E28 {
    /* 0x00 */ boost::weak_ptr<GadgetCharInfo> m_charInfo;
    /* 0x08 */ ElecCircuitEntrance *m_owner;
};
CHECK_SIZEOF(UnkHitContext_801F7E28, 0xC);

extern "C" void fn_8015AA84(ColliDetectArgs *args,
                            void (*callback)(boost::shared_ptr<CharInfo> hit,
                                             UnkHitContext_801F7E28 *context),
                            UnkHitContext_801F7E28 *context);

extern "C" void fn_802E0D6C(const Fn8040Vector3 *position, const Fn8040Vector3 *rotation, int arg2,
                            float arg3, float arg4);

class ElecCircuitEntrance : public Gadget {
  public:
    ElecCircuitEntrance(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecCircuitEntrance();
    virtual void UpdateBody(float dt);
    virtual void fn_800CBC1C();
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the entrance gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and byte setting into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* The two collision callbacks, bound in the constructor. */
    void fn_801F7E28(ColliDetectArgs &args);
    void fn_801F7D80(ColliDetectArgs &args);
    /* Calls fn_8015BE64 on the handle target, then passes fn_802E0D6C the entrance's position
     * offset in x and z by -10 times the sine and cosine of its y rotation, a rotation of
     * (0, y rotation, 0), 0, 10.0 and 3.75 (purpose unknown). */
    void fn_801F7F9C();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ u8 field_0x38; /* copied from the owner; the id passed to fn_8016904C (inferred) */
    /* 0x3C */ boost::shared_ptr<UnkHandleBTarget_8015BE64> field_0x3C;
    /* 0x44 */ boost::weak_ptr<CharInfo> field_0x44;
    /* 0x4C */ u32 field_0x4C;
};
CHECK_SIZEOF(ElecCircuitEntrance, 0x50);

const char *ElecCircuitEntrance::s_name = "ElecCircuitEntrance";

/* Three angles (in radians) built at startup; the constructor passes them to fn_800C7A84. */
static Fn8040Vector3 lbl_80583B78(0.5f * lbl_805FAD38, 0.0f, 0.0f);

/* The angle helper is defined after fn_801F888C, which calls it. */
inline void fn_801F8040(float *sine, float *cosine, float angle, float scale);

/* The callback that fn_8015AA84 calls for each character info in the collision results (name
 * inferred). */
static void fn_801F888C(boost::shared_ptr<CharInfo> hit, UnkHitContext_801F7E28 *context) {
    if (hit.get() != 0 && context != 0 && hit->field_0x50 == 1) {
        if (!context->m_owner->field_0x44.expired()) {
            boost::shared_ptr<CharInfo> current(context->m_owner->field_0x44);

            if (current == hit) {
                return;
            }
        }

        if (fn_800B78B0(hit)) {
            context->m_owner->field_0x44 = hit;
            return;
        }

        SonicteamUSA::System::PathCtrl path;
        Fn8040Vector3 direction;
        int result;

        fn_800091EC(&direction, &lbl_805E07A0);
        fn_801F8040(&direction.x, &direction.z, context->m_owner->m_charInfo->m_rotation.y, -1.0f);

        if (fn_8001CDA0(fn_8001B494(), &path, 0.0f, context->m_owner->m_charInfo.get(), &direction,
                        &result, 0, 0)) {
            UnkHandle_800ABCE8 handle(boost::weak_ptr<CharInfo>(hit), &path, result);
            Player::ElectricCircuitCommand *command =
                new Player::ElectricCircuitCommand(handle, 4, false);

            hit->fn_801DC980(command);

            if (fn_8016904C(fn_800CB69C(), context->m_owner->field_0x38) != 1) {
                fn_801690D0(fn_800CB69C(), context->m_owner->field_0x38, 1);
            }
        }
    }
}

ElecCircuitEntrance::ElecCircuitEntrance(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CB88(0x59, 4, "ElecCircuitEntrance");

    boost::shared_ptr<UnkProbeTarget_800587A0> probe =
        fn_800C7A84(fn_80021B08(), boost::weak_ptr<CharInfo>(m_charInfo), 7.5f, 20.0f, 0x40000, 10,
                    0, &lbl_805E07A0, &lbl_80583B78, 0);
    field_0x3C = fn_8015B8C4(probe);

    field_0x3C->field_0x0.fn_800F5980(
        ColliDetectCallback(boost::bind(&ElecCircuitEntrance::fn_801F7E28, this, _1)));
    field_0x3C->field_0x18.fn_800F5980(
        ColliDetectCallback(boost::bind(&ElecCircuitEntrance::fn_801F7D80, this, _1)));

    field_0x3C->field_0x18.ClearFlags(2);
    CopyFromOwner(m_owner);
}

ElecCircuitEntrance::~ElecCircuitEntrance() {}

void ElecCircuitEntrance::UpdateBody(float dt) {
    fn_8015BE9C(field_0x3C.get(), dt);
}

void ElecCircuitEntrance::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
}

void ElecCircuitEntrance::fn_800CBC1C() {
    fn_801F7F9C();
}

/* Sets *sine and *cosine to scale times the sine and cosine of angle (behaviour inferred). */
inline void fn_801F8040(float *sine, float *cosine, float angle, float scale) {
    *sine = scale * std::sin(angle);
    *cosine = scale * std::cos(angle);
}

void ElecCircuitEntrance::fn_801F7F9C() {
    if (field_0x3C.get() != 0) {
        fn_8015BE64(field_0x3C.get());

        Fn8040Vector3 position;

        fn_800091EC(&position, &lbl_805E07A0);
        fn_801F8040(&position.x, &position.z, m_charInfo->m_rotation.y, -10.0f);
        AddVector(&position, m_charInfo->m_position);

        /* Read into a local first: written inside the constructor call, operator-> is called out of
         * line, which the original does not do. */
        float angle = m_charInfo->m_rotation.y;
        Fn8040Vector3 rotation(0.0f, angle, 0.0f);

        fn_802E0D6C(&position, &rotation, 0, 10.0f, 3.75f);
    }
}

void ElecCircuitEntrance::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &field_0x38);
}

void ElecCircuitEntrance::fn_801F7E28(ColliDetectArgs &args) {
    UnkHitContext_801F7E28 context;

    context.m_charInfo = m_charInfo;
    context.m_owner = this;
    fn_8015AA84(&args, fn_801F888C, &context);
}

void ElecCircuitEntrance::fn_801F7D80(ColliDetectArgs &args) {
    if (!field_0x44.expired()) {
        boost::shared_ptr<CharInfo> current(field_0x44);

        if (current.get() != 0) {
            field_0x44.reset();
        }
    }
}

extern "C" void fn_801F7D7C(void) {}

extern "C" void fn_801F7D78(void) {}

void ElecCircuitEntrance::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new ElecCircuitEntrance(fn_801EB304(fn_8001C4F8(), 11), owner);
}

GadgetTypeInfo lbl_80542370 = {
    "ELEC_ENTRANCE",
    fn_801F7D7C,
    fn_801F7D78,
    ElecCircuitEntrance::Create,
    0,
    0,
    0x7E0,
    1,
    0,
    10,
    0,
};

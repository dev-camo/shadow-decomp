/* TPjsEventActionFade (class name from RTTI): the event code's fade action task and the functions
 * that create and stop it (behaviour in the header). File name inferred from the class. Method
 * names marked "(name inferred)" in the header are not original. Definitions are in reverse address
 * order (deferred inlining emits functions last-defined first).
 *
 * The draw layer's callback is a boost::function object built with
 * boost::bind(&TPjsEventActionFade::OnDraw, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/TPjsEventActionFade.h"
#include "Game/TPjsEventActionManager.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_80044254.h"
#include "Game/fn_800396D0.h"
#include "Game/fn_80204874.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

/* The stop flag: set by fn_80328618 and cleared by fn_80328640 (name inferred). */
static bool s_stopped;

/* The colours the file passes by address (names inferred from their users). The original has four
 * separate objects in this order, although the first and the last are equal. */
static const RwRGBA s_resetColor = {0, 0, 0, 0};
static const RwRGBA s_rectColor = {255, 128, 0, 64};
static const RwRGBA s_kind1Color = {255, 255, 255, 255};
static const RwRGBA s_kind2Color = {0, 0, 0, 0};

/* The original tests the manager pointer, replaces a null pointer by a null constant and tests the
 * result again (USA 0x80328E00-0x80328E14). The source form that gives the second test is not
 * recovered: this function tests the pointer once and is 0x60 bytes where the original's is 0x70. */
void fn_80328DDC(int kind) {
    if (!s_stopped) {
        TPjsEventActionManager *manager = fn_801723E0()->field_0x0;

        if (manager != 0) {
            new TPjsEventActionFade(manager, kind);
        }
    }
}

TPjsEventActionFade::TPjsEventActionFade(Task *parent, int kind)
    : Task(parent), m_kind(kind), field_0x30(true) {
    m_name = "TPjsEventActionFade";

    switch (kind) {
    case 1:
        field_0x2C = 1.0f / 6.0f;
        fn_802051F8(field_0x2C);
        fn_80204874(&s_kind1Color);
        fn_80173878(1.0f, 0, 1);
        break;
    case 2:
        field_0x2C = 1.0f / 12.0f;
        fn_802051F8(field_0x2C);
        fn_80204874(&s_kind2Color);
        fn_80173878(1.0f, 0, 1);
        break;
    case 3:
        m_layer = fn_8004E24C(fn_80046178()->field_0x18);
        m_layer->m_drawCallback =
            RenderDrawCallback(boost::bind(&TPjsEventActionFade::OnDraw, this, _1));
        m_rect.x = 0;
        m_rect.y = 0;
        m_rect.w = lbl_8057D6A0.field_0x44;
        m_rect.h = lbl_8057D6A0.field_0x48;
        break;
    default:
        /* Other kinds leave the constructor here. With "break" or no default case the
         * constructor is 0x1BC bytes instead of the original's 0x1C8. */
        return;
    }
}

TPjsEventActionFade::~TPjsEventActionFade() {
    if (m_kind != 3) {
        fn_803285EC();
    }
}

void TPjsEventActionFade::Update(float dt) {
    if (m_kind != 3 && s_stopped) {
        m_flags |= 1;
        return;
    }

    switch (m_kind) {
    case 1:
        if (field_0x30) {
            if (fn_80044E10() == 1) {
                field_0x30 = false;
                fn_80173834(0.0f, 1);
            }
        } else if (fn_80044E10() == 0) {
            m_flags |= 1;
        }
        break;
    case 3:
        break;
    default:
        m_flags |= 1;
        break;
    }
}

void TPjsEventActionFade::OnDraw(RednerDrawArgs &args) {
    switch (m_kind) {
    case 3:
        fn_800396D0(&m_rect, &s_rectColor, 0);
        break;
    }
}

void fn_80328640(void) {
    s_stopped = false;
}

void fn_80328618(void) {
    s_stopped = true;
    fn_803285EC();
}

void fn_803285EC(void) {
    fn_802051F8(1.0f);
    fn_80204874(&s_resetColor);
}

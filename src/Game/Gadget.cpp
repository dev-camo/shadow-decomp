/* Gadget (class described in Game/Gadget.h). File name inferred from the class. Definitions are
 * in reverse address order (deferred inlining emits functions last-defined first). */
#include "Game/Gadget.h"

const char *Gadget::s_name = "Gadget";

Gadget::Gadget(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Task(parent) {
    m_name = s_name;
    m_owner = 0;
    if (owner != 0) {
        m_owner = owner;
        fn_800C9CD0(m_owner, this);
    }
}

Gadget::~Gadget() {
    if (m_owner != 0) {
        fn_800C9BE8(m_owner);
        m_owner = 0;
    }
}

void Gadget::Update(float dt) {
    if (!IsFinished()) {
        UpdateBody(dt);
    }
}

/* fn_8015A858 and fn_8015A818 are expanded in place here (0x8015A898, USA) and still emitted
 * below as functions, which the BreakableOrnament gadget calls; this needs the unit's
 * -inline auto flag in configure.py. */
bool Gadget::IsFinished() {
    if (fn_8015A858(this)) {
        m_flags |= 1;
        return true;
    }
    if (fn_8015A818(this)) {
        m_flags |= 1;
        return true;
    }
    return false;
}

extern "C" bool fn_8015A858(Gadget *gadget) {
    if (gadget->m_owner != 0 && fn_800C9A64(gadget->m_owner)) {
        return true;
    }
    return false;
}

extern "C" bool fn_8015A818(Gadget *gadget) {
    if (gadget->m_owner != 0 && fn_800C996C(gadget->m_owner)) {
        return true;
    }
    return false;
}

extern "C" void fn_8015A7EC(Gadget *gadget) {
    if (gadget->m_owner != 0) {
        fn_800C9690(gadget->m_owner);
    }
}

extern "C" void fn_8015A7C0(Gadget *gadget) {
    if (gadget->m_owner != 0) {
        fn_800C9AD4(gadget->m_owner);
    }
}

extern "C" void fn_8015A794(Gadget *gadget) {
    if (gadget->m_owner != 0) {
        fn_800C9B34(gadget->m_owner);
    }
}

extern "C" void fn_8015A768(Gadget *gadget) {
    if (gadget->m_owner != 0) {
        fn_800C9B04(gadget->m_owner);
    }
}

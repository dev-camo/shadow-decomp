/* FleetHolder (class name from RTTI); original filename unknown. */
#include "Game/FleetHolder.h"
#include "Game/EggFleet.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/fn_8006_resource_list.h"

FleetHolder::FleetHolder(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    fn_800CBC38(owner);
}

FleetHolder::~FleetHolder() {}

extern "C" void fn_800CC030() {
    fn_8006B474("EGGFLEET.BIN", 1);
}

extern "C" void fn_800CC02C() {}

void FleetHolder::Create(UnkGadgetOwner_800C9CD0 *owner) {
    FleetHolder *holder = new FleetHolder(fn_801EB304(fn_8001C4F8(), 11), owner);
    holder->fn_800CBD58(owner);
}

void FleetHolder::fn_800CBD58(UnkGadgetOwner_800C9CD0 *owner) {
    field_0x30 = boost::shared_ptr<EggFleet>(new EggFleet(this, owner));
}

void FleetHolder::UpdateBody(float dt) {
    if (field_0x30.get() != 0) {
        field_0x30->fn_800CCF30(dt);
    }
}

void FleetHolder::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    FleetHolderParams previous = m_params;
    fn_800CBC38(owner);
    if (previous.kind != m_params.kind) {
        fn_800CBD58(owner);
    }
    field_0x30->fn_800CCEB4();
}

void FleetHolder::fn_800CBC38(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_params.rangeR = 10.0f * m_params.rangeR;
    m_params.rangeHeight = 0.5f * (10.0f * m_params.rangeHeight);
    m_params.moveLength = 10.0f * m_params.moveLength;
    m_params.moveSpeed = 10.0f * m_params.moveSpeed;
}

static GadgetParamInfo lbl_80522D78[] = {
    {2, "0:BATTLE 1:MOTHER", 0, 0, 1, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "RANGE_R m", 0, 0, 0, 100.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "RANGE_HEIGHT m", 0, 0, 0, 50.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MOVE_LENGHT m", 0, 0, 0, 0.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "MOVE_SPEED m/s", 0, 0, 0, 2.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "FIRE_TIMING s", 0, 0, 0, 20.0f, 0.1f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80522E90 = {
    "EggFleet", fn_800CC030, fn_800CC02C, FleetHolder::Create, 0, 0, 0x1069, 1, 0, 30, lbl_80522D78,
};

/* The MonsterControl gadget (class and behaviour described in Game/MonsterControl.h). File name
 * inferred from the class. Definitions are in reverse address order (deferred inlining emits
 * functions last-defined first). */
#include "Game/MonsterControl.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/MonsterCharInfo.h"
#include "Game/UnkCharInfoTable_800DDF20.h"
#include "Game/UnkWorld_80021B08.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;

const char *MonsterControl::s_name = "MonsterControl";

MonsterControl::MonsterControl(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner), m_rotation(0.0f, 0.0f, 0.0f) {
    m_name = s_name;
    ReadOwner(owner);

    /* The scaled lengths go unhalved into the argument where RainEffect passes half of its sizes,
     * so whether X_LENGTH, Y_LENGTH and Z_LENGTH are full or half lengths is not known. */
    Fn8040Vector3 lengths(m_params.xLength, m_params.yLength, m_params.zLength);

    boost::shared_ptr<CharaColliBase> probe =
        fn_800C7D04(fn_80021B08(), &m_position, lengths, m_rotation, 0, 0x100, 10, 0,
                    boost::weak_ptr<CharInfo>(lbl_805EE698), lbl_805E07A0, &lbl_805E07B8, 0);
    m_handle = fn_8015B8C4(probe);

    m_handle->field_0x0.fn_800F5980(
        GadgetColliDetectCallback(boost::bind(&MonsterControl::OnCollision, this, _1)));
    m_handle->ClearFlags(2);
}

MonsterControl::~MonsterControl() {}

extern "C" void fn_8013A594(void) {}

extern "C" void fn_8013A590(void) {}

void MonsterControl::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new MonsterControl(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void MonsterControl::UpdateBody(float dt) {
    if (m_handle.get() != 0) {
        fn_8015BE9C(m_handle.get(), dt);
    }
}

void MonsterControl::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateProbe();

    if (m_handle.get() != 0) {
        fn_8015BE64(m_handle.get());
    }
}

void MonsterControl::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9DBC(owner, &m_params, sizeof(m_params));
    m_params.xLength = 10.0f * m_params.xLength;
    m_params.yLength = 10.0f * m_params.yLength;
    m_params.zLength = 10.0f * m_params.zLength;
    fn_800C9F14(owner, &m_position);
    fn_800C9ED4(owner, &m_rotation);
}

void MonsterControl::UpdateProbe() {
    if (m_handle.get() != 0) {
        m_handle->m_probe->SetPosition(&m_position);
        m_handle->m_probe->SetRotation(m_rotation);
        m_handle->m_probe->SetSize(m_params.xLength, -2, m_params.yLength, m_params.zLength);
    }
}

void MonsterControl::OnCollision(ColliDetectArgs &args) {
    if (fn_8015D2EC(args.m_hits)) {
        /* The Monster gadget constructs its MonsterCharInfo with 0x4B, the value that fn_801DCB14
         * compares with the word at 0x50 of each character info. */
        UnkCharInfoNode_801DCB14 *node = fn_801DCB14(fn_800DDF20(), 0x4B);

        if (node != 0) {
            boost::shared_ptr<MonsterCharInfo> monster =
                boost::static_pointer_cast<MonsterCharInfo>(node->charInfo.lock());

            if (monster.get() != 0) {
                monster->fn_80139F98(m_params.kind, this);
            }
        }
    }
}

void MonsterControl::DisableCollision() {
    if (m_handle.get() != 0) {
        m_handle->SetFlags(2);
    }
}

static GadgetParamInfo s_paramInfo[] = {
    {1, "0:START 1:MISS 2:END", 0, 0, 2, 0.0f, 0.0f, 0.0f, fn_800CBC20, 0},
    {4, "X_LENGTH m", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Y_LENGTH m", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {4, "Z_LENGTH m", 0, 0, 0, 10.0f, 0.0f, 65536.0f, fn_800CBC20, 0},
    {0},
};

GadgetTypeInfo lbl_80528F9C = {
    "MonsterControl", fn_8013A594, fn_8013A590, MonsterControl::Create, 0, 0, 0xBC8, 1, 0, 10,
    s_paramInfo,
};

/* The CityChaosEmerald gadget (behaviour in the class comment). File name inferred from the class.
 * Method names marked "(name inferred)" are not original.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined first).
 *
 * The model's collision callback is a boost::function object built with
 * boost::bind(&CityChaosEmerald::OnCollision, this, _1); its manager, invoker and copy functions
 * come from the boost headers. */
#include "Game/CityChaosEmerald.h"
#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/weak_ptr.hpp"

const char *CityChaosEmerald::s_name = "CityChaosEmerald";

CityChaosEmerald::CityChaosEmerald(Task *parent, UnkGadgetOwner_800C9CD0 *owner)
    : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x4F, 4);

    m_model = fn_8015F800(1, "CITYCHAOSEMERALD.BIN");
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161FC8(0, boost::weak_ptr<CharInfo>(m_charInfo));

    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&CityChaosEmerald::OnCollision, this, _1));

    CopyFromOwner(owner);
    UpdateModelMatrix();
    m_sound = fn_801D3E58(0x712E, &m_charInfo->m_position, this);
}

CityChaosEmerald::~CityChaosEmerald() {
    m_sound.fn_801D455C();
}

extern "C" void fn_800DFDD0(void) {
    fn_8006B474("CITYCHAOSEMERALD.BIN", 1);
}

extern "C" void fn_800DFDCC(void) {}

void CityChaosEmerald::Create(UnkGadgetOwner_800C9CD0 *owner) {
    new CityChaosEmerald(fn_801EB304(fn_8001C4F8(), 11), owner);
}

void CityChaosEmerald::UpdateBody(float dt) {
    m_model->fn_80162FB0(dt);
}

void CityChaosEmerald::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    CopyFromOwner(owner);
    UpdateModelMatrix();
    m_model->fn_80162EF4();
}

void CityChaosEmerald::CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E50(owner, &m_comId);
}

void CityChaosEmerald::UpdateModelMatrix() {
    Fn8040Matrix34 matrix;

    fn_8040C148(&matrix, &lbl_805E0720);
    fn_8040B59C(&matrix, &m_charInfo->m_position);
    fn_8040B848(&matrix, m_charInfo->m_rotation.y);
    m_model->fn_801633DC(&matrix);
}

void CityChaosEmerald::OnCollision(ColliDetectArgs &args) {
    if (fn_8015D2EC(args.m_hits)) {
        Fn8040Vector3 direction(0.0f, 0.0f, -1.0f);

        fn_80168C48(&m_charInfo->m_position, &direction, 0xC);
        fn_801D560C(fn_80053164(), 0xE017, 0, 0);
        lbl_8057D6A0.field_0xC1 = 1;
        fn_8016907C(fn_800CB69C(), m_comId, 1);
        fn_800C9B04(m_owner);
        m_flags |= 1;
    }
}

GadgetTypeInfo lbl_80523F9C = {
    "CityChaosEmerald",
    fn_800DFDD0,
    fn_800DFDCC,
    CityChaosEmerald::Create,
    0,
    0,
    0x3E8,
    1,
    0,
    10,
    0,
};

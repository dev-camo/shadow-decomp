/* The SecretKey gadget (behaviour in the class comment) and the key list that the gadget and the
 * SECRETDOOR gadget use. File name inferred from the class. Definitions are in reverse address
 * order (deferred inlining emits functions last-defined first).
 *
 * The model's collision callback is a boost::function object built with
 * boost::bind(&SecretKey::OnCollision, this, _1); its manager, invoker and copy functions come
 * from the boost headers. */
#include "Game/SecretKey.h"
#include "Game/CharInfo.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkGlobal_8057D6A0.h"
#include "Game/UnkManager_800CB69C.h"
#include "Game/UnkManager_80079808.h"
#include "Game/UnkObject_800BB74C.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"

/* The length of the key list and the entry value that marks a free entry (names inferred). */
enum {
    KEY_COUNT = 5,
    KEY_FREE = -1,
};

/* The key list (name inferred; see fn_8011BDA4 to fn_8011BED0 in the header). */
static s32 s_keys[KEY_COUNT];

namespace SecretKey {

const char *SecretKey::s_name = "SecretKey";

SecretKey::SecretKey(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x13, 4);
    m_model = fn_8015F9D8(0, 0xDE);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->m_collisionCallback =
        ColliDetectCallback(boost::bind(&SecretKey::OnCollision, this, _1));
    m_model->fn_80160BA0("mtn");
    m_model->fn_8015FF60();
    m_model->fn_801607A8("mtn");
    m_model->fn_80160B18(0);
    m_effect = fn_8015D794(&m_charInfo->m_position, 0x8D);
    m_target = fn_801E7128(boost::weak_ptr<CharInfo>(m_charInfo), 0x80);
    ReadOwner(m_owner);
    UpdateModelMatrix();
}

SecretKey::~SecretKey() {
    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }

        m_effect.reset();
    }
}

void SecretKey::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    switch (m_state) {
    case 0: {
        bool found = false;

        for (int i = 0; i < KEY_COUNT; i++) {
            if (m_keyId == s_keys[i]) {
                found = true;
                break;
            }
        }

        if (found) {
            Finish();
        } else {
            m_state = 1;
        }
        break;
    }
    case 2:
        fn_8011BED0(m_keyId);
        fn_801D560C(fn_80053164(), 0xE017, 0, 0);
        Finish();
        break;
    case 3:
        Finish();
        break;
    }

    UpdateModelMatrix();
}

void SecretKey::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void SecretKey::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E50(owner, &m_keyId);
    m_state = 0;
}

void SecretKey::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8040C148(&matrix, &lbl_805E0720);
        fn_8040B59C(&matrix, &m_charInfo->m_position);
        fn_8040B848(&matrix, m_charInfo->m_rotation.y);
        fn_8040B884(&matrix, m_charInfo->m_rotation.x);
        fn_8040B80C(&matrix, m_charInfo->m_rotation.z);
        m_model->fn_801633DC(&matrix);
    }

    if (!m_effect.expired()) {
        boost::shared_ptr<Effect::System::EffectElement> effect(m_effect.lock());

        if (effect.get() != 0) {
            fn_80425E4C(effect.get(), &m_charInfo->m_position);
        }
    }

    if (m_target.get() != 0) {
        fn_800CCE38(m_target.get(), &m_charInfo->m_position);
    }
}

void SecretKey::OnCollision(ColliDetectArgs &args) {
    if (fn_8015AC84(args)) {
        m_state = 2;
    }
}

void SecretKey::Finish() {
    fn_801690D0(fn_800CB69C(), m_keyId, 1);
    fn_8015A794(this);
    m_flags |= 1;
}

} // namespace SecretKey

/* Lists the key's model resource, the number the constructor loads the model from, and calls
 * fn_8011BE14, which clears the key list and copies it back from the selected record when
 * fn_800BB74C does not return null. */
extern "C" void fn_8011BF8C(void) {
    fn_8006B50C(0xDE, 1);
    fn_8011BE14();
}

extern "C" void fn_8011BF88(void) {}

void SecretKey::SecretKey::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SecretKey(parent, owner);
}

extern "C" bool fn_8011BED0(s32 id) {
    for (int i = 0; i < KEY_COUNT; i++) {
        if (s_keys[i] == KEY_FREE) {
            s_keys[i] = id;
            fn_803357F4(&fn_80079808()->field_0x724);
            return true;
        }
    }

    return false;
}

extern "C" u32 fn_8011BE9C(void) {
    u32 count = 0;

    for (int i = 0; i < KEY_COUNT; i++) {
        if (s_keys[i] != KEY_FREE) {
            count++;
        }
    }

    return count;
}

extern "C" void fn_8011BE14(void) {
    for (int i = 0; i < KEY_COUNT; i++) {
        s_keys[i] = KEY_FREE;
    }

    UnkObject_800BB74C *object = fn_800BB74C();

    if (object != 0) {
        for (int i = 0; i < KEY_COUNT; i++) {
            s_keys[i] = object->field_0xA8[lbl_8057D6A0.field_0xA8.field_0x4].field_0x4C[i];
        }
    }
}

extern "C" bool fn_8011BDA4(void) {
    UnkObject_800BB74C *object = fn_800BB74C();

    if (object != 0) {
        /* The original reads it once before the loop: the stores below may alias this s32, so a
         * read inside the loop would be reloaded on every iteration. */
        s32 index = lbl_8057D6A0.field_0xA8.field_0x4;

        for (int i = 0; i < KEY_COUNT; i++) {
            object->field_0xA8[index].field_0x4C[i] = s_keys[i];
        }

        return true;
    }

    return false;
}

GadgetTypeInfo lbl_805271CC = {
    "SECRETKEY", fn_8011BF8C, fn_8011BF88, SecretKey::SecretKey::Create, 0, 0, 0x1D, 1, 0, 10, 0,
};

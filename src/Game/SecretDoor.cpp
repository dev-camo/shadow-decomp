/* The SecretDoor gadget (class described in Game/SecretDoor.h). File name inferred from the class.
 * Definitions are in reverse address order (deferred inlining emits functions last-defined
 * first). */
#include "Game/SecretDoor.h"
#include "Game/CharInfo.h"
#include "Game/EffectElement.h"
#include "Game/GadgetTypeInfo.h"
#include "Game/UnkSoundManager_80053164.h"
#include "Game/UnkStartupFloat_800E2570.h"
#include "Game/fn_8006_resource_list.h"
#include "Game/fn_8011BE9C.h"
#include "Game/fn_8040_vector_types.h"
#include "rw/rpworld.h"
#include "rw/rwcore.h"

static const char *s_modelFile = "SECRETDOOR.BIN";

/* The squared distance from the character within which the door handles new keys (100 units;
 * inferred from the comparison in UpdateBody).
 * Matching workaround: UnkStartupFloat_800E2570 stands in for the original type of this variable.
 * The original sets it with a start-up function in .ctors (a constant float initialiser would be
 * emitted as initialised .sdata data instead), and the shared stand-in reproduces that function.
 * Revisit when the original wrapper type is found. */
static UnkStartupFloat_800E2570 s_keyDistanceSquared(10000.0f);

namespace SecretDoor {

const char *SecretDoor::s_name = "SecretDoor";

/* What RemoveLocks passes to its part callback (name inferred). */
struct RemoveLocksContext {
    /* 0x0 */ s32 count; /* clumps still to remove */
    /* 0x4 */ bool showEffect;
};
CHECK_SIZEOF(RemoveLocksContext, 0x8);

/* RemoveLocks's part callback (name inferred). When the count is 0 or less on entry it returns
 * null, which ends the walk, so the walk stops at the part after the removal that brought the count
 * to 0 (or when the parts run out). Otherwise, for a part whose word at 0x8 is nonzero and whose
 * clump is in a world, it removes the clump from that world and lowers the count; with showEffect
 * and when the clump has a frame, it also starts effect 0x296 at the frame's world matrix and plays
 * sound 0xE018 in between. It returns the part to go on with the walk. */
static UnkModelPart_8015FA78 *RemoveLockPart(UnkModelPart_8015FA78 *part, void *userData) {
    RemoveLocksContext *context = static_cast<RemoveLocksContext *>(userData);

    if (context->count > 0) {
        if (part->field_0x8 != 0) {
            RpWorld *world = fn_8046256C(part->clump);

            if (world != 0) {
                fn_804623EC(world, part->clump);

                if (context->showEffect) {
                    RwFrame *frame = static_cast<RwFrame *>(part->clump->object.parent);

                    if (frame != 0) {
                        RwMatrix *ltm = RwFrameGetLTM(frame);
                        Fn8040Matrix34 matrix;

                        matrix.values[0][0] = ltm->right.x;
                        matrix.values[0][1] = ltm->up.x;
                        matrix.values[0][2] = ltm->at.x;
                        matrix.values[0][3] = ltm->pos.x;
                        matrix.values[1][0] = ltm->right.y;
                        matrix.values[1][1] = ltm->up.y;
                        matrix.values[1][2] = ltm->at.y;
                        matrix.values[1][3] = ltm->pos.y;
                        matrix.values[2][0] = ltm->right.z;
                        matrix.values[2][1] = ltm->up.z;
                        matrix.values[2][2] = ltm->at.z;
                        matrix.values[2][3] = ltm->pos.z;

                        fn_8015D754(&matrix, 0x296);
                        fn_801D560C(fn_80053164(), 0xE018, 0, 0);
                    }
                }

                context->count--;
            }
        }

        return part;
    }

    return 0;
}

SecretDoor::SecretDoor(Task *parent, UnkGadgetOwner_800C9CD0 *owner) : Gadget(parent, owner) {
    m_name = s_name;
    m_charInfo = fn_8015CD90(0x14, 4);
    ReadOwner(owner);

    m_model = fn_8015F800(0, s_modelFile);
    m_model->fn_80162A68(0, 0);
    m_model->fn_80161F08("col0", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->fn_80161F08("col1", boost::weak_ptr<CharInfo>(m_charInfo));
    m_model->fn_80160BA0("mtn");
    m_model->fn_8015FF60();
    UpdateModelMatrix();
}

SecretDoor::~SecretDoor() {}

/* Advances the model when it exists, plays sound 0xE019 (no position, keyed to the gadget) while
 * bit 0 of m_flags is set, works on m_state and finally places the model (UpdateModelMatrix).
 * m_model is checked only before fn_80162FB0; the model calls of STATE_START, of the opening, of
 * STATE_OPENING and of RemoveLocks do not check it. */
void SecretDoor::UpdateBody(float dt) {
    if (m_model.get() != 0) {
        m_model->fn_80162FB0(dt);
    }

    if (m_flags.Test(0)) {
        fn_801D53C4(fn_80053164(), 0xE019, 0, this, 2, 0, 2);
    }

    switch (m_state) {
    case STATE_START:
        m_state = STATE_WAIT_KEYS;
        m_character = fn_800A8B74(fn_80074194(), lbl_805EE588);

        /* When the owner has all of the bits 0x400000 (fn_800C9E80), the door removes up to
         * KEY_COUNT lock parts without effects and opens at once, without setting the owner
         * bits. */
        if (m_owner != 0 && fn_800C9E80(m_owner, 0x400000)) {
            RemoveLocks(KEY_COUNT, false);
            m_model->fn_80161A24(1);
            m_state = STATE_OPENING;
            m_model->fn_80160B18(0);
            m_flags.Set(0);
        }
        break;

    case STATE_WAIT_KEYS: {
        u32 keyCount = fn_8011BE9C();

        if (keyCount != 0) {
            if (m_keyCount != keyCount) {
                if (!m_character.expired()) {
                    boost::shared_ptr<CharInfo> character = m_character.lock();
                    const Fn8040Vector3 &offset = m_charInfo->m_position - character->m_position;

                    /* False for NaN. A result that is not taken here is compared again on the
                     * next update. */
                    if (fn_8040CF2C(&offset) <= s_keyDistanceSquared.value) {
                        m_state = STATE_UNLOCK;
                        m_timer = 0.5f;
                        m_pendingLocks = keyCount - m_keyCount;
                        m_keyCount = keyCount;
                    }
                }
            } else if (m_keyCount >= KEY_COUNT) {
                m_model->fn_80161A24(1);
                m_state = STATE_OPENING;
                m_model->fn_80160B18(0);
                m_flags.Set(0);

                /* The original reads m_owner once (0x8011B99C) and passes that value to both calls;
                 * a member read after the first call would be loaded again, because the call may
                 * change it. */
                UnkGadgetOwner_800C9CD0 *owner = m_owner;

                if (owner != 0) {
                    fn_800C9EB0(owner, 0x400000);
                    fn_800C9EB0(owner, 0x200);
                }
            }
        }
        break;
    }

    case STATE_UNLOCK:
        m_timer -= dt;

        /* False for NaN. Each expiry with m_pendingLocks above 0 makes one removal attempt and
         * lowers m_pendingLocks, whether or not the walk removed a clump. When m_pendingLocks is
         * <= 0, the expiry returns to STATE_WAIT_KEYS instead, so one more 0.5-second wait follows
         * the last attempt. */
        if (m_timer <= 0.0f) {
            m_timer = 0.5f;

            if (m_pendingLocks <= 0) {
                m_state = STATE_WAIT_KEYS;
            } else {
                RemoveLocks(1, true);
                m_pendingLocks--;
            }
        }
        break;

    case STATE_OPENING:
        /* fn_80160040(0) returns true at the end of the model's motion (inferred). */
        if (m_model->fn_80160040(0)) {
            m_flags.Reset(0);
            m_state = STATE_OPEN;
        }
        break;

    /* STATE_OPENING stores this state; nothing more happens in it until a reset (ReadOwner)
     * returns to STATE_START.
     * Matching workaround: the state is listed as an empty case. With it the switch has the
     * original's compare tree; without it UpdateBody is 180 words instead of 181 and 149 of them
     * differ. Revisit when the original's handling of this state is found. */
    case STATE_OPEN:
        break;
    }

    UpdateModelMatrix();
}

void SecretDoor::RemoveLocks(int count, bool showEffect) {
    RemoveLocksContext context;

    context.count = count;
    context.showEffect = showEffect;
    m_model->fn_8015FA78(RemoveLockPart, &context);
}

void SecretDoor::ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner) {
    ReadOwner(owner);
    UpdateModelMatrix();

    if (m_model.get() != 0) {
        m_model->fn_80162EF4();
        m_model->fn_8016108C();
    }
}

void SecretDoor::ReadOwner(UnkGadgetOwner_800C9CD0 *owner) {
    fn_800C9F14(owner, &m_charInfo->m_position);
    fn_800C9ED4(owner, &m_charInfo->m_rotation);
    fn_800C9E60(owner, &m_comId);
    m_keyCount = 0;
    m_timer = 0.0f;
    m_state = STATE_START;
}

void SecretDoor::UpdateModelMatrix() {
    if (m_model.get() != 0) {
        Fn8040Matrix34 matrix;

        fn_8015CEA0(m_charInfo.get(), &matrix);
        m_model->fn_801633DC(&matrix);
    }
}

} // namespace SecretDoor

extern "C" void fn_8011B62C(void) {
    fn_8006B474(s_modelFile, 1);
}

extern "C" void fn_8011B628(void) {}

void SecretDoor::SecretDoor::Create(UnkGadgetOwner_800C9CD0 *owner) {
    Task *parent = fn_801EB304(fn_8001C4F8(), 11);

    new SecretDoor(parent, owner);
}

GadgetTypeInfo lbl_80527158 = {
    "SECRETDOOR", fn_8011B62C, fn_8011B628, SecretDoor::SecretDoor::Create, 0, 0, 0x1E, 1, 0, 10, 0,
};

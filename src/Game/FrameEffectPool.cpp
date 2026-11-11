/* Effects kept on frames of a clump, and the stage functions of five stages: the range
 * 0x80328048-0x803285EC (USA (GUPE8P) addresses). Whether the stage functions and the class are
 * one original file is not established; they are kept together because nothing in the range
 * shows where a file ends. The original file name is unknown: this one and the class name are
 * inferred from what the class's code does. Definitions are in reverse address order (deferred
 * inlining emits functions last-defined first). */
#include "Game/FrameEffectPool.h"
#include "Game/EffectElement.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rphanim.h"
#include "rw/rpworld.h"
#include "rw/rwcore.h"
#include "layout_check.h"

using Effect::System::EffectElement;

/* One kept effect (names inferred). The file's static initialiser constructs the array of these
 * and registers its destruction; the constructor and destructor are the implicit ones. */
struct UnkFrameEffect_803285BC {
    /* 0x0 */ boost::weak_ptr<EffectElement> effect;
    /* 0x8 */ RwMatrix *frameMatrix;  /* the frame's world matrix, owned by the frame */
    /* 0xC */ Fn8040Matrix34 *matrix; /* a copy of it, allocated with new, that the effect reads */
};
CHECK_SIZEOF(UnkFrameEffect_803285BC, 0x10);

/* Which effect a frame with a given id (fn_8044758C) gets (names inferred). */
struct UnkFrameEffectKind_8050D888 {
    /* 0x0 */ RwInt32 frameId;
    /* 0x4 */ int launcherIndex; /* the first argument of fn_801D2C28 */
    /* 0x8 */ int effectId;
};
CHECK_SIZEOF(UnkFrameEffectKind_8050D888, 0xC);

extern "C" {
/* The callback that AttachEffects passes to fn_8048752C for each child frame; returns the frame,
 * so every child is visited (the data argument is not used). */
RwFrame *fn_803281B0(RwFrame *frame, void *data);
/* Stores the effect and the two matrices in the first array entry whose effect does not lock.
 * Returns true when it stored them and false when every entry is in use (return type
 * inferred). */
bool fn_803280E8(const boost::weak_ptr<EffectElement> &effect, RwMatrix *frameMatrix,
                 Fn8040Matrix34 *matrix);
}

static const UnkFrameEffectKind_8050D888 s_kinds[] = {
    {25000, 0, 0x22},
    {25001, 0, 0x1B},
    {25002, 0, 0x1F},
};

static const int POOL_SIZE = 0x80;

static UnkFrameEffect_803285BC s_pool[POOL_SIZE];

/* The number of effects that fn_803281B0 asked for since AttachEffects cleared it. */
static int s_attachedCount;

UnkFrameEffectPool_803284B4::UnkFrameEffectPool_803284B4() {
    for (int i = 0; i < POOL_SIZE; i++) {
        s_pool[i].matrix = 0;
    }
}

void UnkFrameEffectPool_803284B4::fn_80328494() {
    StopEffects();
}

void UnkFrameEffectPool_803284B4::fn_80328474() {
    StopEffects();
}

void UnkFrameEffectPool_803284B4::StopEffects() {
    for (int i = 0; i < POOL_SIZE; i++) {
        boost::shared_ptr<EffectElement> effect = s_pool[i].effect.lock();

        if (effect) {
            effect->RequestStop();
            /* RequestStop already sets this flag; the original sets it a second time here (two
             * read-modify-writes of the flags, USA (GUPE8P) 0x803283E8 and 0x80328408). */
            effect->SetFlags(Effect::System::ELEMENT_STOP_REQUESTED);
            effect->ClearFlags(Effect::System::ELEMENT_ACTIVE);
        }

        s_pool[i].effect.reset();

        if (s_pool[i].matrix != 0) {
            delete s_pool[i].matrix;
            s_pool[i].matrix = 0;
        }
    }
}

int UnkFrameEffectPool_803284B4::AttachEffects(RpClump *clump) {
    s_attachedCount = 0;
    fn_8048752C(static_cast<RwFrame *>(clump->object.parent), fn_803281B0, 0);
    return s_attachedCount;
}

void UnkFrameEffectPool_803284B4::UpdateMatrices() {
    for (int i = 0; i < POOL_SIZE; i++) {
        boost::shared_ptr<EffectElement> effect = s_pool[i].effect.lock();

        if (effect && s_pool[i].matrix != 0 && s_pool[i].frameMatrix != 0) {
            *s_pool[i].matrix = fn_800635A4(s_pool[i].frameMatrix);
        }
    }
}

extern "C" RwFrame *fn_803281B0(RwFrame *frame, void *) {
    /* The count is unsigned, so i is compared with it as unsigned, as in the original. */
    for (int i = 0; i < sizeof(s_kinds) / sizeof(s_kinds[0]); i++) {
        /* Matching workaround: the statement below is a call whose result is dropped. The
         * original calls fn_8044758C on the frame twice in a row (USA (GUPE8P) 0x803281D8 and
         * 0x803281E0) and uses only the second result, and the comparison accounts for one call
         * only. What the first call is in the original source (an accessor expanded twice, an
         * unused value) is unknown. Revisit when that form is identified. */
        fn_8044758C(frame);

        if (s_kinds[i].frameId == fn_8044758C(frame)) {
            RwMatrix *frameMatrix = RwFrameGetLTM(frame);
            Fn8040Matrix34 *matrix = new Fn8040Matrix34;

            *matrix = fn_800635A4(frameMatrix);
            fn_803280E8(fn_801D2C28(s_kinds[i].launcherIndex, matrix, s_kinds[i].effectId, 0),
                        frameMatrix, matrix);
            s_attachedCount++;
        }
    }

    return frame;
}

extern "C" bool fn_803280E8(const boost::weak_ptr<EffectElement> &effect, RwMatrix *frameMatrix,
                            Fn8040Matrix34 *matrix) {
    for (int i = 0; i < POOL_SIZE; i++) {
        boost::shared_ptr<EffectElement> used = s_pool[i].effect.lock();

        if (!used) {
            s_pool[i].effect = effect;
            s_pool[i].frameMatrix = frameMatrix;
            s_pool[i].matrix = matrix;
            return true;
        }
    }

    return false;
}

/* The four functions of the stage "EggmansBase" (see Game/FrameEffectPool.h). */
int fn_803280E0(void) {
    return 1;
}

int fn_803280D8(void) {
    return 1;
}

int fn_803280D0(void) {
    return 1;
}

int fn_803280C8(void) {
    return 1;
}

/* The four functions of the stage "DoomsBase1" (see Game/FrameEffectPool.h). */
int fn_803280C0(void) {
    return 1;
}

int fn_803280B8(void) {
    return 1;
}

int fn_803280B0(void) {
    return 1;
}

int fn_803280A8(void) {
    return 1;
}

/* The four functions of the stage "Space" (see Game/FrameEffectPool.h). */
int fn_803280A0(void) {
    return 1;
}

int fn_80328098(void) {
    return 1;
}

int fn_80328090(void) {
    return 1;
}

int fn_80328088(void) {
    return 1;
}

/* The four functions of the stage "ARKPast2" (see Game/FrameEffectPool.h). */
int fn_80328080(void) {
    return 1;
}

int fn_80328078(void) {
    return 1;
}

int fn_80328070(void) {
    return 1;
}

int fn_80328068(void) {
    return 1;
}

/* The four functions of the stage "PrisonIsland" (see Game/FrameEffectPool.h). */
int fn_80328060(void) {
    return 1;
}

int fn_80328058(void) {
    return 1;
}

int fn_80328050(void) {
    return 1;
}

int fn_80328048(void) {
    return 1;
}

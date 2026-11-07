/* The walks over the hits of a collision callback's arguments (declared in
 * Game/UnkHandleBTarget_8015BE64.h). The original file name is unknown; the unit is named after
 * its first function. The code from fn_8015B8C4 on calls its own local copy of the weak_ptr
 * converting constructor that these walks share, so it belongs to another file; whether the
 * walks start a file or continue the Gadget code before them is not established. Definitions are
 * in reverse address order (deferred inlining emits functions last-defined first).
 *
 * Each walk takes the collision object of every hit, and for each one that still locks, the
 * character info that fn_800C4568 finds for it; a hit is skipped when either does not lock. */
#include "Game/CharInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkWorld_80021B08.h"
#include "Game/UnkWorldProbe_80021FE4.h"
#include "boost/weak_ptr.hpp"

using SonicteamUSA::System::CharaColli::CharaColliBase;
using SonicteamUSA::System::Collision::CollisionResultList;

extern "C" void fn_8015B70C(ColliDetectArgs *args, ColliHitCallback callback, void *context,
                            s32 value) {
    CollisionResultList *hits = args->m_hits;
    UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
    boost::weak_ptr<CharaColliBase> target;
    boost::weak_ptr<CharInfo> ownerRef;

    for (; iterator.node != hits->fn_8001EDEC(); fn_80027398(&iterator)) {
        target = fn_800587A0(&iterator.node->target);
        if (boost::shared_ptr<CharaColliBase> collider = target.lock()) {
            ownerRef = fn_800C4568(fn_80021B08(), collider);
            if (boost::shared_ptr<CharInfo> charInfo = ownerRef.lock()) {
                if (value == charInfo->field_0x5C && callback != 0) {
                    callback(charInfo, context);
                }
            }
        }
    }
}

extern "C" bool fn_8015B544(ColliDetectArgs &args, s32 arg1, s32 arg2) {
    CollisionResultList *hits = args.m_hits;
    UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
    boost::weak_ptr<CharaColliBase> target;
    boost::weak_ptr<CharInfo> ownerRef;

    for (; iterator.node != hits->fn_8001EDEC(); fn_80027398(&iterator)) {
        target = fn_800587A0(&iterator.node->target);
        if (boost::shared_ptr<CharaColliBase> collider = target.lock()) {
            ownerRef = fn_800C4568(fn_80021B08(), collider);
            if (boost::shared_ptr<CharInfo> charInfo = ownerRef.lock()) {
                if (arg1 == charInfo->field_0x5C && arg2 == charInfo->field_0x50) {
                    return true;
                }
            }
        }
    }
    return false;
}

extern "C" void fn_8015B384(ColliDetectArgs *args, ColliHitCallback callback, void *context) {
    CollisionResultList *hits = args->m_hits;
    UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
    boost::weak_ptr<CharaColliBase> target;
    boost::weak_ptr<CharInfo> ownerRef;

    for (; iterator.node != hits->fn_8001EDEC(); fn_80027398(&iterator)) {
        target = fn_800587A0(&iterator.node->target);
        if (boost::shared_ptr<CharaColliBase> collider = target.lock()) {
            ownerRef = fn_800C4568(fn_80021B08(), collider);
            if (boost::shared_ptr<CharInfo> charInfo = ownerRef.lock()) {
                if (charInfo->fn_80073634() && callback != 0) {
                    callback(charInfo, context);
                }
            }
        }
    }
}

extern "C" bool fn_8015B1C4(ColliDetectArgs &args) {
    CollisionResultList *hits = args.m_hits;
    UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
    boost::weak_ptr<CharaColliBase> target;
    boost::weak_ptr<CharInfo> ownerRef;

    for (; iterator.node != hits->fn_8001EDEC(); fn_80027398(&iterator)) {
        target = fn_800587A0(&iterator.node->target);
        if (boost::shared_ptr<CharaColliBase> collider = target.lock()) {
            ownerRef = fn_800C4568(fn_80021B08(), collider);
            if (boost::shared_ptr<CharInfo> charInfo = ownerRef.lock()) {
                if (charInfo->fn_80078B2C()) {
                    return true;
                }
            }
        }
    }
    return false;
}

extern "C" void fn_8015B004(ColliDetectArgs *args, ColliHitCallback callback, void *context) {
    CollisionResultList *hits = args->m_hits;
    UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
    boost::weak_ptr<CharaColliBase> target;
    boost::weak_ptr<CharInfo> ownerRef;

    for (; iterator.node != hits->fn_8001EDEC(); fn_80027398(&iterator)) {
        target = fn_800587A0(&iterator.node->target);
        if (boost::shared_ptr<CharaColliBase> collider = target.lock()) {
            ownerRef = fn_800C4568(fn_80021B08(), collider);
            if (boost::shared_ptr<CharInfo> charInfo = ownerRef.lock()) {
                if (charInfo->fn_80078B2C() && callback != 0) {
                    callback(charInfo, context);
                }
            }
        }
    }
}

extern "C" void fn_8015AE44(ColliDetectArgs *args, ColliHitCallback callback, void *context) {
    CollisionResultList *hits = args->m_hits;
    UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
    boost::weak_ptr<CharaColliBase> target;
    boost::weak_ptr<CharInfo> ownerRef;

    for (; iterator.node != hits->fn_8001EDEC(); fn_80027398(&iterator)) {
        target = fn_800587A0(&iterator.node->target);
        if (boost::shared_ptr<CharaColliBase> collider = target.lock()) {
            ownerRef = fn_800C4568(fn_80021B08(), collider);
            if (boost::shared_ptr<CharInfo> charInfo = ownerRef.lock()) {
                if (charInfo->fn_80078B44() && callback != 0) {
                    callback(charInfo, context);
                }
            }
        }
    }
}

extern "C" bool fn_8015AC84(ColliDetectArgs &args) {
    CollisionResultList *hits = args.m_hits;
    UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
    boost::weak_ptr<CharaColliBase> target;
    boost::weak_ptr<CharInfo> ownerRef;

    for (; iterator.node != hits->fn_8001EDEC(); fn_80027398(&iterator)) {
        target = fn_800587A0(&iterator.node->target);
        if (boost::shared_ptr<CharaColliBase> collider = target.lock()) {
            ownerRef = fn_800C4568(fn_80021B08(), collider);
            if (boost::shared_ptr<CharInfo> charInfo = ownerRef.lock()) {
                if (charInfo->fn_80078B4C()) {
                    return true;
                }
            }
        }
    }
    return false;
}

extern "C" void fn_8015AA84(ColliDetectArgs *args, ColliHitCallback callback, void *context) {
    CollisionResultList *hits = args->m_hits;
    UnkProbeIterator_80027374 iterator(hits->fn_8001EE04());
    boost::weak_ptr<CharaColliBase> target;
    boost::weak_ptr<CharInfo> ownerRef;

    for (; iterator.node != hits->fn_8001EDEC(); fn_80027398(&iterator)) {
        target = fn_800587A0(&iterator.node->target);
        if (boost::shared_ptr<CharaColliBase> collider = target.lock()) {
            ownerRef = fn_800C4568(fn_80021B08(), collider);
            if (boost::shared_ptr<CharInfo> charInfo = ownerRef.lock()) {
                if (charInfo->fn_80078B4C() && callback != 0) {
                    callback(charInfo, context);
                }
            }
        }
    }
}

/* Moves a sphere through the world's collision objects (behaviour inferred; the functions are
 * described in Game/fn_8026EE9C.h). The original file name is unknown. Definitions are in reverse
 * address order (deferred inlining emits functions last-defined first). */
#include "Game/fn_8026EE9C.h"
#include "Game/UnkWorld_80021B08.h"

using SonicteamUSA::System::Collision::CollisionResultList;

bool fn_8026F204(Fn8040Vector3 *velocity, Fn8040Vector3 *position, Fn8040Vector3 *motion,
                 UnkProbePredicate predicate, void *context, float radius) {
    bool blocked = false;
    UnkWorld_80020D40 *world = fn_80020D40();
    CollisionResultList *hits = 0;
    CollisionResultList *previousHits;
    UnkSphere_80021C04 sphere(position, radius);
    Fn8040Vector3 start(*position);
    Fn8040Vector3 current(*position);
    Fn8040Vector3 previous(current);
    Fn8040Vector3 remaining(*motion);
    Fn8040Vector3 lastRemaining(remaining);

    for (int i = 0; i < 5; i++) {
        lastRemaining = remaining;

        Fn8040Vector3 step(remaining);

        previous = current;
        sphere.m_center = current;

        previousHits = hits;
        hits = fn_80021934(world, &sphere, &step, 3, 5, previousHits, predicate, context, true);
        if (previousHits != 0) {
            delete previousHits;
        }

        AddVector(&current, step);
        if (hits != 0) {
            Fn8040Vector3 push = fn_8026EF38(hits);

            AddVector(&current, push);
        }
        if (hits == 0) {
            break;
        }

        Fn8040Vector3 left = remaining - (current - previous);

        remaining = fn_8026F0BC(velocity, &left, hits, &blocked);
        if (remaining.x * remaining.x + remaining.y * remaining.y + remaining.z * remaining.z <
            0.0001f) {
            /* The original stores this, although nothing reads remaining afterwards. */
            remaining = lastRemaining;
            break;
        }
    }

    *motion = current - start;
    *position = current;
    if (hits != 0) {
        delete hits;
    }
    return blocked;
}

Fn8040Vector3 fn_8026F0BC(Fn8040Vector3 *velocity, const Fn8040Vector3 *motion,
                          CollisionResultList *hits, bool *blocked) {
    Fn8040Vector3 result(*motion);
    UnkProbeIterator_80027374 iterator;

    for (iterator = hits->fn_8001EE04(); iterator.node != hits->fn_8001EDEC();
         fn_80027374(&iterator, 0)) {
        Fn8040Vector3 normal(*fn_800266D4(iterator.node->hit));
        float motionAlong = fn_8040CF90(&result, &normal);
        float velocityAlong = fn_8040CF90(velocity, &normal);

        if (motionAlong > 0.0f) {
            continue;
        }
        *blocked = true;
        SubtractVector(&result, normal * motionAlong);
        SubtractVector(velocity, normal * velocityAlong);
    }
    return result;
}

/* Matching workaround: this member of the list class is defined here as an ordinary function.
 * The original has these two instructions at this place, after fn_8026EF38, its only caller in
 * the game, where this compiler puts the out-of-line copy of an inline function; fn_8026EF38
 * also reads the word directly. So it is probably an inline member of the list whose copy the
 * original calls. Defined inline in the class, this compiler expands it at the call and emits no
 * copy. Revisit when the source form that keeps such a call is known, or the list's own file is
 * recovered. */
s32 CollisionResultList::fn_8026F0B4() {
    return field_0x4;
}

Fn8040Vector3 fn_8026EF38(CollisionResultList *hits) {
    if (hits == 0 || hits->field_0x4 == 0) {
        return lbl_805E07A0;
    }

    UnkProbeIterator_80027374 iterator;
    Fn8040Vector3 offset(lbl_805E07A0);
    Fn8040Vector3 push(lbl_805E07A0);

    for (iterator = hits->fn_8001EE04(); iterator.node != hits->fn_8001EDEC();
         fn_80027374(&iterator, 0)) {
        UnkProbeHit_8001E9BC *hit = iterator.node->hit;
        UnkProbeHitPart_8001E9E8 *part = &hit->field_0xC;
        Fn8040Vector3 normal(*fn_800266D4(hit));

        if (fn_80099080(hit) == 1 && part->field_0x10 < 0.0f) {
            AddVector(&push, normal * -fn_8001E9E8(part));
        }
        AddVector(&offset, normal * 0.02f);
    }

    fn_8040CE38(&offset, 1.0f / hits->fn_8026F0B4());
    return push + offset;
}

bool fn_8026EE9C(Fn8040Vector3 *position, const Fn8040Vector3 *start, Fn8040Vector3 *velocity,
                 UnkProbePredicate predicate, void *context, float radius) {
    Fn8040Vector3 current(*start);
    Fn8040Vector3 motion = *position - *start;
    bool result = fn_8026F204(velocity, &current, &motion, predicate, context, radius);

    *position = current;
    return result;
}

/* CodeWarrior runtime global destructor chain (global_destructor_chain.c): objects registered by
 * static constructors are destroyed in reverse order at exit. */
#include "NMWException.h"

DestructorChain *__global_destructor_chain;

void __destroy_global_chain(void) {
    DestructorChain *entry;

    while ((entry = __global_destructor_chain) != 0) {
        __global_destructor_chain = entry->next;
        entry->destructor(entry->object, -1);
    }
}

void *__register_global_object(void *object, void (*destructor)(void *, int),
                               DestructorChain *entry) {
    entry->next = __global_destructor_chain;
    entry->destructor = destructor;
    entry->object = object;
    __global_destructor_chain = entry;
    /* The Metrowerks runtime returns the object; r3 already holds it, so no code is emitted. */
    return object;
}

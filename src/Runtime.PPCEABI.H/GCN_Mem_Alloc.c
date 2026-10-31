/* MSL runtime default-heap allocator (GCN_Mem_Alloc.c). */
#include <dolphin/os.h>

void __sys_free(void *ptr);
void *__sys_alloc(unsigned int size);

/* The diagnostic strings are named after their per-region data addresses (placeholder labels). */
#if defined(VERSION_GUPJ8P)
#define INIT_ERROR_MESSAGE lbl_80514350
#define INIT_HEAP_MESSAGE lbl_80514388
#elif defined(VERSION_GUPP8P)
#define INIT_ERROR_MESSAGE lbl_80514DC8
#define INIT_HEAP_MESSAGE lbl_80514E00
#else
#define INIT_ERROR_MESSAGE lbl_80513CB0
#define INIT_HEAP_MESSAGE lbl_80513CE8
#endif

const char INIT_ERROR_MESSAGE[] = "GCN_Mem_Alloc.c : InitDefaultHeap. No Heap Available\n";
const char INIT_HEAP_MESSAGE[] = "Metrowerks CW runtime library initializing default heap\n";

/* Name from the original diagnostic string above. */
static inline void InitDefaultHeap(void) {
    if (__OSCurrHeap == -1) {
        void *arena_lo;
        void *arena_hi;
        OSHeapHandle heap;
        OSReport(INIT_ERROR_MESSAGE);
        OSReport(INIT_HEAP_MESSAGE);
        arena_lo = OSGetArenaLo();
        arena_hi = OSGetArenaHi();
        arena_lo = OSInitAlloc(arena_lo, arena_hi, 1);
        OSSetArenaLo(arena_lo);
        arena_lo = (void *)OSRoundUp32B(arena_lo);
        arena_hi = (void *)OSRoundDown32B(arena_hi);
        heap = OSCreateHeap(arena_lo, arena_hi);
        OSSetCurrentHeap(heap);
        OSSetArenaLo(arena_hi);
    }
}

void __sys_free(void *ptr) {
    InitDefaultHeap();
    OSFreeToHeap(__OSCurrHeap, ptr);
}

void *__sys_alloc(unsigned int size) {
    InitDefaultHeap();
    return OSAllocFromHeap(__OSCurrHeap, size);
}

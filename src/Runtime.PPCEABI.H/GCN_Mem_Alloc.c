/* MSL runtime default-heap allocator (GCN_mem_alloc.c). */
#include <dolphin/os.h>

void __sys_free(void *ptr);
void *__sys_alloc(unsigned int size);

/* The diagnostic strings keep their per-region data addresses. */
#if defined(VERSION_GUPJ8P)
#define INIT_ERROR_MESSAGE lbl_80514350
#define INIT_HEAP_MESSAGE lbl_80514388
const char lbl_80514350[] = "GCN_Mem_Alloc.c : InitDefaultHeap. No Heap Available\n";
const char lbl_80514388[] = "Metrowerks CW runtime library initializing default heap\n";
#elif defined(VERSION_GUPP8P)
#define INIT_ERROR_MESSAGE lbl_80514DC8
#define INIT_HEAP_MESSAGE lbl_80514E00
const char lbl_80514DC8[] = "GCN_Mem_Alloc.c : InitDefaultHeap. No Heap Available\n";
const char lbl_80514E00[] = "Metrowerks CW runtime library initializing default heap\n";
#else
#define INIT_ERROR_MESSAGE lbl_80513CB0
#define INIT_HEAP_MESSAGE lbl_80513CE8
const char lbl_80513CB0[] = "GCN_Mem_Alloc.c : InitDefaultHeap. No Heap Available\n";
const char lbl_80513CE8[] = "Metrowerks CW runtime library initializing default heap\n";
#endif

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
        arena_lo = (void *)(((unsigned int)arena_lo + 0x1F) & ~0x1F);
        arena_hi = (void *)((unsigned int)arena_hi & ~0x1F);
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

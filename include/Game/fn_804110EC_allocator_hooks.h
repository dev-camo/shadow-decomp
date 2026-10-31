#ifndef GAME_FN_804110EC_ALLOCATOR_HOOKS_H
#define GAME_FN_804110EC_ALLOCATOR_HOOKS_H

/* Replaceable memory functions at 0x804110EC-0x8041116C. The pack loader at 0x80410630 and the
 * other library code up to 0x8042166C allocate and free through them. The original names are
 * unknown; the descriptions are inferred from the code. */

/* The first hook (inferred signature): the game installs wrappers that pass their argument to the
 * allocator fn_80049B90, which CharCommand's operator new calls with the size. Nothing in the game
 * reads the installed hook. */
typedef void *(*AllocateHook)(unsigned long size);
/* Used by fn_80411134 in place of calloc. */
typedef void *(*AllocateClearedHook)(unsigned long count, unsigned long size);
/* Used by fn_804110FC in place of free. */
typedef void (*ReleaseHook)(void *block);

#ifdef __cplusplus
extern "C" {
#endif

/* Installs the hooks; a null hook leaves the standard function in use. */
void fn_804110EC(AllocateHook allocate, AllocateClearedHook allocateCleared, ReleaseHook release);
/* free through the installed hook. */
void fn_804110FC(void *block);
/* calloc through the installed hook. */
void *fn_80411134(unsigned long count, unsigned long size);

#ifdef __cplusplus
}
#endif

#endif

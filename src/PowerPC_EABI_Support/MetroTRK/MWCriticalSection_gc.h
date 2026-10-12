#ifndef TRK_MWCRITICALSECTION_GC_H
#define TRK_MWCRITICALSECTION_GC_H

/* Critical sections for the TRK on GameCube (MWCriticalSection_gc.cpp): the section word holds the
 * interrupt state saved by MWEnterCriticalSection. */

#ifdef __cplusplus
extern "C" {
#endif

void MWInitializeCriticalSection(unsigned int *section);
void MWEnterCriticalSection(unsigned int *section);
void MWExitCriticalSection(unsigned int *section);

#ifdef __cplusplus
}
#endif

#endif /* TRK_MWCRITICALSECTION_GC_H */

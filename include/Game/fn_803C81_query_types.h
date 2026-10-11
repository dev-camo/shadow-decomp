#ifndef GAME_FN_803C81_QUERY_TYPES_H
#define GAME_FN_803C81_QUERY_TYPES_H

#include "Game/fn_803C82_record_types.h"

/* Query-compatible alias of the shared accessed-field view. */
typedef Fn803C82RecordView Fn803C81QueryView;

#ifdef __cplusplus
extern "C" {
#endif

/* Chosen consumed-pointer and result projections; original full interfaces unknown. */
unsigned int LSC_GetNumStm(const Fn803C81QueryView *object);
int LSC_GetStat(const Fn803C81QueryView *object);
int fn_804030A8(const Fn803C81QueryView *object);
/* Chosen side-effect projection; original full arity/types/return unknown. */
void fn_804030D4(void);

#ifdef __cplusplus
}
#endif

#endif

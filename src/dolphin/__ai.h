#ifndef _DOLPHIN_AI_INTERNAL_H_
#define _DOLPHIN_AI_INTERNAL_H_

#include <dolphin/ai.h>
#include <dolphin/os.h>

#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    OSTime t_start;
    OSTime t1;
    OSTime t2;
    OSTime t3;
    OSTime t4;
    OSTime t_end;
} STRUCT_TIMELOG;
CHECK_SIZEOF(STRUCT_TIMELOG, 0x30);

void __AI_DEBUG_set_stream_sample_rate(u32 rate);
STRUCT_TIMELOG *__ai_src_get_time(void);

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_AI_INTERNAL_H_

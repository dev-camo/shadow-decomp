#ifndef FN_8040_STREAM_OBJECT_H
#define FN_8040_STREAM_OBJECT_H

#include "CRI/lsc.h"

/* Object handed through fn_80406F08 -> fn_8040773C -> fn_8040307C (each passes the same
 * pointer on). Only the fields those functions touch are known; "stream" is inferred from
 * the MWSTM and LSC handles it holds. */
typedef struct Fn8040StreamObject {
    /* 0x000 */ unsigned char padding0[0x40];
    /* 0x040 */ void *field_40; /* MWSTM handle */
    /* 0x044 */ unsigned char padding44[0x4];
    /* 0x048 */ LSC lsc;
    /* 0x04C */ unsigned char padding4C[0x1C8 - 0x4C];
    /* 0x1C8 */ int field_1C8;
} Fn8040StreamObject;

#ifdef __cplusplus
extern "C" {
#endif

void fn_8040307C(Fn8040StreamObject *object, Sint32 min_val);
void fn_80406F08(Fn8040StreamObject *object);
void fn_8040773C(Fn8040StreamObject *object, int min_nsct, int max_nsct);

#ifdef __cplusplus
}
#endif

#endif

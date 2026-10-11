#ifndef GAME_FN_803CB_STATE_TYPES_H
#define GAME_FN_803CB_STATE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* CRI svm.c: calls the border hook of a server type (CRI Sint32 is long). */
void SVM_GotoSvrBorder(long svtype);

#ifdef __cplusplus
}
#endif

#endif

#ifndef GAME_FN_803CB_TEXT_TYPES_H
#define GAME_FN_803CB_TEXT_TYPES_H

/* Chosen context-word/text callback projection; original full interface unknown. */
typedef void (*Fn803CBTextCallback)(unsigned int argument, const char *text);

#ifdef __cplusplus
extern "C" {
#endif

/* Chosen text interfaces; original full types/arity/returns unknown. */
void SVM_CallErr1(const char *text);
void SVM_CallErr(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif

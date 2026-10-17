#ifndef NMWEXCEPTION_H
#define NMWEXCEPTION_H

#include "__ppc_eabi_linker.h"
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef short vbase_ctor_arg_type;
typedef char local_cond_type;

/* One registered global object; the entry storage is provided by the caller. */
typedef struct DestructorChain {
    /* 0x00 */ struct DestructorChain *next;
    /* 0x04 */ void (*destructor)(void *object, int flags);
    /* 0x08 */ void *object;
} DestructorChain;
CHECK_SIZEOF(DestructorChain, 0xC);

void __unregister_fragment(int fragmentID);
int __register_fragment(struct __eti_init_info *info, char *TOC);
void *__register_global_object(void *object, void (*destructor)(void *, int),
                               DestructorChain *regmem);
void __destroy_global_chain(void);

#ifdef __cplusplus
}
#endif

#endif /* NMWEXCEPTION_H */

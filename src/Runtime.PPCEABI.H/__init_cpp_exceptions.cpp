/* CodeWarrior runtime C++ exception setup (__init_cpp_exceptions.cpp): registers the exception
 * tables at startup and installs the init/fini entries in .ctors/.dtors. */
#include "NMWException.h"

extern "C" {
static int fragmentID = -2;

void __fini_cpp_exceptions(void) {
    if (fragmentID != -2) {
        __unregister_fragment(fragmentID);
        fragmentID = -2;
    }
}

// The original init passes the caller's TOC from r2 as the second argument.
static inline char *get_toc(void) {
    register char *toc;
    asm {
        mr toc, r2
    }
    return toc;
}

void __init_cpp_exceptions(void) {
    if (fragmentID == -2) {
        fragmentID = __register_fragment(_eti_init_info, get_toc());
    }
}

// MWCC's section attributes place these function pointers in the original tables;
// force_active retains the otherwise unreferenced entries through linking.
#pragma force_active on
#pragma section ".ctors$10"
__declspec(section ".ctors$10") extern void (*const __init_cpp_exceptions_reference)(void) =
    __init_cpp_exceptions;

#pragma section ".dtors$10"
__declspec(section ".dtors$10") extern void (*const __destroy_global_chain_reference)(void) =
    __destroy_global_chain;

#pragma section ".dtors$15"
__declspec(section ".dtors$15") extern void (*const __fini_cpp_exceptions_reference)(void) =
    __fini_cpp_exceptions;
#pragma force_active reset
}

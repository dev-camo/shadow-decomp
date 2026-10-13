/* MSL C++ std::bad_exception destructor (inferred: it installs the bad_exception vtable, then the
 * exception vtable, and frees the object when flags > 0). Names are per-region placeholders. */
#if defined(VERSION_GUPJ8P)
#define BAD_EXCEPTION_DTOR fn_803A239C
#define BAD_EXCEPTION_VTABLE lbl_80568480
#define EXCEPTION_VTABLE lbl_8051CB44
#elif defined(VERSION_GUPP8P)
#define BAD_EXCEPTION_DTOR fn_803A2DAC
#define BAD_EXCEPTION_VTABLE lbl_80568EC0
#define EXCEPTION_VTABLE lbl_8051D5C4
#else
#define BAD_EXCEPTION_DTOR fn_803A1D5C
#define BAD_EXCEPTION_VTABLE lbl_80567E00
#define EXCEPTION_VTABLE lbl_8051C4A4
#endif

extern char BAD_EXCEPTION_VTABLE[];
extern char EXCEPTION_VTABLE[];
/* operator delete(void *), by its C++ link name */
extern void __dl__FPv(void *self);

void *BAD_EXCEPTION_DTOR(void *self, short flags) {
    if (self != 0) {
        *(char **)self = BAD_EXCEPTION_VTABLE;
        if (self != 0) {
            *(char **)self = EXCEPTION_VTABLE;
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

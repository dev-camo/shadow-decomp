/* MSL C++ std::bad_exception destructor, by its C++ link name (slot 2 of the vtable whose RTTI
 * record names std::bad_exception): installs the bad_exception vtable, then the std::exception
 * vtable (the std::exception destructor inlined, hence the repeated null test), and frees the
 * object when flags > 0.
 *
 * Matching workaround: C stand-in for the C++ destructor. Written as a C++ class with a virtual
 * destructor deriving from std::exception, MWCC generates this function with identical code but
 * also emits the bad_exception vtable inside the unit, while the original keeps it outside this
 * unit's split. Revisit when the unit that owns the vtable is split.
 */
extern char __vt__Q23std13bad_exception[];
extern char __vt__Q23std9exception[];
/* operator delete(void *), by its C++ link name */
extern void __dl__FPv(void *self);

void *__dt__Q23std13bad_exceptionFv(void *self, short flags) {
    if (self != 0) {
        *(char **)self = __vt__Q23std13bad_exception;
        if (self != 0) {
            *(char **)self = __vt__Q23std9exception;
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

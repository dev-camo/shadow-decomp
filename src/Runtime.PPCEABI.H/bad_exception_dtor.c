/* MSL C++ std::bad_exception destructor, by its C++ link name (slot 2 of the vtable whose RTTI
 * record names std::bad_exception): installs the bad_exception vtable, then the std::exception
 * vtable, and frees the object when flags > 0. */
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

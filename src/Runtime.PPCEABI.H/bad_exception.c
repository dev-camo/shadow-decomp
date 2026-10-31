/* MSL C++ std::bad_exception::what(), by its C++ link name (slot 3 of the vtable whose RTTI record
 * names std::bad_exception). The returned "bad_exception" lies in a literal pool outside this unit
 * that still carries per-region placeholder names.
 *
 * Matching workaround: C stand-in for the C++ member function, for the same reason as the
 * destructor in bad_exception_dtor.c (a C++ definition makes the unit own the vtable and RTTI,
 * which the original keeps outside this unit's split). Revisit when the vtable's unit is split.
 */
#if defined(VERSION_GUPJ8P)
#define BAD_EXCEPTION_NAME lbl_80514330
#elif defined(VERSION_GUPP8P)
#define BAD_EXCEPTION_NAME lbl_80514DA8
#else
#define BAD_EXCEPTION_NAME lbl_80513C90
#endif

extern const char BAD_EXCEPTION_NAME[];

const char *what__Q23std13bad_exceptionCFv(const void *self) {
    return BAD_EXCEPTION_NAME;
}

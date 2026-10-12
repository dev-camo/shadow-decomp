/* MSL C++ std::bad_exception::what() (inferred from the "bad_exception" string it returns and its
 * vtable slot). The function and string still carry per-region placeholder names. */
#if defined(VERSION_GUPJ8P)
#define BAD_EXCEPTION_WHAT fn_803A2F78
#define BAD_EXCEPTION_NAME lbl_80514330
#elif defined(VERSION_GUPP8P)
#define BAD_EXCEPTION_WHAT fn_803A3988
#define BAD_EXCEPTION_NAME lbl_80514DA8
#else
#define BAD_EXCEPTION_WHAT fn_803A2938
#define BAD_EXCEPTION_NAME lbl_80513C90
#endif

extern const char BAD_EXCEPTION_NAME[];

const char *BAD_EXCEPTION_WHAT(void) {
    return BAD_EXCEPTION_NAME;
}

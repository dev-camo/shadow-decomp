extern void fn_804030F8(const char *format, ...);

/* Forwards the format string to fn_804030F8; `context` is unused and its type is inferred. */
void fn_80406E88(void *context, const char *format) {
    fn_804030F8(format);
}

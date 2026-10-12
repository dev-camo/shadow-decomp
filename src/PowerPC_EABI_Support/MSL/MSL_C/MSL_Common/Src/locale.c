#include <cstring>
#include <locale>

struct lconv __lconv = {
    ".",  "",   "",   "",   "", "",   "",   "",   "",   0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, "", 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
};

/* Dead-stripped in this game; only its "C" literal remains, after __lconv's strings in the pool. */
char *setlocale(int category, const char *locale) {
    if (locale == NULL || *locale == '\0' || strcmp(locale, "C") == 0) {
        return "C";
    }

    return NULL;
}

struct lconv *localeconv(void) {
    return &__lconv;
}

#include <cstring>
#include <locale>

struct lconv __lconv = {
    ".",  "",   "",   "",   "", "",   "",   "",   "",   0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, "", 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
};

/* Matching workaround: order-only stand-in. setlocale is stripped from the game (not in the DOL);
   this standard MSL body is unverified against the original and is kept only for its "C" literal,
   which follows __lconv's strings in the string pool. Revisit when the original setlocale is
   found. */
char *setlocale(int category, const char *locale) {
    if (locale == NULL || *locale == '\0' || strcmp(locale, "C") == 0) {
        return "C";
    }

    return NULL;
}

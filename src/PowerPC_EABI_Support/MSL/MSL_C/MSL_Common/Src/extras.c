/* MSL string case conversion (function name unknown). */
#include <cctype>

char *fn_803B1F68(char *string) {
    char *cursor = string;

    while (*cursor) {
        *cursor = toupper(*cursor);
        ++cursor;
    }
    return string;
}

/* Converts a string to uppercase in place (behaviour inferred; original name and filename unknown). */
#include <cctype>

char *fn_803B1F68(char *string) {
    char *cursor = string;

    while (*cursor) {
        *cursor = toupper(*cursor);
        ++cursor;
    }
    return string;
}

#ifndef MSL_STRING_H
#define MSL_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

/* MSL string.c functions. MSL spells the count parameter size_t; it stays
 * unsigned int here, which matches the existing matched code.
 */
char *strcpy(char *destination, const char *source);
char *strcat(char *destination, const char *source);
int strncmp(const char *left, const char *right, unsigned int count);
char *strncpy(char *destination, const char *source, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif

#ifndef GAME_PJSEVENTCT_H
#define GAME_PJSEVENTCT_H

/* "_CT", the tag in a file name that a two-digit number follows. The unrecovered event code also
 * searches names for it (fn_801720D8 and fn_801726A0). */
extern const char *lbl_805ED3D0;

extern "C" {
/* Searches name for "_CT" up to its first '.' (a '.' in the first character does not count). With
 * the tag it returns (c1 - '0') * 10 + (c2 - '0') for the two characters c1 and c2 after it, or 0
 * without the tag. The two characters are read without checking the length or that they are
 * digits ("_CTXY" gives 441). The first "_CT" found is used: when it ends the name, the second
 * character read lies one byte past the copy's allocation; with one character after it, the
 * second character read is the copy's terminator. name must not be null or empty: the function
 * passes a null pointer on to strstr, and its loop over the characters assumes at least one
 * (behaviour from its body; the parameter's const qualifier is inferred). */
int fn_80329524(const char *name);
}

#endif

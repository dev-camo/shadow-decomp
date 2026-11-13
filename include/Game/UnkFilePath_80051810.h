#ifndef GAME_UNKFILEPATH_80051810_H
#define GAME_UNKFILEPATH_80051810_H

/* The names, the parameter types and the const qualifiers are not known (inferred); the behaviour
 * comes from the bodies (each calls one worker, fn_80051218 and fn_800511D8). */
extern "C" {
/* Returns a copy of path in memory from the allocator that fn_80480530 returns, or null when the
 * allocation fails. The copy skips a leading '.', then '/', then '\', then ':' (each at most once,
 * in that order) and has every '\' and ':' it still contains replaced by '/'. */
char *fn_80051810(const char *path);
/* Gives a copy that fn_80051810 returned back to that allocator (nothing when path is null). */
void fn_80051830(char *path);
}

#endif

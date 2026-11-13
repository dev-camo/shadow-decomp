#ifndef GAME_UNKMANAGER_80014B90_H
#define GAME_UNKMANAGER_80014B90_H

/* The object that fn_80014B90 returns (placeholder name): a table of 128 records of 0x38 bytes,
 * followed at 0x1C00 by the index of the record to try next. Its constructor (0x8025D240) clears
 * the table and that index. Only the two functions below are recovered, so its members are not
 * declared. The addresses are those of USA (GUPE8P); in Japan (GUPJ8P) and PAL (GUPP8P) the two
 * functions also call fn_80050C40 before the search and fn_80050BF8 after it. */
class UnkManager_80014B90;

extern "C" {
/* Returns the table (behaviour inferred): constructs the function-local static object at
 * 0x8056EF08 on first use and caches its address in a static pointer. */
UnkManager_80014B90 *fn_80014B90(void);
/* Takes the first free record, starting at the index at 0x1C00: sets its byte at 0x0 and stores
 * owner at 0x4 and arg2 at 0x8. Returns true, or false when all 128 records are taken (behaviour
 * from its body; no caller uses the result, so its type is inferred from the 1 and 0 it returns).
 * The body does not read name. Every caller passes a string for it ("Player", "File Loading",
 * "EventManager", ...) and 0 or 1 for arg2, which the body stores as a word (its int type is
 * inferred); the callers pass objects of several classes as owner, and the body only stores the
 * pointer. */
bool fn_8025D1C4(UnkManager_80014B90 *manager, void *owner, int arg2, const char *name);
/* Frees the record whose pointer at 0x4 is owner: clears its byte at 0x0 and returns true, or
 * returns false when no record in use holds owner (behaviour from its body; the result type is
 * inferred as for fn_8025D1C4). */
bool fn_8025D144(UnkManager_80014B90 *manager, void *owner);
}

#endif

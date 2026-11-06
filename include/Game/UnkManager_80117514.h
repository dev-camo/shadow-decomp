#ifndef GAME_UNKMANAGER_80117514_H
#define GAME_UNKMANAGER_80117514_H

/* The object that fn_80117514 returns (placeholder name; the original class and function names are
 * unknown). fn_80117514 constructs it on first use (0x802E5B2C), registers its destructor and
 * caches its address in a static pointer (behaviour inferred). */
class UnkManager_80117514;

extern "C" {
UnkManager_80117514 *fn_80117514(void);
/* True when the float at 0x18 of the object that the manager's first word points to equals 0
 * (behaviour inferred from its body; meaning unknown). */
bool fn_802E5810(UnkManager_80117514 *manager);
}

#if !defined(VERSION_GUPE8P)
/* Japan and PAL only (0x80342284 in Japan, 0x80342AB0 in PAL; the USA build has no copy): returns
 * the word at 0x2C of each of two objects that two static pointers hold, or 0 for a null pointer,
 * combined with a bitwise or (behaviour inferred from its body; meaning unknown). */
extern "C" int fn_80342284(void);
#endif

#endif

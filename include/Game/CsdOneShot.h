#ifndef GAME_CSDONESHOT_H
#define GAME_CSDONESHOT_H

/* CsdOneShot (class name from RTTI, with Task as its base): a task that the constructor at
 * 0x803533C0 builds from three strings (placeholder declaration; its layout is not mapped). */
class CsdOneShot;

extern "C" {
/* Allocates a 0x138-byte CsdOneShot under task layer 3, passing the three strings to its
 * constructor, and returns it (null when the allocation fails); returns null without allocating
 * when a check on file fails (behaviour inferred from its body; the name file is inferred, and the
 * meanings of arg1 and arg2 are unknown). The HEAVY_DOG_0410 boss passes "CsdFiles/BChara_HDog",
 * "bchara" and "open". */
CsdOneShot *fn_8035367C(const char *file, const char *arg1, const char *arg2);
}

#endif

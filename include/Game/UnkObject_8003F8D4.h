#ifndef GAME_UNKOBJECT_8003F8D4_H
#define GAME_UNKOBJECT_8003F8D4_H

namespace Chao {
namespace CSD {
class CScene;
template <class T> class RCPtr;
} // namespace CSD
} // namespace Chao

/* The object that fn_8003F8D4 returns (placeholder name: the original class is unknown). */
class UnkObject_8003F8D4;

extern "C" {
/* Returns the object (behaviour inferred): constructs a function-local static object on first
 * use, registers its destructor and caches its address in a static pointer. */
UnkObject_8003F8D4 *fn_8003F8D4(void);
/* Formats the file name of the CSD project name ("%s.GNCP") into a static buffer and returns the
 * buffer (behaviour inferred from its body, which reads neither object nor arg2; the callers pass
 * 1 as arg2). */
const char *fn_8004237C(UnkObject_8003F8D4 *object, const char *name, int arg2);
/* Formats name with "%s.gncp" into a static buffer of its own and returns the buffer (behaviour
 * from its body, which reads neither object nor arg2; the callers pass 0 as arg2). */
const char *fn_800423B8(UnkObject_8003F8D4 *object, const char *name, int arg2);
}

/* A scene pointer that holds no scene (placeholder name): fn_800424A8 assigns it a pointer made
 * from a null scene, and the game's files compare the scenes they look up with it. */
extern Chao::CSD::RCPtr<Chao::CSD::CScene> lbl_805EE118;

#endif

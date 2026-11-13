#ifndef GAME_UNKOBJECT_8003F8D4_H
#define GAME_UNKOBJECT_8003F8D4_H

namespace Chao {
namespace CSD {
class CProject;
class CScene;
template <class T> class RCPtr;
} // namespace CSD
} // namespace Chao

struct RwTexDictionary;

/* The object that fn_8003F8D4 returns (placeholder name: the original class is unknown). */
class UnkObject_8003F8D4;

extern "C" {
/* Returns the object (behaviour inferred): constructs a function-local static object on first
 * use, registers its destructor and caches its address in a static pointer. */
UnkObject_8003F8D4 *fn_8003F8D4(void);
/* When project and dict are both non-null, creates a texture list of type '.RW2' (0x2E525732),
 * loads dict into it through the list's virtual function at 0x14 and, when that returns non-zero,
 * gives the list to the project with SetTexList. Returns 0 (behaviour from its body; the return
 * type is inferred). It takes no object: TGiPause calls fn_8003F8D4 before each call and discards
 * the result, as a call of a static member function through the object compiles (inferred). */
int fn_800421BC(Chao::CSD::CProject *project, RwTexDictionary *dict);
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

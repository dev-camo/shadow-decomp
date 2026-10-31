#ifndef GAME_CSD_CFILESYSTEM_H
#define GAME_CSD_CFILESYSTEM_H

#include <cstddef>
#include <dolphin/types.h>
#include "Game/UnkString_8004F888.h"
#include "layout_check.h"

namespace Chao {
namespace CSD {

/* The library's interface to the target's files. Reads go through a directory the library keeps
 * (the base directory every name is relative to). The class name is inferred from the other
 * interfaces of the library (CPlatform, CMemoryAlloc). Its implementation has the vtable
 * 0x8055FBB0 and the interface's own is 0x8055FBC4: the two first slots are pure and the
 * destructor, declared third, is inline (its out-of-line copy for the interface's vtable is
 * 0x80362D54). The class has no base: both deleting destructors free through the global operator
 * delete, not CBase's, and 0x80362D54 destroys the directory with the string's destructor
 * (fn_80014EC0). Slot names are inferred from the implementation, which opens the file at the path
 * with fopen's mode "rb". Only the vtable pointer and the directory at 0x4 are mapped. */
class CFileSystem {
  public:
    /* The size of the file at path (the name with the library directory prefixed). */
    virtual u32 GetFileSize(const char *path) = 0;
    /* Reads the file at path (the name with the library directory prefixed) into buffer. The
     * implementation returns 1, or 0 when the file does not open (an int success flag; the type
     * is inferred). */
    virtual int ReadFile(void *buffer, const char *path) = 0;
    virtual ~CFileSystem() {}

    /* Copy the directory out and replace it. SetDirectory drops a trailing slash or backslash. */
    char *GetDirectory(char *buffer, size_t size);
    void SetDirectory(const char *directory);
    /* As the virtual slots, with name relative to the directory. */
    int ReadFileInDirectory(void *buffer, const char *name);
    u32 GetFileSizeInDirectory(const char *name);

  private:
    /* name prefixed with the directory and a slash, when there is a directory. */
    void JoinPath(char *path, const char *name);

    /* 0x04 */ UnkString_8004F888 m_directory;
};
CHECK_SIZEOF(Chao::CSD::CFileSystem, 0x10);

/* The current file system. SetFileSystem ignores null and returns the previous one; the others
 * go through it (names inferred); ReadFile returns the slot's success flag. */
CFileSystem *SetFileSystem(CFileSystem *fileSystem);
char *GetDirectory(char *buffer, size_t size);
void SetDirectory(const char *directory);
int ReadFile(void *buffer, const char *name);
u32 GetFileSize(const char *name);

} // namespace CSD
} // namespace Chao

#endif

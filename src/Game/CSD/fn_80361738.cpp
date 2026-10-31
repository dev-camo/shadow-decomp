/* The Chao::CSD library's current file system and the directory its file names are relative to.
 * The original file name is unknown. Definitions are in reverse address order (the library is
 * compiled with deferred inlining, which emits the last definition first). */
#include "Game/CSD/CFileSystem.h"
#include <cstdio>
#include <cstring>

namespace Chao {
namespace CSD {

static CFileSystem *s_pFileSystem;

CFileSystem *SetFileSystem(CFileSystem *fileSystem) {
    if (fileSystem != NULL) {
        CFileSystem *previous = s_pFileSystem;
        s_pFileSystem = fileSystem;
        return previous;
    }
    return s_pFileSystem;
}

u32 GetFileSize(const char *name) {
    return s_pFileSystem->GetFileSizeInDirectory(name);
}

int ReadFile(void *buffer, const char *name) {
    return s_pFileSystem->ReadFileInDirectory(buffer, name);
}

/* Whether the string differs from the text. Its owner and real name are unknown, so it keeps its
 * placeholder symbol. */
extern "C" bool fn_80361970(const UnkString_8004F888 *string, const char *text) {
    return fn_8003C864(string, text) != 0;
}

void CFileSystem::JoinPath(char *path, const char *name) {
    if (fn_80361970(&m_directory, "")) {
        const char *directory = fn_80014E50(&m_directory);
        sprintf(path, "%s/%s", directory, name);
    } else {
        strcpy(path, name);
    }
}

u32 CFileSystem::GetFileSizeInDirectory(const char *name) {
    char path[512];
    JoinPath(path, name);
    return GetFileSize(path);
}

int CFileSystem::ReadFileInDirectory(void *buffer, const char *name) {
    char path[512];
    JoinPath(path, name);
    return ReadFile(buffer, path);
}

void CFileSystem::SetDirectory(const char *directory) {
    char path[512];
    strcpy(path, directory);
    switch (path[strlen(path) - 1]) {
    case '/':
    case '\\':
        path[strlen(path) - 1] = '\0';
        break;
    }
    fn_80014CF4(&m_directory, path);
}

char *CFileSystem::GetDirectory(char *buffer, size_t size) {
    return strncpy(buffer, fn_80014E50(&m_directory), size);
}

void SetDirectory(const char *directory) {
    s_pFileSystem->SetDirectory(directory);
}

char *GetDirectory(char *buffer, size_t size) {
    return s_pFileSystem->GetDirectory(buffer, size);
}

} // namespace CSD
} // namespace Chao

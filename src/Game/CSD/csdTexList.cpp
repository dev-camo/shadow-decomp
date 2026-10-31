/* csdTexList.cpp (original name from the library's allocation calls): the texture-list base class
 * and the registry of texture-list types. Function names are inferred. Definitions are in reverse
 * address order (deferred inlining emits the last definition first). */
#include "Game/CSD/CTexList.h"
#include "Game/CSD/CFileSystem.h"
#include <cstring>
#include <map>

namespace Chao {
namespace CSD {

typedef std::map<u32, CTexList *> TexListMap;

static TexListMap *s_pTexLists;

void InitTexLists() {
    s_pTexLists = new TexListMap;
}

void TermTexLists() {
    if (s_pTexLists != NULL) {
        delete s_pTexLists;
        s_pTexLists = NULL;
    }
}

void RegisterTexList(CTexList *texList) {
    TexListMap::iterator it = s_pTexLists->find(texList->GetType());
    if (it != s_pTexLists->end()) {
        s_pTexLists->erase(it);
    }
    s_pTexLists->insert(std::make_pair(texList->GetType(), texList));
}

CTexList::~CTexList() {}

RCPtr<CTexList> CreateTexList(u32 type) {
    TexListMap::iterator it = s_pTexLists->find(type);
    if (it == s_pTexLists->end()) {
        return RCPtr<CTexList>(NULL);
    }
    return it->second->Create();
}

int CTexList::LoadFile(const char *path) {
    u32 size = GetFileSize(path);
    void *buffer = Alloc(size, __FILE__, __LINE__);
    ReadFile(buffer, path);
    char saved[256];
    GetDirectory(saved, sizeof(saved));
    const char *slash = strrchr(path, '/');
    if (slash == NULL) {
        slash = strrchr(path, '\\');
    }
    if (slash != NULL) {
        char directory[256];
        int length = slash - path;
        strncpy(directory, path, length);
        directory[length] = '\0';
        SetDirectory(directory);
    }
    int result = Load(buffer, size);
    Free(buffer);
    SetDirectory(saved);
    return result;
}

int CTexList::LoadInDirectory(const void *data, u32 size, const char *directory) {
    char saved[256];
    GetDirectory(saved, sizeof(saved));
    if (directory != NULL) {
        SetDirectory(directory);
    }
    int result = Load(data, size);
    SetDirectory(saved);
    return result;
}

} // namespace CSD
} // namespace Chao

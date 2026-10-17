/* csdTexList.cpp (original name from the library's allocation calls): the texture-list base class
 * and the registry of texture-list types. Function names are inferred. Definitions are in reverse
 * address order (deferred inlining emits the last definition first). */
#include "Game/CSD/CTexList.h"
#include <cstring>
#include <map>

/* Undecompiled callees in the library's file-system unit: get and set the current directory, read
 * a file into a buffer and get a file's size (behaviour inferred). */
extern "C" char *fn_80361738(char *buffer, int size);
extern "C" void fn_80361768(const char *directory);
extern "C" void fn_80361998(void *buffer, const char *path);
extern "C" u32 fn_803619C8(const char *path);

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
    u32 size = fn_803619C8(path);
    void *buffer = Alloc(size, __FILE__, __LINE__);
    fn_80361998(buffer, path);
    char saved[256];
    fn_80361738(saved, sizeof(saved));
    const char *slash = strrchr(path, '/');
    if (slash == NULL) {
        slash = strrchr(path, '\\');
    }
    if (slash != NULL) {
        char directory[256];
        int length = slash - path;
        strncpy(directory, path, length);
        directory[length] = '\0';
        fn_80361768(directory);
    }
    int result = Load(buffer, size);
    Free(buffer);
    fn_80361768(saved);
    return result;
}

int CTexList::LoadInDirectory(const void *data, u32 size, const char *directory) {
    char saved[256];
    fn_80361738(saved, sizeof(saved));
    if (directory != NULL) {
        fn_80361768(directory);
    }
    int result = Load(data, size);
    fn_80361768(saved);
    return result;
}

} // namespace CSD
} // namespace Chao

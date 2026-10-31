/* csdAccess.cpp (original name from the library's allocation calls): the lookups in a project's
 * file data (csdAccess.h). Function, variable and helper names are inferred. Definitions are in
 * reverse address order (deferred inlining emits the last definition first). */
#include "Game/CSD/csdAccess.h"
#include "Game/CSD/Matrix.h"
#include "Game/CSD/csdBase.h"
#include <cstdlib>
#include <cstring>

namespace Chao {
namespace CSD {

/* The default value of a channel, an integer or a float (zero until SetUpChannelDefaults). */
class ChannelValue {
  public:
    ChannelValue() : asInt(0) {}

    union {
        s32 asInt;
        f32 asFloat;
    };
};

CHECK_SIZEOF(ChannelValue, 0x4);

static int s_defaultsReady;
static ChannelValue s_defaults[kEntryChannelCount];

/* The last request of FindStoredIndex: the number of stored channels below the channel, for the
 * mask. A request for the same mask and the same or a later channel resumes counting there. */
static int s_cachedCount;
static int s_cachedChannel;
static u32 s_cachedMask;

/* The mask of all channels, built when the defaults are set up. */
static u32 s_fullChannelMask;

/* Functions that are used above their definition. */
static SceneData *FindScene(const char *name, const SceneFolder *folder, const char **foundName);
static SceneData *FindSceneByPath(char *path, const SceneFolder *folder, const char **foundName);
static SceneFolder *FindFolderByName(const char *name, const SceneFolder *folder);
static SceneFolder *FindFolderByPath(char *path, const SceneFolder *folder);
static void RemoveFirstComponent(char *path);

static inline void SetUpChannelDefaults() {
    if (s_defaultsReady) {
        return;
    }
    s_defaults[0].asInt = 0;
    s_defaults[1].asFloat = 0.0f;
    s_defaults[2].asFloat = 0.0f;
    s_defaults[3].asFloat = 0.0f;
    s_defaults[4].asFloat = 1.0f;
    s_defaults[5].asFloat = 1.0f;
    s_defaults[6].asFloat = 0.0f;
    s_defaults[7].asInt = -1;
    s_defaults[8].asInt = -1;
    s_defaults[9].asInt = -1;
    s_defaults[10].asInt = -1;
    s_defaults[11].asInt = -1;
    s_defaults[12].asInt = 0;
    s_defaults[13].asInt = 0;
    s_defaults[14].asInt = 0;
    s_defaultsReady = 1;
    for (int channel = 0; channel < kEntryChannelCount; channel++) {
        s_fullChannelMask |= 1 << channel;
    }
}

u32 GetFullChannelMask() {
    return s_fullChannelMask;
}

/* Returns the position of the channel's value among the stored values, or -1 if the channel is
 * not stored. */
static inline int FindStoredIndex(int channel, u32 channelMask) {
    if (channelMask == 0) {
        return -1;
    }

    int index = 0;
    int bit = 0;
    if (s_cachedMask == channelMask && s_cachedChannel <= channel) {
        index = s_cachedCount;
        bit = s_cachedChannel;
    }
    for (; bit < kEntryChannelCount; bit++) {
        if (bit == channel) {
            s_cachedCount = index;
            s_cachedChannel = channel;
            s_cachedMask = channelMask;
            if (channelMask & (1 << bit)) {
                return index;
            }
            return -1;
        }
        if (channelMask & (1 << bit)) {
            index++;
        }
    }
    s_cachedMask = 0;
    return -1;
}

void GetChannelValue(u32 *value, int channel, u32 channelMask, const u32 *values) {
    int index = FindStoredIndex(channel, channelMask);

    if (index < 0) {
        SetUpChannelDefaults();
        *value = s_defaults[channel].asInt;
        return;
    }
    *value = values[index];
}

static int CompareNodeNames(const void *a, const void *b) {
    return strcmp(static_cast<const NodeEntry *>(a)->name, static_cast<const NodeEntry *>(b)->name);
}

int CompareNames(const void *a, const void *b) {
    return strcmp(static_cast<const NameEntry *>(a)->name, static_cast<const NameEntry *>(b)->name);
}

void ResetEntry(MotionPaletteEntry *entry) {
    SetUpChannelDefaults();
    memset(entry, 0, sizeof(MotionPaletteEntry));
    SetMatrixIdentity(&entry->matrix);
    entry->field_0x24 = -1;
    EntryChannelValues &values = entry->channels.named;
    values.scale.x = 1.0f;
    values.scale.y = 1.0f;
    values.pattern = -100000.0f;
    values.tint = -1;
    values.colors[0] = values.colors[1] = values.colors[2] = values.colors[3] = -1;
    entry->textWidth = -1.0f;
    entry->charSpacing = -1000000.0f;
}

SceneData *GetSceneByName(const char *name, const ProjectData *project, const char **foundName) {
    if (foundName != NULL) {
        *foundName = NULL;
    }
    return FindScene(name, project->root, foundName);
}

/* Matching workaround: the original expands the recursion of FindScene and FindFolderByName once
 * into the function and keeps the out-of-line copies, before their callers; inline declarations
 * emit them after. Revisit if the original's inline helpers are found. */
#pragma auto_inline on
static SceneData *FindScene(const char *name, const SceneFolder *folder, const char **foundName) {
    NameEntry key;
    key.name = name;
    key.index = -1;
    const NameEntry *entry = static_cast<const NameEntry *>(
        bsearch(&key, folder->sceneNames, folder->sceneCount, sizeof(NameEntry), CompareNames));

    if (entry != NULL) {
        if (foundName != NULL) {
            *foundName = entry->name;
        }
        return folder->scenes[entry->index];
    }
    for (int i = 0; i < folder->folderCount; i++) {
        SceneData *scene = FindScene(name, &folder->folders[i], foundName);
        if (scene != NULL) {
            return scene;
        }
    }
    return NULL;
}
#pragma auto_inline reset

const NodeEntry *FindNode(const char *name, const SceneData *scene) {
    NodeEntry key;
    key.name = name;
    key.field_0x4 = -1;
    key.field_0x8 = -1;
    const NodeEntry *entry = static_cast<const NodeEntry *>(
        bsearch(&key, scene->nodes, scene->nodeCount, sizeof(NodeEntry), CompareNodeNames));
    /* Matching workaround: the test gives the original's 0x5C bytes; revisit on a natural form. */
    if (entry != NULL) {
        return entry;
    }
    return NULL;
}

SceneData *GetSceneByPath(const char *path, const ProjectData *project, const char **foundName) {
    if (foundName != NULL) {
        *foundName = NULL;
    }
    /* The path is copied because the lookup removes its components. The copy is not released when
     * the first component is not the project's name. */
    char *copy = static_cast<char *>(Alloc(strlen(path) + 1, __FILE__, __LINE__));
    strcpy(copy, path);
    const char *separator = strchr(copy, '/');
    if (strncmp(copy, project->name, separator - copy) != 0) {
        return NULL;
    }
    RemoveFirstComponent(copy);
    SceneData *scene = FindSceneByPath(copy, project->root, foundName);
    Free(copy);
    return scene;
}

/* Copies the first component of the path (up to the first '/') to the buffer. */
static void CopyFirstComponent(char *buffer, const char *path) {
    const char *separator = strchr(path, '/');
    int length = separator - path;

    strncpy(buffer, path, length);
    buffer[length] = '\0';
}

/* Removes the first component of the path and its '/' from the path itself. A copy of the path is
 * made and released without being read, and the remainder is copied within the path (overlapping
 * strcpy), as in the original code. If the copy cannot be allocated the path is emptied. */
static void RemoveFirstComponent(char *path) {
    char *copy = static_cast<char *>(Alloc(strlen(path) + 1, __FILE__, __LINE__));

    if (copy == NULL) {
        strcpy(path, "");
        return;
    }
    strcpy(copy, path);
    strcpy(path, strchr(path, '/') + 1);
    Free(copy);
}

static int HasSeparator(const char *path) {
    return strchr(path, '/') != NULL;
}

static SceneData *FindSceneByPath(char *path, const SceneFolder *folder, const char **foundName) {
    if (HasSeparator(path)) {
        char *component = static_cast<char *>(Alloc(strlen(path) + 1, __FILE__, __LINE__));
        if (component == NULL) {
            return NULL;
        }
        CopyFirstComponent(component, path);
        NameEntry key;
        key.name = component;
        key.index = -1;
        const NameEntry *entry = static_cast<const NameEntry *>(bsearch(
            &key, folder->folderNames, folder->folderCount, sizeof(NameEntry), CompareNames));
        Free(component);
        if (entry != NULL) {
            RemoveFirstComponent(path);
            return FindSceneByPath(path, &folder->folders[entry->index], foundName);
        }
        return NULL;
    }

    NameEntry key;
    key.name = path;
    key.index = -1;
    const NameEntry *entry = static_cast<const NameEntry *>(
        bsearch(&key, folder->sceneNames, folder->sceneCount, sizeof(NameEntry), CompareNames));
    if (entry != NULL) {
        if (foundName != NULL) {
            *foundName = folder->sceneNames[entry->index].name;
        }
        return folder->scenes[entry->index];
    }
    return NULL;
}

SceneFolder *GetFolderByName(const char *name, const ProjectData *project) {
    return FindFolderByName(name, project->root);
}

SceneFolder *GetFolderByPath(const char *path, const ProjectData *project) {
    /* The path is copied because the lookup removes its components. The copy is not released when
     * the first component is not the project's name. */
    char *copy = static_cast<char *>(Alloc(strlen(path) + 1, __FILE__, __LINE__));
    strcpy(copy, path);
    const char *separator = strchr(copy, '/');
    if (strncmp(copy, project->name, separator - copy) != 0) {
        return NULL;
    }
    RemoveFirstComponent(copy);
    SceneFolder *folder = FindFolderByPath(copy, project->root);
    Free(copy);
    return folder;
}

#pragma auto_inline on // Matching workaround: as for FindScene (once-expanded recursion)
static SceneFolder *FindFolderByName(const char *name, const SceneFolder *folder) {
    NameEntry key;
    key.name = name;
    key.index = -1;
    const NameEntry *entry = static_cast<const NameEntry *>(
        bsearch(&key, folder->folderNames, folder->folderCount, sizeof(NameEntry), CompareNames));

    if (entry != NULL) {
        return &folder->folders[entry->index];
    }
    for (int i = 0; i < folder->folderCount; i++) {
        SceneFolder *found = FindFolderByName(name, &folder->folders[i]);
        if (found != NULL) {
            return found;
        }
    }
    return NULL;
}
#pragma auto_inline reset

static SceneFolder *FindFolderByPath(char *path, const SceneFolder *folder) {
    if (HasSeparator(path)) {
        char *component = static_cast<char *>(Alloc(strlen(path) + 1, __FILE__, __LINE__));
        if (component == NULL) {
            return NULL;
        }
        CopyFirstComponent(component, path);
        NameEntry key;
        key.name = component;
        key.index = -1;
        const NameEntry *entry = static_cast<const NameEntry *>(bsearch(
            &key, folder->folderNames, folder->folderCount, sizeof(NameEntry), CompareNames));
        Free(component);
        if (entry != NULL) {
            RemoveFirstComponent(path);
            return FindFolderByPath(path, &folder->folders[entry->index]);
        }
        return NULL;
    }

    NameEntry key;
    key.name = path;
    key.index = -1;
    const NameEntry *entry = static_cast<const NameEntry *>(
        bsearch(&key, folder->folderNames, folder->folderCount, sizeof(NameEntry), CompareNames));
    if (entry != NULL) {
        return &folder->folders[entry->index];
    }
    return NULL;
}

int FindNameIndex(const NameEntry *names, int count, const char *name) {
    NameEntry key;
    key.name = name;
    key.index = -1;
    const NameEntry *entry = static_cast<const NameEntry *>(
        bsearch(&key, names, count, sizeof(NameEntry), CompareNames));

    if (entry != NULL) {
        return entry->index;
    }
    return -1;
}

} // namespace CSD
} // namespace Chao

#ifndef GAME_CSD_CSDACCESS_H
#define GAME_CSD_CSDACCESS_H

#include <dolphin/types.h>
#include "Game/CSD/MotionPalette.h"
#include "layout_check.h"

/* The lookups csdAccess.cpp performs in a project's file data: scenes and the folders that group
 * them are found by name or by a '/'-separated path, and the animation channels of an element
 * fall back to defaults. Type, member and function names are inferred from that code. */
namespace Chao {
namespace CSD {

/* A folder of a project: scenes and sub-folders, each with a name table. */
struct SceneFolder {
    /* 0x00 */ s32 sceneCount;
    /* 0x04 */ SceneData **scenes;
    /* 0x08 */ NameEntry *sceneNames;
    /* 0x0C */ s32 folderCount;
    /* 0x10 */ SceneFolder *folders;
    /* 0x14 */ NameEntry *folderNames;
};
CHECK_SIZEOF(Chao::CSD::SceneFolder, 0x18);

/* The file data of a project (only the members used so far). The first path component
 * of a project-level lookup is the project's name. The name is from the game's RTTI: CProject
 * derives from CResourceBase<Chao::CSD::Project>, whose data is this. */
struct Project {
    /* 0x00 */ SceneFolder *root;
    /* 0x04 */ const char *name;
    /* The key of the texture-list type the project's textures need (four characters, as
     * CTexList::GetType returns; name inferred from CProject::LoadFile). */
    /* 0x08 */ u32 texListType;
    /* 0x0C */ const FontList *fonts;
};
CHECK_SIZEOF(Chao::CSD::Project, 0x10);

/* Orders name table entries by name, for searching them with bsearch. */
int CompareNames(const void *a, const void *b);

/* Returns the index of the name in a sorted name table, or -1. */
int FindNameIndex(const NameEntry *names, int count, const char *name);

/* The lookups by path take a path with at least one '/': the project's name, the names of the
 * folders below it and the name of the folder or scene to find. The lookups by name search the
 * folders recursively (names are inferred). */
SceneFolder *GetFolderByPath(const char *path, const Project *project);
SceneFolder *GetFolderByName(const char *name, const Project *project);

/* The name table's own string of the scene is stored in *foundName unless that is NULL (*foundName
 * is NULL when the scene is not found). GetSceneByPath takes the name table entry at the scene's
 * index, which is the scene's own entry only when the scenes are stored in name order (as the
 * original does). */
SceneData *GetSceneByPath(const char *path, const Project *project, const char **foundName);
SceneData *GetSceneByName(const char *name, const Project *project, const char **foundName);

/* Looks a node up in a scene's node table. */
const NodeEntry *FindNode(const char *name, const SceneData *scene);

/* Resets an element to the state of a newly created one. */
void ResetEntry(MotionPaletteEntry *entry);

/* Animation values (the meaning is inferred) have up to 15 channels, and a block of them stores
 * values only for the channels of a bit mask. GetFullChannelMask is the mask of all channels (0
 * until the defaults of the channels have been set up, which ResetEntry does and GetChannelValue
 * does when it needs a default). GetChannelValue stores the raw value of a channel: the stored one
 * (values has one word for each channel of the mask, in channel order) or the channel's default. */
u32 GetFullChannelMask();
void GetChannelValue(u32 *value, int channel, u32 channelMask, const u32 *values);

} // namespace CSD
} // namespace Chao

#endif

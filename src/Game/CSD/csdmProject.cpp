/* Project resources and scene collections (original file name: csdmProject.cpp).
 * Definitions with established native identities follow descending original addresses;
 * deferred inlining emits the last definition first. */
#include <cstring>
#include "Game/CSD/csdmProject.h"
#include "Game/CSD/CFileSystem.h"
#include "Game/CSD/CPlatform.h"
#include "Game/CSD/RenderState.h"

namespace Chao {
namespace CSD {

/* The text reported when no registered texture list handles the project's type: "No texture
 * system that handles [type] was found." (in Japanese in the original); the arguments are the
 * four characters of the type. */
#define NO_TEXLIST_MESSAGE "[%c%c%c%c]を処理するテクスチャシステムが見つかりません。\n"

CSceneListener::~CSceneListener() {
    std::list<CScene *>::iterator scene;
    for (scene = m_scenes.begin(); scene != m_scenes.end();) {
        (*scene++)->fn_80365958(this);
    }
}

void CMotionEndPolicy::OnReachEnd(CScene *scene) {
    switch (m_endAction) {
    case MOTION_END_PAUSE:
        scene->Pause();
        break;
    case MOTION_END_WRAP:
        scene->SetFrame(scene->GetMotionStart());
        break;
    case MOTION_END_REVERSE:
        scene->SetSpeed(-1.0f * scene->GetSpeed());
        break;
    case MOTION_END_REMOVE:
        scene->m_project->RemoveScene(scene);
        break;
    }
}

void CMotionEndPolicy::OnReachStart(CScene *scene) {
    switch (m_startAction) {
    case MOTION_END_PAUSE:
        scene->Pause();
        break;
    case MOTION_END_WRAP:
        scene->SetFrame(scene->GetMotionEnd());
        break;
    case MOTION_END_REVERSE:
        scene->SetSpeed(-1.0f * scene->GetSpeed());
        break;
    case MOTION_END_REMOVE:
        scene->m_project->RemoveScene(scene);
        break;
    }
}

void CSceneList::fn_8036F4F0(const RCPtr<CScene> &scene) {
    m_scenes.insert(std::pair<const char *, RCPtr<CScene> >(scene->fn_803674F8(), scene));
}

RCPtr<CScene> CSceneList::fn_8036F470(const char *name) {
    SceneMap::iterator scene;
    scene = m_scenes.find(name);
    if (scene != m_scenes.end()) {
        return scene->second;
    }
    return RCPtr<CScene>(NULL);
}

bool SceneNameCompare::operator()(const char *const &left, const char *const &right) const {
    return strcmp(left, right) < 0;
}

CProject::~CProject() {
    NameMap::iterator scene;
    for (scene = m_scenes.begin(); scene != m_scenes.end();) {
        (scene++)->second.Get()->m_project = NULL;
    }
    if (m_fonts != NULL) {
        delete m_fonts;
        m_fonts = NULL;
    }
}

void CProject::Copy(const CResourceBase<Project> &other) {
    /* The native Copy requires another CProject: it also reads its texture and font fields. */
    const CProject &project = static_cast<const CProject &>(other);

    CResourceBase<Project>::Copy(other);
    m_texList = project.m_texList;
    if (m_fonts != NULL) {
        delete m_fonts;
        m_fonts = NULL;
    }
    if (project.m_fonts != NULL) {
        m_fonts = CSD_NEW CFontList(*project.m_fonts);
    }
}

void CProject::fn_8036E39C() {
    m_fonts = NULL;
}

int CProject::LoadFile(const char *path) {
    int result = 1;

    if (m_data != NULL) {
        return 0;
    }
    u32 size = GetFileSize(path);
    if (size == 0) {
        return 0;
    }
    u32 *file = static_cast<u32 *>(Alloc(size, __FILE__, __LINE__));
    ReadFile(file, path);

    void *projectPart;
    void *texturePart;
    u32 projectSize;
    u32 textureSize;
    Project *project;

    ExtractFileParts(&projectPart, &projectSize, &texturePart, &textureSize, file, size);
    LoadProject(&project, projectPart);
    if (texturePart != NULL) {
        char savedDirectory[256];

        GetDirectory(savedDirectory, sizeof(savedDirectory));

        const char *slash = strrchr(path, '/');
        if (slash == NULL) {
            slash = strrchr(path, '\\');
        }
        if (slash != NULL) {
            char *directory = static_cast<char *>(Alloc(strlen(path) + 1, __FILE__, __LINE__));
            int length = slash - path;

            strncpy(directory, path, length);
            directory[length] = '\0';
            SetDirectory(directory);
            Free(directory);
        }

        RCPtr<CTexList> texList = CreateTexList(project->texListType);
        if (texList.Get() != NULL) {
            result = texList->Load(texturePart, textureSize);
            if (result > 0) {
                SetTexList(texList);
            }
        } else {
            const char *type = reinterpret_cast<const char *>(&project->texListType);

            if (GetPlatform()->fn_80363BE8()) {
                PrintError(NO_TEXLIST_MESSAGE, type[0], type[1], type[2], type[3]);
            } else {
                PrintError(NO_TEXLIST_MESSAGE, type[3], type[2], type[1], type[0]);
            }
        }
        Free(texturePart);
        SetDirectory(savedDirectory);
    }
    Attach(project, static_cast<unsigned char *>(projectPart), RCPtr<unsigned char>::FreeRelease());
    Free(file);
    return result;
}

void CFontListRef::Attach(const FontList *fonts, const RCPtr<unsigned char> &ref) {
    Clear();
    m_ref = ref;
    m_fonts = fonts;
}

int CProject::Load(const u32 *file, u32 size, const char *directory) {
    int result = 1;
    void *projectPart;
    void *texturePart;
    u32 projectSize;
    u32 textureSize;
    Project *project;

    ExtractFileParts(&projectPart, &projectSize, &texturePart, &textureSize, file, size);
    LoadProject(&project, projectPart);
    Attach(project, static_cast<unsigned char *>(projectPart), RCPtr<unsigned char>::FreeRelease());
    if (texturePart != NULL) {
        char savedDirectory[256];

        GetDirectory(savedDirectory, sizeof(savedDirectory));
        if (directory != NULL) {
            SetDirectory(directory);
        }

        RCPtr<CTexList> texList = CreateTexList(project->texListType);
        if (texList.Get() != NULL) {
            result = texList->Load(texturePart, textureSize);
            if (result) {
                SetTexList(texList);
            }
        } else {
            /* The original reads the characters of the type through the type's value taken as a
             * pointer (LoadFile reads them from the project data). */
            const char *type = reinterpret_cast<const char *>(project->texListType);

            if (GetPlatform()->fn_80363BE8()) {
                PrintError(NO_TEXLIST_MESSAGE, type[0], type[1], type[2], type[3]);
            } else {
                PrintError(NO_TEXLIST_MESSAGE, type[3], type[2], type[1], type[0]);
            }
        }
        Free(texturePart);
        SetDirectory(savedDirectory);
    }
    return result;
}

void CProject::fn_8036DAD4() {
    if (m_data != NULL) {
        fn_8036C3AC();
        fn_8036CB58();
        OrderMap::iterator scene;
        for (scene = m_drawOrder.begin(); scene != m_drawOrder.end(); ++scene) {
            scene->second->fn_80366F9C();
        }
    }
}

void CProject::Update(f32 dt) {
    if (m_data != NULL) {
        fn_8036C3AC();
        fn_8036CB58();
        OrderMap::iterator scene;
        for (scene = m_drawOrder.begin(); scene != m_drawOrder.end(); ++scene) {
            scene->second->fn_803669B8(dt);
        }
    }
}

void CProject::Draw() {
    if (m_data != NULL && m_texList.Get() != NULL) {
        GetRenderState().Enter();
        OrderMap::iterator scene;
        for (scene = m_drawOrder.begin(); scene != m_drawOrder.end(); ++scene) {
            scene->second->fn_803667A4(m_texList);
        }
        GetRenderState().Leave();
    }
}

RCPtr<CScene> CProject::CreateScene(const char *name, int motion,
                                    UnkSceneCreator_8036C0C4 *creator) {
    if (m_data == NULL) {
        return RCPtr<CScene>(NULL);
    }
    RCPtr<CScene> scene = fn_8036C0C4(name, creator);
    if (scene.Get() != NULL) {
        scene->SetMotion(motion);
    }
    return scene;
}

RCPtr<CScene> CProject::fn_8036D7D4(const char *name, const char *motion,
                                    UnkSceneCreator_8036C0C4 *creator) {
    if (m_data == NULL) {
        return RCPtr<CScene>(NULL);
    }
    RCPtr<CScene> scene = fn_8036C0C4(name, creator);
    if (scene.Get() != NULL) {
        scene->SetMotion(motion);
    }
    return scene;
}

void CProject::fn_8036D700(CScene *scene) {
    RCPtr<CScene> reference = fn_8036CCEC(scene);
    if (reference.Get() != NULL) {
        m_removals.push_back(reference);
    }
}

void CProject::RemoveScene(CScene *scene) {
    fn_8036D700(scene);
}

RCPtr<CScene> CProject::GetScene(const char *name) {
    std::pair<NameMap::iterator, NameMap::iterator> range = m_scenes.equal_range(name);
    if (range.first != range.second) {
        return range.first->second;
    }
    return RCPtr<CScene>(NULL);
}

RCPtr<CSceneList> CProject::fn_8036D598(UnkSceneCreator_8036C0C4 *creator) {
    if (m_data == NULL) {
        return RCPtr<CSceneList>(NULL);
    }
    RCPtr<CSceneList> scenes(CSD_NEW CSceneList, RCPtr<CSceneList>::DeleteRelease());
    fn_8036D238(m_data->root, scenes, creator);
    return scenes;
}

CSceneList::CSceneList() {}

RCPtr<CSceneList> CProject::fn_8036D2FC(const char *name, UnkSceneCreator_8036C0C4 *creator) {
    if (m_data == NULL) {
        return RCPtr<CSceneList>(NULL);
    }
    SceneFolder *folder;
    if (strchr(name, '/') != NULL) {
        folder = GetFolderByPath(name, m_data);
    } else {
        folder = GetFolderByName(name, m_data);
    }
    if (folder == NULL) {
        return RCPtr<CSceneList>(NULL);
    }
    RCPtr<CSceneList> scenes(CSD_NEW CSceneList, RCPtr<CSceneList>::DeleteRelease());
    fn_8036D238(folder, scenes, creator);
    return scenes;
}

void CProject::fn_8036D238(SceneFolder *folder, const RCPtr<CSceneList> &scenes,
                           UnkSceneCreator_8036C0C4 *creator) {
    for (int i = 0; i < folder->sceneCount; ++i) {
        fn_8036D1B0(folder->scenes[folder->sceneNames[i].index], folder->sceneNames[i].name, scenes,
                    creator);
    }
    for (int i = 0; i < folder->folderCount; ++i) {
        fn_8036D238(&folder->folders[i], scenes, creator);
    }
}

void CProject::fn_8036D1B0(const SceneData *data, const char *name, const RCPtr<CSceneList> &scenes,
                           UnkSceneCreator_8036C0C4 *creator) {
    RCPtr<CScene> scene = fn_8036BCAC(data, name, scenes, creator);
    scene->SetMotion(0);
}

void CProject::fn_8036D0BC(const RCPtr<CScene> &scene) {
    m_scenes.insert(std::pair<const char *, RCPtr<CScene> >(scene->fn_803674F8(), scene));
    m_drawOrder.insert(std::pair<f32, RCPtr<CScene> >(scene->fn_80365424(), scene));
}

void CProject::fn_8036D054(const RCPtr<CScene> &scene) {
    if (scene.Get()->m_project != NULL) {
        fn_8036CE94(scene);
        scene.Get()->m_project = NULL;
        fn_8036CDB0(scene);
    }
}

void CProject::fn_8036CE94(const RCPtr<CScene> &scene) {
    std::pair<NameMap::iterator, NameMap::iterator> range =
        m_scenes.equal_range(scene->fn_803674F8());
    for (; range.first != range.second; ++range.first) {
        if (range.first->second == scene) {
            m_scenes.erase(range.first);
            break;
        }
    }
}

void CProject::fn_8036CDB0(const RCPtr<CScene> &scene) {
    std::pair<OrderMap::iterator, OrderMap::iterator> range =
        m_drawOrder.equal_range(scene->fn_80365424());
    for (; range.first != range.second; ++range.first) {
        if (range.first->second == scene) {
            m_drawOrder.erase(range.first);
            break;
        }
    }
}

RCPtr<CScene> CProject::fn_8036CCEC(CScene *scene) {
    std::pair<OrderMap::iterator, OrderMap::iterator> range =
        m_drawOrder.equal_range(scene->fn_80365424());
    for (; range.first != range.second; ++range.first) {
        if (&*range.first->second == scene) {
            return range.first->second;
        }
    }
    return RCPtr<CScene>(NULL);
}

CProject::iterator CProject::fn_8036CCA4() {
    return iterator(m_drawOrder.begin());
}

CProject::iterator CProject::fn_8036CC50() {
    return iterator(m_drawOrder.end());
}

void CProject::fn_8036CB58() {
    for (OrderChangeList::iterator change = m_orderChanges.begin(); change != m_orderChanges.end();
         ++change) {
        fn_8036C8E0(change);
    }
    m_orderChanges.clear();
}

void CProject::fn_8036C8E0(const OrderChangeList::iterator &change) {
    std::pair<OrderMap::iterator, OrderMap::iterator> range =
        m_drawOrder.equal_range(change->second);
    RCPtr<CScene> scene;
    for (; range.first != range.second; ++range.first) {
        if (&*range.first->second == change->first) {
            scene = range.first->second;
            m_drawOrder.erase(range.first);
            break;
        }
    }
    if (scene.Get() != NULL) {
        m_drawOrder.insert(std::pair<f32, RCPtr<CScene> >(scene->fn_80365424(), scene));
    }
}

void CProject::SetTexList(const RCPtr<CTexList> &texList) {
    m_texList = texList;
}

RCPtr<CTexList> CProject::GetTexList() const {
    return m_texList;
}

void CProject::SetFonts(const CFontListRef *fonts) {
    if (m_fonts != NULL) {
        delete m_fonts;
        m_fonts = NULL;
    }
    if (fonts != NULL) {
        m_fonts = CSD_NEW CFontList(*fonts);
    }
    for (NameMap::iterator scene = m_scenes.begin(); scene != m_scenes.end(); ++scene) {
        scene->second->fn_80364538(fonts);
    }
}

void CProject::fn_8036C3AC() {
    for (RemovalList::iterator scene = m_removals.begin(); scene != m_removals.end(); ++scene) {
        fn_8036D054(*scene);
    }
    m_removals.clear();
}

void CProject::RemoveAllScenes() {
    iterator scene;
    for (scene = fn_8036CCA4(); scene != fn_8036CC50(); ++scene) {
        RemoveScene(&*scene);
    }
}

RCPtr<CScene> CProject::fn_8036C0C4(const char *name, UnkSceneCreator_8036C0C4 *creator) {
    if (m_data == NULL) {
        return RCPtr<CScene>(NULL);
    }
    const char *key = NULL;
    const SceneData *data;
    if (strchr(name, '/') != NULL) {
        data = GetSceneByPath(name, m_data, &key);
    } else {
        data = GetSceneByName(name, m_data, &key);
    }
    if (data == NULL) {
        return RCPtr<CScene>(NULL);
    }
    RCPtr<CScene> scene(NULL);
    if (creator != NULL) {
        scene.Reset(creator->Create());
    } else {
        scene.Reset(CSD_NEW CScene, RCPtr<CScene>::DeleteRelease());
    }
    scene->fn_803648DC(data, m_ref, key, this, m_fonts);
    fn_8036D0BC(scene);
    return scene;
}

CScene::CScene() : m_objects(NULL) {
    fn_80367500();
}

RCPtr<CScene> CProject::fn_8036BCAC(const SceneData *data, const char *name,
                                    const RCPtr<CSceneList> &scenes,
                                    UnkSceneCreator_8036C0C4 *creator) {
    RCPtr<CScene> scene(NULL);
    if (creator != NULL) {
        scene.Reset(creator->Create());
    } else {
        scene.Reset(CSD_NEW CScene, RCPtr<CScene>::DeleteRelease());
    }
    scene->fn_803648DC(data, m_ref, name, this, m_fonts);
    fn_8036D0BC(scene);
    scenes->fn_8036F4F0(scene);
    return scene;
}

} // namespace CSD
} // namespace Chao

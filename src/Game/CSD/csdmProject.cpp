/* csdmProject.cpp (original name from the library's allocation calls): the project object
 * (csdmProject.h). Class and function names are inferred. Definitions are in reverse address order
 * (deferred inlining emits the last definition first). */
#include <cstring>
#include "Game/CSD/csdmProject.h"
#include "Game/CSD/CFileSystem.h"
#include "Game/CSD/CPlatform.h"

namespace Chao {
namespace CSD {

/* The text reported when no registered texture list handles the project's type: "No texture
 * system that handles [type] was found." (in Japanese in the original); the arguments are the
 * four characters of the type. */
#define NO_TEXLIST_MESSAGE "[%c%c%c%c]を処理するテクスチャシステムが見つかりません。\n"

void CMotionEndPolicy::OnReachStart(CScene *scene) {
    switch (m_startAction) {
    case MOTION_END_PAUSE:
        scene->Pause();
        break;
    case MOTION_END_HOLD:
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

void CMotionEndPolicy::OnReachEnd(CScene *scene) {
    switch (m_endAction) {
    case MOTION_END_PAUSE:
        scene->Pause();
        break;
    case MOTION_END_HOLD:
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

void CProject::Copy(const CResourceBase<Project> &other) {
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

void CFontListRef::Attach(const FontList *fonts, const RCPtr<unsigned char> &ref) {
    Clear();
    m_ref = ref;
    m_fonts = fonts;
}

void CProject::SetTexList(const RCPtr<CTexList> &texList) {
    m_texList = texList;
}

RCPtr<CTexList> CProject::GetTexList() const {
    return m_texList;
}

} // namespace CSD
} // namespace Chao

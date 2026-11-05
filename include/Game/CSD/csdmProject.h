#ifndef GAME_CSD_CSDMPROJECT_H
#define GAME_CSD_CSDMPROJECT_H

#include "Game/CSD/CTexList.h"
#include "Game/CSD/csdAccess.h"
#include "Game/CSD/csdLoader.h"
#include "Game/CSD/csdmScene.h"

/* The project object of the library (csdmProject.h/.cpp, the original names from the library's
 * allocation calls): the file data of a CSD project, the scenes created from it and the texture
 * list they draw with. Class and function names are inferred, except CResourceBase and its
 * argument, whose RTTI name the game carries (CResourceBase<Chao::CSD::Project>, Project being
 * the project's file data). */
namespace Chao {
namespace CSD {

/* A handle of a resource with its file data: a counted reference to the block that holds the data
 * and the data's address. The other handles of the library (CSceneDataRef and the like) have this
 * shape; the game's RTTI names this one. Only the members that CProject uses are modelled. */
template <class T> class CResourceBase {
  public:
    CResourceBase();
    virtual ~CResourceBase();
    /* Shares the other handle's reference and copies its data address. */
    virtual void Copy(const CResourceBase &other);

    /* Takes the data and a block that holds it, to be released with Free. */
    void Attach(T *data, unsigned char *block, typename RCPtr<unsigned char>::FreeRelease release) {
        m_ref.Reset(block, release);
        m_data = data;
    }

    /* 0x04 */ RCPtr<unsigned char> m_ref;
    /* 0x08 */ T *m_data;
};
CHECK_SIZEOF(Chao::CSD::CResourceBase<Chao::CSD::Project>, 0xC);

/* A project: the data of a project file, the scenes created from it and the texture list and
 * fonts they use. The member containers are not modelled yet (they hold the scenes by name and by
 * draw order, and the scenes waiting to be added or removed). */
class CProject : public CResourceBase<Project>, public CBase {
  public:
    virtual ~CProject();
    virtual void Copy(const CResourceBase<Project> &other);

    /* Reads the project file at path (name inferred). The file's directory is the current one
     * while the textures load, and the project's texture list type must be registered (an error
     * is reported if it is not). Returns 0 when a project is already loaded or the file is
     * empty, otherwise the texture list's Load result when the file has textures and a texture
     * list for their type exists, or 1. */
    int LoadFile(const char *path);

    /* Loads a project from the contents of a project file in memory, with the textures found
     * in directory when it is given (name inferred). The contents stay the caller's: the
     * project keeps the parts that ExtractFileParts copies out of them. Returns the texture
     * list's Load result when the file has textures and a texture list for their type exists,
     * otherwise 1. */
    int Load(const u32 *file, u32 size, const char *directory);

    /* Takes the project data and the block that holds it: completes the data (NormalizeProject),
     * keeps the fonts as a CFontList for the scenes and sets them (name inferred). */
    void Attach(Project *project, unsigned char *block, RCPtr<unsigned char>::FreeRelease release) {
        NormalizeProject(project);
        CResourceBase<Project>::Attach(project, block, release);
        if (m_data->fonts != NULL) {
            CFontList *fonts = CSD_NEW CFontList(m_data->fonts, m_ref);

            SetFonts(fonts);
            delete fonts;
        }
    }

    /* Takes a scene out of the project when the next update runs (name inferred). */
    void RemoveScene(CScene *scene);

    /* The texture list the scenes are drawn with (names inferred). */
    RCPtr<CTexList> GetTexList() const;
    void SetTexList(const RCPtr<CTexList> &texList);

    /* Replaces the project's copy of the fonts (null removes it) and gives the fonts to every
     * scene (name inferred). */
    void SetFonts(const CFontListRef *fonts);

    /* Builds a project from a loaded project resource: constructs the members, clears the fonts
     * pointer and takes the resource's data with Copy (behaviour inferred from the game's copy at
     * 0x800BFF30 in the USA release, which every caller in the game uses). The game allocates
     * projects through CBase's operator new (AllocObject with csdBase.h and line 40) and the
     * deleting destructor frees them through FreeObject, while CResourceBase<Project>'s type record
     * names no base, so CBase is a base of CProject itself.
     * Matching workaround: the members below are declared after Attach so that the line its
     * allocation passes (CSD_NEW uses __LINE__) does not move. Revisit when the original header's
     * layout is known. */
    explicit CProject(const CResourceBase<Project> &resource);

    /* Takes every scene out of the project with RemoveScene (name inferred). */
    void RemoveAllScenes();

    /* The project's scene of that name, or a pointer that holds none (name inferred). */
    RCPtr<CScene> GetScene(const char *name);

    /* Creates the scene of that name (a path when it contains '/') from the project's data and
     * sets its motion to the given index; returns a pointer that holds none when no project data
     * is loaded or the data has no such scene (name inferred). creator, when given, supplies the
     * new scene through its first virtual function; otherwise the project builds it itself (its
     * class is unknown: the name is a placeholder declared by the parameter). */
    RCPtr<CScene> CreateScene(const char *name, int motion,
                              class UnkSceneCreator_8036C0C4 *creator);

    /* Advances every scene by dt (name inferred). */
    void Update(f32 dt);

    /* Draws the scenes in draw order with the project's texture list, between RenderState's
     * Enter and Leave, when project data and a texture list are present (name inferred). */
    void Draw();

    /* 0x0C */ u8 field_0xC[0x10];
    /* 0x1C */ u8 field_0x1C[0x10];
    /* 0x2C */ RCPtr<CTexList> m_texList;
    /* 0x30 */ CFontListRef *m_fonts;
    /* 0x34 */ u8 field_0x34[0xC];
    /* 0x40 */ u8 field_0x40[0xC];
};
CHECK_OFFSETOF(Chao::CSD::CProject, m_texList, 0x2C);
CHECK_OFFSETOF(Chao::CSD::CProject, m_fonts, 0x30);
CHECK_SIZEOF(Chao::CSD::CProject, 0x4C);

} // namespace CSD
} // namespace Chao

#endif

#ifndef GAME_CSD_CSDMPROJECT_H
#define GAME_CSD_CSDMPROJECT_H

#include "Game/CSD/CTexList.h"
#include "Game/CSD/csdAccess.h"
#include "Game/CSD/csdLoader.h"
#include "Game/CSD/csdmScene.h"

/* The project object of the library (csdmProject.h/.cpp, the original names from the library's
 * allocation calls): the file data of a CSD project, the scenes created from it and the texture
 * list they draw with. Class and function names are inferred, except CResourceBase, Project and
 * CSceneList, whose names occur in the game's RTTI. Project is the project's file data. */
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

/* String ordering shared by the project's name index and CSceneList (name inferred). */
struct SceneNameCompare {
    bool operator()(const char *const &left, const char *const &right) const;
};
CHECK_SIZEOF(Chao::CSD::SceneNameCompare, 0x1);

class CSceneList;

/* The supplied scene factory's observed interface. Only its first virtual slot is known;
 * the method name is inferred from the returned scene. */
class UnkSceneCreator_8036C0C4 {
  public:
    virtual CScene *Create() = 0;
};
CHECK_SIZEOF(Chao::CSD::UnkSceneCreator_8036C0C4, 0x4);

/* A project: its file data, scenes indexed by name and draw order, textures and fonts.
 * With project data, each update first applies queued removals and draw-order changes. */
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

    /* Builds from another CProject passed through its resource base: constructs the members,
     * clears the fonts pointer and calls Copy (behaviour inferred from the game's copy at
     * 0x800BFF30 in the USA release, which every caller in the game uses). The game allocates
     * projects through CBase's operator new (AllocObject with csdBase.h and line 40) and the
     * deleting destructor frees them through FreeObject, while CResourceBase<Project>'s type record
     * names no base, so CBase is a base of CProject itself. */
    explicit CProject(const CResourceBase<Project> &resource);

    /* Builds an empty project: constructs the members and calls fn_8036E39C (behaviour from the
     * game's copy at 0x8003E808 in the USA release, which every caller in the game uses). */
    CProject();

    /* Takes every scene out of the project with RemoveScene (name inferred). */
    void RemoveAllScenes();

    /* The project's scene of that name, or a pointer that holds none (name inferred). */
    RCPtr<CScene> GetScene(const char *name);

    /* Creates the named scene (a path when name contains '/') and requests motion with SetMotion.
     * SetMotion ignores the current index and indices at or above the scene's motion count.
     * Returns an empty handle when project data or the named scene is absent (names inferred).
     * creator, when given, supplies the new scene through its first virtual function; otherwise
     * the project constructs it. */
    RCPtr<CScene> CreateScene(const char *name, int motion, UnkSceneCreator_8036C0C4 *creator);

    /* With project data, forwards dt to each scene's virtual update in draw order.
     * The normal scene update applies its data scaling, pause, speed and frame bounds. */
    void Update(f32 dt);

    /* Draws the scenes in draw order with the project's texture list, between RenderState's
     * Enter and Leave, when project data and a texture list are present (name inferred). */
    void Draw();

    /* The two indexes allow repeated names and draw-order values. Comparator and member names
     * are inferred from the native insertion, strcmp and float comparisons. */
    typedef std::multimap<const char *, RCPtr<CScene>, SceneNameCompare> NameMap;
    typedef std::multimap<f32, RCPtr<CScene> > OrderMap;
    typedef std::list<RCPtr<CScene> > RemovalList;
    typedef std::pair<CScene *, f32> OrderChange;
    typedef std::list<OrderChange> OrderChangeList;

    /* The public traversal wraps the order-map iterator and yields scenes, not map pairs
     * (class and operation names inferred). */
    class iterator {
      public:
        iterator() {}
        iterator(OrderMap::iterator position) : m_position(position) {}
        CScene &operator*() const {
            return *m_position->second;
        }
        iterator &operator++() {
            ++m_position;
            return *this;
        }
        bool operator!=(const iterator &other) const {
            return m_position != other.m_position;
        }

      private:
        OrderMap::iterator m_position;
    };

    iterator fn_8036CC50();
    iterator fn_8036CCA4();
    RCPtr<CScene> fn_8036CCEC(CScene *scene);
    void fn_8036CDB0(const RCPtr<CScene> &scene);
    void fn_8036CE94(const RCPtr<CScene> &scene);
    void fn_8036D054(const RCPtr<CScene> &scene);
    void fn_8036D0BC(const RCPtr<CScene> &scene);
    void fn_8036D700(CScene *scene);
    void fn_8036C3AC();
    void fn_8036C8E0(const OrderChangeList::iterator &change);
    void fn_8036CB58();
    void fn_8036E39C();
    void fn_8036DAD4();
    RCPtr<CScene> fn_8036C0C4(const char *name, UnkSceneCreator_8036C0C4 *creator);
    RCPtr<CScene> fn_8036BCAC(const SceneData *data, const char *name,
                              const RCPtr<CSceneList> &scenes, UnkSceneCreator_8036C0C4 *creator);
    RCPtr<CScene> fn_8036D7D4(const char *name, const char *motion,
                              UnkSceneCreator_8036C0C4 *creator);
    void fn_8036D1B0(const SceneData *data, const char *name, const RCPtr<CSceneList> &scenes,
                     UnkSceneCreator_8036C0C4 *creator);
    void fn_8036D238(SceneFolder *folder, const RCPtr<CSceneList> &scenes,
                     UnkSceneCreator_8036C0C4 *creator);
    RCPtr<CSceneList> fn_8036D2FC(const char *name, UnkSceneCreator_8036C0C4 *creator);
    RCPtr<CSceneList> fn_8036D598(UnkSceneCreator_8036C0C4 *creator);

    /* 0x0C */ NameMap m_scenes;
    /* 0x1C */ OrderMap m_drawOrder;
    /* 0x2C */ RCPtr<CTexList> m_texList;
    /* 0x30 */ CFontListRef *m_fonts;
    /* 0x34 */ RemovalList m_removals;
    /* 0x40 */ OrderChangeList m_orderChanges;
};
CHECK_OFFSETOF(Chao::CSD::CProject, m_texList, 0x2C);
CHECK_OFFSETOF(Chao::CSD::CProject, m_fonts, 0x30);
CHECK_SIZEOF(Chao::CSD::CProject, 0x4C);

CHECK_SIZEOF(Chao::CSD::CProject::NameMap, 0x10);
CHECK_SIZEOF(Chao::CSD::CProject::OrderMap, 0x10);
CHECK_SIZEOF(Chao::CSD::CProject::RemovalList, 0xC);
CHECK_SIZEOF(Chao::CSD::CProject::OrderChange, 0x8);
CHECK_SIZEOF(Chao::CSD::CProject::OrderChangeList, 0xC);
CHECK_SIZEOF(Chao::CSD::CProject::iterator, 0x4);

/* The named collection returned when creating a folder's scenes. The class name is from its
 * RCPtr holder's RTTI; the map and operation names are inferred from the native tree routines. */
class CSceneList : public CBase {
  public:
    typedef std::map<const char *, RCPtr<CScene>, SceneNameCompare> SceneMap;
    CSceneList();
    RCPtr<CScene> fn_8036F470(const char *name);
    void fn_8036F4F0(const RCPtr<CScene> &scene);

    /* 0x00 */ SceneMap m_scenes;
};
CHECK_SIZEOF(Chao::CSD::CSceneList, 0x10);
CHECK_SIZEOF(Chao::CSD::CSceneList::SceneMap, 0x10);

} // namespace CSD
} // namespace Chao

#endif

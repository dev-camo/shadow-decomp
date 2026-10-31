/* The texture list of RenderWare texture dictionaries that already exist (CTexListRW2, which
 * csdTexListRW.cpp installs). The original file name is unknown. Class and function names are
 * inferred. Definitions are in reverse address order (deferred inlining emits the last definition
 * first). */
#include "Game/CSD/CTexListRW.h"

namespace Chao {
namespace CSD {

/* The release policy this unit gives its holders (inferred). */
static RCPtr<CTexList>::DeleteRelease s_deleteRelease;

CTexListRW2::~CTexListRW2() {
    m_dict = NULL;
    if (m_textures != NULL) {
        delete m_textures;
        m_textures = NULL;
    }
}

/* The data argument is the dictionary itself (it is not owned by the texture list). */
int CTexListRW2::Load(const void *data, u32 size) {
    if (data == NULL) {
        return 0;
    }
    m_dict = static_cast<RwTexDictionary *>(const_cast<void *>(data));
    m_textures = new TexArray(m_dict);
    if (m_textures == NULL) {
        return 0;
    }
    return 1;
}

RCPtr<CTexList> CTexListRW2::Create() {
    return RCPtr<CTexList>(new CTexListRW2, s_deleteRelease);
}

u32 CTexListRW2::GetType() {
    return '.RW2';
}

} // namespace CSD
} // namespace Chao

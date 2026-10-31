/* csdTexListRW.cpp (the name in the library's allocation calls): the RenderWare implementations
 * of the library's interfaces (files, drawing, memory and texture lists) and the function that
 * installs them. Names are inferred except CPlatformRW, DrawPrimitive2D and VERTEX_BUFFER_SIZE,
 * which the assertion text gives. Definitions are in reverse address order (deferred inlining
 * emits the last definition first). */
#include "Game/CSD/CFileSystem.h"
#include "Game/CSD/CMemoryAlloc.h"
#include "Game/CSD/CPlatform.h"
#include "Game/CSD/CTexListRW.h"
#include "Game/CSD/MotionPalette.h"
#include "Game/CSD/Screen.h"
#include "Game/CSD/csdBase.h"
#include "Game/CSD/csdChunkTags.h"
#include "rw/rtfsyst.h"
#include "rw/rwcore.h"
#include <cstring>

namespace Chao {
namespace CSD {

/* The release policy the texture lists give their holders (inferred). */
static RCPtr<CTexList>::DeleteRelease s_deleteRelease;

/* The directory file names are relative to; it is never changed. */
static char s_rootDirectory[256] = "";

/* The RenderWare implementation of the library's file interface. It reads files through
 * RenderWare's file functions, with their names prefixed by s_rootDirectory. */
class CFileSystemRW : public CFileSystem {
  public:
    virtual u32 GetFileSize(const char *path);
    virtual int ReadFile(void *buffer, const char *path);
    virtual ~CFileSystemRW() {}
};

/* The RenderWare implementation of the library's drawing interface (the class name is in the
 * text of its assertion). The vertex buffer holds up to VERTEX_BUFFER_SIZE vertices; the render
 * states that OnBegin changes are saved in the members and restored by OnEnd. */
class CPlatformRW : public CPlatform {
  public:
    enum { VERTEX_BUFFER_SIZE = 4 };

    virtual ~CPlatformRW() {}
    virtual void Print(const char *message) {}
    virtual void fn_803632E4(int blendMode, const s32 *values, int valueCount);
    virtual void fn_803632A0(int mode, const s32 *values, int valueCount);
    virtual void DrawPrimitive2D(const Vertex *vertices, int vertexCount, const s32 *values,
                                 int valueCount);
    virtual void fn_80362E74(const s32 *values, int valueCount) {}
    virtual void fn_80363198(int mode, const s32 *values, int valueCount);
    virtual void fn_803630C8(const PlainVertex *vertices, int vertexCount, const s32 *values,
                             int valueCount);
    virtual void fn_80362E78(const s32 *values, int valueCount) {}
    virtual void OnBegin();
    virtual void OnEnd();

    /* 0x08 */ RwIm2DVertex m_vertices[VERTEX_BUFFER_SIZE];
    /* 0x68 */ s32 m_savedSrcBlend;
    /* 0x6C */ s32 m_savedDestBlend;
    /* 0x70 */ s32 m_savedVertexAlpha;
    /* 0x74 */ s32 m_savedZWrite;
    /* 0x78 */ s32 m_savedZTest;
    /* 0x7C */ s32 m_savedCullMode;
    /* 0x80 */ RwTexDictionary *m_savedDictionary;
};
CHECK_SIZEOF(Chao::CSD::CPlatformRW, 0x84);

/* The class derived from CPlatformRW that supplies the slot it leaves pure (name inferred). */
class CPlatformRWDerived : public CPlatformRW {
  public:
    virtual ~CPlatformRWDerived() {}
    virtual int fn_80363BE8() {
        return 1;
    }
};

/* The RenderWare implementation of the library's allocator: its memory comes from RenderWare's
 * allocation functions. */
class CMemoryAllocRW : public CMemoryAlloc {
  public:
    virtual ~CMemoryAllocRW() {}
    virtual void *Alloc(size_t size) {
        return RwMalloc(size, rwMEMHINTDUR_EVENT);
    }
    virtual void Free(void *ptr) {
        RwFree(ptr);
    }
};

/* Installs the RenderWare implementations (behaviour inferred), sets the screen size and fills in
 * the project file's tags. */
void InitLibraryRW(f32 width, f32 height) {
    static CPlatformRWDerived platform;
    SetPlatform(&platform);
    static CMemoryAllocRW memoryAlloc;
    SetMemoryAlloc(&memoryAlloc);
    InitLibrary();
    static CFileSystemRW fileSystem;
    SetFileSystem(&fileSystem);
    static CTexListRW texListRW;
    static CTexListRW2 texListRW2;
    RegisterTexList(&texListRW);
    RegisterTexList(&texListRW2);
    SetScreenSize(width, height);

    lbl_805859E0 = 'NGIF';
    lbl_805859E4 = 'NGTL';
    lbl_805859E8 = 'NGOB';
    lbl_805859EC = 'NGMO';
    lbl_805859F0 = 'NGMM';
    lbl_805859F4 = 'NGMC';
    lbl_805859F8 = 'NGML';
    lbl_805859FC = 'NGMA';
    lbl_80585A00 = 'NGLI';
    lbl_80585A04 = 'NGCA';
    lbl_80585A08 = 'NGMT';
    lbl_80585A0C = 'NGNN';
    lbl_80585A10 = 'NOF0';
    lbl_80585A14 = 'NEND';
}

/* Called for every texture of a dictionary: the first pass counts them, the second stores their
 * rasters through the cursor its data argument points at. */
static RwTexture *CountTexture(RwTexture *texture, void *count) {
    ++*static_cast<int *>(count);
    return texture;
}

static RwTexture *StoreTexture(RwTexture *texture, void *cursor) {
    RwRaster **&slot = *static_cast<RwRaster ***>(cursor);
    *slot = texture->raster;
    ++slot;
    return texture;
}

TexArray::TexArray(RwTexDictionary *dict) : m_rasters(NULL), m_count(0) {
    if (dict == NULL) {
        return;
    }
    fn_8048CBE0(dict, CountTexture, &m_count);
    m_rasters = static_cast<RwRaster **>(Alloc(m_count * sizeof(RwRaster *), __FILE__, __LINE__));
    RwRaster **first = m_rasters;
    if (first == NULL) {
        return;
    }
    fn_8048CBE0(dict, StoreTexture, &m_rasters);
    m_rasters = first;
}

TexArray::~TexArray() {
    if (m_rasters != NULL) {
        Free(m_rasters);
    }
}

CTexListRW::~CTexListRW() {
    /* The explicit test matters: a bare delete compiles to one branch fewer than the original. */
    if (m_textures != NULL) {
        delete m_textures;
    }
    if (m_dict != NULL) {
        fn_8048CB38(m_dict);
    }
}

int CTexListRW::Load(const void *data, u32 size) {
    if (size != 0) {
        RwMemory memory;
        memory.length = size;
        memory.start = const_cast<RwUInt8 *>(static_cast<const RwUInt8 *>(data));
        RwStream *stream = fn_80481368(rwSTREAMMEMORY, rwSTREAMREAD, &memory);
        if (stream != NULL) {
            if (fn_8047CF50(stream, rwID_TEXDICTIONARY, NULL, NULL)) {
                m_dict = fn_80483FBC(stream);
            }
            fn_8048124C(stream, &memory);
        }
        if (stream == NULL || m_dict == NULL) {
            return 0;
        }
    }
    m_textures = new TexArray(m_dict);
    if (m_textures == NULL) {
        return 0;
    }
    return 1;
}

/* A negative texture number selects no texture. */
void CTexListRW::fn_803637CC(int texture, const s32 *values, int valueCount) {
    if (texture < 0) {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
        return;
    }
    fn_8048D0C4(m_dict);
    if (m_textures != NULL) {
        RwRaster *raster = m_textures->Get(texture);
        if (raster != NULL) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
        }
    }
}

void CTexListRW::fn_80363724(int addressU, int addressV, const s32 *values, int valueCount) {
    int u;
    int v;

    switch (addressU) {
    case 0:
        u = rwTEXTUREADDRESSWRAP;
        break;
    case 1:
        u = rwTEXTUREADDRESSCLAMP;
        break;
    }
    switch (addressV) {
    case 0:
        v = rwTEXTUREADDRESSWRAP;
        break;
    case 1:
        v = rwTEXTUREADDRESSCLAMP;
        break;
    }
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSU, (void *)u);
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSV, (void *)v);
}

void CTexListRW::fn_803636C8(int texture, int filter, const s32 *values, int valueCount) {
    int rwFilter;

    switch (filter) {
    case 0:
        rwFilter = rwFILTERNEAREST;
        break;
    case 1:
        rwFilter = rwFILTERLINEAR;
        break;
    }
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFilter);
}

RCPtr<CTexList> CTexListRW::Create() {
    return RCPtr<CTexList>(new CTexListRW, s_deleteRelease);
}

u32 CTexListRW::GetType() {
    return '.RWT';
}

void CPlatformRW::fn_803632E4(int blendMode, const s32 *values, int valueCount) {
    int srcBlend;
    int destBlend;

    switch (blendMode) {
    case 0:
        srcBlend = rwBLENDSRCALPHA;
        destBlend = rwBLENDONE;
        break;
    case 1:
        srcBlend = rwBLENDSRCALPHA;
        destBlend = rwBLENDINVSRCALPHA;
        break;
    }
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)srcBlend);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)destBlend);
}

void CPlatformRW::fn_803632A0(int mode, const s32 *values, int valueCount) {
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)(mode == 1 ? 1 : 0));
}

void CPlatformRW::DrawPrimitive2D(const Vertex *vertices, int vertexCount, const s32 *values,
                                  int valueCount) {
    if (vertexCount > VERTEX_BUFFER_SIZE) {
        PrintError("libcsd_rw ASSERT: CPlatformRW::DrawPrimitive2D VERTEX_BUFFER_SIZE over\n");
        return;
    }
    for (int i = 0; i < vertexCount; i++) {
        RwIm2DVertex &out = m_vertices[i];
        out.x = vertices[i].x;
        out.y = vertices[i].y;
        u32 color = vertices[i].color;
        out.emissiveColor.red = color >> 24;
        out.emissiveColor.green = color >> 16;
        out.emissiveColor.blue = color >> 8;
        out.emissiveColor.alpha = color;
        out.u = vertices[i].u;
        out.v = vertices[i].v;
    }
    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_vertices, vertexCount);
}

void CPlatformRW::fn_80363198(int mode, const s32 *values, int valueCount) {
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)(mode == 1 ? 1 : 0));
}

void CPlatformRW::fn_803630C8(const PlainVertex *vertices, int vertexCount, const s32 *values,
                              int valueCount) {
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    if (vertexCount > VERTEX_BUFFER_SIZE) {
        return;
    }
    for (int i = 0; i < vertexCount; i++) {
        RwIm2DVertex &out = m_vertices[i];
        out.x = vertices[i].x;
        out.y = vertices[i].y;
        u32 color = vertices[i].color;
        out.emissiveColor.red = color >> 24;
        out.emissiveColor.green = color >> 16;
        out.emissiveColor.blue = color >> 8;
        out.emissiveColor.alpha = color;
    }
    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, m_vertices, vertexCount);
}

void CPlatformRW::OnBegin() {
    RwRenderStateGet(rwRENDERSTATESRCBLEND, &m_savedSrcBlend);
    RwRenderStateGet(rwRENDERSTATEDESTBLEND, &m_savedDestBlend);
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &m_savedVertexAlpha);
    RwRenderStateGet(rwRENDERSTATEZWRITEENABLE, &m_savedZWrite);
    RwRenderStateGet(rwRENDERSTATEZTESTENABLE, &m_savedZTest);
    RwRenderStateGet(rwRENDERSTATECULLMODE, &m_savedCullMode);
    m_savedDictionary = fn_8048D0D8();
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)0);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)0);
    RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
    /* The original keeps an empty counted loop of four whose body compiled to nothing. */
    for (int i = 0; i < 4; i++) {
    }
}

void CPlatformRW::OnEnd() {
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)m_savedSrcBlend);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)m_savedDestBlend);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)m_savedVertexAlpha);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)m_savedZWrite);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)m_savedZTest);
    RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)m_savedCullMode);
    fn_8048D0C4(m_savedDictionary);
}

u32 CFileSystemRW::GetFileSize(const char *path) {
    char fullPath[256];

    strcpy(fullPath, s_rootDirectory);
    strcat(fullPath, path);
    RtFile *file = static_cast<RtFile *>(fn_8047A178(fullPath, "rb"));
    if (file != NULL) {
        u32 length = file->length;
        fn_8047A388(file);
        return length;
    }
    return 0;
}

int CFileSystemRW::ReadFile(void *buffer, const char *path) {
    u32 size = GetFileSize(path);
    char fullPath[256];

    strcpy(fullPath, s_rootDirectory);
    strcat(fullPath, path);
    void *file = fn_8047A178(fullPath, "rb");
    if (file != NULL) {
        fn_8047A3E0(buffer, size, 1, file);
        fn_8047A388(file);
        return 1;
    }
    return 0;
}

} // namespace CSD
} // namespace Chao

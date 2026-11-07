/* RW3 particle methods; class identities come from RTTI, original filename unknown. */
#include "Game/EffectParticleRW3.h"
#include "Game/EffectParticle3DRW3.h"
#include "Game/EffectLaunchInfoRW3.h"
#include "Game/EffectManagerRW3.h"
#include "Game/UnkEffectFileData.h"
#include "Game/SharedConstants.h"
#include "rw/rpptank.h"

namespace Effect {
namespace RW3 {

/* Per-particle render storage, inferred from the vertex and clump operations. */
struct UnkEffectParticle3DRW3Data {
    /* 0x00 */ RwIm3DVertex vertices[4];
    /* 0x90 */ RpClump *clump;
    /* 0x94 */ unsigned char inWorld;
};
CHECK_SIZEOF(UnkEffectParticle3DRW3Data, 0x98);

EffectParticleRW3::EffectParticleRW3(const System::UnkEffectElementData2 *definition,
                                     System::EffectLaunchInfo *info, System::EffectManager *manager)
    : System::EffectParticle(definition, info, manager), m_atomic(0), m_frame(0), field_0x144(0) {
    m_size -= sizeof(System::EffectParticle);
    m_size += sizeof(EffectParticleRW3);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    m_atomic = static_cast<EffectManagerRW3 *>(m_manager)->fn_804324C4(m_maxCount);
    if (m_atomic == 0) {
        m_atomic = fn_80448074(m_maxCount,
                               rpPTANKDFLAGPOSITION | rpPTANKDFLAGCOLOR | rpPTANKDFLAGSIZE |
                                   rpPTANKDFLAG2DROTATE | rpPTANKDFLAGVTX2TEXCOORDS,
                               0);
    }
    if (m_atomic == 0) {
        m_flags &= ~System::ELEMENT_ACTIVE;
        return;
    }
    m_frame = fn_80486D00();
    if (m_frame == 0) {
        m_flags &= ~System::ELEMENT_ACTIVE;
        return;
    }
    fn_80458CBC(m_atomic, m_frame);
    PTANKEXT(m_atomic)->instFlags |= 0x800000;
    PTANKEXT(m_atomic)->actPCount = 0;
    PTANKEXT(m_atomic)->vertexAlphaBlend = TRUE;
    switch (definition->field_0x50) {
    case 0:
        if ((m_definitionFlags & 4) != 0) {
            PTANKEXT(m_atomic)->srcBlend = rwBLENDSRCALPHA;
            PTANKEXT(m_atomic)->dstBlend = rwBLENDINVSRCALPHA;
            PTANKEXT(m_atomic)->instFlags |= 0x10000000;
        } else {
            PTANKEXT(m_atomic)->srcBlend = rwBLENDONE;
            PTANKEXT(m_atomic)->dstBlend = rwBLENDZERO;
            PTANKEXT(m_atomic)->instFlags |= 0x10000000;
        }
        break;
    case 1:
        if ((m_definitionFlags & 4) != 0) {
            PTANKEXT(m_atomic)->srcBlend = rwBLENDSRCALPHA;
            PTANKEXT(m_atomic)->dstBlend = rwBLENDONE;
            PTANKEXT(m_atomic)->instFlags |= 0x10000000;
        } else {
            PTANKEXT(m_atomic)->srcBlend = rwBLENDONE;
            PTANKEXT(m_atomic)->dstBlend = rwBLENDONE;
            PTANKEXT(m_atomic)->instFlags |= 0x10000000;
        }
        break;
    case 2:
        PTANKEXT(m_atomic)->srcBlend = rwBLENDDESTCOLOR;
        PTANKEXT(m_atomic)->dstBlend = rwBLENDZERO;
        PTANKEXT(m_atomic)->instFlags |= 0x10000000;
        break;
    }
    if ((m_definitionFlags & 0x10) != 0 && definition->field_0x58 != 0) {
        EffectLaunchInfoRW3 *rwInfo = dynamic_cast<EffectLaunchInfoRW3 *>(info);
        if (rwInfo != 0 && rwInfo->field_0x8 != 0) {
            RwTexture *texture =
                fn_8048CFFC(rwInfo->field_0x8, static_cast<const char *>(definition->field_0x58));
            if (texture != 0) {
                fn_8045B270(m_atomic->geometry->matList.materials[0], texture);
            }
        }
    }
    RpPTankLockStruct position;
    RpPTankLockStruct color;
    RpPTankLockStruct size;
    RpPTankLockStruct rotation;
    RpPTankLockStruct texCoords;
    fn_80448408(m_atomic, &position, rpPTANKDFLAGPOSITION, rpPTANKLOCKWRITE);
    fn_80448408(m_atomic, &color, rpPTANKDFLAGCOLOR, rpPTANKLOCKWRITE);
    fn_80448408(m_atomic, &size, rpPTANKDFLAGSIZE, rpPTANKLOCKWRITE);
    fn_80448408(m_atomic, &rotation, rpPTANKDFLAG2DROTATE, rpPTANKLOCKWRITE);
    fn_80448408(m_atomic, &texCoords, rpPTANKDFLAGVTX2TEXCOORDS, rpPTANKLOCKWRITE);
    if (position.data == 0 || color.data == 0 || size.data == 0 || rotation.data == 0 ||
        texCoords.data == 0) {
        m_flags &= ~System::ELEMENT_ACTIVE;
    }
    fn_80448598(m_atomic);
}

EffectParticleRW3::~EffectParticleRW3() {
    if (m_atomic != 0) {
        RpWorld *world = fn_804622F0(m_atomic);
        if (world != 0) {
            fn_8046221C(world, m_atomic);
        }
        fn_80458CBC(m_atomic, 0);
        PTANKEXT(m_atomic)->instFlags |= 0x800000;
        PTANKEXT(m_atomic)->actPCount = 0;
        fn_8045B270(m_atomic->geometry->matList.materials[0], 0);
        static_cast<EffectManagerRW3 *>(m_manager)->fn_804324A0(m_atomic, m_maxCount);
    }
    if (m_frame != 0) {
        fn_80486E2C(m_frame);
    }
}

void EffectParticleRW3::Update(float deltaTime) {
    System::EffectParticle::Update(deltaTime);
    if ((m_flags & System::ELEMENT_ACTIVE) != 0 && (m_flags & System::ELEMENT_FLAG_4) == 0 &&
        m_count != 0) {
        if (m_frame != 0) {
            RwMatrix *matrix = &m_frame->modelling;
            matrix->right.x = m_baseTransform.values[0][0];
            matrix->right.y = m_baseTransform.values[1][0];
            matrix->right.z = m_baseTransform.values[2][0];
            matrix->up.x = m_baseTransform.values[0][1];
            matrix->up.y = m_baseTransform.values[1][1];
            matrix->up.z = m_baseTransform.values[2][1];
            matrix->at.x = m_baseTransform.values[0][2];
            matrix->at.y = m_baseTransform.values[1][2];
            matrix->at.z = m_baseTransform.values[2][2];
            matrix->pos.x = m_baseTransform.values[0][3];
            matrix->pos.y = m_baseTransform.values[1][3];
            matrix->pos.z = m_baseTransform.values[2][3];
            fn_8048722C(m_frame);
        }
    }
}

void EffectParticleRW3::Render() {
    if ((m_flags & System::ELEMENT_ACTIVE) != 0 && (m_flags & System::ELEMENT_FLAG_8) == 0 &&
        m_count != 0) {
        RpPTankLockStruct position;
        RpPTankLockStruct color;
        RpPTankLockStruct size;
        RpPTankLockStruct rotation;
        RpPTankLockStruct texCoords;
        fn_80448408(m_atomic, &position, rpPTANKDFLAGPOSITION, rpPTANKLOCKWRITE);
        fn_80448408(m_atomic, &color, rpPTANKDFLAGCOLOR, rpPTANKLOCKWRITE);
        fn_80448408(m_atomic, &size, rpPTANKDFLAGSIZE, rpPTANKLOCKWRITE);
        fn_80448408(m_atomic, &rotation, rpPTANKDFLAG2DROTATE, rpPTANKLOCKWRITE);
        fn_80448408(m_atomic, &texCoords, rpPTANKDFLAGVTX2TEXCOORDS, rpPTANKLOCKWRITE);

        System::EffectParticleExecUnit *particle = m_head;
        if ((m_definitionFlags & 0x800) != 0) {
            for (; particle != 0; particle = particle->m_next) {
                Fn8040Vector3 transformed;
                transformed = m_transform * particle->m_position;
                RwV3d *output = reinterpret_cast<RwV3d *>(position.data);
                position.data += position.stride;
                output->x = transformed.x;
                output->y = transformed.y;
                output->z = transformed.z;
            }
        } else {
            for (; particle != 0; particle = particle->m_next) {
                RwV3d *output = reinterpret_cast<RwV3d *>(position.data);
                position.data += position.stride;
                output->x = particle->m_position.x;
                output->y = particle->m_position.y;
                output->z = particle->m_position.z;
            }
        }
        for (particle = m_head; particle != 0; particle = particle->m_next) {
            RwRGBA *outputColor = reinterpret_cast<RwRGBA *>(color.data);
            color.data += color.stride;
            outputColor->red = particle->m_color[0] >> 22;
            outputColor->green = particle->m_color[1] >> 22;
            outputColor->blue = particle->m_color[2] >> 22;
            outputColor->alpha = particle->m_color[3] >> 22;
            RwV2d *outputSize = reinterpret_cast<RwV2d *>(size.data);
            size.data += size.stride;
            outputSize->x = particle->m_scale.x;
            outputSize->y = particle->m_scale.y;
            float *outputRotation = reinterpret_cast<float *>(rotation.data);
            rotation.data += rotation.stride;
            while (particle->m_rotation.z > lbl_805FAD38 ||
                   particle->m_rotation.z < -lbl_805FAD38) {
                if (particle->m_rotation.z < 0.0f) {
                    particle->m_rotation.z += 2.0f * lbl_805FAD38;
                } else {
                    particle->m_rotation.z -= 2.0f * lbl_805FAD38;
                }
            }
            *outputRotation = particle->m_rotation.z;
            RwTexCoords *outputTexCoords = reinterpret_cast<RwTexCoords *>(texCoords.data);
            texCoords.data += texCoords.stride;
            outputTexCoords[0].u = particle->m_uvLower.x;
            outputTexCoords[0].v = particle->m_uvLower.y;
            outputTexCoords[1].u = particle->m_uvUpper.x;
            outputTexCoords[1].v = particle->m_uvUpper.y;
        }
        fn_80448598(m_atomic);
        PTANKEXT(m_atomic)->instFlags |= 0x800000;
        PTANKEXT(m_atomic)->actPCount = m_count;
        if (field_0x144 == 0) {
            RwBool fog;
            RwRenderStateGet(rwRENDERSTATEFOGENABLE, &fog);
            if ((m_definitionFlags & 2) != 0) {
                RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
            } else {
                RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)TRUE);
            }
            m_atomic->renderCallBack(m_atomic);
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)fog);
        }
    }
}

EffectParticle3DRW3::EffectParticle3DRW3(const System::UnkEffectElementData5 *definition,
                                         System::EffectLaunchInfo *info,
                                         System::EffectManager *manager)
    : System::EffectParticle(definition, info, manager), m_particles(0), m_clump(0), m_texture(0),
      m_world(0) {
    m_size -= sizeof(System::EffectParticle);
    m_size += sizeof(EffectParticle3DRW3);
    if ((m_flags & System::ELEMENT_ACTIVE) == 0) {
        return;
    }
    m_particles = new UnkEffectParticle3DRW3Data[m_maxCount];
    if (m_particles == 0) {
        m_flags &= ~System::ELEMENT_ACTIVE;
        return;
    }
    for (int i = 0; i < m_maxCount; i++) {
        m_particles[i].clump = 0;
        m_particles[i].inWorld = 0;
    }
    m_corners[0] = Fn8040Vector3(-0.5f, -0.5f, 0.0f);
    m_corners[1] = Fn8040Vector3(-0.5f, 0.5f, 0.0f);
    m_corners[2] = Fn8040Vector3(0.5f, -0.5f, 0.0f);
    m_corners[3] = Fn8040Vector3(0.5f, 0.5f, 0.0f);
    m_size += sizeof(UnkEffectParticle3DRW3Data) * m_maxCount;
}

EffectParticle3DRW3::~EffectParticle3DRW3() {
    if (m_particles != 0) {
        for (int i = 0; i < m_maxCount; i++) {
            if (m_particles[i].clump != 0) {
                RpWorld *world = fn_8046256C(m_particles[i].clump);
                if (world != 0) {
                    fn_804623EC(world, m_particles[i].clump);
                }
                fn_80457468(m_particles[i].clump);
            }
        }
        delete[] m_particles;
        m_particles = 0;
    }
}

void EffectParticle3DRW3::Update(float deltaTime) {
    System::EffectParticle::Update(deltaTime);
    if ((m_flags & System::ELEMENT_ACTIVE) != 0 && (m_flags & System::ELEMENT_FLAG_4) == 0) {
        if (m_clump != 0) {
            System::EffectParticleExecUnit *particle = m_head;
            for (int i = 0; i < m_maxCount; i++) {
                if (particle == 0) {
                    return;
                }
                if (m_particles[i].clump == 0) {
                    m_particles[i].clump = fn_80457090(m_clump);
                }
                Fn8040Matrix34 transform(lbl_805E0720);
                fn_8040B59C(&transform, particle->m_position);
                fn_8040B80C(&transform, particle->m_rotation.z);
                fn_8040B848(&transform, particle->m_rotation.y);
                fn_8040B884(&transform, particle->m_rotation.x);
                fn_8040B790(&transform, particle->m_scale);
                if ((m_definitionFlags & 0x800) != 0) {
                    Fn8040Matrix34 combined;
                    fn_8001E044(&combined, &m_transform, &transform);
                    fn_8040C148(&transform, &combined);
                }
                RwFrame *frame = static_cast<RwFrame *>(m_particles[i].clump->object.parent);
                RwMatrix *matrix = &frame->modelling;
                matrix->right.x = transform.values[0][0];
                matrix->right.y = transform.values[1][0];
                matrix->right.z = transform.values[2][0];
                matrix->up.x = transform.values[0][1];
                matrix->up.y = transform.values[1][1];
                matrix->up.z = transform.values[2][1];
                matrix->at.x = transform.values[0][2];
                matrix->at.y = transform.values[1][2];
                matrix->at.z = transform.values[2][2];
                matrix->pos.x = transform.values[0][3];
                matrix->pos.y = transform.values[1][3];
                matrix->pos.z = transform.values[2][3];
                matrix->flags &= ~(rwMATRIXTYPEORTHONORMAL | rwMATRIXINTERNALIDENTITY);
                fn_8048722C(frame);
                particle = particle->m_next;
            }
        } else {
            System::EffectParticleExecUnit *particle = m_head;
            for (int i = 0; i < m_maxCount; i++) {
                if (particle == 0) {
                    return;
                }
                Fn8040Matrix34 transform(lbl_805E0720);
                fn_8040B59C(&transform, particle->m_position);
                fn_8040B80C(&transform, particle->m_rotation.z);
                fn_8040B848(&transform, particle->m_rotation.y);
                fn_8040B884(&transform, particle->m_rotation.x);
                fn_8040B790(&transform, particle->m_scale);
                if ((m_definitionFlags & 0x800) != 0) {
                    Fn8040Matrix34 combined;
                    fn_8001E044(&combined, &m_transform, &transform);
                    fn_8040C148(&transform, &combined);
                }
                for (int corner = 0; corner < 4; corner++) {
                    Fn8040Vector3 position;
                    position = transform * m_corners[corner];
                    m_particles[i].vertices[corner].objVertex.x = position.x;
                    m_particles[i].vertices[corner].objVertex.y = position.y;
                    m_particles[i].vertices[corner].objVertex.z = position.z;
                    m_particles[i].vertices[corner].color.red = particle->m_color[0] >> 22;
                    m_particles[i].vertices[corner].color.green = particle->m_color[1] >> 22;
                    m_particles[i].vertices[corner].color.blue = particle->m_color[2] >> 22;
                    m_particles[i].vertices[corner].color.alpha = particle->m_color[3] >> 22;
                    m_particles[i].vertices[corner].u =
                        corner / 2 == 0 ? particle->m_uvUpper.x : particle->m_uvLower.x;
                    m_particles[i].vertices[corner].v =
                        corner % 2 == 0 ? particle->m_uvUpper.y : particle->m_uvLower.y;
                }
                particle = particle->m_next;
            }
        }
    }
}

void EffectParticle3DRW3::Render() {
    if ((m_flags & System::ELEMENT_ACTIVE) == 0 || (m_flags & System::ELEMENT_FLAG_8) != 0) {
        if (m_world != 0) {
            for (int i = 0; i < m_maxCount; i++) {
                if (m_particles[i].clump != 0 && m_particles[i].inWorld != 0) {
                    RpWorld *world = fn_8046256C(m_particles[i].clump);
                    if (world != 0) {
                        fn_804623EC(world, m_particles[i].clump);
                    }
                    m_particles[i].inWorld = 0;
                }
            }
        }
    } else if (m_clump != 0) {
        System::EffectParticleExecUnit *particle = m_head;
        for (int i = 0; i < m_maxCount; i++) {
            if (particle != 0) {
                if (m_world != 0) {
                    if (m_particles[i].inWorld == 0) {
                        fn_804622FC(m_world, m_particles[i].clump);
                        m_particles[i].inWorld = 1;
                    }
                } else {
                    fn_80456BC4(m_particles[i].clump);
                }
                particle = particle->m_next;
            } else if (m_particles[i].inWorld != 0) {
                RpWorld *world = fn_8046256C(m_particles[i].clump);
                if (world != 0) {
                    fn_804623EC(world, m_particles[i].clump);
                }
                m_particles[i].inWorld = 0;
            }
        }
    } else {
        RwInt32 cullMode;
        RwBool vertexAlpha;
        RwBool fog;
        RwRaster *raster;
        RwInt32 filter;
        RwRenderStateGet(rwRENDERSTATECULLMODE, &cullMode);
        RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &vertexAlpha);
        RwRenderStateGet(rwRENDERSTATEFOGENABLE, &fog);
        RwRenderStateGet(rwRENDERSTATETEXTURERASTER, &raster);
        RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &filter);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
        if ((m_definitionFlags & 2) != 0) {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
        } else {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)TRUE);
        }
        if (m_texture != 0) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, m_texture->raster);
        } else {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
        }
        System::EffectParticleExecUnit *particle = m_head;
        for (int i = 0; i < m_maxCount; i++) {
            if (particle == 0) {
                break;
            }
            if (fn_8049A82C(m_particles[i].vertices, 4, 0,
                            rwIM3D_VERTEXUV | rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA) != 0) {
                fn_8049AAF8(rwPRIMTYPETRISTRIP);
            }
            particle = particle->m_next;
        }
        fn_8049A90C();
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)fog);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)vertexAlpha);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)cullMode);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, raster);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)filter);
    }
}

} // namespace RW3
} // namespace Effect

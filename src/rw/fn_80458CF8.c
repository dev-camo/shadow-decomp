/* RenderWare geometries: the geometry module, creating geometries with their morph targets and
 * arrays in one allocation, adding morph targets, materials of triangles, locking and unlocking
 * (rebuilding the mesh), destroying, and reading a geometry from a stream. The original file name
 * is unknown. */
#include "rw/rpworld.h"
#include <string.h>
#include "layout_check.h"

/* errors */
#define E_RP_GEOMETRYTOOMANYVERTICES 0x6 /* inferred */

/* the most vertices a geometry can have (16-bit indices) */
#define rpGEOMETRYMAXVERTICES 0x10000

/* The stream header of a geometry (0x10 bytes) */
typedef struct GeometryChunk {
    /* 0x0 */ RwUInt32 format;
    /* 0x4 */ RwInt32 numTriangles;
    /* 0x8 */ RwInt32 numVertices;
    /* 0xC */ RwInt32 numMorphTargets;
} GeometryChunk;
CHECK_SIZEOF(GeometryChunk, 0x10);

/* The stream header of a morph target (0x18 bytes) */
typedef struct MorphTargetChunk {
    /* 0x00 */ RwSphere boundingSphere;
    /* 0x10 */ RwBool pointsPresent;
    /* 0x14 */ RwBool normalsPresent;
} MorphTargetChunk;
CHECK_SIZEOF(MorphTargetChunk, 0x18);

/* A triangle as the stream stores it: two words, each with two 16-bit indices, the upper first */
typedef union StreamTriangle {
    RpTriangle triangle;
    RwUInt32 words[2];
} StreamTriangle;
CHECK_SIZEOF(StreamTriangle, 0x8);

RwPluginRegistry lbl_8056D810 = {sizeof(RpGeometry), sizeof(RpGeometry), 0, 0, NULL, NULL};

RwModuleInfo lbl_805F14D0;

/* engine plugin constructor */
void *fn_80458CF8(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F14D0.globalsOffset = offset;
    lbl_805F14D0.numInstances++;
    return instance;
}

/* engine plugin destructor */
void *fn_80458D10(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F14D0.numInstances--;
    return instance;
}

RpGeometry *fn_80458D24(RwReal radius) {
    RpGeometry *geometry = fn_80459464(0, 0, 0);

    if (geometry != NULL) {
        RpMorphTarget *morphTarget = geometry->morphTarget;

        morphTarget->boundingSphere.center.x = 0.0f;
        morphTarget->boundingSphere.center.y = 0.0f;
        morphTarget->boundingSphere.center.z = 0.0f;
        morphTarget->boundingSphere.radius = radius;
    }
    /* as in the original, a NULL geometry (creation failed) is passed on to these two calls */
    if (fn_804591C4(geometry) == NULL) {
        fn_8045985C(geometry);
        return NULL;
    }
    return geometry;
}

/* Adds morph targets to a geometry: each target's vertex arrays follow all the targets in one
 * block, so the block grows and the old arrays move up past the new targets */
RwInt32 fn_80458DB0(RpGeometry *geometry, RwInt32 numMorphTargets) {
    RwUInt32 targetSize;
    RwUInt32 size;
    RpMorphTarget *morphTargets;
    RwInt32 i;
    RwUInt8 *data;
    RpMorphTarget *morphTarget;

    if (geometry->flags & rpGEOMETRYNATIVE) {
        targetSize = sizeof(RpMorphTarget);
    } else {
        targetSize = sizeof(RpMorphTarget) + geometry->numVertices * sizeof(RwV3d);
        if (geometry->flags & rpGEOMETRYNORMALS) {
            targetSize += geometry->numVertices * sizeof(RwV3d);
        }
    }
    size = targetSize * (geometry->numMorphTargets + numMorphTargets);

    if (geometry->morphTarget != NULL) {
        RwUInt8 *src;
        RwUInt8 *dst;
        RwUInt32 length;

        morphTargets = RwRealloc(geometry->morphTarget, size, rwMEMHINTDUR_EVENT | rwID_GEOMETRY);
        if (morphTargets == NULL) {
            RPWORLDERROR((E_RW_NOMEM, size));
            return -1;
        }
        /* move the old vertex arrays up, from the end */
        src = (RwUInt8 *)morphTargets + targetSize * geometry->numMorphTargets - 1;
        dst = src + numMorphTargets * sizeof(RpMorphTarget);
        length = targetSize * geometry->numMorphTargets -
                 geometry->numMorphTargets * sizeof(RpMorphTarget);
        while (length != 0) {
            *dst-- = *src--;
            length--;
        }
    } else {
        morphTargets = RwMalloc(size, rwMEMHINTDUR_EVENT | rwID_GEOMETRY);
        if (morphTargets == NULL) {
            RPWORLDERROR((E_RW_NOMEM, size));
            return -1;
        }
    }

    geometry->numMorphTargets += numMorphTargets;
    geometry->morphTarget = morphTargets;

    data = (RwUInt8 *)morphTargets + geometry->numMorphTargets * sizeof(RpMorphTarget);
    for (i = 0; i < geometry->numMorphTargets; i++) {
        morphTarget = &geometry->morphTarget[i];
        morphTarget->verts = NULL;
        morphTarget->normals = NULL;
        if (!(geometry->flags & rpGEOMETRYNATIVE) && geometry->numVertices != 0) {
            morphTarget->verts = (RwV3d *)data;
            data += geometry->numVertices * sizeof(RwV3d);
            if (geometry->flags & rpGEOMETRYNORMALS) {
                morphTarget->normals = (RwV3d *)data;
                data += geometry->numVertices * sizeof(RwV3d);
            }
        }
    }

    for (i = geometry->numMorphTargets - numMorphTargets; i < geometry->numMorphTargets; i++) {
        morphTarget = &geometry->morphTarget[i];
        morphTarget->boundingSphere.center.x = 0.0f;
        morphTarget->boundingSphere.center.y = 0.0f;
        morphTarget->boundingSphere.center.z = 0.0f;
        morphTarget->boundingSphere.radius = 0.0f;
        morphTarget->parentGeom = geometry;
    }
    return geometry->numMorphTargets - numMorphTargets;
}

/* Sets the material of a triangle, adding it to the geometry's material list; no material is
 * 0xFFFF */
RpGeometry *fn_8045905C(RpGeometry *geometry, RpTriangle *triangle, RpMaterial *material) {
    RwInt32 index;

    if (material != NULL) {
        index = fn_8045B8EC(&geometry->matList, material);
        if (index < 0) {
            index = fn_8045B7AC(&geometry->matList, material);
            if (index < 0) {
                return NULL;
            }
        }
        triangle->matIndex = index;
    } else {
        triangle->matIndex = 0xFFFF;
    }
    return geometry;
}

/* Calls a function for each material of a geometry until it returns NULL */
RpGeometry *fn_804590E8(RpGeometry *geometry, RpMaterialCallBack callBack, void *data) {
    RwInt32 numMaterials = geometry->matList.numMaterials;
    RwInt32 i;

    for (i = 0; i < numMaterials; i++) {
        if (callBack(geometry->matList.materials[i], data) == NULL) {
            return geometry;
        }
    }
    return geometry;
}

/* Locks parts of a geometry for editing; locking the triangles drops the mesh */
RpGeometry *fn_8045916C(RpGeometry *geometry, RwInt32 lockMode) {
    geometry->lockedSinceLastInst |= lockMode;
    if ((lockMode & rpGEOMETRYLOCKPOLYGONS) && geometry->mesh != NULL) {
        fn_8045C2D4(geometry->mesh);
        geometry->mesh = NULL;
    }
    return geometry;
}

/* Unlocks a geometry: rebuilds its mesh when it has none, ordering the triangles by material and
 * the indices of their textures, rasters and pipelines */
RpGeometry *fn_804591C4(RpGeometry *geometry) {
    RwTexture **textures;
    RwRaster **rasters;
    RxPipeline **pipelines;
    RwUInt16 numTextures = 0;
    RwUInt16 numRasters = 0;
    RwUInt16 numPipelines = 0;
    RpBuildMesh *buildMesh;
    RpMeshHeader *mesh;
    RpMaterial *material;
    RwTexture *texture;
    RwUInt16 textureIndex;
    RwUInt16 rasterIndex;
    RwUInt32 size;
    RwUInt16 pipelineIndex;
    RxPipeline *pipeline;
    RwInt32 i;
    RpTriangle *triangle;
    RwRaster *raster;

    if (geometry->mesh == NULL) {
        buildMesh = fn_8045C144(geometry->numTriangles);
        if (buildMesh != NULL) {

            if (geometry->matList.numMaterials > 0) {
                size = geometry->matList.numMaterials * sizeof(void *);
                textures = RwMalloc(size, rwMEMHINTDUR_FUNCTION | rwID_TEXTURE);
                rasters = RwMalloc(size, rwMEMHINTDUR_FUNCTION | rwID_GEOMETRYMODULE);
                pipelines = RwMalloc(size, rwMEMHINTDUR_FUNCTION | rwID_GEOMETRYMODULE);

                for (i = 0; i < geometry->numTriangles; i++) {
                    raster = NULL;
                    triangle = &geometry->triangles[i];
                    material = fn_8045B79C(&geometry->matList, triangle->matIndex);
                    texture = material->texture;

                    for (textureIndex = 0; textureIndex < numTextures; textureIndex++) {
                        if (textures[textureIndex] == texture) {
                            break;
                        }
                    }
                    if (textureIndex == numTextures) {
                        textures[textureIndex] = texture;
                        numTextures++;
                    }

                    if (texture != NULL) {
                        raster = texture->raster;
                    }
                    for (rasterIndex = 0; rasterIndex < numRasters; rasterIndex++) {
                        if (rasters[rasterIndex] == raster) {
                            break;
                        }
                    }
                    if (rasterIndex == numRasters) {
                        rasters[rasterIndex] = raster;
                        numRasters++;
                    }

                    pipeline = material->pipeline;
                    for (pipelineIndex = 0; pipelineIndex < numPipelines; pipelineIndex++) {
                        if (pipelines[pipelineIndex] == pipeline) {
                            break;
                        }
                    }
                    if (pipelineIndex == numPipelines) {
                        pipelines[pipelineIndex] = pipeline;
                        numPipelines++;
                    }

                    fn_8045C340(buildMesh, material, triangle->vertIndex[0], triangle->vertIndex[1],
                                triangle->vertIndex[2], triangle->matIndex, textureIndex,
                                rasterIndex, pipelineIndex);
                }

                RwFree(textures);
                RwFree(rasters);
                RwFree(pipelines);
            }

            if (geometry->flags & rpGEOMETRYTRISTRIP) {
                mesh = fn_8045F1FC(buildMesh, TRUE);
            } else {
                mesh = fn_8045F1FC(buildMesh, FALSE);
            }
            if (mesh != NULL) {
                geometry->mesh = mesh;
                return geometry;
            }
            fn_8045C26C(buildMesh);
            return NULL;
        }
        return NULL;
    }
    return geometry;
}

/* Creates a geometry: the format's flags and texture coordinate sets, and one allocation for the
 * geometry, its prelit colours, texture coordinates and triangles; then one morph target */
RpGeometry *fn_80459464(RwInt32 numVertices, RwInt32 numTriangles, RwUInt32 format) {
    RwUInt8 *data;
    RwUInt32 size;
    RwUInt32 numTexCoordSets;
    RwUInt32 native;
    RwUInt32 flags;
    RwUInt32 formatFlags;
    RwUInt32 texCoordSetBits;
    RpGeometry *geometry;
    RwUInt32 i;
    RwInt32 triangleIndex;

    if (numVertices < 0 || numVertices >= rpGEOMETRYMAXVERTICES || numTriangles < 0) {
        if (numVertices >= 0 && numVertices >= rpGEOMETRYMAXVERTICES) {
            RPWORLDERROR((E_RP_GEOMETRYTOOMANYVERTICES));
        }
        return NULL;
    }

    size = lbl_8056D810.sizeOfStruct;
    formatFlags = format & 0xFF;
    texCoordSetBits = format & 0xFF0000;
    if (texCoordSetBits != 0) {
        numTexCoordSets = texCoordSetBits >> 16;
    } else if (format & rpGEOMETRYTEXTURED2) {
        numTexCoordSets = 2;
    } else {
        numTexCoordSets = (format & rpGEOMETRYTEXTURED) ? 1 : 0;
    }
    /* the texture flags follow the number of sets */
    flags = (formatFlags & ~(rpGEOMETRYTEXTURED | rpGEOMETRYTEXTURED2)) |
            (numTexCoordSets == 1 ? rpGEOMETRYTEXTURED
                                  : (numTexCoordSets > 1 ? rpGEOMETRYTEXTURED2 : 0));

    native = format & rpGEOMETRYNATIVE;
    if (!native) {
        if (flags & rpGEOMETRYPRELIT) {
            size += numVertices * sizeof(RwRGBA);
        }
        if (numTexCoordSets != 0) {
            size += numTexCoordSets * (numVertices * sizeof(RwTexCoords));
        }
        size += numTriangles * sizeof(RpTriangle);
    }

    geometry = RwMalloc(size, rwMEMHINTDUR_EVENT | rwID_GEOMETRY);
    if (geometry == NULL) {
        return NULL;
    }
    if (fn_8045B788(&geometry->matList) == NULL) {
        return NULL;
    }

    geometry->morphTarget = NULL;
    geometry->numMorphTargets = 0;
    RWOBJECTINITIALIZE(&geometry->object, rpGEOMETRY, 0);
    geometry->repEntry = NULL;
    geometry->lockedSinceLastInst = 0;
    geometry->refCount = 1;
    geometry->mesh = NULL;
    geometry->numTexCoordSets = numTexCoordSets;
    memset(geometry->texCoords, 0, sizeof(geometry->texCoords));
    geometry->preLitLum = NULL;
    geometry->triangles = NULL;
    geometry->numTriangles = numTriangles;
    /* the high format bits (the native flags and the two above them) pass through unchanged */
    geometry->flags = flags | (format & 0x0F000000);
    geometry->numVertices = numVertices;

    if (!native) {
        data = (RwUInt8 *)geometry + lbl_8056D810.sizeOfStruct;
        if ((flags & rpGEOMETRYPRELIT) && numVertices != 0) {
            geometry->preLitLum = (RwRGBA *)data;
            data += numVertices * sizeof(RwRGBA);
        }
        if (numTexCoordSets != 0 && numVertices != 0) {
            for (i = 0; i < numTexCoordSets; i++) {
                geometry->texCoords[i] = (RwTexCoords *)data;
                data += numVertices * sizeof(RwTexCoords);
            }
        }
        if (numTriangles != 0) {
            geometry->triangles = (RpTriangle *)data;
            for (triangleIndex = 0; triangleIndex < numTriangles; triangleIndex++) {
                geometry->triangles[triangleIndex].matIndex = 0xFFFF;
            }
        }
    }

    if (fn_80458DB0(geometry, 1) < 0) {
        fn_8045B6F0(&geometry->matList);
        RwFree(geometry);
        return NULL;
    }
    fn_8048209C(&lbl_8056D810, geometry);
    return geometry;
}

RpGeometry *fn_8045984C(RpGeometry *geometry) {
    geometry->refCount++;
    return geometry;
}

/* Frees a geometry whose last reference has been dropped: the plugin data, the morph targets
 * and the material list.
 * Matching workaround: declared inline only so that no unused local copy remains; plain static
 * leaves an extra 0xB8-byte local copy of this helper in the object (the final link strips the
 * copy; only the object comparison differs). Revisit if the original helper form is found or an
 * unused local copy becomes acceptable. */
static inline RwBool GeometryAnnihilate(RpGeometry *geometry) {
    /* the caller has dropped the last reference; it is raised again while the geometry is locked
     * and torn down, and dropped before the free */
    geometry->refCount++;
    fn_8045916C(geometry, rpGEOMETRYLOCKALL);
    fn_80482144(&lbl_8056D810, geometry);
    if (geometry->morphTarget != NULL) {
        RwFree(geometry->morphTarget);
        geometry->morphTarget = NULL;
    }
    fn_8045B6F0(&geometry->matList);
    geometry->refCount--;
    RwFree(geometry);
    return TRUE;
}

/* Releases a geometry; the last reference destroys it */
RwBool fn_8045985C(RpGeometry *geometry) {
    RwBool result = TRUE;

    if (geometry->refCount - 1 <= 0) {
        if (geometry->repEntry != NULL) {
            fn_8048071C(geometry->repEntry);
        }
        geometry->refCount--;
        result = GeometryAnnihilate(geometry);
    } else {
        geometry->refCount--;
    }
    return result;
}

/* plugin registration with geometries */
RwInt32 fn_8045994C(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructor,
                    RwPluginObjectDestructor destructor, RwPluginObjectCopy copy) {
    return fn_80481DC8(&lbl_8056D810, size, pluginID, constructor, destructor, copy);
}

RwInt32 fn_8045998C(RwUInt32 pluginID, RwPluginDataChunkReadCallBack read,
                    RwPluginDataChunkWriteCallBack write,
                    RwPluginDataChunkGetSizeCallBack getSize) {
    return fn_804815F0(&lbl_8056D810, pluginID, read, write, getSize);
}

/* Reads a geometry: its header, prelit colours, texture coordinates and triangles (unless it is
 * native), its morph targets, its material list and plugin data, then builds its mesh */
RpGeometry *fn_804599C8(RwStream *stream) {
    RpGeometry *geometry;
    GeometryChunk header;
    RwUInt32 version;

    if (!fn_8047CF50(stream, rwID_STRUCT, NULL, &version)) {
        return NULL;
    }
    if (version < rwLIBRARYBASEVERSION || version > RW_STREAM_VERSION) {
        RPWORLDERROR((E_RW_BADVERSION));
        return NULL;
    }
    if (fn_80480D4C(stream, &header, sizeof(header)) != sizeof(header)) {
        return NULL;
    }
    fn_8047D254(&header, sizeof(header));

    geometry = fn_80459464(header.numVertices, header.numTriangles, header.format);
    if (geometry == NULL) {
        return NULL;
    }
    if (header.numMorphTargets > 1 && fn_80458DB0(geometry, header.numMorphTargets - 1) < 0) {
        fn_8045985C(geometry);
        return NULL;
    }

    if (!(geometry->flags & rpGEOMETRYNATIVE) && geometry->numVertices != 0) {
        if (header.format & rpGEOMETRYPRELIT) {
            RwUInt32 size = geometry->numVertices * sizeof(RwRGBA);

            if (size != fn_80480D4C(stream, geometry->preLitLum, size)) {
                fn_8045985C(geometry);
                return NULL;
            }
        }

        if (geometry->numTexCoordSets > 0) {
            RwInt32 i;
            const RwUInt32 size = geometry->numVertices * sizeof(RwTexCoords);

            for (i = 0; i < geometry->numTexCoordSets; i++) {
                if (fn_8047D784(stream, (RwReal *)geometry->texCoords[i], size) == NULL) {
                    fn_8045985C(geometry);
                    return NULL;
                }
            }
        }

        if (geometry->numTriangles != 0) {
            StreamTriangle *triangles;
            RwInt32 numTriangles;
            RwUInt32 size;

            triangles = (StreamTriangle *)geometry->triangles;
            numTriangles = geometry->numTriangles;
            size = numTriangles * sizeof(RpTriangle);
            if (size != fn_80480D4C(stream, triangles, size)) {
                fn_8045985C(geometry);
                return NULL;
            }
            fn_8047D254(triangles, size);
            while (numTriangles--) {
                RwUInt32 word;

                word = triangles->words[0];
                triangles->triangle.vertIndex[0] = word >> 16;
                triangles->triangle.vertIndex[1] = word;
                word = triangles->words[1];
                triangles->triangle.vertIndex[2] = word >> 16;
                triangles->triangle.matIndex = word;
                triangles++;
            }
        }
    }

    {
        RwInt32 i;

        for (i = 0; i < geometry->numMorphTargets; i++) {
            MorphTargetChunk morphTargetHeader;
            RpMorphTarget *morphTarget = &geometry->morphTarget[i];

            if (fn_80480D4C(stream, &morphTargetHeader, sizeof(morphTargetHeader)) !=
                sizeof(morphTargetHeader)) {
                fn_8045985C(geometry);
                return NULL;
            }
            fn_8047D254(&morphTargetHeader, sizeof(morphTargetHeader));
            morphTarget->boundingSphere = morphTargetHeader.boundingSphere;

            if (morphTargetHeader.pointsPresent && morphTargetHeader.normalsPresent) {
                /* the normals follow the points */
                if (fn_8047D784(stream, (RwReal *)morphTarget->verts,
                                geometry->numVertices * (sizeof(RwV3d) * 2)) == NULL) {
                    fn_8045985C(geometry);
                    return NULL;
                }
            } else {
                if (morphTargetHeader.pointsPresent &&
                    fn_8047D784(stream, (RwReal *)morphTarget->verts,
                                geometry->numVertices * sizeof(RwV3d)) == NULL) {
                    fn_8045985C(geometry);
                    return NULL;
                }
                if (morphTargetHeader.normalsPresent &&
                    fn_8047D784(stream, (RwReal *)morphTarget->normals,
                                geometry->numVertices * sizeof(RwV3d)) == NULL) {
                    fn_8045985C(geometry);
                    return NULL;
                }
            }
        }
    }

    if (!fn_8047CF50(stream, rwID_MATLIST, NULL, &version)) {
        /* as in the original, the geometry is not destroyed here */
        return NULL;
    }
    if (version < rwLIBRARYBASEVERSION || version > RW_STREAM_VERSION) {
        fn_8045985C(geometry);
        RPWORLDERROR((E_RW_BADVERSION));
        return NULL;
    }
    if (fn_8045B924(stream, &geometry->matList) == NULL) {
        fn_8045985C(geometry);
        return NULL;
    }
    if (fn_804816AC(&lbl_8056D810, stream, geometry) == NULL) {
        fn_8045985C(geometry);
        return NULL;
    }
    if (fn_804591C4(geometry) == NULL) {
        fn_8045985C(geometry);
        return NULL;
    }
    return geometry;
}

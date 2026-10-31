/* RenderWare worlds: the world and world sector "rights" stream extensions (the plugin that owns
 * their pipelines), and reading a world from a stream: the world, its material list and its tree of
 * plane and world sectors, all in one allocation. The original file name is unknown. */
#include "rw/rpworld.h"
#include <string.h>
#include "layout_check.h"

/* the plugin ID the world module reports errors with (inferred) */
#define rwWORLDERRORPLUGIN 2

/* Reports an error: the arguments are the error code and its parameters */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwWORLDERRORPLUGIN;                                                      \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

/* the first stream version with the vertex indices before the material index in triangles */
#define rwSTREAMTRIANGLEVERSION 0x36002

/* A triangle as older streams store it, read in place of a current one (8 bytes) */
typedef union OldTriangle {
    RpTriangle read;
    struct {
        /* 0x0 */ RwUInt16 matIndex;
        /* 0x2 */ RwUInt16 vertIndex[3];
    } old;
} OldTriangle;
CHECK_SIZEOF(OldTriangle, 0x8);

/* The stream header of a world (0x40 bytes) */
typedef struct WorldChunk {
    /* 0x00 */ RwBool rootIsWorldSector;
    /* 0x04 */ RwV3d invWorldOrigin;
    /* 0x10 */ RwInt32 numTriangles;
    /* 0x14 */ RwInt32 numVertices;
    /* 0x18 */ RwInt32 numPlaneSectors;
    /* 0x1C */ RwInt32 numWorldSectors;
    /* 0x20 */ RwInt32 colSectorSize; /* only in old streams */
    /* 0x24 */ RwInt32 format;        /* the world's flags */
    /* 0x28 */ RwBBox boundingBox;
} WorldChunk;
CHECK_SIZEOF(WorldChunk, 0x40);

/* The stream header of a plane sector (0x18 bytes) */
typedef struct PlaneSectorChunk {
    /* 0x00 */ RwInt32 type;
    /* 0x04 */ RwReal value;
    /* 0x08 */ RwBool leftIsWorldSector;
    /* 0x0C */ RwBool rightIsWorldSector;
    /* 0x10 */ RwReal leftValue;
    /* 0x14 */ RwReal rightValue;
} PlaneSectorChunk;
CHECK_SIZEOF(PlaneSectorChunk, 0x18);

/* The stream header of a world sector (0x2C bytes) */
typedef struct WorldSectorChunk {
    /* 0x00 */ RwInt32 matListWindowBase;
    /* 0x04 */ RwInt32 numTriangles;
    /* 0x08 */ RwInt32 numVertices;
    /* 0x0C */ RwV3d inf;
    /* 0x18 */ RwV3d sup;
    /* 0x24 */ RwInt32 field_0x24;
    /* 0x28 */ RwInt32 field_0x28;
} WorldSectorChunk;
CHECK_SIZEOF(WorldSectorChunk, 0x2C);

/* the rights read from the last world and world sector streams: the plugin that owns the pipeline
 * and its data, declared in reverse address order (the compiler places them in .sbss last
 * first) */
typedef struct WorldModuleGlobals {
    /* 0x0 */ RwInt32 field_0x0;
    /* 0x4 */ RwInt32 numInstances; /* the open engine instances */
} WorldModuleGlobals;
CHECK_SIZEOF(WorldModuleGlobals, 0x8);

WorldModuleGlobals lbl_805F14B0;
RwUInt32 lbl_805F14AC; /* world pipeline plugin */
RwUInt32 lbl_805F14A8; /* world pipeline plugin data */
RwUInt32 lbl_805F14A4; /* world sector pipeline plugin */
RwUInt32 lbl_805F14A0; /* world sector pipeline plugin data */

/* Takes the next size bytes of a world's allocation */
static inline RwUInt8 *WorldMemoryTake(RwUInt8 **memory, RwUInt32 size) {
    RwUInt8 *taken = *memory;

    *memory = taken + size;
    return taken;
}

/* world rights stream read */
RwStream *fn_80454AA4(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    if (fn_8047D868(stream, (RwInt32 *)&lbl_805F14AC, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (length == 8 && fn_8047D868(stream, (RwInt32 *)&lbl_805F14A8, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    return stream;
}

/* world rights stream write */
RwStream *fn_80454B20(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    const RpWorld *world = object;

    if (fn_8047D458(stream, (RwInt32 *)&world->pipeline->pluginId, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D458(stream, (RwInt32 *)&world->pipeline->pluginData, sizeof(RwInt32)) != NULL) {
        return stream;
    }
    return NULL;
}

/* world rights stream size: only for a pipeline that a plugin owns */
RwInt32 fn_80454B9C(const void *object, RwInt32 offset, RwInt32 size) {
    const RpWorld *world = object;

    if (world->pipeline != NULL && world->pipeline->pluginId != 0) {
        return 8;
    }
    return 0;
}

/* world sector rights stream read */
RwStream *fn_80454BC4(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    if (fn_8047D868(stream, (RwInt32 *)&lbl_805F14A4, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (length == 8 && fn_8047D868(stream, (RwInt32 *)&lbl_805F14A0, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    return stream;
}

/* world sector rights stream write */
RwStream *fn_80454C40(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    const RpWorldSector *sector = object;

    if (fn_8047D458(stream, (RwInt32 *)&sector->pipeline->pluginId, sizeof(RwInt32)) == NULL) {
        return NULL;
    }
    if (fn_8047D458(stream, (RwInt32 *)&sector->pipeline->pluginData, sizeof(RwInt32)) != NULL) {
        return stream;
    }
    return NULL;
}

/* world sector rights stream size */
RwInt32 fn_80454CBC(const void *object, RwInt32 offset, RwInt32 size) {
    const RpWorldSector *sector = object;

    if (sector->pipeline != NULL && sector->pipeline->pluginId != 0) {
        return 8;
    }
    return 0;
}

/* Reads a world sector: its header, then (unless the world is native) its vertices, normals,
 * prelit colours, texture coordinates and triangles, then its plugin data. The sector and its
 * arrays are taken from *memory. */
RpWorldSector *fn_80454CE4(RwStream *stream, RwUInt8 **memory, RpWorld *world, RwInt32 format) {
    RwUInt32 version;
    WorldSectorChunk header;
    RpWorldSector *sector;
    RwBool status = TRUE;
    RwInt32 i;

    if (!fn_8047CF50(stream, rwID_STRUCT, NULL, &version)) {
        return NULL;
    }
    if (version < rwLIBRARYBASEVERSION || version > RW_STREAM_VERSION) {
        RWERROR((E_RW_BADVERSION));
        return NULL;
    }
    if (fn_80480D4C(stream, &header, sizeof(header)) != sizeof(header)) {
        return NULL;
    }
    fn_8047D254(&header, sizeof(header));

    sector = (RpWorldSector *)WorldMemoryTake(memory, lbl_8056D880.sizeOfStruct);
    if (sector == NULL) {
        RWERROR((E_RW_NOMEM, lbl_8056D880.sizeOfStruct));
        return NULL;
    }

    sector->type = rpWORLDSECTORTYPE;
    sector->boundingBox.inf = header.inf;
    sector->boundingBox.sup = header.sup;
    sector->matListWindowBase = header.matListWindowBase;
    sector->numTriangles = header.numTriangles;
    sector->numVertices = header.numVertices;
    sector->triangles = NULL;
    sector->vertices = NULL;
    sector->preLitLum = NULL;
    sector->normals = NULL;
    memset(sector->texCoords, 0, sizeof(sector->texCoords));
    sector->pipeline = NULL;
    sector->repEntry = NULL;
    sector->mesh = NULL;
    rwLinkListInitialize(&sector->collAtomicsInWorldSector);
    rwLinkListInitialize(&sector->lightsInWorldSector);

    if (!(world->flags & rpWORLDNATIVE)) {
        if (header.numVertices != 0) {
            RwUInt32 vertexSize = header.numVertices * sizeof(RwV3d);

            sector->vertices = (RwV3d *)WorldMemoryTake(memory, vertexSize);
            if (sector->vertices == NULL) {
                RWERROR((E_RW_NOMEM, vertexSize));
                status = FALSE;
            }
            if (status && fn_8047D784(stream, (RwReal *)sector->vertices, vertexSize) == NULL) {
                status = FALSE;
            }

            if (status && (format & rpWORLDNORMALS)) {
                RwUInt32 normalSize = header.numVertices * sizeof(RpVertexNormal);

                sector->normals = (RpVertexNormal *)WorldMemoryTake(memory, normalSize);
                if (sector->normals == NULL) {
                    RWERROR((E_RW_NOMEM, normalSize));
                    status = FALSE;
                }
                if (status && normalSize != fn_80480D4C(stream, sector->normals, normalSize)) {
                    status = FALSE;
                }
            }

            if (status && (format & rpWORLDPRELIT)) {
                RwUInt32 preLitSize = header.numVertices * sizeof(RwRGBA);

                sector->preLitLum = (RwRGBA *)WorldMemoryTake(memory, preLitSize);
                if (sector->preLitLum == NULL) {
                    RWERROR((E_RW_NOMEM, preLitSize));
                    status = FALSE;
                }
                if (status && fn_80480D4C(stream, sector->preLitLum, preLitSize) == 0) {
                    status = FALSE;
                }
            }

            if (status && world->numTexCoordSets > 0) {
                RwInt32 texCoordSize = sector->numVertices * sizeof(RwTexCoords);

                for (i = 0; i < world->numTexCoordSets; i++) {
                    sector->texCoords[i] = (RwTexCoords *)WorldMemoryTake(memory, texCoordSize);
                    if (status && sector->texCoords[i] == NULL) {
                        RWERROR((E_RW_NOMEM, texCoordSize));
                        status = FALSE;
                        break;
                    }
                    if (status &&
                        fn_8047D784(stream, (RwReal *)sector->texCoords[i], texCoordSize) == NULL) {
                        status = FALSE;
                        break;
                    }
                }
            }
        }

        if (status && header.numTriangles != 0) {
            RwUInt32 triangleSize = header.numTriangles * sizeof(RpTriangle);

            sector->triangles = (RpTriangle *)WorldMemoryTake(memory, triangleSize);
            if (sector->triangles == NULL) {
                RWERROR((E_RW_NOMEM, triangleSize));
                status = FALSE;
            }
            if (status && fn_8047D684(stream, (RwInt16 *)sector->triangles, triangleSize) == NULL) {
                status = FALSE;
            }

            /* older streams store the material index before the vertex indices */
            if (status && version < rwSTREAMTRIANGLEVERSION) {
                for (i = 0; i < header.numTriangles; i++) {
                    OldTriangle triangle;

                    triangle.read = sector->triangles[i];
                    sector->triangles[i].vertIndex[0] = triangle.old.vertIndex[0];
                    sector->triangles[i].vertIndex[1] = triangle.old.vertIndex[1];
                    sector->triangles[i].vertIndex[2] = triangle.old.vertIndex[2];
                    sector->triangles[i].matIndex = triangle.old.matIndex;
                }
            }
        }
    }

    if (status) {
        fn_8048209C(&lbl_8056D880, sector);
        lbl_805F14A4 = 0;
        lbl_805F14A0 = 0;
        if (status && fn_804816AC(&lbl_8056D880, stream, sector) == NULL) {
            status = FALSE;
        }
        if (status && lbl_805F14A4 != 0) {
            fn_80481870(&lbl_8056D880, lbl_805F14A4, sector, lbl_805F14A0);
        }
    }

    if (!status) {
        if (world->object.privateFlags & rpWORLDSINGLEMALLOC) {
            fn_8045F6E0((RpSector *)sector);
        } else {
            fn_8045F7D0((RpSector *)sector);
        }
        return NULL;
    }
    return sector;
}

/* Reads a plane sector and the subtrees on each side of it */
RpPlaneSector *fn_804552AC(RwStream *stream, RwUInt8 **memory, RpWorld *world, RwInt32 format) {
    RwUInt32 length;
    RwUInt32 version;
    RpSector *subTree;
    PlaneSectorChunk header;
    RpPlaneSector *plane;
    RwBool status = TRUE;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        return NULL;
    }
    if (version < rwLIBRARYBASEVERSION || version > RW_STREAM_VERSION) {
        RWERROR((E_RW_BADVERSION));
        return NULL;
    }
    memset(&header, 0, sizeof(header));
    if (length != fn_80480D4C(stream, &header, length)) {
        return NULL;
    }
    fn_8047D254(&header, sizeof(header));

    plane = (RpPlaneSector *)WorldMemoryTake(memory, sizeof(RpPlaneSector));
    if (plane == NULL) {
        RWERROR((E_RW_NOMEM, sizeof(RpPlaneSector)));
        return NULL;
    }

    plane->type = header.type;
    plane->value = header.value;
    if (format & rpWORLDSECTORSOVERLAP) {
        plane->leftValue = header.leftValue;
        plane->rightValue = header.rightValue;
    } else {
        plane->leftValue = header.value;
        plane->rightValue = header.value;
    }

    if (header.leftIsWorldSector) {
        if (!fn_8047CF50(stream, rwID_ATOMICSECT, NULL, &version)) {
            status = FALSE;
        }
        if (status && (version < rwLIBRARYBASEVERSION || version > RW_STREAM_VERSION)) {
            RWERROR((E_RW_BADVERSION));
            status = FALSE;
        }
        if (status) {
            subTree = (RpSector *)fn_80454CE4(stream, memory, world, format);
            plane->leftSubTree = subTree;
            if (subTree == NULL) {
                status = FALSE;
            }
        }
    } else {
        if (!fn_8047CF50(stream, rwID_PLANESECT, NULL, &version)) {
            status = FALSE;
        }
        if (status && (version < rwLIBRARYBASEVERSION || version > RW_STREAM_VERSION)) {
            RWERROR((E_RW_BADVERSION));
            status = FALSE;
        }
        if (status) {
            subTree = (RpSector *)fn_804552AC(stream, memory, world, format);
            plane->leftSubTree = subTree;
            if (subTree == NULL) {
                status = FALSE;
            }
        }
    }

    if (status) {
        if (header.rightIsWorldSector) {
            if (!fn_8047CF50(stream, rwID_ATOMICSECT, NULL, &version)) {
                status = FALSE;
            }
            if (status && (version < rwLIBRARYBASEVERSION || version > RW_STREAM_VERSION)) {
                RWERROR((E_RW_BADVERSION));
                status = FALSE;
            }
            if (status) {
                subTree = (RpSector *)fn_80454CE4(stream, memory, world, format);
                plane->rightSubTree = subTree;
                if (subTree == NULL) {
                    status = FALSE;
                }
            }
        } else {
            if (!fn_8047CF50(stream, rwID_PLANESECT, NULL, &version)) {
                status = FALSE;
            }
            if (status && (version < rwLIBRARYBASEVERSION || version > RW_STREAM_VERSION)) {
                RWERROR((E_RW_BADVERSION));
                status = FALSE;
            }
            if (status) {
                subTree = (RpSector *)fn_804552AC(stream, memory, world, format);
                plane->rightSubTree = subTree;
                if (subTree == NULL) {
                    status = FALSE;
                }
            }
        }
    }

    if (!status) {
        if (plane->leftSubTree != NULL) {
            if (world->object.privateFlags & rpWORLDSINGLEMALLOC) {
                fn_8045F6E0(plane->leftSubTree);
            } else {
                fn_8045F7D0(plane->leftSubTree);
            }
            plane->leftSubTree = NULL;
        }
        return NULL;
    }
    return plane;
}

/* Reads a world: the header sizes one allocation for the world, its sectors and their arrays */
RpWorld *fn_804556F4(RwStream *stream) {
    RwUInt8 *memory;
    RwUInt32 length;
    RwUInt32 version;
    RwUInt32 numTexCoordSets;
    RpSector *rootSector;
    RwInt32 texCoordSetBits;
    WorldChunk header;
    RpWorld *world;
    RwUInt32 size;
    RwUInt32 chunkVersion;
    RwUInt32 format;
    RpMaterial *material;
    RwInt32 i;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        return NULL;
    }
    if (version < rwLIBRARYBASEVERSION || version > RW_STREAM_VERSION) {
        RWERROR((E_RW_BADVERSION));
        return NULL;
    }
    memset(&header, 0, sizeof(header));
    if (length != fn_80480D4C(stream, &header, length)) {
        return NULL;
    }
    /* old streams with collision sectors are not read */
    if (header.colSectorSize > 0) {
        RWERROR((E_RW_BADVERSION));
        return NULL;
    }
    fn_8047D254(&header, sizeof(header));

    size = lbl_8056D898.sizeOfStruct;
    size += header.numPlaneSectors * sizeof(RpPlaneSector);
    size += header.numWorldSectors * lbl_8056D880.sizeOfStruct;
    format = header.format;
    texCoordSetBits = header.format & 0xFF0000;
    if (texCoordSetBits != 0) {
        numTexCoordSets = texCoordSetBits >> 16;
    } else if (header.format & rpWORLDTEXTURED2) {
        numTexCoordSets = 2;
    } else {
        numTexCoordSets = (header.format & rpWORLDTEXTURED) ? 1 : 0;
    }
    if (!(format & rpWORLDNATIVE)) {
        size += header.numVertices * sizeof(RwV3d);
        if (format & rpWORLDNORMALS) {
            size += header.numVertices * sizeof(RpVertexNormal);
        }
        if (format & rpWORLDPRELIT) {
            size += header.numVertices * sizeof(RwRGBA);
        }
        if (numTexCoordSets != 0) {
            size += numTexCoordSets * header.numVertices * sizeof(RwTexCoords);
        }
        size += header.numTriangles * sizeof(RpTriangle);
    }

    world = RwMalloc(size, rwMEMHINTDUR_EVENT | rwID_WORLD);
    if (world == NULL) {
        RWERROR((E_RW_NOMEM, size));
        return NULL;
    }
    memset(world, 0, size);
    memory = (RwUInt8 *)world + lbl_8056D898.sizeOfStruct;

    world->object.type = rpWORLD;
    world->object.subType = 0;
    world->object.flags = 0;
    world->object.privateFlags = 0;
    world->object.parent = NULL;
    world->object.privateFlags = rpWORLDSINGLEMALLOC;
    fn_8045FCFC(world, size);

    world->flags = format;
    world->renderOrder = 2;
    world->worldOrigin.x = -1.0f * header.invWorldOrigin.x;
    world->worldOrigin.y = -1.0f * header.invWorldOrigin.y;
    world->worldOrigin.z = -1.0f * header.invWorldOrigin.z;
    world->numTexCoordSets = numTexCoordSets;
    world->rootSector = NULL;
    world->pipeline = NULL;
    rwLinkListInitialize(&world->clumpList);
    rwLinkListInitialize(&world->lightList);
    rwLinkListInitialize(&world->directionalLightList);

    if (!fn_8047CF50(stream, rwID_MATLIST, NULL, &chunkVersion)) {
        fn_8045FFA0(world);
        return NULL;
    }
    if (chunkVersion < rwLIBRARYBASEVERSION || chunkVersion > RW_STREAM_VERSION) {
        RWERROR((E_RW_BADVERSION));
        fn_8045FFA0(world);
        return NULL;
    }
    if (fn_8045B924(stream, &world->matList) == NULL) {
        fn_8045FFA0(world);
        return NULL;
    }

    if (header.rootIsWorldSector) {
        if (!fn_8047CF50(stream, rwID_ATOMICSECT, NULL, &chunkVersion)) {
            fn_8045FFA0(world);
            return NULL;
        }
        if (chunkVersion < rwLIBRARYBASEVERSION || chunkVersion > RW_STREAM_VERSION) {
            RWERROR((E_RW_BADVERSION));
            fn_8045FFA0(world);
            return NULL;
        }
        rootSector = (RpSector *)fn_80454CE4(stream, &memory, world, format);
        world->rootSector = rootSector;
        if (rootSector == NULL) {
            fn_8045FFA0(world);
            return NULL;
        }
    } else {
        if (!fn_8047CF50(stream, rwID_PLANESECT, NULL, &chunkVersion)) {
            fn_8045FFA0(world);
            return NULL;
        }
        if (chunkVersion < rwLIBRARYBASEVERSION || chunkVersion > RW_STREAM_VERSION) {
            RWERROR((E_RW_BADVERSION));
            fn_8045FFA0(world);
            return NULL;
        }
        rootSector = (RpSector *)fn_804552AC(stream, &memory, world, format);
        world->rootSector = rootSector;
        if (rootSector == NULL) {
            fn_8045FFA0(world);
            return NULL;
        }
    }

    world->numClumpsInWorld = 0;
    world->currentClumpLink = &world->clumpList.link;
    world->boundingBox = header.boundingBox;
    fn_8045FBA4(world);
    fn_80460120(world, NULL);

    /* set each textured material's texture again */
    for (i = 0; i < world->matList.numMaterials; i++) {
        material = world->matList.materials[i];
        if (material->texture != NULL) {
            fn_8045B270(material, material->texture);
        }
    }

    fn_8048209C(&lbl_8056D898, world);
    lbl_805F14AC = 0;
    lbl_805F14A8 = 0;
    if (fn_804816AC(&lbl_8056D898, stream, world) == NULL) {
        fn_8045FFA0(world);
        return NULL;
    }
    if (lbl_805F14AC != 0) {
        fn_80481870(&lbl_8056D898, lbl_805F14AC, world, lbl_805F14A8);
    }
    if (fn_8045FDB0(world) == NULL) {
        fn_8045FFA0(world);
        return NULL;
    }
    return world;
}

/* engine plugin destructor */
void *fn_80455CE4(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F14B0.numInstances--;
    return instance;
}

/* engine plugin constructor */
void *fn_80455CF8(void *instance, RwInt32 offset, RwInt32 size) {
    lbl_805F14B0.numInstances++;
    return instance;
}

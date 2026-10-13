/* RenderWare delta morph plugin (plugin ID 0x122), geometry part: the delta morph targets of a
 * geometry, their creation and destruction and their stream format. The original file name is
 * unknown. */
#include "rw/rpdmorph.h"
#include <string.h>

#define rwMEMHINT_DMORPH 0x30122

/* Reports an error of this plugin; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rpID_DMORPHPLUGIN;                                                       \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

#define DMORPHGEOMETRY(geometry) (*RWPLUGINOFFSET(RpDMorphGeometry *, geometry, lbl_805F1428))

RwInt32 lbl_805F1428;

/* The size of a vertex in a target's vertex data */
static inline RwUInt32 DMorphTargetVertexSize(RwUInt32 flags) {
    return ((flags & rpDMORPHTARGETPOSITIONS) ? sizeof(RwV3d) : 0) +
           ((flags & rpDMORPHTARGETNORMALS) ? sizeof(RwV3d) : 0) +
           ((flags & rpDMORPHTARGETPRELIT) ? sizeof(RwRGBA) : 0) +
           ((flags & rpDMORPHTARGETTEXCOORDS) ? sizeof(RwTexCoords) : 0);
}

/* geometry plugin constructor */
void *fn_80443044(void *object, RwInt32 offset, RwInt32 size) {
    DMORPHGEOMETRY(object) = NULL;
    return object;
}

/* geometry plugin destructor */
void *fn_80443054(void *object, RwInt32 offset, RwInt32 size) {
    if (DMORPHGEOMETRY(object) != NULL && DMORPHGEOMETRY(object)->numDMorphTargets != 0) {
        fn_80443908(object);
    }
    DMORPHGEOMETRY(object) = NULL;
    return object;
}

/* geometry plugin copy: the targets are not copied */
void *fn_804430AC(void *dst, const void *src, RwInt32 offset, RwInt32 size) {
    return dst;
}

/* geometry plugin stream read */
RwStream *fn_804430B0(RwStream *stream, RwInt32 length, void *object, RwInt32 offset,
                      RwInt32 size) {
    RwUInt32 numDMorphTargets;
    RpDMorphGeometry *dmorphGeometry;
    RpGeometry *geometry = object;
    RwUInt32 i;

    if (fn_8047D868(stream, (RwInt32 *)&numDMorphTargets, sizeof(numDMorphTargets)) == NULL) {
        return NULL;
    }
    fn_8044382C(geometry, numDMorphTargets);
    dmorphGeometry = DMORPHGEOMETRY(geometry);
    for (i = 0; i < dmorphGeometry->numDMorphTargets; i++) {
        RpDMorphTarget *target = &dmorphGeometry->dmorphTargets[i];
        RwUInt32 nameLength;
        RwUInt32 header[4];
        RwUInt32 byteSize;

        if (fn_8047D868(stream, (RwInt32 *)&nameLength, sizeof(nameLength)) == NULL) {
            return NULL;
        }
        if (nameLength != 0) {
            target->name = RwMalloc(nameLength, rwMEMHINT_DMORPH);
            if (target->name == NULL) {
                RWERROR((0x80000013, nameLength));
                return NULL;
            }
            if (fn_80480D4C(stream, target->name, nameLength) == 0) {
                return NULL;
            }
        } else {
            target->name = NULL;
        }

        if (fn_8047D868(stream, (RwInt32 *)header, sizeof(header)) == NULL) {
            return NULL;
        }
        target->flags = header[0];
        target->field_0xC = header[1];
        target->field_0x14 = header[2];
        target->numVertices = header[3];

        byteSize = target->field_0x14;
        target->field_0x10 = RwMalloc(byteSize, rwMEMHINT_DMORPH);
        if (target->field_0x10 == NULL) {
            RWERROR((0x80000013, byteSize));
            return NULL;
        }
        if (fn_80480D4C(stream, target->field_0x10, byteSize) == 0) {
            return NULL;
        }

        if (target->numVertices == 0) {
            target->vertexData = NULL;
        } else {
            RwUInt8 *data;

            byteSize = DMorphTargetVertexSize(target->flags);
            byteSize *= target->numVertices;
            target->vertexData = RwMalloc(byteSize, rwMEMHINT_DMORPH);
            if (target->vertexData == NULL) {
                RWERROR((0x80000013, byteSize));
                return NULL;
            }
            data = target->vertexData;
            if (target->flags & rpDMORPHTARGETPOSITIONS) {
                byteSize = target->numVertices * sizeof(RwV3d);
                if (fn_8047D784(stream, (RwReal *)data, byteSize) == NULL) {
                    return NULL;
                }
                target->positions = (RwV3d *)data;
                data += byteSize;
            }
            if (target->flags & rpDMORPHTARGETNORMALS) {
                byteSize = target->numVertices * sizeof(RwV3d);
                if (fn_8047D784(stream, (RwReal *)data, byteSize) == NULL) {
                    return NULL;
                }
                target->normals = (RwV3d *)data;
                data += byteSize;
            }
            if (target->flags & rpDMORPHTARGETPRELIT) {
                byteSize = target->numVertices * sizeof(RwRGBA);
                if (fn_80480D4C(stream, data, byteSize) == 0) {
                    return NULL;
                }
                target->preLitLum = (RwRGBA *)data;
                data += byteSize;
            }
            if (target->flags & rpDMORPHTARGETTEXCOORDS) {
                if (fn_8047D784(stream, (RwReal *)data,
                                target->numVertices * sizeof(RwTexCoords)) == NULL) {
                    return NULL;
                }
                target->texCoords = (RwTexCoords *)data;
            }
        }

        if (fn_8047D784(stream, (RwReal *)&target->boundingSphere, sizeof(RwSphere)) == NULL) {
            return NULL;
        }
        target->geometry = geometry;
    }
    return stream;
}

/* geometry plugin stream write */
RwStream *fn_80443474(RwStream *stream, RwInt32 length, const void *object, RwInt32 offset,
                      RwInt32 size) {
    RpDMorphGeometry *dmorphGeometry = DMORPHGEOMETRY(object);
    RwUInt32 i;

    if (fn_8047D458(stream, (RwInt32 *)&dmorphGeometry->numDMorphTargets, sizeof(RwUInt32)) ==
        NULL) {
        return NULL;
    }
    for (i = 0; i < dmorphGeometry->numDMorphTargets; i++) {
        RpDMorphTarget *target = &dmorphGeometry->dmorphTargets[i];
        RwUInt32 nameLength;
        RwUInt32 header[4];

        if (target->name != NULL) {
            nameLength = rwstrlen(target->name) + 1;
        } else {
            nameLength = 0;
        }
        if (fn_8047D458(stream, (RwInt32 *)&nameLength, sizeof(nameLength)) == NULL) {
            return NULL;
        }
        if (target->name != NULL) {
            if (fn_80480ED0(stream, target->name, nameLength) == NULL) {
                return NULL;
            }
        }

        header[0] = target->flags;
        header[1] = target->field_0xC;
        header[2] = target->field_0x14;
        header[3] = target->numVertices;
        if (fn_8047D458(stream, (RwInt32 *)header, sizeof(header)) == NULL) {
            return NULL;
        }
        if (fn_80480ED0(stream, target->field_0x10, target->field_0x14) == NULL) {
            return NULL;
        }

        if (target->numVertices != 0) {
            if (target->flags & rpDMORPHTARGETPOSITIONS) {
                if (fn_8047D350(stream, (RwReal *)target->positions,
                                target->numVertices * sizeof(RwV3d)) == NULL) {
                    return NULL;
                }
            }
            if (target->flags & rpDMORPHTARGETNORMALS) {
                if (fn_8047D350(stream, (RwReal *)target->normals,
                                target->numVertices * sizeof(RwV3d)) == NULL) {
                    return NULL;
                }
            }
            if (target->flags & rpDMORPHTARGETPRELIT) {
                if (fn_80480ED0(stream, target->preLitLum, target->numVertices * sizeof(RwRGBA)) ==
                    NULL) {
                    return NULL;
                }
            }
            if (target->flags & rpDMORPHTARGETTEXCOORDS) {
                if (fn_8047D350(stream, (RwReal *)target->texCoords,
                                target->numVertices * sizeof(RwTexCoords)) == NULL) {
                    return NULL;
                }
            }
        }

        if (fn_8047D350(stream, (RwReal *)&target->boundingSphere, sizeof(RwSphere)) == NULL) {
            return NULL;
        }
    }
    return stream;
}

/* geometry plugin stream size */
RwInt32 fn_804436B0(const void *object, RwInt32 offset, RwInt32 size) {
    RpDMorphGeometry *dmorphGeometry = DMORPHGEOMETRY(object);
    RwInt32 streamSize = 0;

    if (dmorphGeometry != NULL) {
        RwUInt32 i;

        streamSize += sizeof(RwUInt32);
        for (i = 0; i < dmorphGeometry->numDMorphTargets; i++) {
            RpDMorphTarget *target = &dmorphGeometry->dmorphTargets[i];
            RwUInt32 flags = target->flags;

            streamSize += sizeof(RwUInt32);
            if (target->name != NULL) {
                streamSize += rwstrlen(target->name) + 1;
            }
            streamSize += 4 * sizeof(RwUInt32);
            streamSize += target->field_0x14;
            streamSize += target->numVertices * DMorphTargetVertexSize(flags);
            streamSize += sizeof(RwSphere);
        }
    }
    return streamSize;
}

/* Attaches the plugin to geometries */
RwBool fn_804437B0(void) {
    lbl_805F1428 = fn_8045994C(sizeof(RpDMorphGeometry *), rpID_DMORPHPLUGIN, fn_80443044,
                               fn_80443054, fn_804430AC);
    if (lbl_805F1428 < 0) {
        return FALSE;
    }
    return fn_8045998C(rpID_DMORPHPLUGIN, fn_804430B0, fn_80443474, fn_804436B0) >= 0;
}

/* Gives a geometry numDMorphTargets empty delta morph targets */
RpGeometry *fn_8044382C(RpGeometry *geometry, RwUInt32 numDMorphTargets) {
    RpDMorphGeometry **plugin = RWPLUGINOFFSET(RpDMorphGeometry *, geometry, lbl_805F1428);
    RwUInt32 size;
    RpDMorphTarget *targets;
    RpDMorphGeometry *dmorphGeometry;

    dmorphGeometry = *plugin = RwMalloc(sizeof(RpDMorphGeometry), rwMEMHINT_DMORPH);
    if (dmorphGeometry == NULL) {
        return NULL;
    }
    memset(dmorphGeometry, 0, sizeof(RpDMorphGeometry));
    dmorphGeometry->numDMorphTargets = numDMorphTargets;

    size = numDMorphTargets * (sizeof(RpDMorphTarget) + sizeof(RwReal));
    targets = RwMalloc(size, rwMEMHINT_DMORPH);
    if (targets == NULL) {
        RwFree(dmorphGeometry);
        return NULL;
    }
    memset(targets, 0, size);
    dmorphGeometry->dmorphTargets = targets;
    dmorphGeometry->values = (RwReal *)&targets[numDMorphTargets];
    return geometry;
}

/* Destroys the delta morph targets of a geometry */
RpGeometry *fn_80443908(RpGeometry *geometry) {
    RpDMorphGeometry **plugin = RWPLUGINOFFSET(RpDMorphGeometry *, geometry, lbl_805F1428);
    RpDMorphGeometry *dmorphGeometry = *plugin;
    RwUInt32 i;

    for (i = 0; i < dmorphGeometry->numDMorphTargets; i++) {
        fn_804439A8(geometry, i);
    }
    RwFree(dmorphGeometry->dmorphTargets);
    RwFree(dmorphGeometry);
    *plugin = NULL;
    return geometry;
}

/* Frees the name and data of a geometry's delta morph target index and clears it */
RpGeometry *fn_804439A8(RpGeometry *geometry, RwUInt32 index) {
    RpDMorphTarget *target = &DMORPHGEOMETRY(geometry)->dmorphTargets[index];

    if (target->name != NULL) {
        RwFree(target->name);
    }
    if (target->field_0x10 != NULL) {
        RwFree(target->field_0x10);
    }
    if (target->vertexData != NULL) {
        RwFree(target->vertexData);
    }
    memset(target, 0, sizeof(RpDMorphTarget));
    return geometry;
}

/* The number of delta morph targets of a geometry */
RwUInt32 fn_80443A54(const RpGeometry *geometry) {
    RpDMorphGeometry *dmorphGeometry = DMORPHGEOMETRY(geometry);

    if (dmorphGeometry != NULL) {
        return dmorphGeometry->numDMorphTargets;
    }
    return 0;
}

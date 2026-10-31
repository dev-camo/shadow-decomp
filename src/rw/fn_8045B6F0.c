/* RenderWare material lists: initialising, emptying, growing and searching a list of materials,
 * and reading one from a stream. The original file name is unknown. */
#include "rw/rpworld.h"

/* how many slots a full list grows by */
#define rpMATERIALLISTGRANULARITY 20

/* Empties a list, releasing its materials */
RpMaterialList *fn_8045B6F0(RpMaterialList *matList) {
    RpMaterial **materials = matList->materials;

    if (materials != NULL) {
        RwInt32 count = matList->numMaterials;
        RwInt32 i;

        for (i = 0; i < count; i++) {
            fn_8045B1F4(materials[i]);
            materials[i] = NULL;
        }
        RwFree(materials);
        matList->materials = NULL;
    }
    matList->numMaterials = 0;
    matList->space = 0;
    return matList;
}

RpMaterialList *fn_8045B788(RpMaterialList *matList) {
    matList->space = 0;
    matList->materials = NULL;
    matList->numMaterials = 0;
    return matList;
}

RpMaterial *fn_8045B79C(const RpMaterialList *matList, RwInt32 index) {
    return matList->materials[index];
}

/* Makes room for at least size materials (inferred) */
static inline RpMaterialList *MaterialListSetSize(RpMaterialList *matList, RwInt32 size) {
    if (matList->space < size) {
        RwUInt32 bytes = size * sizeof(RpMaterial *);
        RpMaterial **materials;

        if (matList->materials != NULL) {
            materials = RwRealloc(matList->materials, bytes,
                                  rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_EVENT | rwID_MATLIST);
        } else {
            materials =
                RwMalloc(bytes, rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_EVENT | rwID_MATLIST);
        }
        if (materials == NULL) {
            RPWORLDERROR((E_RW_NOMEM, bytes));
            return NULL;
        }
        matList->materials = materials;
        matList->space = size;
    }
    return matList;
}

/* Appends a material, taking a reference to it: its index, or -1 */
RwInt32 fn_8045B7AC(RpMaterialList *matList, RpMaterial *material) {
    if (matList->space > matList->numMaterials) {
        matList->materials[matList->numMaterials] = material;
        material->refCount++;
        matList->numMaterials++;
        return matList->numMaterials - 1;
    } else {
        /* Matching workaround: the allocation calls below repeat the size expression instead of
         * passing bytes (which only the error report uses). Passing bytes changes the register
         * assignment and the placement of a register move in this function. Revisit when the
         * original's allocation form is found. */
        RwUInt32 bytes = (matList->space + rpMATERIALLISTGRANULARITY) * sizeof(RpMaterial *);
        RpMaterial **materials;

        if (matList->materials != NULL) {
            materials =
                RwRealloc(matList->materials,
                          (matList->space + rpMATERIALLISTGRANULARITY) * sizeof(RpMaterial *),
                          rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_EVENT | rwID_MATLIST);
        } else {
            materials =
                RwMalloc((matList->space + rpMATERIALLISTGRANULARITY) * sizeof(RpMaterial *),
                         rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_EVENT | rwID_MATLIST);
        }
        if (materials == NULL) {
            RPWORLDERROR((E_RW_NOMEM, bytes));
            return -1;
        }
        matList->materials = materials;
        matList->space += rpMATERIALLISTGRANULARITY;
        materials[matList->numMaterials] = material;
        material->refCount++;
        matList->numMaterials++;
        return matList->numMaterials - 1;
    }
}

/* The index of a material in a list, or -1 */
RwInt32 fn_8045B8EC(const RpMaterialList *matList, const RpMaterial *material) {
    RwInt32 i = matList->numMaterials;

    while (i-- > 0) {
        if (matList->materials[i] == material) {
            break;
        }
    }
    return i;
}

/* Reads a list: each entry is a new material in the stream, or the index of an earlier one */
RpMaterialList *fn_8045B924(RwStream *stream, RpMaterialList *matList) {
    RwInt32 numMaterials;
    RwUInt32 length;
    RwUInt32 version;
    RwBool status;
    RwInt32 *indices;
    RwInt32 i;
    RpMaterial *material;

    if (!fn_8047CF50(stream, rwID_STRUCT, &length, &version)) {
        return NULL;
    }
    if (version >= rwLIBRARYBASEVERSION && version <= RW_STREAM_VERSION) {
        status = fn_8047D868(stream, &numMaterials, sizeof(RwInt32)) != NULL;
        if (!status) {
            return NULL;
        }
        fn_8045B788(matList);
        if (numMaterials == 0) {
            return matList;
        }
        if (MaterialListSetSize(matList, numMaterials) == NULL) {
            fn_8045B6F0(matList);
            return NULL;
        }

        indices =
            RwMalloc(numMaterials * sizeof(RwInt32), rwMEMHINTDUR_FUNCTION | rwID_MATERIALMODULE);
        status = fn_8047D868(stream, indices, numMaterials * sizeof(RwInt32)) != NULL;
        if (!status) {
            RwFree(indices);
            fn_8045B6F0(matList);
            return NULL;
        }

        for (i = 0; i < numMaterials; i++) {
            if (indices[i] < 0) {
                /* a new material */
                if (!fn_8047CF50(stream, rwID_MATERIAL, NULL, &version)) {
                    RwFree(indices);
                    fn_8045B6F0(matList);
                    return NULL;
                }
                if (version >= rwLIBRARYBASEVERSION && version <= RW_STREAM_VERSION) {
                    material = fn_8045B348(stream);
                    if (material == NULL) {
                        RwFree(indices);
                        fn_8045B6F0(matList);
                        return NULL;
                    }
                } else {
                    RPWORLDERROR((E_RW_BADVERSION));
                    RwFree(indices);
                    fn_8045B6F0(matList);
                    return NULL;
                }
            } else {
                /* an earlier material */
                material = fn_8045B79C(matList, indices[i]);
                material->refCount++;
            }
            fn_8045B7AC(matList, material);
            fn_8045B1F4(material);
        }
        RwFree(indices);
        return matList;
    }
    RPWORLDERROR((E_RW_BADVERSION));
    return NULL;
}

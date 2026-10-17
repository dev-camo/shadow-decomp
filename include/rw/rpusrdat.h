#ifndef RW_RPUSRDAT_H
#define RW_RPUSRDAT_H

#include "rw/rpworld.h"
#include "layout_check.h"

/* RenderWare user data (plugin ID 0x11F): named arrays of integers, reals or strings that the
 * plugin keeps for geometries, world sectors, frames, cameras, lights, materials and textures and
 * carries through their streams. The type and field names are inferred from the stream format and
 * the code that uses them. */

#ifdef __cplusplus
extern "C" {
#endif

#define rpID_USERDATAPLUGIN 0x11F

/* RpUserDataArray formats (inferred from the element sizes and stream functions) */
#define rpNAUSERDATAFORMAT 0
#define rpINTUSERDATA 1    /* RwInt32 elements */
#define rpREALUSERDATA 2   /* RwReal elements */
#define rpSTRINGUSERDATA 3 /* RwChar * elements, each allocated */

/* A named array of user data (0x10 bytes) */
typedef struct RpUserDataArray {
    /* 0x0 */ RwChar *name;
    /* 0x4 */ RwInt32 format; /* rp*USERDATA */
    /* 0x8 */ RwInt32 numElements;
    /* 0xC */ void *data;
} RpUserDataArray;
CHECK_SIZEOF(RpUserDataArray, 0x10);

/* The user data arrays of an object: the plugin data in each object type (8 bytes) */
typedef struct RpUserDataList {
    /* 0x0 */ RwInt32 numElements;
    /* 0x4 */ RpUserDataArray *arrays;
} RpUserDataList;
CHECK_SIZEOF(RpUserDataList, 0x8);

/* the plugin offsets of the user data list in each object type, followed by the stream extension
 * offsets (inferred from the attach order) */
extern RwInt32 lbl_805F1458; /* geometry */
extern RwInt32 lbl_805F1460; /* world sector */

/* object accessors (inferred) */
RwInt32 fn_80452B90(RpGeometry *geometry); /* the number of arrays with data */
RpUserDataArray *fn_80452BD8(RpGeometry *geometry, RwInt32 index);
RwInt32 fn_80452C04(RpWorldSector *sector); /* the number of arrays with data */
RpUserDataArray *fn_80452C4C(RpWorldSector *sector, RwInt32 index);

/* array accessors (inferred) */
RwChar *fn_80452C78(RpUserDataArray *array);                /* name */
RwInt32 fn_80452C80(RpUserDataArray *array);                /* format */
RwChar *fn_80452C88(RpUserDataArray *array, RwInt32 index); /* string element */
RwInt32 fn_80452C98(RwInt32 format);                        /* element size */

/* attaches the plugin to the engine and all object types */
RwBool fn_80452CDC(void);

#ifdef __cplusplus
}
#endif

#endif

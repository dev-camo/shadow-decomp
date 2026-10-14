/* RenderWare bounding boxes. The original file name is unknown. */
#include "rw/rwcore.h"

/* The bounding box of numVerts points */
RwBBox *fn_8048349C(RwBBox *boundBox, const RwV3d *verts, RwInt32 numVerts) {
    boundBox->inf = *verts;
    boundBox->sup = *verts;
    numVerts--;
    verts++;
    while (numVerts--) {
        if (boundBox->inf.x > verts->x) {
            boundBox->inf.x = verts->x;
        }
        if (boundBox->inf.y > verts->y) {
            boundBox->inf.y = verts->y;
        }
        if (boundBox->inf.z > verts->z) {
            boundBox->inf.z = verts->z;
        }
        if (boundBox->sup.x < verts->x) {
            boundBox->sup.x = verts->x;
        }
        if (boundBox->sup.y < verts->y) {
            boundBox->sup.y = verts->y;
        }
        if (boundBox->sup.z < verts->z) {
            boundBox->sup.z = verts->z;
        }
        verts++;
    }
    return boundBox;
}

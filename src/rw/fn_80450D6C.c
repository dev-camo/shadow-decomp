/* RenderWare skinning plugin (plugin ID 0x116), GameCube: the skinning kernels called by the skin
 * pipeline (fn_8044D344, fn_8044D554 and fn_8044D6E8). fn_80450D6C combines the bone matrices; the
 * others skin a geometry's vertices with up to two, three or four weights per vertex, writing
 * positions alone or positions and normals. The kernels are hand-written assembly rather than
 * compiler output: they save only the registers they use, at the bottom of the frame where MWCC's
 * prologues put them at the top (two use f24 without saving it), and compute in paired singles
 * with quantized loads and stores. The original file name is unknown.
 *
 * The vertex kernels share one scheme:
 * - matrices holds a matrix per bone, and skin->indices and skin->weights hold each vertex's bone
 *   indices and weights as bytes, heaviest first (fn_8044E408). Two of them are read as a
 *   halfword, four as a word and three byte by byte.
 * - The caller sets GQR5 to load the weights as unsigned bytes scaled by 1/128, and GQR6 and GQR7
 *   to the position and normal formats, which serve both the source and the destination.
 * - A skinned position is the sum over the vertex's weights of weight * (position * matrix);
 *   normals are transformed without the translation. A zero weight ends the sum, since the
 *   weights are sorted.
 * - The stream pointers start one component back for the update-form loads and stores (psq_lux,
 *   psq_stux), which advance by a component and then by two for z. Each normal is followed by
 *   nbtSize bytes of binormal and tangent, which are skipped. */
#include "rw/rpskin.h"

/* Combines the bone matrices of a skin: for each of the numBoneIds bones listed in boneIds,
 * matrices[bone] = skinToBone[bone] * boneMatrices[bone] * toSkin, multiplying row vectors with the
 * translation. The matrices' flags are not written. */
asm void fn_80450D6C(register RwMatrix *matrices, register RwMatrix *skinToBone,
                     register RwMatrix *boneMatrices, register RwMatrix *toSkin,
                     register RwUInt8 *boneIds, register RwUInt32 numBoneIds) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the non-volatile registers it uses */
    stwu     r1, -0x98(r1)
    stfd     f14, 0x8(r1)
    stfd     f15, 0x10(r1)
    stfd     f16, 0x18(r1)
    stfd     f17, 0x20(r1)
    stfd     f18, 0x28(r1)
    stfd     f19, 0x30(r1)
    stfd     f20, 0x38(r1)
    stfd     f21, 0x40(r1)
    stfd     f22, 0x48(r1)
    stfd     f23, 0x50(r1)
    stfd     f24, 0x58(r1)
    stfd     f25, 0x60(r1)
    stfd     f26, 0x68(r1)
    stfd     f27, 0x70(r1)
    stfd     f28, 0x78(r1)
    stfd     f29, 0x80(r1)
    stfd     f30, 0x88(r1)
    stfd     f31, 0x90(r1)
    /* one pass per listed bone; f0-f7 = toSkin */
    mtctr    numBoneIds
    psq_l    f0, RwMatrix.right(toSkin), 0, 0
    psq_l    f1, RwMatrix.right.z(toSkin), 1, 0
    psq_l    f2, RwMatrix.up(toSkin), 0, 0
    psq_l    f3, RwMatrix.up.z(toSkin), 1, 0
    psq_l    f4, RwMatrix.at(toSkin), 0, 0
    psq_l    f5, RwMatrix.at.z(toSkin), 1, 0
    psq_l    f6, RwMatrix.pos(toSkin), 0, 0
    psq_l    f7, RwMatrix.pos.z(toSkin), 1, 0
    subi     r7, boneIds, 0x1
bone:
    /* f24-f31 = boneMatrices[bone] * toSkin, then matrices[bone] = skinToBone[bone] * f24-f31 */
    lbzu     r11, 0x1(r7)
    rotlwi   r11, r11, 6
    add      r12, boneMatrices, r11
    psq_l    f8, RwMatrix.right(r12), 0, 0
    psq_l    f10, RwMatrix.up(r12), 0, 0
    ps_muls0 f24, f0, f8
    psq_l    f9, RwMatrix.right.z(r12), 1, 0
    ps_muls0 f25, f1, f8
    psq_l    f11, RwMatrix.up.z(r12), 1, 0
    ps_muls0 f26, f0, f10
    psq_l    f12, RwMatrix.at(r12), 0, 0
    ps_muls0 f27, f1, f10
    psq_l    f13, RwMatrix.at.z(r12), 1, 0
    ps_madds1 f24, f2, f8, f24
    psq_l    f14, RwMatrix.pos(r12), 0, 0
    ps_madds1 f25, f3, f8, f25
    psq_l    f15, RwMatrix.pos.z(r12), 1, 0
    ps_madds1 f26, f2, f10, f26
    ps_madds1 f27, f3, f10, f27
    ps_madds0 f24, f4, f9, f24
    ps_madds0 f25, f5, f9, f25
    ps_madds0 f26, f4, f11, f26
    add      r12, skinToBone, r11
    psq_l    f16, RwMatrix.right(r12), 0, 0
    ps_madds0 f27, f5, f11, f27
    psq_l    f17, RwMatrix.right.z(r12), 1, 0
    ps_muls0 f28, f0, f12
    psq_l    f18, RwMatrix.up(r12), 0, 0
    ps_muls0 f29, f1, f12
    psq_l    f19, RwMatrix.up.z(r12), 1, 0
    ps_madds0 f30, f0, f14, f6
    psq_l    f20, RwMatrix.at(r12), 0, 0
    ps_madds0 f31, f1, f14, f7
    psq_l    f21, RwMatrix.at.z(r12), 1, 0
    ps_madds1 f28, f2, f12, f28
    psq_l    f22, RwMatrix.pos(r12), 0, 0
    ps_madds1 f29, f3, f12, f29
    psq_l    f23, RwMatrix.pos.z(r12), 1, 0
    ps_madds1 f30, f2, f14, f30
    ps_madds1 f31, f3, f14, f31
    ps_madds0 f28, f4, f13, f28
    ps_madds0 f29, f5, f13, f29
    ps_madds0 f30, f4, f15, f30
    ps_madds0 f31, f5, f15, f31
    ps_muls0 f8, f24, f16
    ps_muls0 f9, f25, f16
    ps_muls0 f10, f24, f18
    ps_muls0 f11, f25, f18
    ps_madds1 f8, f26, f16, f8
    ps_madds1 f9, f27, f16, f9
    ps_madds1 f10, f26, f18, f10
    ps_madds1 f11, f27, f18, f11
    ps_madds0 f8, f28, f17, f8
    ps_madds0 f9, f29, f17, f9
    ps_madds0 f10, f28, f19, f10
    add      r12, matrices, r11
    psq_st   f8, RwMatrix.right(r12), 0, 0
    ps_madds0 f11, f29, f19, f11
    psq_st   f9, RwMatrix.right.z(r12), 1, 0
    ps_muls0 f12, f24, f20
    psq_st   f10, RwMatrix.up(r12), 0, 0
    ps_muls0 f13, f25, f20
    psq_st   f11, RwMatrix.up.z(r12), 1, 0
    ps_madds0 f14, f24, f22, f30
    ps_madds0 f15, f25, f22, f31
    ps_madds1 f12, f26, f20, f12
    ps_madds1 f13, f27, f20, f13
    ps_madds1 f14, f26, f22, f14
    ps_madds1 f15, f27, f22, f15
    ps_madds0 f12, f28, f21, f12
    ps_madds0 f13, f29, f21, f13
    ps_madds0 f14, f28, f23, f14
    ps_madds0 f15, f29, f23, f15
    psq_st   f12, RwMatrix.at(r12), 0, 0
    psq_st   f13, RwMatrix.at.z(r12), 1, 0
    psq_st   f14, RwMatrix.pos(r12), 0, 0
    psq_st   f15, RwMatrix.pos.z(r12), 1, 0
    bdnz     bone
    lfd      f14, 0x8(r1)
    lfd      f15, 0x10(r1)
    lfd      f16, 0x18(r1)
    lfd      f17, 0x20(r1)
    lfd      f18, 0x28(r1)
    lfd      f19, 0x30(r1)
    lfd      f20, 0x38(r1)
    lfd      f21, 0x40(r1)
    lfd      f22, 0x48(r1)
    lfd      f23, 0x50(r1)
    lfd      f24, 0x58(r1)
    lfd      f25, 0x60(r1)
    lfd      f26, 0x68(r1)
    lfd      f27, 0x70(r1)
    lfd      f28, 0x78(r1)
    lfd      f29, 0x80(r1)
    lfd      f30, 0x88(r1)
    lfd      f31, 0x90(r1)
    addi     r1, r1, 0x98
    blr
#endif // clang-format on
}

/* Skins the positions of a geometry with up to two weights per vertex. */
asm void fn_80450F68(register RwMatrix *matrices, register RpSkin *skin,
                     register SkinKernelParams *params) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the non-volatile registers it uses */
    stwu     r1, -0x18(r1)
    stfd     f30, 0x8(r1)
    stfd     f31, 0x10(r1)
    /* nothing to do without vertices */
    lwz      r0, SkinKernelParams.numVertices(params)
    cmpwi    r0, 0
    mtctr    r0
    ble      done
    /* the positions, and the indices and weights as a halfword per vertex; r9 is twice the
     * component size and f30 zero */
    lwz      r6, SkinKernelParams.dstPositions(params)
    lwz      r7, SkinKernelParams.srcPositions(params)
    lwz      r8, SkinKernelParams.positionSize(params)
    lwz      r5, RpSkin.indices(skin)
    lwz      r4, RpSkin.weights(skin)
    rotlwi   r9, r8, 1
    subf     r6, r8, r6
    subf     r7, r8, r7
    ps_sub   f30, f30, f30
    subi     r5, r5, 0x2
    subi     r4, r4, 0x2
    /* the first vertex */
    lhzu     r0, 0x2(r5)
    psq_lux  f8, r7, r8, 0, 6
    psq_lux  f9, r7, r9, 1, 6
vertex:
    /* f10/f11 = position * matrices[index 0] * weight 0 */
    rlwinm   r10, r0, 30, 18, 25
    add      r10, r10, matrices
    psq_l    f0, RwMatrix.right(r10), 0, 0
    psq_l    f1, RwMatrix.right.z(r10), 1, 0
    psq_l    f6, RwMatrix.pos(r10), 0, 0
    psq_l    f7, RwMatrix.pos.z(r10), 1, 0
    psq_l    f2, RwMatrix.up(r10), 0, 0
    psq_l    f3, RwMatrix.up.z(r10), 1, 0
    ps_madds0 f10, f0, f8, f6
    ps_madds0 f11, f1, f8, f7
    psq_l    f4, RwMatrix.at(r10), 0, 0
    psq_l    f5, RwMatrix.at.z(r10), 1, 0
    ps_madds1 f10, f2, f8, f10
    ps_madds1 f11, f3, f8, f11
    psq_lu   f31, 0x2(r4), 0, 5
    ps_madds0 f10, f4, f9, f10
    ps_madds0 f11, f5, f9, f11
    ps_cmpo1 cr5, f31, f30
    ps_muls0 f10, f10, f31
    ps_muls0 f11, f11, f31
    ble      cr5, store
    /* unless weight 1 is zero: += position * matrices[index 1] * weight 1 */
    clrlslwi r10, r0, 24, 6
    add      r10, r10, matrices
    psq_l    f0, RwMatrix.right(r10), 0, 0
    psq_l    f1, RwMatrix.right.z(r10), 1, 0
    psq_l    f6, RwMatrix.pos(r10), 0, 0
    psq_l    f7, RwMatrix.pos.z(r10), 1, 0
    psq_l    f2, RwMatrix.up(r10), 0, 0
    psq_l    f3, RwMatrix.up.z(r10), 1, 0
    ps_madds0 f12, f0, f8, f6
    ps_madds0 f13, f1, f8, f7
    psq_l    f4, RwMatrix.at(r10), 0, 0
    psq_l    f5, RwMatrix.at.z(r10), 1, 0
    ps_madds1 f12, f2, f8, f12
    ps_madds1 f13, f3, f8, f13
    ps_madds0 f12, f4, f9, f12
    ps_madds0 f13, f5, f9, f13
    ps_madds1 f10, f12, f31, f10
    ps_madds1 f11, f13, f31, f11
    /* load the next vertex and write the skinned position */
store:
    lhzu     r0, 0x2(r5)
    psq_lux  f8, r7, r8, 0, 6
    psq_lux  f9, r7, r9, 1, 6
    psq_stux f10, r6, r8, 0, 6
    psq_stux f11, r6, r9, 1, 6
    bdnz     vertex
done:
    lfd      f30, 0x8(r1)
    lfd      f31, 0x10(r1)
    addi     r1, r1, 0x18
    blr
#endif // clang-format on
}

/* Skins the positions and normals of a geometry with up to two weights per vertex. */
asm void fn_80451080(register RwMatrix *matrices, register RpSkin *skin,
                     register SkinKernelParams *params) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the non-volatile registers it uses */
    stwu     r1, -0x58(r1)
    stw      r29, 0x8(r1)
    stw      r30, 0xc(r1)
    stw      r31, 0x10(r1)
    stfd     f24, 0x18(r1)
    stfd     f25, 0x20(r1)
    stfd     f26, 0x28(r1)
    stfd     f27, 0x30(r1)
    stfd     f28, 0x38(r1)
    stfd     f29, 0x40(r1)
    stfd     f30, 0x48(r1)
    stfd     f31, 0x50(r1)
    /* nothing to do without vertices */
    lwz      r0, SkinKernelParams.numVertices(params)
    cmpwi    r0, 0
    mtctr    r0
    ble      done
    /* the positions and normals, the binormal and tangent size, and the indices and weights as a
     * halfword per vertex; r30 and r31 are twice the component sizes and f13 zero */
    lwz      r6, SkinKernelParams.dstPositions(params)
    lwz      r7, SkinKernelParams.dstNormals(params)
    lwz      r8, SkinKernelParams.srcPositions(params)
    lwz      r9, SkinKernelParams.srcNormals(params)
    lwz      r10, SkinKernelParams.positionSize(params)
    lwz      r11, SkinKernelParams.normalSize(params)
    lwz      r12, SkinKernelParams.nbtSize(params)
    lwz      r5, RpSkin.indices(skin)
    lwz      r4, RpSkin.weights(skin)
    rotlwi   r30, r10, 1
    rotlwi   r31, r11, 1
    subi     r5, r5, 0x2
    subi     r4, r4, 0x2
    subf     r6, r10, r6
    subf     r7, r11, r7
    subf     r8, r10, r8
    subf     r9, r11, r9
    ps_sub   f13, f13, f13
    /* the first vertex */
    lhzu     r0, 0x2(r5)
    psq_lux  f8, r8, r10, 0, 6
    psq_lux  f9, r8, r30, 1, 6
vertex:
    /* the normal; f24/f25 = position * matrices[index 0] * weight 0 and f26/f27 = normal *
     * matrices[index 0] * weight 0, without the translation */
    rlwinm   r29, r0, 30, 18, 25
    add      r29, r29, matrices
    psq_lux  f10, r9, r11, 0, 7
    psq_lux  f11, r9, r31, 1, 7
    add      r9, r9, r12
    psq_l    f0, RwMatrix.right(r29), 0, 0
    psq_l    f1, RwMatrix.right.z(r29), 1, 0
    psq_l    f6, RwMatrix.pos(r29), 0, 0
    psq_l    f7, RwMatrix.pos.z(r29), 1, 0
    ps_muls0 f26, f0, f10
    ps_muls0 f27, f1, f10
    psq_l    f2, RwMatrix.up(r29), 0, 0
    psq_l    f3, RwMatrix.up.z(r29), 1, 0
    ps_madds0 f24, f0, f8, f6
    ps_madds0 f25, f1, f8, f7
    psq_l    f4, RwMatrix.at(r29), 0, 0
    psq_l    f5, RwMatrix.at.z(r29), 1, 0
    ps_madds1 f26, f2, f10, f26
    ps_madds1 f27, f3, f10, f27
    psq_lu   f12, 0x2(r4), 0, 5
    ps_madds1 f24, f2, f8, f24
    ps_madds1 f25, f3, f8, f25
    ps_cmpo1 cr7, f12, f13
    ps_madds0 f26, f4, f11, f26
    ps_madds0 f27, f5, f11, f27
    ps_madds0 f24, f4, f9, f24
    ps_madds0 f25, f5, f9, f25
    ps_muls0 f26, f26, f12
    ps_muls0 f27, f27, f12
    ps_muls0 f24, f24, f12
    ps_muls0 f25, f25, f12
    ble      cr7, store
    /* unless weight 1 is zero: add matrices[index 1] the same way */
    clrlslwi r29, r0, 24, 6
    add      r29, r29, matrices
    psq_l    f0, RwMatrix.right(r29), 0, 0
    psq_l    f1, RwMatrix.right.z(r29), 1, 0
    psq_l    f6, RwMatrix.pos(r29), 0, 0
    psq_l    f7, RwMatrix.pos.z(r29), 1, 0
    ps_muls0 f30, f0, f10
    ps_muls0 f31, f1, f10
    psq_l    f2, RwMatrix.up(r29), 0, 0
    psq_l    f3, RwMatrix.up.z(r29), 1, 0
    ps_madds0 f28, f0, f8, f6
    ps_madds0 f29, f1, f8, f7
    psq_l    f4, RwMatrix.at(r29), 0, 0
    psq_l    f5, RwMatrix.at.z(r29), 1, 0
    ps_madds1 f30, f2, f10, f30
    ps_madds1 f31, f3, f10, f31
    ps_madds1 f28, f2, f8, f28
    ps_madds1 f29, f3, f8, f29
    ps_madds0 f30, f4, f11, f30
    ps_madds0 f31, f5, f11, f31
    ps_madds0 f28, f4, f9, f28
    ps_madds0 f29, f5, f9, f29
    ps_madds1 f26, f30, f12, f26
    ps_madds1 f27, f31, f12, f27
    ps_madds1 f24, f28, f12, f24
    ps_madds1 f25, f29, f12, f25
    /* load the next position and write the skinned normal and position */
store:
    lhzu     r0, 0x2(r5)
    psq_lux  f8, r8, r10, 0, 6
    psq_lux  f9, r8, r30, 1, 6
    psq_stux f26, r7, r11, 0, 7
    psq_stux f27, r7, r31, 1, 7
    add      r7, r7, r12
    psq_stux f24, r6, r10, 0, 6
    psq_stux f25, r6, r30, 1, 6
    bdnz     vertex
done:
    lwz      r29, 0x8(r1)
    lwz      r30, 0xc(r1)
    lwz      r31, 0x10(r1)
    lfd      f24, 0x18(r1)
    lfd      f25, 0x20(r1)
    lfd      f26, 0x28(r1)
    lfd      f27, 0x30(r1)
    lfd      f28, 0x38(r1)
    lfd      f29, 0x40(r1)
    lfd      f30, 0x48(r1)
    lfd      f31, 0x50(r1)
    addi     r1, r1, 0x58
    blr
#endif // clang-format on
}

/* Skins the positions of a geometry with up to three weights per vertex. */
asm void fn_80451254(register RwMatrix *matrices, register RpSkin *skin,
                     register SkinKernelParams *params) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the non-volatile registers it uses */
    stwu     r1, -0x18(r1)
    stfd     f30, 0x8(r1)
    stfd     f31, 0x10(r1)
    /* nothing to do without vertices */
    lwz      r0, SkinKernelParams.numVertices(params)
    cmpwi    r0, 0
    mtctr    r0
    ble      done
    /* the positions, and the indices and weights as three bytes per vertex; r9 is twice the
     * component size */
    lwz      r6, SkinKernelParams.dstPositions(params)
    lwz      r7, SkinKernelParams.srcPositions(params)
    lwz      r8, SkinKernelParams.positionSize(params)
    lwz      r5, RpSkin.indices(skin)
    lwz      r4, RpSkin.weights(skin)
    rotlwi   r9, r8, 1
    subi     r5, r5, 0x1
    subi     r4, r4, 0x1
    subf     r6, r8, r6
    subf     r7, r8, r7
    /* f30 = (weight 0, weight 1), f31 = (weight 2, 1); the indices and the position */
vertex:
    ps_sub   f0, f0, f0
    psq_lu   f30, 0x1(r4), 1, 5
    psq_lu   f31, 0x1(r4), 1, 5
    ps_merge00 f30, f30, f31
    psq_lu   f31, 0x1(r4), 1, 5
    lbzu     r10, 0x1(r5)
    lbzu     r11, 0x1(r5)
    lbzu     r12, 0x1(r5)
    ps_cmpo1 cr5, f30, f0
    ps_cmpo0 cr6, f31, f0
    psq_lux  f8, r7, r8, 0, 6
    psq_lux  f9, r7, r9, 1, 6
    /* f10/f11 = position * matrices[index 0] * weight 0 */
    clrlslwi r10, r10, 24, 6
    add      r10, r10, matrices
    psq_l    f0, RwMatrix.right(r10), 0, 0
    psq_l    f1, RwMatrix.right.z(r10), 1, 0
    psq_l    f6, RwMatrix.pos(r10), 0, 0
    psq_l    f7, RwMatrix.pos.z(r10), 1, 0
    psq_l    f2, RwMatrix.up(r10), 0, 0
    psq_l    f3, RwMatrix.up.z(r10), 1, 0
    ps_madds0 f10, f0, f8, f6
    ps_madds0 f11, f1, f8, f7
    psq_l    f4, RwMatrix.at(r10), 0, 0
    psq_l    f5, RwMatrix.at.z(r10), 1, 0
    ps_madds1 f10, f2, f8, f10
    ps_madds1 f11, f3, f8, f11
    ps_madds0 f10, f4, f9, f10
    ps_madds0 f11, f5, f9, f11
    ps_muls0 f10, f10, f30
    ps_muls0 f11, f11, f30
    ble      cr5, store
    /* unless weight 1 is zero: += position * matrices[index 1] * weight 1 */
    clrlslwi r11, r11, 24, 6
    add      r11, r11, matrices
    psq_l    f0, RwMatrix.right(r11), 0, 0
    psq_l    f1, RwMatrix.right.z(r11), 1, 0
    psq_l    f6, RwMatrix.pos(r11), 0, 0
    psq_l    f7, RwMatrix.pos.z(r11), 1, 0
    psq_l    f2, RwMatrix.up(r11), 0, 0
    psq_l    f3, RwMatrix.up.z(r11), 1, 0
    ps_madds0 f12, f0, f8, f6
    ps_madds0 f13, f1, f8, f7
    psq_l    f4, RwMatrix.at(r11), 0, 0
    psq_l    f5, RwMatrix.at.z(r11), 1, 0
    ps_madds1 f12, f2, f8, f12
    ps_madds1 f13, f3, f8, f13
    ps_madds0 f12, f4, f9, f12
    ps_madds0 f13, f5, f9, f13
    ps_madds1 f10, f12, f30, f10
    ps_madds1 f11, f13, f30, f11
    ble      cr6, store
    /* unless weight 2 is zero: += position * matrices[index 2] * weight 2 */
    clrlslwi r12, r12, 24, 6
    add      r12, r12, matrices
    psq_l    f0, RwMatrix.right(r12), 0, 0
    psq_l    f1, RwMatrix.right.z(r12), 1, 0
    psq_l    f6, RwMatrix.pos(r12), 0, 0
    psq_l    f7, RwMatrix.pos.z(r12), 1, 0
    psq_l    f2, RwMatrix.up(r12), 0, 0
    psq_l    f3, RwMatrix.up.z(r12), 1, 0
    ps_madds0 f12, f0, f8, f6
    ps_madds0 f13, f1, f8, f7
    psq_l    f4, RwMatrix.at(r12), 0, 0
    psq_l    f5, RwMatrix.at.z(r12), 1, 0
    ps_madds1 f12, f2, f8, f12
    ps_madds1 f13, f3, f8, f13
    ps_madds0 f12, f4, f9, f12
    ps_madds0 f13, f5, f9, f13
    ps_madds0 f10, f12, f31, f10
    ps_madds0 f11, f13, f31, f11
    /* write the skinned position */
store:
    psq_stux f10, r6, r8, 0, 6
    psq_stux f11, r6, r9, 1, 6
    bdnz     vertex
done:
    lfd      f30, 0x8(r1)
    lfd      f31, 0x10(r1)
    addi     r1, r1, 0x18
    blr
#endif // clang-format on
}

/* Skins the positions and normals of a geometry with up to three weights per vertex. */
asm void fn_804513C4(register RwMatrix *matrices, register RpSkin *skin,
                     register SkinKernelParams *params) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the non-volatile registers it uses; f24 is used too but is not saved */
    stwu     r1, -0x58(r1)
    stw      r27, 0x8(r1)
    stw      r28, 0xc(r1)
    stw      r29, 0x10(r1)
    stw      r30, 0x14(r1)
    stw      r31, 0x18(r1)
    stfd     f25, 0x20(r1)
    stfd     f26, 0x28(r1)
    stfd     f27, 0x30(r1)
    stfd     f28, 0x38(r1)
    stfd     f29, 0x40(r1)
    stfd     f30, 0x48(r1)
    stfd     f31, 0x50(r1)
    /* nothing to do without vertices */
    lwz      r0, SkinKernelParams.numVertices(params)
    cmpwi    r0, 0
    mtctr    r0
    ble      done
    /* the positions and normals, the binormal and tangent size, and the indices and weights as
     * three bytes per vertex; r27 and r28 are twice the component sizes */
    lwz      r6, SkinKernelParams.dstPositions(params)
    lwz      r7, SkinKernelParams.dstNormals(params)
    lwz      r8, SkinKernelParams.srcPositions(params)
    lwz      r9, SkinKernelParams.srcNormals(params)
    lwz      r10, SkinKernelParams.positionSize(params)
    lwz      r11, SkinKernelParams.normalSize(params)
    lwz      r12, SkinKernelParams.nbtSize(params)
    lwz      r5, RpSkin.indices(skin)
    lwz      r4, RpSkin.weights(skin)
    rotlwi   r27, r10, 1
    rotlwi   r28, r11, 1
    subi     r5, r5, 0x1
    subi     r4, r4, 0x1
    subf     r6, r10, r6
    subf     r7, r11, r7
    subf     r8, r10, r8
    subf     r9, r11, r9
    /* f12 = (weight 0, weight 1), f13 = (weight 2, 1); the position, the normal and the three
     * matrices */
vertex:
    ps_sub   f0, f0, f0
    psq_lu   f12, 0x1(r4), 1, 5
    psq_lu   f13, 0x1(r4), 1, 5
    ps_merge00 f12, f12, f13
    psq_lu   f13, 0x1(r4), 1, 5
    psq_lux  f8, r8, r10, 0, 6
    psq_lux  f9, r8, r27, 1, 6
    ps_cmpo1 cr5, f12, f0
    ps_cmpo0 cr6, f13, f0
    psq_lux  f10, r9, r11, 0, 7
    psq_lux  f11, r9, r28, 1, 7
    add      r9, r9, r12
    lbzu     r29, 0x1(r5)
    clrlslwi r29, r29, 24, 6
    add      r29, r29, matrices
    lbzu     r30, 0x1(r5)
    clrlslwi r30, r30, 24, 6
    add      r30, r30, matrices
    lbzu     r31, 0x1(r5)
    clrlslwi r31, r31, 24, 6
    add      r31, r31, matrices
    /* f24/f25 = position * matrices[index 0] * weight 0 and f26/f27 = normal * matrices[index 0] *
     * weight 0, without the translation */
    psq_l    f0, RwMatrix.right(r29), 0, 0
    psq_l    f1, RwMatrix.right.z(r29), 1, 0
    psq_l    f6, RwMatrix.pos(r29), 0, 0
    psq_l    f7, RwMatrix.pos.z(r29), 1, 0
    ps_muls0 f26, f0, f10
    ps_muls0 f27, f1, f10
    psq_l    f2, RwMatrix.up(r29), 0, 0
    psq_l    f3, RwMatrix.up.z(r29), 1, 0
    ps_madds0 f24, f0, f8, f6
    ps_madds0 f25, f1, f8, f7
    psq_l    f4, RwMatrix.at(r29), 0, 0
    psq_l    f5, RwMatrix.at.z(r29), 1, 0
    ps_madds1 f26, f2, f10, f26
    ps_madds1 f27, f3, f10, f27
    ps_madds1 f24, f2, f8, f24
    ps_madds1 f25, f3, f8, f25
    ps_madds0 f26, f4, f11, f26
    ps_madds0 f27, f5, f11, f27
    ps_madds0 f24, f4, f9, f24
    ps_madds0 f25, f5, f9, f25
    ps_muls0 f26, f26, f12
    ps_muls0 f27, f27, f12
    ps_muls0 f24, f24, f12
    ps_muls0 f25, f25, f12
    /* unless weight 1 is zero: add matrices[index 1] the same way */
    ble      cr5, store
    psq_l    f0, RwMatrix.right(r30), 0, 0
    psq_l    f1, RwMatrix.right.z(r30), 1, 0
    psq_l    f6, RwMatrix.pos(r30), 0, 0
    psq_l    f7, RwMatrix.pos.z(r30), 1, 0
    ps_muls0 f30, f0, f10
    ps_muls0 f31, f1, f10
    psq_l    f2, RwMatrix.up(r30), 0, 0
    psq_l    f3, RwMatrix.up.z(r30), 1, 0
    ps_madds0 f28, f0, f8, f6
    ps_madds0 f29, f1, f8, f7
    psq_l    f4, RwMatrix.at(r30), 0, 0
    psq_l    f5, RwMatrix.at.z(r30), 1, 0
    ps_madds1 f30, f2, f10, f30
    ps_madds1 f31, f3, f10, f31
    ps_madds1 f28, f2, f8, f28
    ps_madds1 f29, f3, f8, f29
    ps_madds0 f30, f4, f11, f30
    ps_madds0 f31, f5, f11, f31
    ps_madds0 f28, f4, f9, f28
    ps_madds0 f29, f5, f9, f29
    ps_madds1 f26, f30, f12, f26
    ps_madds1 f27, f31, f12, f27
    ps_madds1 f24, f28, f12, f24
    ps_madds1 f25, f29, f12, f25
    /* unless weight 2 is zero: add matrices[index 2] the same way */
    ble      cr6, store
    psq_l    f0, RwMatrix.right(r31), 0, 0
    psq_l    f1, RwMatrix.right.z(r31), 1, 0
    psq_l    f6, RwMatrix.pos(r31), 0, 0
    psq_l    f7, RwMatrix.pos.z(r31), 1, 0
    ps_muls0 f30, f0, f10
    ps_muls0 f31, f1, f10
    psq_l    f2, RwMatrix.up(r31), 0, 0
    psq_l    f3, RwMatrix.up.z(r31), 1, 0
    ps_madds0 f28, f0, f8, f6
    ps_madds0 f29, f1, f8, f7
    psq_l    f4, RwMatrix.at(r31), 0, 0
    psq_l    f5, RwMatrix.at.z(r31), 1, 0
    ps_madds1 f30, f2, f10, f30
    ps_madds1 f31, f3, f10, f31
    ps_madds1 f28, f2, f8, f28
    ps_madds1 f29, f3, f8, f29
    ps_madds0 f30, f4, f11, f30
    ps_madds0 f31, f5, f11, f31
    ps_madds0 f28, f4, f9, f28
    ps_madds0 f29, f5, f9, f29
    ps_madds0 f26, f30, f13, f26
    ps_madds0 f27, f31, f13, f27
    ps_madds0 f24, f28, f13, f24
    ps_madds0 f25, f29, f13, f25
    /* write the skinned normal and position */
store:
    psq_stux f26, r7, r11, 0, 7
    psq_stux f27, r7, r28, 1, 7
    add      r7, r7, r12
    psq_stux f24, r6, r10, 0, 6
    psq_stux f25, r6, r27, 1, 6
    bdnz     vertex
done:
    lwz      r27, 0x8(r1)
    lwz      r28, 0xc(r1)
    lwz      r29, 0x10(r1)
    lwz      r30, 0x14(r1)
    lwz      r31, 0x18(r1)
    lfd      f25, 0x20(r1)
    lfd      f26, 0x28(r1)
    lfd      f27, 0x30(r1)
    lfd      f28, 0x38(r1)
    lfd      f29, 0x40(r1)
    lfd      f30, 0x48(r1)
    lfd      f31, 0x50(r1)
    addi     r1, r1, 0x58
    blr
#endif // clang-format on
}

/* Skins the positions of a geometry with up to four weights per vertex. */
asm void fn_80451618(register RwMatrix *matrices, register RpSkin *skin,
                     register SkinKernelParams *params) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the non-volatile registers it uses */
    stwu     r1, -0x18(r1)
    stfd     f30, 0x8(r1)
    stfd     f31, 0x10(r1)
    /* nothing to do without vertices */
    lwz      r0, SkinKernelParams.numVertices(params)
    cmpwi    r0, 0
    mtctr    r0
    ble      done
    /* the positions, and the indices and weights as a word per vertex; r9 is twice the component
     * size */
    lwz      r6, SkinKernelParams.dstPositions(params)
    lwz      r7, SkinKernelParams.srcPositions(params)
    lwz      r8, SkinKernelParams.positionSize(params)
    lwz      r5, RpSkin.indices(skin)
    lwz      r4, RpSkin.weights(skin)
    rotlwi   r9, r8, 1
    subi     r5, r5, 0x4
    subi     r4, r4, 0x2
    subf     r6, r8, r6
    subf     r7, r8, r7
    /* f30 = (weight 0, weight 1), f31 = (weight 2, weight 3); the indices and the position */
vertex:
    ps_sub   f0, f0, f0
    psq_lu   f30, 0x2(r4), 0, 5
    psq_lu   f31, 0x2(r4), 0, 5
    lwzu     r0, 0x4(r5)
    psq_lux  f8, r7, r8, 0, 6
    psq_lux  f9, r7, r9, 1, 6
    ps_cmpo1 cr5, f30, f0
    ps_cmpo0 cr6, f31, f0
    ps_cmpo1 cr7, f31, f0
    /* f10/f11 = position * matrices[index 0] * weight 0 */
    rlwinm   r10, r0, 14, 18, 25
    add      r10, r10, matrices
    psq_l    f0, RwMatrix.right(r10), 0, 0
    psq_l    f1, RwMatrix.right.z(r10), 1, 0
    psq_l    f6, RwMatrix.pos(r10), 0, 0
    psq_l    f7, RwMatrix.pos.z(r10), 1, 0
    psq_l    f2, RwMatrix.up(r10), 0, 0
    psq_l    f3, RwMatrix.up.z(r10), 1, 0
    ps_madds0 f10, f0, f8, f6
    ps_madds0 f11, f1, f8, f7
    psq_l    f4, RwMatrix.at(r10), 0, 0
    psq_l    f5, RwMatrix.at.z(r10), 1, 0
    ps_madds1 f10, f2, f8, f10
    ps_madds1 f11, f3, f8, f11
    ps_madds0 f10, f4, f9, f10
    ps_madds0 f11, f5, f9, f11
    ps_muls0 f10, f10, f30
    ps_muls0 f11, f11, f30
    ble      cr5, store
    /* unless weight 1 is zero: += position * matrices[index 1] * weight 1 */
    rlwinm   r10, r0, 22, 18, 25
    add      r10, r10, matrices
    psq_l    f0, RwMatrix.right(r10), 0, 0
    psq_l    f1, RwMatrix.right.z(r10), 1, 0
    psq_l    f6, RwMatrix.pos(r10), 0, 0
    psq_l    f7, RwMatrix.pos.z(r10), 1, 0
    psq_l    f2, RwMatrix.up(r10), 0, 0
    psq_l    f3, RwMatrix.up.z(r10), 1, 0
    ps_madds0 f12, f0, f8, f6
    ps_madds0 f13, f1, f8, f7
    psq_l    f4, RwMatrix.at(r10), 0, 0
    psq_l    f5, RwMatrix.at.z(r10), 1, 0
    ps_madds1 f12, f2, f8, f12
    ps_madds1 f13, f3, f8, f13
    ps_madds0 f12, f4, f9, f12
    ps_madds0 f13, f5, f9, f13
    ps_madds1 f10, f12, f30, f10
    ps_madds1 f11, f13, f30, f11
    ble      cr6, store
    /* unless weight 2 is zero: += position * matrices[index 2] * weight 2 */
    rlwinm   r10, r0, 30, 18, 25
    add      r10, r10, matrices
    psq_l    f0, RwMatrix.right(r10), 0, 0
    psq_l    f1, RwMatrix.right.z(r10), 1, 0
    psq_l    f6, RwMatrix.pos(r10), 0, 0
    psq_l    f7, RwMatrix.pos.z(r10), 1, 0
    psq_l    f2, RwMatrix.up(r10), 0, 0
    psq_l    f3, RwMatrix.up.z(r10), 1, 0
    ps_madds0 f12, f0, f8, f6
    ps_madds0 f13, f1, f8, f7
    psq_l    f4, RwMatrix.at(r10), 0, 0
    psq_l    f5, RwMatrix.at.z(r10), 1, 0
    ps_madds1 f12, f2, f8, f12
    ps_madds1 f13, f3, f8, f13
    ps_madds0 f12, f4, f9, f12
    ps_madds0 f13, f5, f9, f13
    ps_madds0 f10, f12, f31, f10
    ps_madds0 f11, f13, f31, f11
    ble      cr7, store
    /* unless weight 3 is zero: += position * matrices[index 3] * weight 3 */
    clrlslwi r10, r0, 24, 6
    add      r10, r10, matrices
    psq_l    f0, RwMatrix.right(r10), 0, 0
    psq_l    f1, RwMatrix.right.z(r10), 1, 0
    psq_l    f6, RwMatrix.pos(r10), 0, 0
    psq_l    f7, RwMatrix.pos.z(r10), 1, 0
    psq_l    f2, RwMatrix.up(r10), 0, 0
    psq_l    f3, RwMatrix.up.z(r10), 1, 0
    ps_madds0 f12, f0, f8, f6
    ps_madds0 f13, f1, f8, f7
    psq_l    f4, RwMatrix.at(r10), 0, 0
    psq_l    f5, RwMatrix.at.z(r10), 1, 0
    ps_madds1 f12, f2, f8, f12
    ps_madds1 f13, f3, f8, f13
    ps_madds0 f12, f4, f9, f12
    ps_madds0 f13, f5, f9, f13
    ps_madds1 f10, f12, f31, f10
    ps_madds1 f11, f13, f31, f11
    /* write the skinned position */
store:
    psq_stux f10, r6, r8, 0, 6
    psq_stux f11, r6, r9, 1, 6
    bdnz     vertex
done:
    lfd      f30, 0x8(r1)
    lfd      f31, 0x10(r1)
    addi     r1, r1, 0x18
    blr
#endif // clang-format on
}

/* Skins the positions and normals of a geometry with up to four weights per vertex. */
asm void fn_804517C8(register RwMatrix *matrices, register RpSkin *skin,
                     register SkinKernelParams *params) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the non-volatile registers it uses; f24 is used too but is not saved */
    stwu     r1, -0x50(r1)
    stw      r29, 0x8(r1)
    stw      r30, 0xc(r1)
    stw      r31, 0x10(r1)
    stfd     f25, 0x18(r1)
    stfd     f26, 0x20(r1)
    stfd     f27, 0x28(r1)
    stfd     f28, 0x30(r1)
    stfd     f29, 0x38(r1)
    stfd     f30, 0x40(r1)
    stfd     f31, 0x48(r1)
    /* nothing to do without vertices */
    lwz      r0, SkinKernelParams.numVertices(params)
    cmpwi    r0, 0
    mtctr    r0
    ble      done
    /* the positions and normals, the binormal and tangent size, and the indices and weights as a
     * word per vertex; r29 and r30 are twice the component sizes */
    lwz      r6, SkinKernelParams.dstPositions(params)
    lwz      r7, SkinKernelParams.dstNormals(params)
    lwz      r8, SkinKernelParams.srcPositions(params)
    lwz      r9, SkinKernelParams.srcNormals(params)
    lwz      r10, SkinKernelParams.positionSize(params)
    lwz      r11, SkinKernelParams.normalSize(params)
    lwz      r12, SkinKernelParams.nbtSize(params)
    lwz      r5, RpSkin.indices(skin)
    lwz      r4, RpSkin.weights(skin)
    rotlwi   r29, r10, 1
    rotlwi   r30, r11, 1
    subi     r5, r5, 0x4
    subi     r4, r4, 0x2
    subf     r6, r10, r6
    subf     r7, r11, r7
    subf     r8, r10, r8
    subf     r9, r11, r9
    /* f12 = (weight 0, weight 1), f13 = (weight 2, weight 3); the indices, the position and the
     * normal */
vertex:
    ps_sub   f0, f0, f0
    psq_lu   f12, 0x2(r4), 0, 5
    psq_lu   f13, 0x2(r4), 0, 5
    lwzu     r0, 0x4(r5)
    psq_lux  f8, r8, r10, 0, 6
    psq_lux  f9, r8, r29, 1, 6
    ps_cmpo1 cr5, f12, f0
    ps_cmpo0 cr6, f13, f0
    ps_cmpo1 cr7, f13, f0
    psq_lux  f10, r9, r11, 0, 7
    psq_lux  f11, r9, r30, 1, 7
    add      r9, r9, r12
    /* f24/f25 = position * matrices[index 0] * weight 0 and f26/f27 = normal * matrices[index 0] *
     * weight 0, without the translation */
    rlwinm   r31, r0, 14, 18, 25
    add      r31, r31, matrices
    psq_l    f0, RwMatrix.right(r31), 0, 0
    psq_l    f1, RwMatrix.right.z(r31), 1, 0
    psq_l    f6, RwMatrix.pos(r31), 0, 0
    psq_l    f7, RwMatrix.pos.z(r31), 1, 0
    ps_muls0 f26, f0, f10
    ps_muls0 f27, f1, f10
    psq_l    f2, RwMatrix.up(r31), 0, 0
    psq_l    f3, RwMatrix.up.z(r31), 1, 0
    ps_madds0 f24, f0, f8, f6
    ps_madds0 f25, f1, f8, f7
    psq_l    f4, RwMatrix.at(r31), 0, 0
    psq_l    f5, RwMatrix.at.z(r31), 1, 0
    ps_madds1 f26, f2, f10, f26
    ps_madds1 f27, f3, f10, f27
    ps_madds1 f24, f2, f8, f24
    ps_madds1 f25, f3, f8, f25
    ps_madds0 f26, f4, f11, f26
    ps_madds0 f27, f5, f11, f27
    ps_madds0 f24, f4, f9, f24
    ps_madds0 f25, f5, f9, f25
    ps_muls0 f26, f26, f12
    ps_muls0 f27, f27, f12
    ps_muls0 f24, f24, f12
    ps_muls0 f25, f25, f12
    ble      cr5, store
    /* unless weight 1 is zero: add matrices[index 1] the same way */
    rlwinm   r31, r0, 22, 18, 25
    add      r31, r31, matrices
    psq_l    f0, RwMatrix.right(r31), 0, 0
    psq_l    f1, RwMatrix.right.z(r31), 1, 0
    psq_l    f6, RwMatrix.pos(r31), 0, 0
    psq_l    f7, RwMatrix.pos.z(r31), 1, 0
    ps_muls0 f30, f0, f10
    ps_muls0 f31, f1, f10
    psq_l    f2, RwMatrix.up(r31), 0, 0
    psq_l    f3, RwMatrix.up.z(r31), 1, 0
    ps_madds0 f28, f0, f8, f6
    ps_madds0 f29, f1, f8, f7
    psq_l    f4, RwMatrix.at(r31), 0, 0
    psq_l    f5, RwMatrix.at.z(r31), 1, 0
    ps_madds1 f30, f2, f10, f30
    ps_madds1 f31, f3, f10, f31
    ps_madds1 f28, f2, f8, f28
    ps_madds1 f29, f3, f8, f29
    ps_madds0 f30, f4, f11, f30
    ps_madds0 f31, f5, f11, f31
    ps_madds0 f28, f4, f9, f28
    ps_madds0 f29, f5, f9, f29
    ps_madds1 f26, f30, f12, f26
    ps_madds1 f27, f31, f12, f27
    ps_madds1 f24, f28, f12, f24
    ps_madds1 f25, f29, f12, f25
    ble      cr6, store
    /* unless weight 2 is zero: add matrices[index 2] the same way */
    rlwinm   r31, r0, 30, 18, 25
    add      r31, r31, matrices
    psq_l    f0, RwMatrix.right(r31), 0, 0
    psq_l    f1, RwMatrix.right.z(r31), 1, 0
    psq_l    f6, RwMatrix.pos(r31), 0, 0
    psq_l    f7, RwMatrix.pos.z(r31), 1, 0
    ps_muls0 f30, f0, f10
    ps_muls0 f31, f1, f10
    psq_l    f2, RwMatrix.up(r31), 0, 0
    psq_l    f3, RwMatrix.up.z(r31), 1, 0
    ps_madds0 f28, f0, f8, f6
    ps_madds0 f29, f1, f8, f7
    psq_l    f4, RwMatrix.at(r31), 0, 0
    psq_l    f5, RwMatrix.at.z(r31), 1, 0
    ps_madds1 f30, f2, f10, f30
    ps_madds1 f31, f3, f10, f31
    ps_madds1 f28, f2, f8, f28
    ps_madds1 f29, f3, f8, f29
    ps_madds0 f30, f4, f11, f30
    ps_madds0 f31, f5, f11, f31
    ps_madds0 f28, f4, f9, f28
    ps_madds0 f29, f5, f9, f29
    ps_madds0 f26, f30, f13, f26
    ps_madds0 f27, f31, f13, f27
    ps_madds0 f24, f28, f13, f24
    ps_madds0 f25, f29, f13, f25
    ble      cr7, store
    /* unless weight 3 is zero: add matrices[index 3] the same way */
    clrlslwi r31, r0, 24, 6
    add      r31, r31, matrices
    psq_l    f0, RwMatrix.right(r31), 0, 0
    psq_l    f1, RwMatrix.right.z(r31), 1, 0
    psq_l    f6, RwMatrix.pos(r31), 0, 0
    psq_l    f7, RwMatrix.pos.z(r31), 1, 0
    ps_muls0 f30, f0, f10
    ps_muls0 f31, f1, f10
    psq_l    f2, RwMatrix.up(r31), 0, 0
    psq_l    f3, RwMatrix.up.z(r31), 1, 0
    ps_madds0 f28, f0, f8, f6
    ps_madds0 f29, f1, f8, f7
    psq_l    f4, RwMatrix.at(r31), 0, 0
    psq_l    f5, RwMatrix.at.z(r31), 1, 0
    ps_madds1 f30, f2, f10, f30
    ps_madds1 f31, f3, f10, f31
    ps_madds1 f28, f2, f8, f28
    ps_madds1 f29, f3, f8, f29
    ps_madds0 f30, f4, f11, f30
    ps_madds0 f31, f5, f11, f31
    ps_madds0 f28, f4, f9, f28
    ps_madds0 f29, f5, f9, f29
    ps_madds1 f26, f30, f13, f26
    ps_madds1 f27, f31, f13, f27
    ps_madds1 f24, f28, f13, f24
    ps_madds1 f25, f29, f13, f25
    /* write the skinned normal and position */
store:
    psq_stux f26, r7, r11, 0, 7
    psq_stux f27, r7, r30, 1, 7
    add      r7, r7, r12
    psq_stux f24, r6, r10, 0, 6
    psq_stux f25, r6, r29, 1, 6
    bdnz     vertex
done:
    lwz      r29, 0x8(r1)
    lwz      r30, 0xc(r1)
    lwz      r31, 0x10(r1)
    lfd      f25, 0x18(r1)
    lfd      f26, 0x20(r1)
    lfd      f27, 0x28(r1)
    lfd      f28, 0x30(r1)
    lfd      f29, 0x38(r1)
    lfd      f30, 0x40(r1)
    lfd      f31, 0x48(r1)
    addi     r1, r1, 0x50
    blr
#endif // clang-format on
}

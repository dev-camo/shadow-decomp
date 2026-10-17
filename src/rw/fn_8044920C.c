/* RenderWare particle tank (plugin ID 0x12F), GameCube: the particle kernels. Each writes the four
 * corners of every active particle straight to the GX vertex FIFO, inside the GXBegin of the tank's
 * render callback (fn_80448D04); fn_80448658 selects one kernel per tank from its data flags. The
 * kernels are hand-written assembly rather than compiler output: each saves the condition register
 * and every non-volatile register by hand, at the bottom of its frame where MWCC's prologues put
 * them at the top, keeps nop instructions in its schedule and computes in paired singles. The
 * original file name is unknown.
 *
 * All ten follow one scheme:
 * - They take the particle count, the tank's platform flags and its streams. The streams are the
 *   publicData member of the tank's RpPTankAtomicExtPrv, so the tank's constants (center, size,
 *   corner colors and texture coordinates) are read relative to it (PTANKCONSTANT).
 * - The condition register and the non-volatile registers are saved in a 0xE8-byte frame.
 * - The platform flags are tested once into condition fields: cr7 any colors (0x70000), cr2 a
 *   color per particle (0x10000), cr1 a color per corner (0x20000), r10 the constant corner
 *   colors (0x40000); cr6 any texture coordinates (0xF00000), cr4 two per particle (0x100000),
 *   cr3 four per particle (0x200000), r8 the constant two (0x400000), r9 the constant four
 *   (0x800000); cr5 normals (0x1000000).
 * - Stream pointers and strides: r23/r19 the positions (or matrices), r24/r20 the colors,
 *   r25/r21 the texture coordinates and r26/r22 the normals; where used, r18/r17 the sizes and
 *   r16/r15 the rotations. r27 holds the vertex FIFO address, r11 the camera axes, r12 the
 *   {0.5, -0.5} pair and r14 the rotation constants.
 * - For each particle, f24-f31 receive the four corner positions (x and y paired, then z),
 *   r28-r31 the four colors, f20-f23 the four texture coordinates and f18/f19 the normal. The
 *   render callback sets GQR6 and GQR7 to the vertex format, so positions are written quantized
 *   with GQR6 and normals with GQR7; colors and texture coordinates are written as they are.
 * - With two texture coordinates per particle, the other two corners combine their components. */
#include "rw/rpptank.h"

/* a tank constant, addressed from the tank's streams (RpPTankAtomicExtPrv.publicData) */
#define PTANKCONSTANT(member) (RpPTankAtomicExtPrv.member - RpPTankAtomicExtPrv.publicData)

/* a member of one of the two camera axes (RpPTankData.cameraAxes) */
#define PTANKCAMERAAXIS(index, member) ((index) * sizeof(RwV3d) + RwV3d.member)

/* the offsets of a particle's corners from its center, in particle sizes */
const RwReal lbl_805FB098[2] = {0.5f, -0.5f};

/* the address of the GX vertex FIFO (the write-gather pipe) */
const RwUInt32 lbl_805FB0A0 = 0xCC008000;

/* The constants of the rotating kernels' sine and cosine, as paired singles. Lane 1 computes cos a
 * as the sine of a - pi/2 -/+ pi (the pi brings the angle back into [-pi, pi]), and both lanes use
 * sin x ~ 4 / pi^2 * x * (pi - |x|) for x in [-pi, pi]. */
const RwReal lbl_8051BCF0[4][2] = {
    {0.0f, 3.14159274f}, {0.0f, -1.57079637f}, {1.0f, 1.0f}, {0.405284703f, 1.0f}};

/* the kernels load the two tables through these pointers */
const RwReal *lbl_805EDD78 = lbl_805FB098;
const RwReal *lbl_805EDD7C = lbl_8051BCF0[0];

/* Writes the particles of a tank with a matrix per particle: each is the quad spanned by half its
 * matrix's right and up axes around its matrix's position. */
asm void fn_8044920C(register RwUInt16 count, register RwUInt32 platFlags,
                     register RpPTankData *ptankGlobal) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the condition register and the non-volatile registers */
    stwu     r1, -0xe8(r1)
    mfcr     r0
    stw      r0, 0x8(r1)
    stw      r14, 0x10(r1)
    stw      r15, 0x14(r1)
    stw      r16, 0x18(r1)
    stw      r17, 0x1c(r1)
    stw      r18, 0x20(r1)
    stw      r19, 0x24(r1)
    stw      r20, 0x28(r1)
    stw      r21, 0x2c(r1)
    stw      r22, 0x30(r1)
    stw      r23, 0x34(r1)
    stw      r24, 0x38(r1)
    stw      r25, 0x3c(r1)
    stw      r26, 0x40(r1)
    stw      r27, 0x44(r1)
    stw      r28, 0x48(r1)
    stw      r29, 0x4c(r1)
    stw      r30, 0x50(r1)
    stw      r31, 0x54(r1)
    stfd     f14, 0x58(r1)
    stfd     f15, 0x60(r1)
    stfd     f16, 0x68(r1)
    stfd     f17, 0x70(r1)
    stfd     f18, 0x78(r1)
    stfd     f19, 0x80(r1)
    stfd     f20, 0x88(r1)
    stfd     f21, 0x90(r1)
    stfd     f22, 0x98(r1)
    stfd     f23, 0xa0(r1)
    stfd     f24, 0xa8(r1)
    stfd     f25, 0xb0(r1)
    stfd     f26, 0xb8(r1)
    stfd     f27, 0xc0(r1)
    stfd     f28, 0xc8(r1)
    stfd     f29, 0xd0(r1)
    stfd     f30, 0xd8(r1)
    stfd     f31, 0xe0(r1)
    /* the stream selections of platFlags, tested once for the whole loop */
    rlwinm   r8, platFlags, 0, 9, 9
    rlwinm   r9, platFlags, 0, 8, 8
    rlwinm   r10, platFlags, 0, 13, 13
    rlwinm   r11, platFlags, 0, 13, 15
    rlwinm   r12, platFlags, 0, 15, 15
    rlwinm   r14, platFlags, 0, 14, 14
    rlwinm   r15, platFlags, 0, 8, 11
    rlwinm   r16, platFlags, 0, 11, 11
    rlwinm   r17, platFlags, 0, 10, 10
    rlwinm   r18, platFlags, 0, 7, 7
    cmplwi   cr7, r11, 0
    cmplwi   cr6, r15, 0
    cmplwi   cr5, r18, 0
    cmplwi   cr4, r16, 0
    cmplwi   cr3, r17, 0
    cmplwi   cr2, r12, 0
    cmplwi   cr1, r14, 0
    /* the matrices; f0 = (0.5, 1) */
    lwz      r12, lbl_805EDD78
    lwz      r23, RpPTankData.dataMatrix.data(ptankGlobal)
    lwz      r19, RpPTankData.dataMatrix.stride(ptankGlobal)
    psq_l    f0, 0(r12), 1, 0
    /* colors: one per particle, one per corner, or the constant four */
    beq      cr7, texcoords
    beq      cr2, colors_per_vertex
    lwz      r24, RpPTankData.dataColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataColor.stride(ptankGlobal)
    b        texcoords
colors_per_vertex:
    beq+     cr1, colors_constant
    lwz      r24, RpPTankData.dataVtxColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataVtxColor.stride(ptankGlobal)
    b        texcoords
colors_constant:
    cmplwi   r10, 0
    beq+     texcoords
    lwz      r31, PTANKCONSTANT(cVtxColor[0])(ptankGlobal)
    lwz      r30, PTANKCONSTANT(cVtxColor[1])(ptankGlobal)
    lwz      r29, PTANKCONSTANT(cVtxColor[2])(ptankGlobal)
    lwz      r28, PTANKCONSTANT(cVtxColor[3])(ptankGlobal)
    /* texture coordinates: two or four per particle, or the constant two or four */
texcoords:
    beq+     cr6, normals
    beq+     cr4, texcoords_per_vertex
    lwz      r25, RpPTankData.dataVtx2TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx2TexCoords.stride(ptankGlobal)
    b        normals
texcoords_per_vertex:
    beq+     cr3, texcoords_constant2
    lwz      r25, RpPTankData.dataVtx4TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx4TexCoords.stride(ptankGlobal)
    b        normals
texcoords_constant2:
    cmplwi   r8, 0
    beq+     texcoords_constant4
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        normals
texcoords_constant4:
    cmplwi   r9, 0
    beq      normals
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f22, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[2])(ptankGlobal), 0, 0
    psq_l    f20, PTANKCONSTANT(cVtxUV[3])(ptankGlobal), 0, 0
    /* normals: one per particle */
normals:
    beq+     cr5, setup_done
    lwz      r26, RpPTankData.dataNormals.data(ptankGlobal)
    lwz      r22, RpPTankData.dataNormals.stride(ptankGlobal)
    /* the vertex FIFO; nothing to do without particles */
setup_done:
    cmpwi    count, 0
    lwz      r27, lbl_805FB0A0
    mtctr    count
    ble      done
particle:
    /* r = right / 2 and u = up / 2; the corners are pos - r + u, pos + r + u, pos + r - u and pos -
     * r - u */
    psq_l    f9, RwMatrix.right(r23), 0, 0
    psq_l    f8, RwMatrix.right.z(r23), 1, 0
    psq_l    f7, RwMatrix.up(r23), 0, 0
    psq_l    f6, RwMatrix.up.z(r23), 1, 0
    add      r18, r23, r19
    ps_muls0 f9, f9, f0
    ps_muls0 f8, f8, f0
    ps_muls0 f7, f7, f0
    ps_muls0 f6, f6, f0
    psq_l    f5, RwMatrix.pos(r23), 0, 0
    psq_l    f4, RwMatrix.pos.z(r23), 1, 0
    ps_sub   f17, f9, f7
    ps_sub   f16, f8, f6
    ps_add   f15, f9, f7
    ps_add   f14, f8, f6
    ps_sub   f31, f5, f17
    ps_sub   f30, f4, f16
    ps_add   f29, f5, f15
    ps_add   f28, f4, f14
    ps_add   f27, f5, f17
    ps_add   f26, f4, f16
    ps_sub   f25, f5, f15
    ps_sub   f24, f4, f14
    /* prefetch the starts of the next two matrices */
    dcbt     r23, r19
    dcbt     r18, r19
    /* this particle's colors, texture coordinates and normal */
    beq      cr7, load_texcoords
    beq      cr2, load_colors_per_vertex
    lwz      r31, 0(r24)
    lwz      r30, 0(r24)
    lwz      r29, 0(r24)
    lwz      r28, 0(r24)
    b        load_texcoords
load_colors_per_vertex:
    beq+     cr1, load_texcoords
    lwz      r31, 0(r24)
    lwz      r30, 0x4(r24)
    lwz      r29, 0x8(r24)
    lwz      r28, 0xc(r24)
load_texcoords:
    beq+     cr6, load_normal
    beq+     cr4, load_texcoords_per_vertex
    psq_l    f23, 0(r25), 0, 0
    psq_l    f21, 0x8(r25), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        vertex0
load_texcoords_per_vertex:
    beq+     cr3, load_normal
    psq_l    f23, 0(r25), 0, 0
    psq_l    f22, 0x8(r25), 0, 0
    psq_l    f21, 0x10(r25), 0, 0
    psq_l    f20, 0x18(r25), 0, 0
load_normal:
    beq+     cr5, vertex0
    psq_l    f19, 0(r26), 0, 0
    psq_l    f18, 0x8(r26), 1, 0
    /* write the four corners: position, then normal, color and texture coordinate if present */
vertex0:
    psq_st   f31, 0(r27), 0, 6
    psq_st   f30, 0(r27), 1, 6
    beq+     cr5, vertex0_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex0_color:
    beq      cr7, vertex0_texcoord
    stw      r31, 0(r27)
vertex0_texcoord:
    beq+     cr6, vertex1
    psq_st   f20, 0(r27), 0, 0
vertex1:
    psq_st   f29, 0(r27), 0, 6
    psq_st   f28, 0(r27), 1, 6
    beq+     cr5, vertex1_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex1_color:
    beq      cr7, vertex1_texcoord
    stw      r30, 0(r27)
vertex1_texcoord:
    beq+     cr6, vertex2
    psq_st   f23, 0(r27), 0, 0
vertex2:
    psq_st   f27, 0(r27), 0, 6
    psq_st   f26, 0(r27), 1, 6
    beq+     cr5, vertex2_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex2_color:
    beq      cr7, vertex2_texcoord
    stw      r29, 0(r27)
vertex2_texcoord:
    beq+     cr6, vertex3
    psq_st   f22, 0(r27), 0, 0
vertex3:
    psq_st   f25, 0(r27), 0, 6
    psq_st   f24, 0(r27), 1, 6
    add      r23, r23, r19
    beq+     cr5, vertex3_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
    add      r26, r26, r22
vertex3_color:
    beq      cr7, vertex3_texcoord
    stw      r28, 0(r27)
    add      r24, r24, r20
vertex3_texcoord:
    beq+     cr6, next_particle
    psq_st   f21, 0(r27), 0, 0
    add      r25, r25, r21
next_particle:
    bdnz     particle
    /* restore the registers */
done:
    lwz      r0, 0x8(r1)
    mtcrf    255, r0
    lwz      r14, 0x10(r1)
    lwz      r15, 0x14(r1)
    lwz      r16, 0x18(r1)
    lwz      r17, 0x1c(r1)
    lwz      r18, 0x20(r1)
    lwz      r19, 0x24(r1)
    lwz      r20, 0x28(r1)
    lwz      r21, 0x2c(r1)
    lwz      r22, 0x30(r1)
    lwz      r23, 0x34(r1)
    lwz      r24, 0x38(r1)
    lwz      r25, 0x3c(r1)
    lwz      r26, 0x40(r1)
    lwz      r27, 0x44(r1)
    lwz      r28, 0x48(r1)
    lwz      r29, 0x4c(r1)
    lwz      r30, 0x50(r1)
    lwz      r31, 0x54(r1)
    lfd      f14, 0x58(r1)
    lfd      f15, 0x60(r1)
    lfd      f16, 0x68(r1)
    lfd      f17, 0x70(r1)
    lfd      f18, 0x78(r1)
    lfd      f19, 0x80(r1)
    lfd      f20, 0x88(r1)
    lfd      f21, 0x90(r1)
    lfd      f22, 0x98(r1)
    lfd      f23, 0xa0(r1)
    lfd      f24, 0xa8(r1)
    lfd      f25, 0xb0(r1)
    lfd      f26, 0xb8(r1)
    lfd      f27, 0xc0(r1)
    lfd      f28, 0xc8(r1)
    lfd      f29, 0xd0(r1)
    lfd      f30, 0xd8(r1)
    lfd      f31, 0xe0(r1)
    addi     r1, r1, 0xe8
    blr
#endif // clang-format on
}

/* Writes the particles of a tank with a matrix per particle and a center: as fn_8044920C, with each
 * quad moved by the tank's center along its matrix's right and up axes. */
asm void fn_804495D0(register RwUInt16 count, register RwUInt32 platFlags,
                     register RpPTankData *ptankGlobal) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the condition register and the non-volatile registers */
    stwu     r1, -0xe8(r1)
    mfcr     r0
    stw      r0, 0x8(r1)
    stw      r14, 0x10(r1)
    stw      r15, 0x14(r1)
    stw      r16, 0x18(r1)
    stw      r17, 0x1c(r1)
    stw      r18, 0x20(r1)
    stw      r19, 0x24(r1)
    stw      r20, 0x28(r1)
    stw      r21, 0x2c(r1)
    stw      r22, 0x30(r1)
    stw      r23, 0x34(r1)
    stw      r24, 0x38(r1)
    stw      r25, 0x3c(r1)
    stw      r26, 0x40(r1)
    stw      r27, 0x44(r1)
    stw      r28, 0x48(r1)
    stw      r29, 0x4c(r1)
    stw      r30, 0x50(r1)
    stw      r31, 0x54(r1)
    stfd     f14, 0x58(r1)
    stfd     f15, 0x60(r1)
    stfd     f16, 0x68(r1)
    stfd     f17, 0x70(r1)
    stfd     f18, 0x78(r1)
    stfd     f19, 0x80(r1)
    stfd     f20, 0x88(r1)
    stfd     f21, 0x90(r1)
    stfd     f22, 0x98(r1)
    stfd     f23, 0xa0(r1)
    stfd     f24, 0xa8(r1)
    stfd     f25, 0xb0(r1)
    stfd     f26, 0xb8(r1)
    stfd     f27, 0xc0(r1)
    stfd     f28, 0xc8(r1)
    stfd     f29, 0xd0(r1)
    stfd     f30, 0xd8(r1)
    stfd     f31, 0xe0(r1)
    /* the stream selections of platFlags, tested once for the whole loop */
    rlwinm   r8, platFlags, 0, 9, 9
    rlwinm   r9, platFlags, 0, 8, 8
    rlwinm   r10, platFlags, 0, 13, 13
    rlwinm   r11, platFlags, 0, 13, 15
    rlwinm   r12, platFlags, 0, 15, 15
    rlwinm   r14, platFlags, 0, 14, 14
    rlwinm   r15, platFlags, 0, 8, 11
    rlwinm   r16, platFlags, 0, 11, 11
    rlwinm   r17, platFlags, 0, 10, 10
    rlwinm   r18, platFlags, 0, 7, 7
    cmplwi   cr7, r11, 0
    cmplwi   cr6, r15, 0
    cmplwi   cr5, r18, 0
    cmplwi   cr4, r16, 0
    cmplwi   cr3, r17, 0
    cmplwi   cr2, r12, 0
    cmplwi   cr1, r14, 0
    /* f1 = the center, f0 = (0.5, -0.5); the matrices */
    lwz      r12, lbl_805EDD78
    psq_l    f1, PTANKCONSTANT(cCenter)(ptankGlobal), 0, 0
    psq_l    f0, 0(r12), 0, 0
    lwz      r23, RpPTankData.dataMatrix.data(ptankGlobal)
    lwz      r19, RpPTankData.dataMatrix.stride(ptankGlobal)
    /* colors: one per particle, one per corner, or the constant four */
    beq      cr7, texcoords
    beq      cr2, colors_per_vertex
    lwz      r24, RpPTankData.dataColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataColor.stride(ptankGlobal)
    b        texcoords
colors_per_vertex:
    beq+     cr1, colors_constant
    lwz      r24, RpPTankData.dataVtxColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataVtxColor.stride(ptankGlobal)
    b        texcoords
colors_constant:
    cmplwi   r10, 0
    beq+     texcoords
    lwz      r31, PTANKCONSTANT(cVtxColor[0])(ptankGlobal)
    lwz      r30, PTANKCONSTANT(cVtxColor[1])(ptankGlobal)
    lwz      r29, PTANKCONSTANT(cVtxColor[2])(ptankGlobal)
    lwz      r28, PTANKCONSTANT(cVtxColor[3])(ptankGlobal)
    /* texture coordinates: two or four per particle, or the constant two or four */
texcoords:
    beq+     cr6, normals
    beq+     cr4, texcoords_per_vertex
    lwz      r25, RpPTankData.dataVtx2TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx2TexCoords.stride(ptankGlobal)
    b        normals
texcoords_per_vertex:
    beq+     cr3, texcoords_constant2
    lwz      r25, RpPTankData.dataVtx4TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx4TexCoords.stride(ptankGlobal)
    b        normals
texcoords_constant2:
    cmplwi   r8, 0
    beq+     texcoords_constant4
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        normals
texcoords_constant4:
    cmplwi   r9, 0
    beq      normals
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f22, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[2])(ptankGlobal), 0, 0
    psq_l    f20, PTANKCONSTANT(cVtxUV[3])(ptankGlobal), 0, 0
    /* normals: one per particle */
normals:
    beq+     cr5, setup_done
    lwz      r26, RpPTankData.dataNormals.data(ptankGlobal)
    lwz      r22, RpPTankData.dataNormals.stride(ptankGlobal)
    /* the vertex FIFO; nothing to do without particles */
setup_done:
    cmpwi    count, 0
    lwz      r27, lbl_805FB0A0
    mtctr    count
    ble      done
particle:
    /* pos += center.x * right + center.y * up; f13 = (right + up) / 2 and f11 = (right - up) / 2;
     * the corners are pos - f11, pos + f13, pos + f11 and pos - f13 */
    psq_l    f9, RwMatrix.right(r23), 0, 0
    psq_l    f8, RwMatrix.right.z(r23), 1, 0
    psq_l    f5, RwMatrix.pos(r23), 0, 0
    psq_l    f4, RwMatrix.pos.z(r23), 1, 0
    psq_l    f7, RwMatrix.up(r23), 0, 0
    psq_l    f6, RwMatrix.up.z(r23), 1, 0
    ps_madds0 f5, f9, f1, f5
    ps_madds0 f4, f8, f1, f4
    ps_muls0 f13, f9, f0
    ps_muls0 f12, f8, f0
    ps_madds1 f5, f7, f1, f5
    ps_madds1 f4, f6, f1, f4
    ps_madds1 f11, f7, f0, f13
    ps_madds1 f10, f6, f0, f12
    ps_madds0 f13, f7, f0, f13
    ps_madds0 f12, f6, f0, f12
    ps_sub   f31, f5, f11
    ps_sub   f30, f4, f10
    ps_add   f29, f5, f13
    ps_add   f28, f4, f12
    ps_add   f27, f5, f11
    ps_add   f26, f4, f10
    ps_sub   f25, f5, f13
    ps_sub   f24, f4, f12
    /* this particle's colors, texture coordinates and normal */
    beq      cr7, load_texcoords
    beq      cr2, load_colors_per_vertex
    lwz      r31, 0(r24)
    lwz      r30, 0(r24)
    lwz      r29, 0(r24)
    lwz      r28, 0(r24)
    b        load_texcoords
load_colors_per_vertex:
    beq+     cr1, load_texcoords
    lwz      r31, 0(r24)
    lwz      r30, 0x4(r24)
    lwz      r29, 0x8(r24)
    lwz      r28, 0xc(r24)
load_texcoords:
    beq+     cr6, load_normal
    beq+     cr4, load_texcoords_per_vertex
    psq_l    f23, 0(r25), 0, 0
    psq_l    f21, 0x8(r25), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        vertex0
load_texcoords_per_vertex:
    beq+     cr3, load_normal
    psq_l    f23, 0(r25), 0, 0
    psq_l    f22, 0x8(r25), 0, 0
    psq_l    f21, 0x10(r25), 0, 0
    psq_l    f20, 0x18(r25), 0, 0
load_normal:
    beq+     cr5, vertex0
    psq_l    f19, 0(r26), 0, 0
    psq_l    f18, 0x8(r26), 1, 0
    /* write the four corners: position, then normal, color and texture coordinate if present */
vertex0:
    psq_st   f31, 0(r27), 0, 6
    psq_st   f30, 0(r27), 1, 6
    beq+     cr5, vertex0_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex0_color:
    beq      cr7, vertex0_texcoord
    stw      r31, 0(r27)
vertex0_texcoord:
    beq+     cr6, vertex1
    psq_st   f20, 0(r27), 0, 0
vertex1:
    psq_st   f29, 0(r27), 0, 6
    psq_st   f28, 0(r27), 1, 6
    beq+     cr5, vertex1_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex1_color:
    beq      cr7, vertex1_texcoord
    stw      r30, 0(r27)
vertex1_texcoord:
    beq+     cr6, vertex2
    psq_st   f23, 0(r27), 0, 0
vertex2:
    psq_st   f27, 0(r27), 0, 6
    psq_st   f26, 0(r27), 1, 6
    beq+     cr5, vertex2_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex2_color:
    beq      cr7, vertex2_texcoord
    stw      r29, 0(r27)
vertex2_texcoord:
    beq+     cr6, vertex3
    psq_st   f22, 0(r27), 0, 0
vertex3:
    psq_st   f25, 0(r27), 0, 6
    psq_st   f24, 0(r27), 1, 6
    add      r23, r23, r19
    beq+     cr5, vertex3_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
    add      r26, r26, r22
vertex3_color:
    beq      cr7, vertex3_texcoord
    stw      r28, 0(r27)
    add      r24, r24, r20
vertex3_texcoord:
    beq+     cr6, next_particle
    psq_st   f21, 0(r27), 0, 0
    add      r25, r25, r21
next_particle:
    bdnz     particle
    /* restore the registers */
done:
    lwz      r0, 0x8(r1)
    mtcrf    255, r0
    lwz      r14, 0x10(r1)
    lwz      r15, 0x14(r1)
    lwz      r16, 0x18(r1)
    lwz      r17, 0x1c(r1)
    lwz      r18, 0x20(r1)
    lwz      r19, 0x24(r1)
    lwz      r20, 0x28(r1)
    lwz      r21, 0x2c(r1)
    lwz      r22, 0x30(r1)
    lwz      r23, 0x34(r1)
    lwz      r24, 0x38(r1)
    lwz      r25, 0x3c(r1)
    lwz      r26, 0x40(r1)
    lwz      r27, 0x44(r1)
    lwz      r28, 0x48(r1)
    lwz      r29, 0x4c(r1)
    lwz      r30, 0x50(r1)
    lwz      r31, 0x54(r1)
    lfd      f14, 0x58(r1)
    lfd      f15, 0x60(r1)
    lfd      f16, 0x68(r1)
    lfd      f17, 0x70(r1)
    lfd      f18, 0x78(r1)
    lfd      f19, 0x80(r1)
    lfd      f20, 0x88(r1)
    lfd      f21, 0x90(r1)
    lfd      f22, 0x98(r1)
    lfd      f23, 0xa0(r1)
    lfd      f24, 0xa8(r1)
    lfd      f25, 0xb0(r1)
    lfd      f26, 0xb8(r1)
    lfd      f27, 0xc0(r1)
    lfd      f28, 0xc8(r1)
    lfd      f29, 0xd0(r1)
    lfd      f30, 0xd8(r1)
    lfd      f31, 0xe0(r1)
    addi     r1, r1, 0xe8
    blr
#endif // clang-format on
}

/* Writes the particles of a tank with the constant size: each is the quad spanned by the camera
 * axes, scaled by half the size, around its position. Also used for the constant matrix, whose axes
 * the render callback passes in place of the camera's. */
asm void fn_80449994(register RwUInt16 count, register RwUInt32 platFlags,
                     register RpPTankData *ptankGlobal) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the condition register and the non-volatile registers */
    stwu     r1, -0xe8(r1)
    mfcr     r0
    stw      r0, 0x8(r1)
    stw      r14, 0x10(r1)
    stw      r15, 0x14(r1)
    stw      r16, 0x18(r1)
    stw      r17, 0x1c(r1)
    stw      r18, 0x20(r1)
    stw      r19, 0x24(r1)
    stw      r20, 0x28(r1)
    stw      r21, 0x2c(r1)
    stw      r22, 0x30(r1)
    stw      r23, 0x34(r1)
    stw      r24, 0x38(r1)
    stw      r25, 0x3c(r1)
    stw      r26, 0x40(r1)
    stw      r27, 0x44(r1)
    stw      r28, 0x48(r1)
    stw      r29, 0x4c(r1)
    stw      r30, 0x50(r1)
    stw      r31, 0x54(r1)
    stfd     f14, 0x58(r1)
    stfd     f15, 0x60(r1)
    stfd     f16, 0x68(r1)
    stfd     f17, 0x70(r1)
    stfd     f18, 0x78(r1)
    stfd     f19, 0x80(r1)
    stfd     f20, 0x88(r1)
    stfd     f21, 0x90(r1)
    stfd     f22, 0x98(r1)
    stfd     f23, 0xa0(r1)
    stfd     f24, 0xa8(r1)
    stfd     f25, 0xb0(r1)
    stfd     f26, 0xb8(r1)
    stfd     f27, 0xc0(r1)
    stfd     f28, 0xc8(r1)
    stfd     f29, 0xd0(r1)
    stfd     f30, 0xd8(r1)
    stfd     f31, 0xe0(r1)
    /* the stream selections of platFlags, tested once for the whole loop */
    rlwinm   r8, platFlags, 0, 9, 9
    rlwinm   r9, platFlags, 0, 8, 8
    rlwinm   r10, platFlags, 0, 13, 13
    rlwinm   r11, platFlags, 0, 13, 15
    rlwinm   r12, platFlags, 0, 15, 15
    rlwinm   r14, platFlags, 0, 14, 14
    rlwinm   r15, platFlags, 0, 8, 11
    rlwinm   r16, platFlags, 0, 11, 11
    rlwinm   r17, platFlags, 0, 10, 10
    rlwinm   r18, platFlags, 0, 7, 7
    cmplwi   cr7, r11, 0
    cmplwi   cr6, r15, 0
    cmplwi   cr5, r18, 0
    cmplwi   cr4, r16, 0
    cmplwi   cr3, r17, 0
    cmplwi   cr2, r12, 0
    cmplwi   cr1, r14, 0
    /* the positions; r = axes[0] * size.x / 2 and u = axes[1] * size.y / 2 for every particle, f15
     * = r + u and f17 = r - u */
    lwz      r12, lbl_805EDD78
    lwz      r23, RpPTankData.dataPosition.data(ptankGlobal)
    lwz      r19, RpPTankData.dataPosition.stride(ptankGlobal)
    lwz      r11, RpPTankData.cameraAxes(ptankGlobal)
    psq_l    f13, PTANKCONSTANT(cSize)(ptankGlobal), 0, 0
    psq_l    f1, 0(r12), 1, 0
    psq_l    f5, PTANKCAMERAAXIS(0, x)(r11), 0, 0
    psq_l    f4, PTANKCAMERAAXIS(0, z)(r11), 1, 0
    ps_muls0 f12, f13, f1
    psq_l    f3, PTANKCAMERAAXIS(1, x)(r11), 0, 0
    psq_l    f2, PTANKCAMERAAXIS(1, z)(r11), 1, 0
    ps_muls0 f9, f5, f12
    ps_muls0 f8, f4, f12
    ps_muls1 f7, f3, f12
    ps_muls1 f6, f2, f12
    ps_add   f15, f9, f7
    ps_add   f14, f8, f6
    ps_sub   f17, f9, f7
    ps_sub   f16, f8, f6
    /* colors: one per particle, one per corner, or the constant four */
    beq      cr7, texcoords
    beq      cr2, colors_per_vertex
    lwz      r24, RpPTankData.dataColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataColor.stride(ptankGlobal)
    b        texcoords
colors_per_vertex:
    beq+     cr1, colors_constant
    lwz      r24, RpPTankData.dataVtxColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataVtxColor.stride(ptankGlobal)
    b        texcoords
colors_constant:
    cmplwi   r10, 0
    beq+     texcoords
    lwz      r31, PTANKCONSTANT(cVtxColor[0])(ptankGlobal)
    lwz      r30, PTANKCONSTANT(cVtxColor[1])(ptankGlobal)
    lwz      r29, PTANKCONSTANT(cVtxColor[2])(ptankGlobal)
    lwz      r28, PTANKCONSTANT(cVtxColor[3])(ptankGlobal)
    /* texture coordinates: two or four per particle, or the constant two or four */
texcoords:
    beq+     cr6, normals
    beq+     cr4, texcoords_per_vertex
    lwz      r25, RpPTankData.dataVtx2TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx2TexCoords.stride(ptankGlobal)
    b        normals
texcoords_per_vertex:
    beq+     cr3, texcoords_constant2
    lwz      r25, RpPTankData.dataVtx4TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx4TexCoords.stride(ptankGlobal)
    b        normals
texcoords_constant2:
    cmplwi   r8, 0
    beq+     texcoords_constant4
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        normals
texcoords_constant4:
    cmplwi   r9, 0
    beq      normals
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f22, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[2])(ptankGlobal), 0, 0
    psq_l    f20, PTANKCONSTANT(cVtxUV[3])(ptankGlobal), 0, 0
    /* normals: one per particle */
normals:
    beq+     cr5, setup_done
    lwz      r26, RpPTankData.dataNormals.data(ptankGlobal)
    lwz      r22, RpPTankData.dataNormals.stride(ptankGlobal)
    /* the vertex FIFO; nothing to do without particles */
setup_done:
    cmpwi    count, 0
    lwz      r27, lbl_805FB0A0
    mtctr    count
    ble      done
particle:
    /* the corners are pos - f15, pos + f17, pos + f15 and pos - f17 */
    psq_l    f9, RwV3d.x(r23), 0, 0
    psq_l    f8, RwV3d.z(r23), 1, 0
    nop
    ps_sub   f31, f9, f15
    ps_sub   f30, f8, f14
    ps_add   f29, f9, f17
    ps_add   f28, f8, f16
    ps_add   f27, f9, f15
    ps_add   f26, f8, f14
    ps_sub   f25, f9, f17
    ps_sub   f24, f8, f16
    /* this particle's colors, texture coordinates and normal */
    beq      cr7, load_texcoords
    beq      cr2, load_colors_per_vertex
    lwz      r31, 0(r24)
    lwz      r30, 0(r24)
    lwz      r29, 0(r24)
    lwz      r28, 0(r24)
    b        load_texcoords
load_colors_per_vertex:
    beq+     cr1, load_texcoords
    lwz      r31, 0(r24)
    lwz      r30, 0x4(r24)
    lwz      r29, 0x8(r24)
    lwz      r28, 0xc(r24)
load_texcoords:
    beq+     cr6, load_normal
    beq+     cr4, load_texcoords_per_vertex
    psq_l    f23, 0(r25), 0, 0
    psq_l    f21, 0x8(r25), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        vertex0
load_texcoords_per_vertex:
    beq+     cr3, load_normal
    psq_l    f23, 0(r25), 0, 0
    psq_l    f22, 0x8(r25), 0, 0
    psq_l    f21, 0x10(r25), 0, 0
    psq_l    f20, 0x18(r25), 0, 0
load_normal:
    beq+     cr5, vertex0
    psq_l    f19, 0(r26), 0, 0
    psq_l    f18, 0x8(r26), 1, 0
    /* write the four corners: position, then normal, color and texture coordinate if present */
vertex0:
    psq_st   f31, 0(r27), 0, 6
    psq_st   f30, 0(r27), 1, 6
    beq+     cr5, vertex0_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex0_color:
    beq      cr7, vertex0_texcoord
    stw      r31, 0(r27)
vertex0_texcoord:
    beq+     cr6, vertex1
    psq_st   f20, 0(r27), 0, 0
vertex1:
    psq_st   f29, 0(r27), 0, 6
    psq_st   f28, 0(r27), 1, 6
    beq+     cr5, vertex1_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex1_color:
    beq      cr7, vertex1_texcoord
    stw      r30, 0(r27)
vertex1_texcoord:
    beq+     cr6, vertex2
    psq_st   f23, 0(r27), 0, 0
vertex2:
    psq_st   f27, 0(r27), 0, 6
    psq_st   f26, 0(r27), 1, 6
    beq+     cr5, vertex2_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex2_color:
    beq      cr7, vertex2_texcoord
    stw      r29, 0(r27)
vertex2_texcoord:
    beq+     cr6, vertex3
    psq_st   f22, 0(r27), 0, 0
vertex3:
    psq_st   f25, 0(r27), 0, 6
    psq_st   f24, 0(r27), 1, 6
    add      r23, r23, r19
    beq+     cr5, vertex3_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
    add      r26, r26, r22
vertex3_color:
    beq      cr7, vertex3_texcoord
    stw      r28, 0(r27)
    add      r24, r24, r20
vertex3_texcoord:
    beq+     cr6, next_particle
    psq_st   f21, 0(r27), 0, 0
    add      r25, r25, r21
next_particle:
    bdnz     particle
    /* restore the registers */
done:
    lwz      r0, 0x8(r1)
    mtcrf    255, r0
    lwz      r14, 0x10(r1)
    lwz      r15, 0x14(r1)
    lwz      r16, 0x18(r1)
    lwz      r17, 0x1c(r1)
    lwz      r18, 0x20(r1)
    lwz      r19, 0x24(r1)
    lwz      r20, 0x28(r1)
    lwz      r21, 0x2c(r1)
    lwz      r22, 0x30(r1)
    lwz      r23, 0x34(r1)
    lwz      r24, 0x38(r1)
    lwz      r25, 0x3c(r1)
    lwz      r26, 0x40(r1)
    lwz      r27, 0x44(r1)
    lwz      r28, 0x48(r1)
    lwz      r29, 0x4c(r1)
    lwz      r30, 0x50(r1)
    lwz      r31, 0x54(r1)
    lfd      f14, 0x58(r1)
    lfd      f15, 0x60(r1)
    lfd      f16, 0x68(r1)
    lfd      f17, 0x70(r1)
    lfd      f18, 0x78(r1)
    lfd      f19, 0x80(r1)
    lfd      f20, 0x88(r1)
    lfd      f21, 0x90(r1)
    lfd      f22, 0x98(r1)
    lfd      f23, 0xa0(r1)
    lfd      f24, 0xa8(r1)
    lfd      f25, 0xb0(r1)
    lfd      f26, 0xb8(r1)
    lfd      f27, 0xc0(r1)
    lfd      f28, 0xc8(r1)
    lfd      f29, 0xd0(r1)
    lfd      f30, 0xd8(r1)
    lfd      f31, 0xe0(r1)
    addi     r1, r1, 0xe8
    blr
#endif // clang-format on
}

/* Writes the particles of a tank with the constant size and a center: as fn_80449994, with each
 * quad moved back by the tank's center along the camera axes. */
asm void fn_80449D5C(register RwUInt16 count, register RwUInt32 platFlags,
                     register RpPTankData *ptankGlobal) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the condition register and the non-volatile registers */
    stwu     r1, -0xe8(r1)
    mfcr     r0
    stw      r0, 0x8(r1)
    stw      r14, 0x10(r1)
    stw      r15, 0x14(r1)
    stw      r16, 0x18(r1)
    stw      r17, 0x1c(r1)
    stw      r18, 0x20(r1)
    stw      r19, 0x24(r1)
    stw      r20, 0x28(r1)
    stw      r21, 0x2c(r1)
    stw      r22, 0x30(r1)
    stw      r23, 0x34(r1)
    stw      r24, 0x38(r1)
    stw      r25, 0x3c(r1)
    stw      r26, 0x40(r1)
    stw      r27, 0x44(r1)
    stw      r28, 0x48(r1)
    stw      r29, 0x4c(r1)
    stw      r30, 0x50(r1)
    stw      r31, 0x54(r1)
    stfd     f14, 0x58(r1)
    stfd     f15, 0x60(r1)
    stfd     f16, 0x68(r1)
    stfd     f17, 0x70(r1)
    stfd     f18, 0x78(r1)
    stfd     f19, 0x80(r1)
    stfd     f20, 0x88(r1)
    stfd     f21, 0x90(r1)
    stfd     f22, 0x98(r1)
    stfd     f23, 0xa0(r1)
    stfd     f24, 0xa8(r1)
    stfd     f25, 0xb0(r1)
    stfd     f26, 0xb8(r1)
    stfd     f27, 0xc0(r1)
    stfd     f28, 0xc8(r1)
    stfd     f29, 0xd0(r1)
    stfd     f30, 0xd8(r1)
    stfd     f31, 0xe0(r1)
    /* the stream selections of platFlags, tested once for the whole loop */
    rlwinm   r8, platFlags, 0, 9, 9
    rlwinm   r9, platFlags, 0, 8, 8
    rlwinm   r10, platFlags, 0, 13, 13
    rlwinm   r11, platFlags, 0, 13, 15
    rlwinm   r12, platFlags, 0, 15, 15
    rlwinm   r14, platFlags, 0, 14, 14
    rlwinm   r15, platFlags, 0, 8, 11
    rlwinm   r16, platFlags, 0, 11, 11
    rlwinm   r17, platFlags, 0, 10, 10
    rlwinm   r18, platFlags, 0, 7, 7
    cmplwi   cr7, r11, 0
    cmplwi   cr6, r15, 0
    cmplwi   cr5, r18, 0
    cmplwi   cr4, r16, 0
    cmplwi   cr3, r17, 0
    cmplwi   cr2, r12, 0
    cmplwi   cr1, r14, 0
    /* for every particle: c = -(center.x * axes[0] + center.y * axes[1]), r = axes[0] * size.x / 2
     * and u = axes[1] * size.y / 2, and the corner offsets c - (r + u), c + (r - u), c + (r + u)
     * and c - (r - u) in f16/f17, f14/f15, f12/f13 and f10/f11; the positions */
    lwz      r11, RpPTankData.cameraAxes(ptankGlobal)
    lwz      r12, lbl_805EDD78
    psq_l    f11, PTANKCONSTANT(cCenter)(ptankGlobal), 0, 0
    psq_l    f5, PTANKCAMERAAXIS(0, x)(r11), 0, 0
    psq_l    f4, PTANKCAMERAAXIS(0, z)(r11), 1, 0
    ps_neg   f11, f11
    psq_l    f3, PTANKCAMERAAXIS(1, x)(r11), 0, 0
    psq_l    f2, PTANKCAMERAAXIS(1, z)(r11), 1, 0
    ps_muls0 f17, f5, f11
    ps_muls0 f16, f4, f11
    ps_muls1 f15, f3, f11
    ps_muls1 f14, f2, f11
    psq_l    f13, PTANKCONSTANT(cSize)(ptankGlobal), 0, 0
    psq_l    f1, 0(r12), 1, 0
    ps_add   f31, f17, f15
    ps_add   f30, f16, f14
    ps_muls0 f12, f13, f1
    lwz      r23, RpPTankData.dataPosition.data(ptankGlobal)
    lwz      r19, RpPTankData.dataPosition.stride(ptankGlobal)
    ps_muls0 f9, f5, f12
    ps_muls0 f8, f4, f12
    ps_muls1 f7, f3, f12
    ps_muls1 f6, f2, f12
    nop
    ps_add   f27, f9, f7
    ps_add   f26, f8, f6
    ps_sub   f29, f9, f7
    ps_sub   f28, f8, f6
    ps_sub   f17, f31, f27
    ps_sub   f16, f30, f26
    ps_add   f15, f31, f29
    ps_add   f14, f30, f28
    ps_add   f13, f31, f27
    ps_add   f12, f30, f26
    ps_sub   f11, f31, f29
    ps_sub   f10, f30, f28
    /* colors: one per particle, one per corner, or the constant four */
    beq      cr7, texcoords
    beq      cr2, colors_per_vertex
    lwz      r24, RpPTankData.dataColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataColor.stride(ptankGlobal)
    b        texcoords
colors_per_vertex:
    beq+     cr1, colors_constant
    lwz      r24, RpPTankData.dataVtxColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataVtxColor.stride(ptankGlobal)
    b        texcoords
colors_constant:
    cmplwi   r10, 0
    beq+     texcoords
    lwz      r31, PTANKCONSTANT(cVtxColor[0])(ptankGlobal)
    lwz      r30, PTANKCONSTANT(cVtxColor[1])(ptankGlobal)
    lwz      r29, PTANKCONSTANT(cVtxColor[2])(ptankGlobal)
    lwz      r28, PTANKCONSTANT(cVtxColor[3])(ptankGlobal)
    /* texture coordinates: two or four per particle, or the constant two or four */
texcoords:
    beq+     cr6, normals
    beq+     cr4, texcoords_per_vertex
    lwz      r25, RpPTankData.dataVtx2TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx2TexCoords.stride(ptankGlobal)
    b        normals
texcoords_per_vertex:
    beq+     cr3, texcoords_constant2
    lwz      r25, RpPTankData.dataVtx4TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx4TexCoords.stride(ptankGlobal)
    b        normals
texcoords_constant2:
    cmplwi   r8, 0
    beq+     texcoords_constant4
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        normals
texcoords_constant4:
    cmplwi   r9, 0
    beq      normals
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f22, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[2])(ptankGlobal), 0, 0
    psq_l    f20, PTANKCONSTANT(cVtxUV[3])(ptankGlobal), 0, 0
    /* normals: one per particle */
normals:
    beq+     cr5, setup_done
    lwz      r26, RpPTankData.dataNormals.data(ptankGlobal)
    lwz      r22, RpPTankData.dataNormals.stride(ptankGlobal)
    /* the vertex FIFO; nothing to do without particles */
setup_done:
    cmpwi    count, 0
    lwz      r27, lbl_805FB0A0
    mtctr    count
    ble      done
particle:
    /* the corners are the position plus the corner offsets */
    psq_l    f9, RwV3d.x(r23), 0, 0
    psq_l    f8, RwV3d.z(r23), 1, 0
    nop
    ps_add   f31, f9, f17
    ps_add   f30, f8, f16
    ps_add   f29, f9, f15
    ps_add   f28, f8, f14
    ps_add   f27, f9, f13
    ps_add   f26, f8, f12
    ps_add   f25, f9, f11
    ps_add   f24, f8, f10
    /* this particle's colors, texture coordinates and normal */
    beq      cr7, load_texcoords
    beq      cr2, load_colors_per_vertex
    lwz      r31, 0(r24)
    lwz      r30, 0(r24)
    lwz      r29, 0(r24)
    lwz      r28, 0(r24)
    b        load_texcoords
load_colors_per_vertex:
    beq+     cr1, load_texcoords
    lwz      r31, 0(r24)
    lwz      r30, 0x4(r24)
    lwz      r29, 0x8(r24)
    lwz      r28, 0xc(r24)
load_texcoords:
    beq+     cr6, load_normal
    beq+     cr4, load_texcoords_per_vertex
    psq_l    f23, 0(r25), 0, 0
    psq_l    f21, 0x8(r25), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        vertex0
load_texcoords_per_vertex:
    beq+     cr3, load_normal
    psq_l    f23, 0(r25), 0, 0
    psq_l    f22, 0x8(r25), 0, 0
    psq_l    f21, 0x10(r25), 0, 0
    psq_l    f20, 0x18(r25), 0, 0
load_normal:
    beq+     cr5, vertex0
    psq_l    f19, 0(r26), 0, 0
    psq_l    f18, 0x8(r26), 1, 0
    /* write the four corners: position, then normal, color and texture coordinate if present */
vertex0:
    psq_st   f31, 0(r27), 0, 6
    psq_st   f30, 0(r27), 1, 6
    beq+     cr5, vertex0_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex0_color:
    beq      cr7, vertex0_texcoord
    stw      r31, 0(r27)
vertex0_texcoord:
    beq+     cr6, vertex1
    psq_st   f20, 0(r27), 0, 0
vertex1:
    psq_st   f29, 0(r27), 0, 6
    psq_st   f28, 0(r27), 1, 6
    beq+     cr5, vertex1_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex1_color:
    beq      cr7, vertex1_texcoord
    stw      r30, 0(r27)
vertex1_texcoord:
    beq+     cr6, vertex2
    psq_st   f23, 0(r27), 0, 0
vertex2:
    psq_st   f27, 0(r27), 0, 6
    psq_st   f26, 0(r27), 1, 6
    beq+     cr5, vertex2_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex2_color:
    beq      cr7, vertex2_texcoord
    stw      r29, 0(r27)
vertex2_texcoord:
    beq+     cr6, vertex3
    psq_st   f22, 0(r27), 0, 0
vertex3:
    psq_st   f25, 0(r27), 0, 6
    psq_st   f24, 0(r27), 1, 6
    add      r23, r23, r19
    beq+     cr5, vertex3_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
    add      r26, r26, r22
vertex3_color:
    beq      cr7, vertex3_texcoord
    stw      r28, 0(r27)
    add      r24, r24, r20
vertex3_texcoord:
    beq+     cr6, next_particle
    psq_st   f21, 0(r27), 0, 0
    add      r25, r25, r21
next_particle:
    bdnz     particle
    /* restore the registers */
done:
    lwz      r0, 0x8(r1)
    mtcrf    255, r0
    lwz      r14, 0x10(r1)
    lwz      r15, 0x14(r1)
    lwz      r16, 0x18(r1)
    lwz      r17, 0x1c(r1)
    lwz      r18, 0x20(r1)
    lwz      r19, 0x24(r1)
    lwz      r20, 0x28(r1)
    lwz      r21, 0x2c(r1)
    lwz      r22, 0x30(r1)
    lwz      r23, 0x34(r1)
    lwz      r24, 0x38(r1)
    lwz      r25, 0x3c(r1)
    lwz      r26, 0x40(r1)
    lwz      r27, 0x44(r1)
    lwz      r28, 0x48(r1)
    lwz      r29, 0x4c(r1)
    lwz      r30, 0x50(r1)
    lwz      r31, 0x54(r1)
    lfd      f14, 0x58(r1)
    lfd      f15, 0x60(r1)
    lfd      f16, 0x68(r1)
    lfd      f17, 0x70(r1)
    lfd      f18, 0x78(r1)
    lfd      f19, 0x80(r1)
    lfd      f20, 0x88(r1)
    lfd      f21, 0x90(r1)
    lfd      f22, 0x98(r1)
    lfd      f23, 0xa0(r1)
    lfd      f24, 0xa8(r1)
    lfd      f25, 0xb0(r1)
    lfd      f26, 0xb8(r1)
    lfd      f27, 0xc0(r1)
    lfd      f28, 0xc8(r1)
    lfd      f29, 0xd0(r1)
    lfd      f30, 0xd8(r1)
    lfd      f31, 0xe0(r1)
    addi     r1, r1, 0xe8
    blr
#endif // clang-format on
}

/* Writes the particles of a tank with a size per particle: as fn_80449994, scaling the camera axes
 * by half each particle's size. */
asm void fn_8044A168(register RwUInt16 count, register RwUInt32 platFlags,
                     register RpPTankData *ptankGlobal) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the condition register and the non-volatile registers */
    stwu     r1, -0xe8(r1)
    mfcr     r0
    stw      r0, 0x8(r1)
    stw      r14, 0x10(r1)
    stw      r15, 0x14(r1)
    stw      r16, 0x18(r1)
    stw      r17, 0x1c(r1)
    stw      r18, 0x20(r1)
    stw      r19, 0x24(r1)
    stw      r20, 0x28(r1)
    stw      r21, 0x2c(r1)
    stw      r22, 0x30(r1)
    stw      r23, 0x34(r1)
    stw      r24, 0x38(r1)
    stw      r25, 0x3c(r1)
    stw      r26, 0x40(r1)
    stw      r27, 0x44(r1)
    stw      r28, 0x48(r1)
    stw      r29, 0x4c(r1)
    stw      r30, 0x50(r1)
    stw      r31, 0x54(r1)
    stfd     f14, 0x58(r1)
    stfd     f15, 0x60(r1)
    stfd     f16, 0x68(r1)
    stfd     f17, 0x70(r1)
    stfd     f18, 0x78(r1)
    stfd     f19, 0x80(r1)
    stfd     f20, 0x88(r1)
    stfd     f21, 0x90(r1)
    stfd     f22, 0x98(r1)
    stfd     f23, 0xa0(r1)
    stfd     f24, 0xa8(r1)
    stfd     f25, 0xb0(r1)
    stfd     f26, 0xb8(r1)
    stfd     f27, 0xc0(r1)
    stfd     f28, 0xc8(r1)
    stfd     f29, 0xd0(r1)
    stfd     f30, 0xd8(r1)
    stfd     f31, 0xe0(r1)
    /* the stream selections of platFlags, tested once for the whole loop */
    rlwinm   r8, platFlags, 0, 9, 9
    rlwinm   r9, platFlags, 0, 8, 8
    rlwinm   r10, platFlags, 0, 13, 13
    rlwinm   r11, platFlags, 0, 13, 15
    rlwinm   r12, platFlags, 0, 15, 15
    rlwinm   r14, platFlags, 0, 14, 14
    rlwinm   r15, platFlags, 0, 8, 11
    rlwinm   r16, platFlags, 0, 11, 11
    rlwinm   r17, platFlags, 0, 10, 10
    rlwinm   r18, platFlags, 0, 7, 7
    cmplwi   cr7, r11, 0
    cmplwi   cr6, r15, 0
    cmplwi   cr5, r18, 0
    cmplwi   cr4, r16, 0
    cmplwi   cr3, r17, 0
    cmplwi   cr2, r12, 0
    cmplwi   cr1, r14, 0
    /* the positions and sizes; f1 = (0.5, 1) and the camera axes */
    lwz      r23, RpPTankData.dataPosition.data(ptankGlobal)
    lwz      r19, RpPTankData.dataPosition.stride(ptankGlobal)
    lwz      r18, RpPTankData.dataSize.data(ptankGlobal)
    lwz      r17, RpPTankData.dataSize.stride(ptankGlobal)
    lwz      r12, lbl_805EDD78
    lwz      r11, RpPTankData.cameraAxes(ptankGlobal)
    psq_l    f1, 0(r12), 1, 0
    psq_l    f5, PTANKCAMERAAXIS(0, x)(r11), 0, 0
    psq_l    f4, PTANKCAMERAAXIS(0, z)(r11), 1, 0
    psq_l    f3, PTANKCAMERAAXIS(1, x)(r11), 0, 0
    psq_l    f2, PTANKCAMERAAXIS(1, z)(r11), 1, 0
    /* colors: one per particle, one per corner, or the constant four */
    beq      cr7, texcoords
    beq      cr2, colors_per_vertex
    lwz      r24, RpPTankData.dataColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataColor.stride(ptankGlobal)
    b        texcoords
colors_per_vertex:
    beq+     cr1, colors_constant
    lwz      r24, RpPTankData.dataVtxColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataVtxColor.stride(ptankGlobal)
    b        texcoords
colors_constant:
    cmplwi   r10, 0
    beq+     texcoords
    lwz      r31, PTANKCONSTANT(cVtxColor[0])(ptankGlobal)
    lwz      r30, PTANKCONSTANT(cVtxColor[1])(ptankGlobal)
    lwz      r29, PTANKCONSTANT(cVtxColor[2])(ptankGlobal)
    lwz      r28, PTANKCONSTANT(cVtxColor[3])(ptankGlobal)
    /* texture coordinates: two or four per particle, or the constant two or four */
texcoords:
    beq+     cr6, normals
    beq+     cr4, texcoords_per_vertex
    lwz      r25, RpPTankData.dataVtx2TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx2TexCoords.stride(ptankGlobal)
    b        normals
texcoords_per_vertex:
    beq+     cr3, texcoords_constant2
    lwz      r25, RpPTankData.dataVtx4TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx4TexCoords.stride(ptankGlobal)
    b        normals
texcoords_constant2:
    cmplwi   r8, 0
    beq+     texcoords_constant4
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        normals
texcoords_constant4:
    cmplwi   r9, 0
    beq      normals
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f22, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[2])(ptankGlobal), 0, 0
    psq_l    f20, PTANKCONSTANT(cVtxUV[3])(ptankGlobal), 0, 0
    /* normals: one per particle */
normals:
    beq+     cr5, setup_done
    lwz      r26, RpPTankData.dataNormals.data(ptankGlobal)
    lwz      r22, RpPTankData.dataNormals.stride(ptankGlobal)
    /* the vertex FIFO; nothing to do without particles */
setup_done:
    cmpwi    count, 0
    lwz      r27, lbl_805FB0A0
    mtctr    count
    ble      done
particle:
    /* r = axes[0] * size.x / 2 and u = axes[1] * size.y / 2; the corners are pos - (r + u), pos +
     * (r - u), pos + (r + u) and pos - (r - u) */
    psq_l    f13, RwV2d.x(r18), 0, 0
    psq_l    f11, RwV3d.x(r23), 0, 0
    psq_l    f10, RwV3d.z(r23), 1, 0
    ps_muls0 f12, f13, f1
    nop
    nop
    ps_muls0 f9, f5, f12
    ps_muls0 f8, f4, f12
    ps_muls1 f7, f3, f12
    ps_muls1 f6, f2, f12
    ps_add   f15, f9, f7
    ps_add   f14, f8, f6
    ps_sub   f17, f9, f7
    ps_sub   f16, f8, f6
    ps_add   f27, f11, f15
    ps_add   f26, f10, f14
    ps_add   f29, f11, f17
    ps_add   f28, f10, f16
    ps_sub   f31, f11, f15
    ps_sub   f30, f10, f14
    ps_sub   f25, f11, f17
    ps_sub   f24, f10, f16
    /* this particle's colors, texture coordinates and normal */
    beq      cr7, load_texcoords
    beq      cr2, load_colors_per_vertex
    lwz      r31, 0(r24)
    lwz      r30, 0(r24)
    lwz      r29, 0(r24)
    lwz      r28, 0(r24)
    b        load_texcoords
load_colors_per_vertex:
    beq+     cr1, load_texcoords
    lwz      r31, 0(r24)
    lwz      r30, 0x4(r24)
    lwz      r29, 0x8(r24)
    lwz      r28, 0xc(r24)
load_texcoords:
    beq+     cr6, load_normal
    beq+     cr4, load_texcoords_per_vertex
    psq_l    f23, 0(r25), 0, 0
    psq_l    f21, 0x8(r25), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        vertex0
load_texcoords_per_vertex:
    beq+     cr3, load_normal
    psq_l    f23, 0(r25), 0, 0
    psq_l    f22, 0x8(r25), 0, 0
    psq_l    f21, 0x10(r25), 0, 0
    psq_l    f20, 0x18(r25), 0, 0
load_normal:
    beq+     cr5, vertex0
    psq_l    f19, 0(r26), 0, 0
    psq_l    f18, 0x8(r26), 1, 0
    /* write the four corners: position, then normal, color and texture coordinate if present */
vertex0:
    psq_st   f31, 0(r27), 0, 6
    psq_st   f30, 0(r27), 1, 6
    beq+     cr5, vertex0_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex0_color:
    beq      cr7, vertex0_texcoord
    stw      r31, 0(r27)
vertex0_texcoord:
    beq+     cr6, vertex1
    psq_st   f20, 0(r27), 0, 0
vertex1:
    psq_st   f29, 0(r27), 0, 6
    psq_st   f28, 0(r27), 1, 6
    beq+     cr5, vertex1_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex1_color:
    beq      cr7, vertex1_texcoord
    stw      r30, 0(r27)
vertex1_texcoord:
    beq+     cr6, vertex2
    psq_st   f23, 0(r27), 0, 0
vertex2:
    psq_st   f27, 0(r27), 0, 6
    psq_st   f26, 0(r27), 1, 6
    beq+     cr5, vertex2_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex2_color:
    beq      cr7, vertex2_texcoord
    stw      r29, 0(r27)
vertex2_texcoord:
    beq+     cr6, vertex3
    psq_st   f22, 0(r27), 0, 0
vertex3:
    psq_st   f25, 0(r27), 0, 6
    add      r18, r18, r17
    psq_st   f24, 0(r27), 1, 6
    add      r23, r23, r19
    beq+     cr5, vertex3_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
    add      r26, r26, r22
vertex3_color:
    beq      cr7, vertex3_texcoord
    stw      r28, 0(r27)
    add      r24, r24, r20
vertex3_texcoord:
    beq+     cr6, next_particle
    psq_st   f21, 0(r27), 0, 0
    add      r25, r25, r21
next_particle:
    bdnz     particle
    /* restore the registers */
done:
    lwz      r0, 0x8(r1)
    mtcrf    255, r0
    lwz      r14, 0x10(r1)
    lwz      r15, 0x14(r1)
    lwz      r16, 0x18(r1)
    lwz      r17, 0x1c(r1)
    lwz      r18, 0x20(r1)
    lwz      r19, 0x24(r1)
    lwz      r20, 0x28(r1)
    lwz      r21, 0x2c(r1)
    lwz      r22, 0x30(r1)
    lwz      r23, 0x34(r1)
    lwz      r24, 0x38(r1)
    lwz      r25, 0x3c(r1)
    lwz      r26, 0x40(r1)
    lwz      r27, 0x44(r1)
    lwz      r28, 0x48(r1)
    lwz      r29, 0x4c(r1)
    lwz      r30, 0x50(r1)
    lwz      r31, 0x54(r1)
    lfd      f14, 0x58(r1)
    lfd      f15, 0x60(r1)
    lfd      f16, 0x68(r1)
    lfd      f17, 0x70(r1)
    lfd      f18, 0x78(r1)
    lfd      f19, 0x80(r1)
    lfd      f20, 0x88(r1)
    lfd      f21, 0x90(r1)
    lfd      f22, 0x98(r1)
    lfd      f23, 0xa0(r1)
    lfd      f24, 0xa8(r1)
    lfd      f25, 0xb0(r1)
    lfd      f26, 0xb8(r1)
    lfd      f27, 0xc0(r1)
    lfd      f28, 0xc8(r1)
    lfd      f29, 0xd0(r1)
    lfd      f30, 0xd8(r1)
    lfd      f31, 0xe0(r1)
    addi     r1, r1, 0xe8
    blr
#endif // clang-format on
}

/* Writes the particles of a tank with a size per particle and a center: as fn_8044A168, with each
 * quad moved back by the tank's center along the camera axes. */
asm void fn_8044A540(register RwUInt16 count, register RwUInt32 platFlags,
                     register RpPTankData *ptankGlobal) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the condition register and the non-volatile registers */
    stwu     r1, -0xe8(r1)
    mfcr     r0
    stw      r0, 0x8(r1)
    stw      r14, 0x10(r1)
    stw      r15, 0x14(r1)
    stw      r16, 0x18(r1)
    stw      r17, 0x1c(r1)
    stw      r18, 0x20(r1)
    stw      r19, 0x24(r1)
    stw      r20, 0x28(r1)
    stw      r21, 0x2c(r1)
    stw      r22, 0x30(r1)
    stw      r23, 0x34(r1)
    stw      r24, 0x38(r1)
    stw      r25, 0x3c(r1)
    stw      r26, 0x40(r1)
    stw      r27, 0x44(r1)
    stw      r28, 0x48(r1)
    stw      r29, 0x4c(r1)
    stw      r30, 0x50(r1)
    stw      r31, 0x54(r1)
    stfd     f14, 0x58(r1)
    stfd     f15, 0x60(r1)
    stfd     f16, 0x68(r1)
    stfd     f17, 0x70(r1)
    stfd     f18, 0x78(r1)
    stfd     f19, 0x80(r1)
    stfd     f20, 0x88(r1)
    stfd     f21, 0x90(r1)
    stfd     f22, 0x98(r1)
    stfd     f23, 0xa0(r1)
    stfd     f24, 0xa8(r1)
    stfd     f25, 0xb0(r1)
    stfd     f26, 0xb8(r1)
    stfd     f27, 0xc0(r1)
    stfd     f28, 0xc8(r1)
    stfd     f29, 0xd0(r1)
    stfd     f30, 0xd8(r1)
    stfd     f31, 0xe0(r1)
    /* the stream selections of platFlags, tested once for the whole loop */
    rlwinm   r8, platFlags, 0, 9, 9
    rlwinm   r9, platFlags, 0, 8, 8
    rlwinm   r10, platFlags, 0, 13, 13
    rlwinm   r11, platFlags, 0, 13, 15
    rlwinm   r12, platFlags, 0, 15, 15
    rlwinm   r14, platFlags, 0, 14, 14
    rlwinm   r15, platFlags, 0, 8, 11
    rlwinm   r16, platFlags, 0, 11, 11
    rlwinm   r17, platFlags, 0, 10, 10
    rlwinm   r18, platFlags, 0, 7, 7
    cmplwi   cr7, r11, 0
    cmplwi   cr6, r15, 0
    cmplwi   cr5, r18, 0
    cmplwi   cr4, r16, 0
    cmplwi   cr3, r17, 0
    cmplwi   cr2, r12, 0
    cmplwi   cr1, r14, 0
    /* f1 = (0.5, 1), the camera axes, the positions and sizes, and the center offset f10/f11 =
     * -(center.x * axes[0] + center.y * axes[1]) */
    lwz      r12, lbl_805EDD78
    lwz      r11, RpPTankData.cameraAxes(ptankGlobal)
    psq_l    f1, 0(r12), 1, 0
    psq_l    f5, PTANKCAMERAAXIS(0, x)(r11), 0, 0
    psq_l    f4, PTANKCAMERAAXIS(0, z)(r11), 1, 0
    psq_l    f12, PTANKCONSTANT(cCenter)(ptankGlobal), 0, 0
    psq_l    f3, PTANKCAMERAAXIS(1, x)(r11), 0, 0
    psq_l    f2, PTANKCAMERAAXIS(1, z)(r11), 1, 0
    ps_neg   f12, f12
    lwz      r23, RpPTankData.dataPosition.data(ptankGlobal)
    lwz      r19, RpPTankData.dataPosition.stride(ptankGlobal)
    ps_muls0 f17, f5, f12
    ps_muls0 f16, f4, f12
    ps_muls1 f15, f3, f12
    ps_muls1 f14, f2, f12
    lwz      r18, RpPTankData.dataSize.data(ptankGlobal)
    lwz      r17, RpPTankData.dataSize.stride(ptankGlobal)
    ps_add   f11, f17, f15
    ps_add   f10, f16, f14
    /* colors: one per particle, one per corner, or the constant four */
    beq      cr7, texcoords
    beq      cr2, colors_per_vertex
    lwz      r24, RpPTankData.dataColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataColor.stride(ptankGlobal)
    b        texcoords
colors_per_vertex:
    beq+     cr1, colors_constant
    lwz      r24, RpPTankData.dataVtxColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataVtxColor.stride(ptankGlobal)
    b        texcoords
colors_constant:
    cmplwi   r10, 0
    beq+     texcoords
    lwz      r31, PTANKCONSTANT(cVtxColor[0])(ptankGlobal)
    lwz      r30, PTANKCONSTANT(cVtxColor[1])(ptankGlobal)
    lwz      r29, PTANKCONSTANT(cVtxColor[2])(ptankGlobal)
    lwz      r28, PTANKCONSTANT(cVtxColor[3])(ptankGlobal)
    /* texture coordinates: two or four per particle, or the constant two or four */
texcoords:
    beq+     cr6, normals
    beq+     cr4, texcoords_per_vertex
    lwz      r25, RpPTankData.dataVtx2TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx2TexCoords.stride(ptankGlobal)
    b        normals
texcoords_per_vertex:
    beq+     cr3, texcoords_constant2
    lwz      r25, RpPTankData.dataVtx4TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx4TexCoords.stride(ptankGlobal)
    b        normals
texcoords_constant2:
    cmplwi   r8, 0
    beq+     texcoords_constant4
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        normals
texcoords_constant4:
    cmplwi   r9, 0
    beq      normals
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f22, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[2])(ptankGlobal), 0, 0
    psq_l    f20, PTANKCONSTANT(cVtxUV[3])(ptankGlobal), 0, 0
    /* normals: one per particle */
normals:
    beq+     cr5, setup_done
    lwz      r26, RpPTankData.dataNormals.data(ptankGlobal)
    lwz      r22, RpPTankData.dataNormals.stride(ptankGlobal)
    /* the vertex FIFO; nothing to do without particles */
setup_done:
    cmpwi    count, 0
    lwz      r27, lbl_805FB0A0
    mtctr    count
    ble      done
particle:
    /* pos += the center offset; r = axes[0] * size.x / 2 and u = axes[1] * size.y / 2; the corners
     * are pos - (r + u), pos + (r - u), pos + (r + u) and pos - (r - u) */
    psq_l    f13, RwV2d.x(r18), 0, 0
    psq_l    f19, RwV3d.x(r23), 0, 0
    psq_l    f18, RwV3d.z(r23), 1, 0
    ps_muls0 f12, f13, f1
    ps_add   f19, f19, f11
    ps_add   f18, f18, f10
    ps_muls0 f9, f5, f12
    ps_muls0 f8, f4, f12
    ps_muls1 f7, f3, f12
    ps_muls1 f6, f2, f12
    nop
    ps_add   f15, f9, f7
    ps_add   f14, f8, f6
    ps_sub   f17, f9, f7
    ps_sub   f16, f8, f6
    ps_add   f27, f19, f15
    ps_add   f26, f18, f14
    ps_add   f29, f19, f17
    ps_add   f28, f18, f16
    ps_sub   f31, f19, f15
    ps_sub   f30, f18, f14
    ps_sub   f25, f19, f17
    ps_sub   f24, f18, f16
    /* this particle's colors, texture coordinates and normal */
    beq      cr7, load_texcoords
    beq      cr2, load_colors_per_vertex
    lwz      r31, 0(r24)
    lwz      r30, 0(r24)
    lwz      r29, 0(r24)
    lwz      r28, 0(r24)
    b        load_texcoords
load_colors_per_vertex:
    beq+     cr1, load_texcoords
    lwz      r31, 0(r24)
    lwz      r30, 0x4(r24)
    lwz      r29, 0x8(r24)
    lwz      r28, 0xc(r24)
load_texcoords:
    beq+     cr6, load_normal
    beq+     cr4, load_texcoords_per_vertex
    psq_l    f23, 0(r25), 0, 0
    psq_l    f21, 0x8(r25), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        vertex0
load_texcoords_per_vertex:
    beq+     cr3, load_normal
    psq_l    f23, 0(r25), 0, 0
    psq_l    f22, 0x8(r25), 0, 0
    psq_l    f21, 0x10(r25), 0, 0
    psq_l    f20, 0x18(r25), 0, 0
load_normal:
    beq+     cr5, vertex0
    psq_l    f19, 0(r26), 0, 0
    psq_l    f18, 0x8(r26), 1, 0
    /* write the four corners: position, then normal, color and texture coordinate if present */
vertex0:
    psq_st   f31, 0(r27), 0, 6
    psq_st   f30, 0(r27), 1, 6
    beq+     cr5, vertex0_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex0_color:
    beq      cr7, vertex0_texcoord
    stw      r31, 0(r27)
vertex0_texcoord:
    beq+     cr6, vertex1
    psq_st   f20, 0(r27), 0, 0
vertex1:
    psq_st   f29, 0(r27), 0, 6
    psq_st   f28, 0(r27), 1, 6
    beq+     cr5, vertex1_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex1_color:
    beq      cr7, vertex1_texcoord
    stw      r30, 0(r27)
vertex1_texcoord:
    beq+     cr6, vertex2
    psq_st   f23, 0(r27), 0, 0
vertex2:
    psq_st   f27, 0(r27), 0, 6
    psq_st   f26, 0(r27), 1, 6
    beq+     cr5, vertex2_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex2_color:
    beq      cr7, vertex2_texcoord
    stw      r29, 0(r27)
vertex2_texcoord:
    beq+     cr6, vertex3
    psq_st   f22, 0(r27), 0, 0
vertex3:
    psq_st   f25, 0(r27), 0, 6
    add      r18, r18, r17
    psq_st   f24, 0(r27), 1, 6
    add      r23, r23, r19
    beq+     cr5, vertex3_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
    add      r26, r26, r22
vertex3_color:
    beq      cr7, vertex3_texcoord
    stw      r28, 0(r27)
    add      r24, r24, r20
vertex3_texcoord:
    beq+     cr6, next_particle
    psq_st   f21, 0(r27), 0, 0
    add      r25, r25, r21
next_particle:
    bdnz     particle
    /* restore the registers */
done:
    lwz      r0, 0x8(r1)
    mtcrf    255, r0
    lwz      r14, 0x10(r1)
    lwz      r15, 0x14(r1)
    lwz      r16, 0x18(r1)
    lwz      r17, 0x1c(r1)
    lwz      r18, 0x20(r1)
    lwz      r19, 0x24(r1)
    lwz      r20, 0x28(r1)
    lwz      r21, 0x2c(r1)
    lwz      r22, 0x30(r1)
    lwz      r23, 0x34(r1)
    lwz      r24, 0x38(r1)
    lwz      r25, 0x3c(r1)
    lwz      r26, 0x40(r1)
    lwz      r27, 0x44(r1)
    lwz      r28, 0x48(r1)
    lwz      r29, 0x4c(r1)
    lwz      r30, 0x50(r1)
    lwz      r31, 0x54(r1)
    lfd      f14, 0x58(r1)
    lfd      f15, 0x60(r1)
    lfd      f16, 0x68(r1)
    lfd      f17, 0x70(r1)
    lfd      f18, 0x78(r1)
    lfd      f19, 0x80(r1)
    lfd      f20, 0x88(r1)
    lfd      f21, 0x90(r1)
    lfd      f22, 0x98(r1)
    lfd      f23, 0xa0(r1)
    lfd      f24, 0xa8(r1)
    lfd      f25, 0xb0(r1)
    lfd      f26, 0xb8(r1)
    lfd      f27, 0xc0(r1)
    lfd      f28, 0xc8(r1)
    lfd      f29, 0xd0(r1)
    lfd      f30, 0xd8(r1)
    lfd      f31, 0xe0(r1)
    addi     r1, r1, 0xe8
    blr
#endif // clang-format on
}

/* Writes the particles of a tank with a rotation per particle and the constant size: the camera
 * axes are rotated by each particle's angle before they span its quad. */
asm void fn_8044A93C(register RwUInt16 count, register RwUInt32 platFlags,
                     register RpPTankData *ptankGlobal) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the condition register and the non-volatile registers */
    stwu     r1, -0xe8(r1)
    mfcr     r0
    stw      r0, 0x8(r1)
    stw      r14, 0x10(r1)
    stw      r15, 0x14(r1)
    stw      r16, 0x18(r1)
    stw      r17, 0x1c(r1)
    stw      r18, 0x20(r1)
    stw      r19, 0x24(r1)
    stw      r20, 0x28(r1)
    stw      r21, 0x2c(r1)
    stw      r22, 0x30(r1)
    stw      r23, 0x34(r1)
    stw      r24, 0x38(r1)
    stw      r25, 0x3c(r1)
    stw      r26, 0x40(r1)
    stw      r27, 0x44(r1)
    stw      r28, 0x48(r1)
    stw      r29, 0x4c(r1)
    stw      r30, 0x50(r1)
    stw      r31, 0x54(r1)
    stfd     f14, 0x58(r1)
    stfd     f15, 0x60(r1)
    stfd     f16, 0x68(r1)
    stfd     f17, 0x70(r1)
    stfd     f18, 0x78(r1)
    stfd     f19, 0x80(r1)
    stfd     f20, 0x88(r1)
    stfd     f21, 0x90(r1)
    stfd     f22, 0x98(r1)
    stfd     f23, 0xa0(r1)
    stfd     f24, 0xa8(r1)
    stfd     f25, 0xb0(r1)
    stfd     f26, 0xb8(r1)
    stfd     f27, 0xc0(r1)
    stfd     f28, 0xc8(r1)
    stfd     f29, 0xd0(r1)
    stfd     f30, 0xd8(r1)
    stfd     f31, 0xe0(r1)
    /* the stream selections of platFlags, tested once for the whole loop */
    rlwinm   r8, platFlags, 0, 9, 9
    rlwinm   r9, platFlags, 0, 8, 8
    rlwinm   r10, platFlags, 0, 13, 13
    rlwinm   r11, platFlags, 0, 13, 15
    rlwinm   r12, platFlags, 0, 15, 15
    rlwinm   r14, platFlags, 0, 14, 14
    rlwinm   r15, platFlags, 0, 8, 11
    rlwinm   r16, platFlags, 0, 11, 11
    rlwinm   r17, platFlags, 0, 10, 10
    rlwinm   r18, platFlags, 0, 7, 7
    cmplwi   cr7, r11, 0
    cmplwi   cr6, r15, 0
    cmplwi   cr5, r18, 0
    cmplwi   cr4, r16, 0
    cmplwi   cr3, r17, 0
    cmplwi   cr2, r12, 0
    cmplwi   cr1, r14, 0
    /* the positions and angles; with the camera axes scaled by half the size, f8/f9 = axes[0] *
     * size.x / 2, f6/f7 = -axes[1] * size.x / 2, f4/f5 = axes[0] * size.y / 2 and f2/f3 = axes[1] *
     * size.y / 2; then the sine constants in f10-f13 */
    lwz      r12, lbl_805EDD78
    lwz      r11, RpPTankData.cameraAxes(ptankGlobal)
    lwz      r23, RpPTankData.dataPosition.data(ptankGlobal)
    lwz      r19, RpPTankData.dataPosition.stride(ptankGlobal)
    lwz      r16, RpPTankData.dataRotate.data(ptankGlobal)
    lwz      r15, RpPTankData.dataRotate.stride(ptankGlobal)
    psq_l    f13, PTANKCONSTANT(cSize)(ptankGlobal), 0, 0
    psq_l    f1, 0(r12), 1, 0
    psq_l    f17, PTANKCAMERAAXIS(0, x)(r11), 0, 0
    psq_l    f16, PTANKCAMERAAXIS(0, z)(r11), 1, 0
    ps_muls0 f12, f13, f1
    psq_l    f15, PTANKCAMERAAXIS(1, x)(r11), 0, 0
    psq_l    f14, PTANKCAMERAAXIS(1, z)(r11), 1, 0
    ps_muls0 f9, f17, f12
    ps_muls0 f8, f16, f12
    ps_muls0 f7, f15, f12
    ps_muls0 f6, f14, f12
    ps_muls1 f5, f17, f12
    ps_muls1 f4, f16, f12
    ps_muls1 f3, f15, f12
    ps_muls1 f2, f14, f12
    lwz      r14, lbl_805EDD7C
    ps_neg   f7, f7
    ps_neg   f6, f6
    psq_l    f10, 0(r14), 0, 0
    psq_l    f11, 0x8(r14), 0, 0
    psq_l    f12, 0x10(r14), 0, 0
    psq_l    f13, 0x18(r14), 0, 0
    /* colors: one per particle, one per corner, or the constant four */
    beq      cr7, texcoords
    beq      cr2, colors_per_vertex
    lwz      r24, RpPTankData.dataColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataColor.stride(ptankGlobal)
    b        texcoords
colors_per_vertex:
    beq+     cr1, colors_constant
    lwz      r24, RpPTankData.dataVtxColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataVtxColor.stride(ptankGlobal)
    b        texcoords
colors_constant:
    cmplwi   r10, 0
    beq+     texcoords
    lwz      r31, PTANKCONSTANT(cVtxColor[0])(ptankGlobal)
    lwz      r30, PTANKCONSTANT(cVtxColor[1])(ptankGlobal)
    lwz      r29, PTANKCONSTANT(cVtxColor[2])(ptankGlobal)
    lwz      r28, PTANKCONSTANT(cVtxColor[3])(ptankGlobal)
    /* texture coordinates: two or four per particle, or the constant two or four */
texcoords:
    beq+     cr6, normals
    beq+     cr4, texcoords_per_vertex
    lwz      r25, RpPTankData.dataVtx2TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx2TexCoords.stride(ptankGlobal)
    b        normals
texcoords_per_vertex:
    beq+     cr3, texcoords_constant2
    lwz      r25, RpPTankData.dataVtx4TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx4TexCoords.stride(ptankGlobal)
    b        normals
texcoords_constant2:
    cmplwi   r8, 0
    beq+     texcoords_constant4
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        normals
texcoords_constant4:
    cmplwi   r9, 0
    beq      normals
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f22, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[2])(ptankGlobal), 0, 0
    psq_l    f20, PTANKCONSTANT(cVtxUV[3])(ptankGlobal), 0, 0
    /* normals: one per particle */
normals:
    beq+     cr5, setup_done
    lwz      r26, RpPTankData.dataNormals.data(ptankGlobal)
    lwz      r22, RpPTankData.dataNormals.stride(ptankGlobal)
    /* the vertex FIFO; nothing to do without particles */
setup_done:
    cmpwi    count, 0
    lwz      r27, lbl_805FB0A0
    mtctr    count
    ble      done
particle:
    /* the sine and cosine of the particle's angle a: f17 = (a, a - pi/2), wrapped into [-pi, pi] by
     * -/+ pi, then 4 / pi^2 * x * (pi - |x|) */
    psq_l    f17, 0(r16), 1, 0
    ps_neg   f14, f10
    nop
    ps_madds0 f17, f12, f17, f11
    nop
    nop
    ps_sel   f16, f17, f14, f10
    nop
    nop
    ps_add   f17, f17, f16
    nop
    nop
    ps_nabs  f16, f17
    ps_muls0 f17, f17, f13
    nop
    ps_madds1 f16, f12, f10, f16
    psq_l    f19, RwV3d.x(r23), 0, 0
    psq_l    f18, RwV3d.z(r23), 1, 0
    ps_mul   f17, f16, f17
    nop
    nop
    /* r = (axes[0] * cos a - axes[1] * sin a) * size.x / 2 and u = (axes[0] * sin a + axes[1] * cos
     * a) * size.y / 2; the corners are pos - (r + u), pos + (r - u), pos + (r + u) and pos - (r -
     * u) */
    ps_muls0 f31, f7, f17
    ps_muls0 f30, f6, f17
    ps_muls0 f29, f5, f17
    ps_muls0 f28, f4, f17
    ps_madds1 f31, f9, f17, f31
    ps_madds1 f30, f8, f17, f30
    ps_madds1 f29, f3, f17, f29
    ps_madds1 f28, f2, f17, f28
    nop
    ps_add   f15, f31, f29
    ps_add   f14, f30, f28
    ps_sub   f17, f31, f29
    ps_sub   f16, f30, f28
    ps_sub   f31, f19, f15
    ps_sub   f30, f18, f14
    ps_add   f29, f19, f17
    ps_add   f28, f18, f16
    ps_add   f27, f19, f15
    ps_add   f26, f18, f14
    ps_sub   f25, f19, f17
    ps_sub   f24, f18, f16
    /* this particle's colors, texture coordinates and normal */
    beq      cr7, load_texcoords
    beq      cr2, load_colors_per_vertex
    lwz      r31, 0(r24)
    lwz      r30, 0(r24)
    lwz      r29, 0(r24)
    lwz      r28, 0(r24)
    b        load_texcoords
load_colors_per_vertex:
    beq+     cr1, load_texcoords
    lwz      r31, 0(r24)
    lwz      r30, 0x4(r24)
    lwz      r29, 0x8(r24)
    lwz      r28, 0xc(r24)
load_texcoords:
    beq+     cr6, load_normal
    beq+     cr4, load_texcoords_per_vertex
    psq_l    f23, 0(r25), 0, 0
    psq_l    f21, 0x8(r25), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        vertex0
load_texcoords_per_vertex:
    beq+     cr3, load_normal
    psq_l    f23, 0(r25), 0, 0
    psq_l    f22, 0x8(r25), 0, 0
    psq_l    f21, 0x10(r25), 0, 0
    psq_l    f20, 0x18(r25), 0, 0
load_normal:
    beq+     cr5, vertex0
    psq_l    f19, 0(r26), 0, 0
    psq_l    f18, 0x8(r26), 1, 0
    /* write the four corners: position, then normal, color and texture coordinate if present */
vertex0:
    psq_st   f31, 0(r27), 0, 6
    psq_st   f30, 0(r27), 1, 6
    beq+     cr5, vertex0_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex0_color:
    beq      cr7, vertex0_texcoord
    stw      r31, 0(r27)
vertex0_texcoord:
    beq+     cr6, vertex1
    psq_st   f20, 0(r27), 0, 0
vertex1:
    psq_st   f29, 0(r27), 0, 6
    psq_st   f28, 0(r27), 1, 6
    beq+     cr5, vertex1_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex1_color:
    beq      cr7, vertex1_texcoord
    stw      r30, 0(r27)
vertex1_texcoord:
    beq+     cr6, vertex2
    psq_st   f23, 0(r27), 0, 0
vertex2:
    psq_st   f27, 0(r27), 0, 6
    psq_st   f26, 0(r27), 1, 6
    beq+     cr5, vertex2_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex2_color:
    beq      cr7, vertex2_texcoord
    stw      r29, 0(r27)
vertex2_texcoord:
    beq+     cr6, vertex3
    psq_st   f22, 0(r27), 0, 0
vertex3:
    psq_st   f25, 0(r27), 0, 6
    add      r16, r16, r15
    psq_st   f24, 0(r27), 1, 6
    add      r23, r23, r19
    beq+     cr5, vertex3_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
    add      r26, r26, r22
vertex3_color:
    beq      cr7, vertex3_texcoord
    stw      r28, 0(r27)
    add      r24, r24, r20
vertex3_texcoord:
    beq+     cr6, next_particle
    psq_st   f21, 0(r27), 0, 0
    add      r25, r25, r21
next_particle:
    bdnz     particle
    /* restore the registers */
done:
    lwz      r0, 0x8(r1)
    mtcrf    255, r0
    lwz      r14, 0x10(r1)
    lwz      r15, 0x14(r1)
    lwz      r16, 0x18(r1)
    lwz      r17, 0x1c(r1)
    lwz      r18, 0x20(r1)
    lwz      r19, 0x24(r1)
    lwz      r20, 0x28(r1)
    lwz      r21, 0x2c(r1)
    lwz      r22, 0x30(r1)
    lwz      r23, 0x34(r1)
    lwz      r24, 0x38(r1)
    lwz      r25, 0x3c(r1)
    lwz      r26, 0x40(r1)
    lwz      r27, 0x44(r1)
    lwz      r28, 0x48(r1)
    lwz      r29, 0x4c(r1)
    lwz      r30, 0x50(r1)
    lwz      r31, 0x54(r1)
    lfd      f14, 0x58(r1)
    lfd      f15, 0x60(r1)
    lfd      f16, 0x68(r1)
    lfd      f17, 0x70(r1)
    lfd      f18, 0x78(r1)
    lfd      f19, 0x80(r1)
    lfd      f20, 0x88(r1)
    lfd      f21, 0x90(r1)
    lfd      f22, 0x98(r1)
    lfd      f23, 0xa0(r1)
    lfd      f24, 0xa8(r1)
    lfd      f25, 0xb0(r1)
    lfd      f26, 0xb8(r1)
    lfd      f27, 0xc0(r1)
    lfd      f28, 0xc8(r1)
    lfd      f29, 0xd0(r1)
    lfd      f30, 0xd8(r1)
    lfd      f31, 0xe0(r1)
    addi     r1, r1, 0xe8
    blr
#endif // clang-format on
}

/* Writes the particles of a tank with a rotation per particle, the constant size and a center: as
 * fn_8044A93C, with each quad moved back by the tank's center along its rotated axes. */
asm void fn_8044ADA8(register RwUInt16 count, register RwUInt32 platFlags,
                     register RpPTankData *ptankGlobal) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the condition register and the non-volatile registers */
    stwu     r1, -0xe8(r1)
    mfcr     r0
    stw      r0, 0x8(r1)
    stw      r14, 0x10(r1)
    stw      r15, 0x14(r1)
    stw      r16, 0x18(r1)
    stw      r17, 0x1c(r1)
    stw      r18, 0x20(r1)
    stw      r19, 0x24(r1)
    stw      r20, 0x28(r1)
    stw      r21, 0x2c(r1)
    stw      r22, 0x30(r1)
    stw      r23, 0x34(r1)
    stw      r24, 0x38(r1)
    stw      r25, 0x3c(r1)
    stw      r26, 0x40(r1)
    stw      r27, 0x44(r1)
    stw      r28, 0x48(r1)
    stw      r29, 0x4c(r1)
    stw      r30, 0x50(r1)
    stw      r31, 0x54(r1)
    stfd     f14, 0x58(r1)
    stfd     f15, 0x60(r1)
    stfd     f16, 0x68(r1)
    stfd     f17, 0x70(r1)
    stfd     f18, 0x78(r1)
    stfd     f19, 0x80(r1)
    stfd     f20, 0x88(r1)
    stfd     f21, 0x90(r1)
    stfd     f22, 0x98(r1)
    stfd     f23, 0xa0(r1)
    stfd     f24, 0xa8(r1)
    stfd     f25, 0xb0(r1)
    stfd     f26, 0xb8(r1)
    stfd     f27, 0xc0(r1)
    stfd     f28, 0xc8(r1)
    stfd     f29, 0xd0(r1)
    stfd     f30, 0xd8(r1)
    stfd     f31, 0xe0(r1)
    /* the stream selections of platFlags, tested once for the whole loop */
    rlwinm   r8, platFlags, 0, 9, 9
    rlwinm   r9, platFlags, 0, 8, 8
    rlwinm   r10, platFlags, 0, 13, 13
    rlwinm   r11, platFlags, 0, 13, 15
    rlwinm   r12, platFlags, 0, 15, 15
    rlwinm   r14, platFlags, 0, 14, 14
    rlwinm   r15, platFlags, 0, 8, 11
    rlwinm   r16, platFlags, 0, 11, 11
    rlwinm   r17, platFlags, 0, 10, 10
    rlwinm   r18, platFlags, 0, 7, 7
    cmplwi   cr7, r11, 0
    cmplwi   cr6, r15, 0
    cmplwi   cr5, r18, 0
    cmplwi   cr4, r16, 0
    cmplwi   cr3, r17, 0
    cmplwi   cr2, r12, 0
    cmplwi   cr1, r14, 0
    /* f1 = half the size, f0 = -center, the positions and angles, and the sine constants in
     * f10-f13 */
    lwz      r12, lbl_805EDD78
    lwz      r11, RpPTankData.cameraAxes(ptankGlobal)
    lwz      r14, lbl_805EDD7C
    psq_l    f0, 0(r12), 1, 0
    psq_l    f1, PTANKCONSTANT(cSize)(ptankGlobal), 0, 0
    lwz      r23, RpPTankData.dataPosition.data(ptankGlobal)
    lwz      r19, RpPTankData.dataPosition.stride(ptankGlobal)
    lwz      r18, RpPTankData.dataSize.data(ptankGlobal)
    lwz      r17, RpPTankData.dataSize.stride(ptankGlobal)
    lwz      r16, RpPTankData.dataRotate.data(ptankGlobal)
    lwz      r15, RpPTankData.dataRotate.stride(ptankGlobal)
    ps_muls0 f1, f1, f0
    psq_l    f0, PTANKCONSTANT(cCenter)(ptankGlobal), 0, 0
    psq_l    f10, 0(r14), 0, 0
    psq_l    f11, 0x8(r14), 0, 0
    psq_l    f12, 0x10(r14), 0, 0
    psq_l    f13, 0x18(r14), 0, 0
    ps_neg   f0, f0
    /* colors: one per particle, one per corner, or the constant four */
    beq      cr7, texcoords
    beq      cr2, colors_per_vertex
    lwz      r24, RpPTankData.dataColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataColor.stride(ptankGlobal)
    b        texcoords
colors_per_vertex:
    beq+     cr1, colors_constant
    lwz      r24, RpPTankData.dataVtxColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataVtxColor.stride(ptankGlobal)
    b        texcoords
colors_constant:
    cmplwi   r10, 0
    beq+     texcoords
    lwz      r31, PTANKCONSTANT(cVtxColor[0])(ptankGlobal)
    lwz      r30, PTANKCONSTANT(cVtxColor[1])(ptankGlobal)
    lwz      r29, PTANKCONSTANT(cVtxColor[2])(ptankGlobal)
    lwz      r28, PTANKCONSTANT(cVtxColor[3])(ptankGlobal)
    /* texture coordinates: two or four per particle, or the constant two or four */
texcoords:
    beq+     cr6, normals
    beq+     cr4, texcoords_per_vertex
    lwz      r25, RpPTankData.dataVtx2TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx2TexCoords.stride(ptankGlobal)
    b        normals
texcoords_per_vertex:
    beq+     cr3, texcoords_constant2
    lwz      r25, RpPTankData.dataVtx4TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx4TexCoords.stride(ptankGlobal)
    b        normals
texcoords_constant2:
    cmplwi   r8, 0
    beq+     texcoords_constant4
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        normals
texcoords_constant4:
    cmplwi   r9, 0
    beq      normals
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f22, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[2])(ptankGlobal), 0, 0
    psq_l    f20, PTANKCONSTANT(cVtxUV[3])(ptankGlobal), 0, 0
    /* normals: one per particle */
normals:
    beq+     cr5, setup_done
    lwz      r26, RpPTankData.dataNormals.data(ptankGlobal)
    lwz      r22, RpPTankData.dataNormals.stride(ptankGlobal)
    /* the vertex FIFO; nothing to do without particles */
setup_done:
    cmpwi    count, 0
    lwz      r27, lbl_805FB0A0
    mtctr    count
    ble      done
particle:
    /* the sine and cosine of the particle's angle a: f17 = (a, a - pi/2), wrapped into [-pi, pi] by
     * -/+ pi, then 4 / pi^2 * x * (pi - |x|) */
    psq_l    f17, 0(r16), 1, 0
    ps_neg   f14, f10
    nop
    ps_madds0 f17, f12, f17, f11
    psq_l    f9, PTANKCAMERAAXIS(0, x)(r11), 0, 0
    psq_l    f8, PTANKCAMERAAXIS(0, z)(r11), 1, 0
    ps_sel   f16, f17, f14, f10
    psq_l    f7, PTANKCAMERAAXIS(1, x)(r11), 0, 0
    psq_l    f6, PTANKCAMERAAXIS(1, z)(r11), 1, 0
    ps_add   f17, f17, f16
    nop
    nop
    ps_nabs  f16, f17
    ps_muls0 f17, f17, f13
    nop
    ps_madds1 f16, f12, f10, f16
    psq_l    f19, RwV3d.x(r23), 0, 0
    psq_l    f18, RwV3d.z(r23), 1, 0
    ps_mul   f17, f16, f17
    /* the rotated axes r = axes[0] * cos a - axes[1] * sin a and u = axes[0] * sin a + axes[1] *
     * cos a; pos -= center.x * r + center.y * u; r *= size.x / 2 and u *= size.y / 2; the corners
     * are pos - (r + u), pos + (r - u), pos + (r + u) and pos - (r - u) */
    ps_neg   f5, f7
    ps_neg   f4, f6
    ps_muls1 f31, f9, f17
    ps_muls1 f30, f8, f17
    ps_muls0 f29, f9, f17
    ps_muls0 f28, f8, f17
    ps_madds0 f31, f5, f17, f31
    ps_madds0 f30, f4, f17, f30
    ps_madds1 f29, f7, f17, f29
    ps_madds1 f28, f6, f17, f28
    ps_muls0 f3, f31, f0
    ps_muls0 f2, f30, f0
    ps_madds1 f3, f29, f0, f3
    ps_madds1 f2, f28, f0, f2
    ps_muls0 f31, f31, f1
    ps_muls0 f30, f30, f1
    ps_muls1 f29, f29, f1
    ps_muls1 f28, f28, f1
    ps_add   f19, f19, f3
    ps_add   f18, f18, f2
    ps_add   f15, f31, f29
    ps_add   f14, f30, f28
    ps_sub   f17, f31, f29
    ps_sub   f16, f30, f28
    ps_sub   f31, f19, f15
    ps_sub   f30, f18, f14
    ps_add   f29, f19, f17
    ps_add   f28, f18, f16
    ps_add   f27, f19, f15
    ps_add   f26, f18, f14
    ps_sub   f25, f19, f17
    ps_sub   f24, f18, f16
    /* this particle's colors, texture coordinates and normal */
    beq      cr7, load_texcoords
    beq      cr2, load_colors_per_vertex
    lwz      r31, 0(r24)
    lwz      r30, 0(r24)
    lwz      r29, 0(r24)
    lwz      r28, 0(r24)
    b        load_texcoords
load_colors_per_vertex:
    beq+     cr1, load_texcoords
    lwz      r31, 0(r24)
    lwz      r30, 0x4(r24)
    lwz      r29, 0x8(r24)
    lwz      r28, 0xc(r24)
load_texcoords:
    beq+     cr6, load_normal
    beq+     cr4, load_texcoords_per_vertex
    psq_l    f23, 0(r25), 0, 0
    psq_l    f21, 0x8(r25), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        vertex0
load_texcoords_per_vertex:
    beq+     cr3, load_normal
    psq_l    f23, 0(r25), 0, 0
    psq_l    f22, 0x8(r25), 0, 0
    psq_l    f21, 0x10(r25), 0, 0
    psq_l    f20, 0x18(r25), 0, 0
load_normal:
    beq+     cr5, vertex0
    psq_l    f19, 0(r26), 0, 0
    psq_l    f18, 0x8(r26), 1, 0
    /* write the four corners: position, then normal, color and texture coordinate if present */
vertex0:
    psq_st   f31, 0(r27), 0, 6
    psq_st   f30, 0(r27), 1, 6
    beq+     cr5, vertex0_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex0_color:
    beq      cr7, vertex0_texcoord
    stw      r31, 0(r27)
vertex0_texcoord:
    beq+     cr6, vertex1
    psq_st   f20, 0(r27), 0, 0
vertex1:
    psq_st   f29, 0(r27), 0, 6
    psq_st   f28, 0(r27), 1, 6
    beq+     cr5, vertex1_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex1_color:
    beq      cr7, vertex1_texcoord
    stw      r30, 0(r27)
vertex1_texcoord:
    beq+     cr6, vertex2
    psq_st   f23, 0(r27), 0, 0
vertex2:
    psq_st   f27, 0(r27), 0, 6
    psq_st   f26, 0(r27), 1, 6
    beq+     cr5, vertex2_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex2_color:
    beq      cr7, vertex2_texcoord
    stw      r29, 0(r27)
vertex2_texcoord:
    beq+     cr6, vertex3
    psq_st   f22, 0(r27), 0, 0
vertex3:
    psq_st   f25, 0(r27), 0, 6
    add      r16, r16, r15
    psq_st   f24, 0(r27), 1, 6
    add      r23, r23, r19
    beq+     cr5, vertex3_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
    add      r26, r26, r22
vertex3_color:
    beq      cr7, vertex3_texcoord
    stw      r28, 0(r27)
    add      r24, r24, r20
vertex3_texcoord:
    beq+     cr6, next_particle
    psq_st   f21, 0(r27), 0, 0
    add      r25, r25, r21
next_particle:
    bdnz     particle
    /* restore the registers */
done:
    lwz      r0, 0x8(r1)
    mtcrf    255, r0
    lwz      r14, 0x10(r1)
    lwz      r15, 0x14(r1)
    lwz      r16, 0x18(r1)
    lwz      r17, 0x1c(r1)
    lwz      r18, 0x20(r1)
    lwz      r19, 0x24(r1)
    lwz      r20, 0x28(r1)
    lwz      r21, 0x2c(r1)
    lwz      r22, 0x30(r1)
    lwz      r23, 0x34(r1)
    lwz      r24, 0x38(r1)
    lwz      r25, 0x3c(r1)
    lwz      r26, 0x40(r1)
    lwz      r27, 0x44(r1)
    lwz      r28, 0x48(r1)
    lwz      r29, 0x4c(r1)
    lwz      r30, 0x50(r1)
    lwz      r31, 0x54(r1)
    lfd      f14, 0x58(r1)
    lfd      f15, 0x60(r1)
    lfd      f16, 0x68(r1)
    lfd      f17, 0x70(r1)
    lfd      f18, 0x78(r1)
    lfd      f19, 0x80(r1)
    lfd      f20, 0x88(r1)
    lfd      f21, 0x90(r1)
    lfd      f22, 0x98(r1)
    lfd      f23, 0xa0(r1)
    lfd      f24, 0xa8(r1)
    lfd      f25, 0xb0(r1)
    lfd      f26, 0xb8(r1)
    lfd      f27, 0xc0(r1)
    lfd      f28, 0xc8(r1)
    lfd      f29, 0xd0(r1)
    lfd      f30, 0xd8(r1)
    lfd      f31, 0xe0(r1)
    addi     r1, r1, 0xe8
    blr
#endif // clang-format on
}

/* Writes the particles of a tank with a rotation and a size per particle: the camera axes are
 * rotated by each particle's angle and scaled by half its size. */
asm void fn_8044B210(register RwUInt16 count, register RwUInt32 platFlags,
                     register RpPTankData *ptankGlobal) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the condition register and the non-volatile registers */
    stwu     r1, -0xe8(r1)
    mfcr     r0
    stw      r0, 0x8(r1)
    stw      r14, 0x10(r1)
    stw      r15, 0x14(r1)
    stw      r16, 0x18(r1)
    stw      r17, 0x1c(r1)
    stw      r18, 0x20(r1)
    stw      r19, 0x24(r1)
    stw      r20, 0x28(r1)
    stw      r21, 0x2c(r1)
    stw      r22, 0x30(r1)
    stw      r23, 0x34(r1)
    stw      r24, 0x38(r1)
    stw      r25, 0x3c(r1)
    stw      r26, 0x40(r1)
    stw      r27, 0x44(r1)
    stw      r28, 0x48(r1)
    stw      r29, 0x4c(r1)
    stw      r30, 0x50(r1)
    stw      r31, 0x54(r1)
    stfd     f14, 0x58(r1)
    stfd     f15, 0x60(r1)
    stfd     f16, 0x68(r1)
    stfd     f17, 0x70(r1)
    stfd     f18, 0x78(r1)
    stfd     f19, 0x80(r1)
    stfd     f20, 0x88(r1)
    stfd     f21, 0x90(r1)
    stfd     f22, 0x98(r1)
    stfd     f23, 0xa0(r1)
    stfd     f24, 0xa8(r1)
    stfd     f25, 0xb0(r1)
    stfd     f26, 0xb8(r1)
    stfd     f27, 0xc0(r1)
    stfd     f28, 0xc8(r1)
    stfd     f29, 0xd0(r1)
    stfd     f30, 0xd8(r1)
    stfd     f31, 0xe0(r1)
    /* the stream selections of platFlags, tested once for the whole loop */
    rlwinm   r8, platFlags, 0, 9, 9
    rlwinm   r9, platFlags, 0, 8, 8
    rlwinm   r10, platFlags, 0, 13, 13
    rlwinm   r11, platFlags, 0, 13, 15
    rlwinm   r12, platFlags, 0, 15, 15
    rlwinm   r14, platFlags, 0, 14, 14
    rlwinm   r15, platFlags, 0, 8, 11
    rlwinm   r16, platFlags, 0, 11, 11
    rlwinm   r17, platFlags, 0, 10, 10
    rlwinm   r18, platFlags, 0, 7, 7
    cmplwi   cr7, r11, 0
    cmplwi   cr6, r15, 0
    cmplwi   cr5, r18, 0
    cmplwi   cr4, r16, 0
    cmplwi   cr3, r17, 0
    cmplwi   cr2, r12, 0
    cmplwi   cr1, r14, 0
    /* the positions, sizes and angles, and the sine constants in f10-f13 */
    lwz      r12, lbl_805EDD78
    lwz      r11, RpPTankData.cameraAxes(ptankGlobal)
    lwz      r14, lbl_805EDD7C
    lwz      r23, RpPTankData.dataPosition.data(ptankGlobal)
    lwz      r19, RpPTankData.dataPosition.stride(ptankGlobal)
    lwz      r18, RpPTankData.dataSize.data(ptankGlobal)
    lwz      r17, RpPTankData.dataSize.stride(ptankGlobal)
    lwz      r16, RpPTankData.dataRotate.data(ptankGlobal)
    lwz      r15, RpPTankData.dataRotate.stride(ptankGlobal)
    psq_l    f10, 0(r14), 0, 0
    psq_l    f11, 0x8(r14), 0, 0
    psq_l    f12, 0x10(r14), 0, 0
    psq_l    f13, 0x18(r14), 0, 0
    /* colors: one per particle, one per corner, or the constant four */
    beq      cr7, texcoords
    beq      cr2, colors_per_vertex
    lwz      r24, RpPTankData.dataColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataColor.stride(ptankGlobal)
    b        texcoords
colors_per_vertex:
    beq+     cr1, colors_constant
    lwz      r24, RpPTankData.dataVtxColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataVtxColor.stride(ptankGlobal)
    b        texcoords
colors_constant:
    cmplwi   r10, 0
    beq+     texcoords
    lwz      r31, PTANKCONSTANT(cVtxColor[0])(ptankGlobal)
    lwz      r30, PTANKCONSTANT(cVtxColor[1])(ptankGlobal)
    lwz      r29, PTANKCONSTANT(cVtxColor[2])(ptankGlobal)
    lwz      r28, PTANKCONSTANT(cVtxColor[3])(ptankGlobal)
    /* texture coordinates: two or four per particle, or the constant two or four */
texcoords:
    beq+     cr6, normals
    beq+     cr4, texcoords_per_vertex
    lwz      r25, RpPTankData.dataVtx2TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx2TexCoords.stride(ptankGlobal)
    b        normals
texcoords_per_vertex:
    beq+     cr3, texcoords_constant2
    lwz      r25, RpPTankData.dataVtx4TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx4TexCoords.stride(ptankGlobal)
    b        normals
texcoords_constant2:
    cmplwi   r8, 0
    beq+     texcoords_constant4
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        normals
texcoords_constant4:
    cmplwi   r9, 0
    beq      normals
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f22, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[2])(ptankGlobal), 0, 0
    psq_l    f20, PTANKCONSTANT(cVtxUV[3])(ptankGlobal), 0, 0
    /* normals: one per particle */
normals:
    beq+     cr5, setup_done
    lwz      r26, RpPTankData.dataNormals.data(ptankGlobal)
    lwz      r22, RpPTankData.dataNormals.stride(ptankGlobal)
    /* the vertex FIFO; nothing to do without particles */
setup_done:
    cmpwi    count, 0
    lwz      r27, lbl_805FB0A0
    mtctr    count
    ble      done
particle:
    /* the sine and cosine of the particle's angle a: f17 = (a, a - pi/2), wrapped into [-pi, pi] by
     * -/+ pi, then 4 / pi^2 * x * (pi - |x|) */
    psq_l    f17, 0(r16), 1, 0
    ps_neg   f14, f10
    psq_l    f1, 0(r12), 1, 0
    ps_madds0 f17, f12, f17, f11
    psq_l    f9, PTANKCAMERAAXIS(0, x)(r11), 0, 0
    psq_l    f8, PTANKCAMERAAXIS(0, z)(r11), 1, 0
    ps_sel   f16, f17, f14, f10
    psq_l    f14, RwV2d.x(r18), 0, 0
    psq_l    f7, PTANKCAMERAAXIS(1, x)(r11), 0, 0
    ps_add   f17, f17, f16
    ps_muls0 f14, f14, f1
    psq_l    f6, PTANKCAMERAAXIS(1, z)(r11), 1, 0
    ps_nabs  f16, f17
    ps_muls0 f17, f17, f13
    nop
    ps_madds1 f16, f12, f10, f16
    psq_l    f19, RwV3d.x(r23), 0, 0
    psq_l    f18, RwV3d.z(r23), 1, 0
    ps_mul   f17, f16, f17
    /* the rotated axes r = axes[0] * cos a - axes[1] * sin a and u = axes[0] * sin a + axes[1] *
     * cos a, scaled by half the size (f14); the corners are pos - (r + u), pos + (r - u), pos + (r
     * + u) and pos - (r - u) */
    ps_neg   f5, f7
    ps_neg   f4, f6
    ps_muls1 f31, f9, f17
    ps_muls1 f30, f8, f17
    ps_muls0 f29, f9, f17
    ps_muls0 f28, f8, f17
    ps_madds0 f31, f5, f17, f31
    ps_madds0 f30, f4, f17, f30
    ps_madds1 f29, f7, f17, f29
    ps_madds1 f28, f6, f17, f28
    ps_muls0 f31, f31, f14
    ps_muls0 f30, f30, f14
    ps_muls1 f29, f29, f14
    ps_muls1 f28, f28, f14
    nop
    ps_add   f15, f31, f29
    ps_add   f14, f30, f28
    ps_sub   f17, f31, f29
    ps_sub   f16, f30, f28
    ps_sub   f31, f19, f15
    ps_sub   f30, f18, f14
    ps_add   f29, f19, f17
    ps_add   f28, f18, f16
    ps_add   f27, f19, f15
    ps_add   f26, f18, f14
    ps_sub   f25, f19, f17
    ps_sub   f24, f18, f16
    /* this particle's colors, texture coordinates and normal */
    beq      cr7, load_texcoords
    beq      cr2, load_colors_per_vertex
    lwz      r31, 0(r24)
    lwz      r30, 0(r24)
    lwz      r29, 0(r24)
    lwz      r28, 0(r24)
    b        load_texcoords
load_colors_per_vertex:
    beq+     cr1, load_texcoords
    lwz      r31, 0(r24)
    lwz      r30, 0x4(r24)
    lwz      r29, 0x8(r24)
    lwz      r28, 0xc(r24)
load_texcoords:
    beq+     cr6, load_normal
    beq+     cr4, load_texcoords_per_vertex
    psq_l    f23, 0(r25), 0, 0
    psq_l    f21, 0x8(r25), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        vertex0
load_texcoords_per_vertex:
    beq+     cr3, load_normal
    psq_l    f23, 0(r25), 0, 0
    psq_l    f22, 0x8(r25), 0, 0
    psq_l    f21, 0x10(r25), 0, 0
    psq_l    f20, 0x18(r25), 0, 0
load_normal:
    beq+     cr5, vertex0
    psq_l    f19, 0(r26), 0, 0
    psq_l    f18, 0x8(r26), 1, 0
    /* write the four corners: position, then normal, color and texture coordinate if present */
vertex0:
    psq_st   f31, 0(r27), 0, 6
    psq_st   f30, 0(r27), 1, 6
    beq+     cr5, vertex0_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex0_color:
    beq      cr7, vertex0_texcoord
    stw      r31, 0(r27)
vertex0_texcoord:
    beq+     cr6, vertex1
    psq_st   f20, 0(r27), 0, 0
vertex1:
    psq_st   f29, 0(r27), 0, 6
    psq_st   f28, 0(r27), 1, 6
    beq+     cr5, vertex1_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex1_color:
    beq      cr7, vertex1_texcoord
    stw      r30, 0(r27)
vertex1_texcoord:
    beq+     cr6, vertex2
    psq_st   f23, 0(r27), 0, 0
vertex2:
    psq_st   f27, 0(r27), 0, 6
    psq_st   f26, 0(r27), 1, 6
    beq+     cr5, vertex2_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex2_color:
    beq      cr7, vertex2_texcoord
    stw      r29, 0(r27)
vertex2_texcoord:
    beq+     cr6, vertex3
    psq_st   f22, 0(r27), 0, 0
vertex3:
    add      r18, r18, r17
    psq_st   f25, 0(r27), 0, 6
    add      r16, r16, r15
    psq_st   f24, 0(r27), 1, 6
    add      r23, r23, r19
    beq+     cr5, vertex3_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
    add      r26, r26, r22
vertex3_color:
    beq      cr7, vertex3_texcoord
    stw      r28, 0(r27)
    add      r24, r24, r20
vertex3_texcoord:
    beq+     cr6, next_particle
    psq_st   f21, 0(r27), 0, 0
    add      r25, r25, r21
next_particle:
    bdnz     particle
    /* restore the registers */
done:
    lwz      r0, 0x8(r1)
    mtcrf    255, r0
    lwz      r14, 0x10(r1)
    lwz      r15, 0x14(r1)
    lwz      r16, 0x18(r1)
    lwz      r17, 0x1c(r1)
    lwz      r18, 0x20(r1)
    lwz      r19, 0x24(r1)
    lwz      r20, 0x28(r1)
    lwz      r21, 0x2c(r1)
    lwz      r22, 0x30(r1)
    lwz      r23, 0x34(r1)
    lwz      r24, 0x38(r1)
    lwz      r25, 0x3c(r1)
    lwz      r26, 0x40(r1)
    lwz      r27, 0x44(r1)
    lwz      r28, 0x48(r1)
    lwz      r29, 0x4c(r1)
    lwz      r30, 0x50(r1)
    lwz      r31, 0x54(r1)
    lfd      f14, 0x58(r1)
    lfd      f15, 0x60(r1)
    lfd      f16, 0x68(r1)
    lfd      f17, 0x70(r1)
    lfd      f18, 0x78(r1)
    lfd      f19, 0x80(r1)
    lfd      f20, 0x88(r1)
    lfd      f21, 0x90(r1)
    lfd      f22, 0x98(r1)
    lfd      f23, 0xa0(r1)
    lfd      f24, 0xa8(r1)
    lfd      f25, 0xb0(r1)
    lfd      f26, 0xb8(r1)
    lfd      f27, 0xc0(r1)
    lfd      f28, 0xc8(r1)
    lfd      f29, 0xd0(r1)
    lfd      f30, 0xd8(r1)
    lfd      f31, 0xe0(r1)
    addi     r1, r1, 0xe8
    blr
#endif // clang-format on
}

/* Writes the particles of a tank with a rotation and a size per particle and a center: as
 * fn_8044B210, with each quad moved back by the tank's center along its rotated axes. */
asm void fn_8044B654(register RwUInt16 count, register RwUInt32 platFlags,
                     register RpPTankData *ptankGlobal) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    /* save the condition register and the non-volatile registers */
    stwu     r1, -0xe8(r1)
    mfcr     r0
    stw      r0, 0x8(r1)
    stw      r14, 0x10(r1)
    stw      r15, 0x14(r1)
    stw      r16, 0x18(r1)
    stw      r17, 0x1c(r1)
    stw      r18, 0x20(r1)
    stw      r19, 0x24(r1)
    stw      r20, 0x28(r1)
    stw      r21, 0x2c(r1)
    stw      r22, 0x30(r1)
    stw      r23, 0x34(r1)
    stw      r24, 0x38(r1)
    stw      r25, 0x3c(r1)
    stw      r26, 0x40(r1)
    stw      r27, 0x44(r1)
    stw      r28, 0x48(r1)
    stw      r29, 0x4c(r1)
    stw      r30, 0x50(r1)
    stw      r31, 0x54(r1)
    stfd     f14, 0x58(r1)
    stfd     f15, 0x60(r1)
    stfd     f16, 0x68(r1)
    stfd     f17, 0x70(r1)
    stfd     f18, 0x78(r1)
    stfd     f19, 0x80(r1)
    stfd     f20, 0x88(r1)
    stfd     f21, 0x90(r1)
    stfd     f22, 0x98(r1)
    stfd     f23, 0xa0(r1)
    stfd     f24, 0xa8(r1)
    stfd     f25, 0xb0(r1)
    stfd     f26, 0xb8(r1)
    stfd     f27, 0xc0(r1)
    stfd     f28, 0xc8(r1)
    stfd     f29, 0xd0(r1)
    stfd     f30, 0xd8(r1)
    stfd     f31, 0xe0(r1)
    /* the stream selections of platFlags, tested once for the whole loop */
    rlwinm   r8, platFlags, 0, 9, 9
    rlwinm   r9, platFlags, 0, 8, 8
    rlwinm   r10, platFlags, 0, 13, 13
    rlwinm   r11, platFlags, 0, 13, 15
    rlwinm   r12, platFlags, 0, 15, 15
    rlwinm   r14, platFlags, 0, 14, 14
    rlwinm   r15, platFlags, 0, 8, 11
    rlwinm   r16, platFlags, 0, 11, 11
    rlwinm   r17, platFlags, 0, 10, 10
    rlwinm   r18, platFlags, 0, 7, 7
    cmplwi   cr7, r11, 0
    cmplwi   cr6, r15, 0
    cmplwi   cr5, r18, 0
    cmplwi   cr4, r16, 0
    cmplwi   cr3, r17, 0
    cmplwi   cr2, r12, 0
    cmplwi   cr1, r14, 0
    /* the positions, sizes and angles, f0 = -center, and the sine constants in f10-f13 */
    lwz      r12, lbl_805EDD78
    lwz      r11, RpPTankData.cameraAxes(ptankGlobal)
    lwz      r14, lbl_805EDD7C
    lwz      r23, RpPTankData.dataPosition.data(ptankGlobal)
    lwz      r19, RpPTankData.dataPosition.stride(ptankGlobal)
    lwz      r18, RpPTankData.dataSize.data(ptankGlobal)
    lwz      r17, RpPTankData.dataSize.stride(ptankGlobal)
    lwz      r16, RpPTankData.dataRotate.data(ptankGlobal)
    lwz      r15, RpPTankData.dataRotate.stride(ptankGlobal)
    psq_l    f0, PTANKCONSTANT(cCenter)(ptankGlobal), 0, 0
    psq_l    f10, 0(r14), 0, 0
    psq_l    f11, 0x8(r14), 0, 0
    psq_l    f12, 0x10(r14), 0, 0
    psq_l    f13, 0x18(r14), 0, 0
    ps_neg   f0, f0
    /* colors: one per particle, one per corner, or the constant four */
    beq      cr7, texcoords
    beq      cr2, colors_per_vertex
    lwz      r24, RpPTankData.dataColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataColor.stride(ptankGlobal)
    b        texcoords
colors_per_vertex:
    beq+     cr1, colors_constant
    lwz      r24, RpPTankData.dataVtxColor.data(ptankGlobal)
    lwz      r20, RpPTankData.dataVtxColor.stride(ptankGlobal)
    b        texcoords
colors_constant:
    cmplwi   r10, 0
    beq+     texcoords
    lwz      r31, PTANKCONSTANT(cVtxColor[0])(ptankGlobal)
    lwz      r30, PTANKCONSTANT(cVtxColor[1])(ptankGlobal)
    lwz      r29, PTANKCONSTANT(cVtxColor[2])(ptankGlobal)
    lwz      r28, PTANKCONSTANT(cVtxColor[3])(ptankGlobal)
    /* texture coordinates: two or four per particle, or the constant two or four */
texcoords:
    beq+     cr6, normals
    beq+     cr4, texcoords_per_vertex
    lwz      r25, RpPTankData.dataVtx2TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx2TexCoords.stride(ptankGlobal)
    b        normals
texcoords_per_vertex:
    beq+     cr3, texcoords_constant2
    lwz      r25, RpPTankData.dataVtx4TexCoords.data(ptankGlobal)
    lwz      r21, RpPTankData.dataVtx4TexCoords.stride(ptankGlobal)
    b        normals
texcoords_constant2:
    cmplwi   r8, 0
    beq+     texcoords_constant4
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        normals
texcoords_constant4:
    cmplwi   r9, 0
    beq      normals
    psq_l    f23, PTANKCONSTANT(cVtxUV[0])(ptankGlobal), 0, 0
    psq_l    f22, PTANKCONSTANT(cVtxUV[1])(ptankGlobal), 0, 0
    psq_l    f21, PTANKCONSTANT(cVtxUV[2])(ptankGlobal), 0, 0
    psq_l    f20, PTANKCONSTANT(cVtxUV[3])(ptankGlobal), 0, 0
    /* normals: one per particle */
normals:
    beq+     cr5, setup_done
    lwz      r26, RpPTankData.dataNormals.data(ptankGlobal)
    lwz      r22, RpPTankData.dataNormals.stride(ptankGlobal)
    /* the vertex FIFO; nothing to do without particles */
setup_done:
    cmpwi    count, 0
    lwz      r27, lbl_805FB0A0
    mtctr    count
    ble      done
particle:
    /* the sine and cosine of the particle's angle a: f17 = (a, a - pi/2), wrapped into [-pi, pi] by
     * -/+ pi, then 4 / pi^2 * x * (pi - |x|) */
    psq_l    f17, 0(r16), 1, 0
    ps_neg   f14, f10
    psq_l    f1, 0(r12), 1, 0
    ps_madds0 f17, f12, f17, f11
    psq_l    f9, PTANKCAMERAAXIS(0, x)(r11), 0, 0
    psq_l    f8, PTANKCAMERAAXIS(0, z)(r11), 1, 0
    ps_sel   f16, f17, f14, f10
    psq_l    f14, RwV2d.x(r18), 0, 0
    psq_l    f7, PTANKCAMERAAXIS(1, x)(r11), 0, 0
    ps_add   f17, f17, f16
    ps_muls0 f14, f14, f1
    psq_l    f6, PTANKCAMERAAXIS(1, z)(r11), 1, 0
    ps_nabs  f16, f17
    ps_muls0 f17, f17, f13
    nop
    ps_madds1 f16, f12, f10, f16
    psq_l    f19, RwV3d.x(r23), 0, 0
    psq_l    f18, RwV3d.z(r23), 1, 0
    ps_mul   f17, f16, f17
    /* the rotated axes r = axes[0] * cos a - axes[1] * sin a and u = axes[0] * sin a + axes[1] *
     * cos a; pos -= center.x * r + center.y * u; r and u are scaled by half the size (f14); the
     * corners are pos - (r + u), pos + (r - u), pos + (r + u) and pos - (r - u) */
    ps_neg   f5, f7
    ps_neg   f4, f6
    ps_muls1 f31, f9, f17
    ps_muls1 f30, f8, f17
    ps_muls0 f29, f9, f17
    ps_muls0 f28, f8, f17
    ps_madds0 f31, f5, f17, f31
    ps_madds0 f30, f4, f17, f30
    ps_madds1 f29, f7, f17, f29
    ps_madds1 f28, f6, f17, f28
    ps_muls0 f3, f31, f0
    ps_muls0 f2, f30, f0
    ps_madds1 f3, f29, f0, f3
    ps_madds1 f2, f28, f0, f2
    ps_muls0 f31, f31, f14
    ps_muls0 f30, f30, f14
    ps_muls1 f29, f29, f14
    ps_muls1 f28, f28, f14
    ps_add   f19, f19, f3
    ps_add   f18, f18, f2
    ps_add   f15, f31, f29
    ps_add   f14, f30, f28
    ps_sub   f17, f31, f29
    ps_sub   f16, f30, f28
    ps_sub   f31, f19, f15
    ps_sub   f30, f18, f14
    ps_add   f29, f19, f17
    ps_add   f28, f18, f16
    ps_add   f27, f19, f15
    ps_add   f26, f18, f14
    ps_sub   f25, f19, f17
    ps_sub   f24, f18, f16
    /* this particle's colors, texture coordinates and normal */
    beq      cr7, load_texcoords
    beq      cr2, load_colors_per_vertex
    lwz      r31, 0(r24)
    lwz      r30, 0(r24)
    lwz      r29, 0(r24)
    lwz      r28, 0(r24)
    b        load_texcoords
load_colors_per_vertex:
    beq+     cr1, load_texcoords
    lwz      r31, 0(r24)
    lwz      r30, 0x4(r24)
    lwz      r29, 0x8(r24)
    lwz      r28, 0xc(r24)
load_texcoords:
    beq+     cr6, load_normal
    beq+     cr4, load_texcoords_per_vertex
    psq_l    f23, 0(r25), 0, 0
    psq_l    f21, 0x8(r25), 0, 0
    nop
    nop
    ps_merge01 f22, f23, f21
    ps_merge01 f20, f21, f23
    b        vertex0
load_texcoords_per_vertex:
    beq+     cr3, load_normal
    psq_l    f23, 0(r25), 0, 0
    psq_l    f22, 0x8(r25), 0, 0
    psq_l    f21, 0x10(r25), 0, 0
    psq_l    f20, 0x18(r25), 0, 0
load_normal:
    beq+     cr5, vertex0
    psq_l    f19, 0(r26), 0, 0
    psq_l    f18, 0x8(r26), 1, 0
    /* write the four corners: position, then normal, color and texture coordinate if present */
vertex0:
    psq_st   f31, 0(r27), 0, 6
    psq_st   f30, 0(r27), 1, 6
    beq+     cr5, vertex0_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex0_color:
    beq      cr7, vertex0_texcoord
    stw      r31, 0(r27)
vertex0_texcoord:
    beq+     cr6, vertex1
    psq_st   f20, 0(r27), 0, 0
vertex1:
    psq_st   f29, 0(r27), 0, 6
    psq_st   f28, 0(r27), 1, 6
    beq+     cr5, vertex1_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex1_color:
    beq      cr7, vertex1_texcoord
    stw      r30, 0(r27)
vertex1_texcoord:
    beq+     cr6, vertex2
    psq_st   f23, 0(r27), 0, 0
vertex2:
    psq_st   f27, 0(r27), 0, 6
    psq_st   f26, 0(r27), 1, 6
    beq+     cr5, vertex2_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
vertex2_color:
    beq      cr7, vertex2_texcoord
    stw      r29, 0(r27)
vertex2_texcoord:
    beq+     cr6, vertex3
    psq_st   f22, 0(r27), 0, 0
vertex3:
    add      r18, r18, r17
    psq_st   f25, 0(r27), 0, 6
    add      r16, r16, r15
    psq_st   f24, 0(r27), 1, 6
    add      r23, r23, r19
    beq+     cr5, vertex3_color
    psq_st   f19, 0(r27), 0, 7
    psq_st   f18, 0(r27), 1, 7
    add      r26, r26, r22
vertex3_color:
    beq      cr7, vertex3_texcoord
    stw      r28, 0(r27)
    add      r24, r24, r20
vertex3_texcoord:
    beq+     cr6, next_particle
    psq_st   f21, 0(r27), 0, 0
    add      r25, r25, r21
next_particle:
    bdnz     particle
    /* restore the registers */
done:
    lwz      r0, 0x8(r1)
    mtcrf    255, r0
    lwz      r14, 0x10(r1)
    lwz      r15, 0x14(r1)
    lwz      r16, 0x18(r1)
    lwz      r17, 0x1c(r1)
    lwz      r18, 0x20(r1)
    lwz      r19, 0x24(r1)
    lwz      r20, 0x28(r1)
    lwz      r21, 0x2c(r1)
    lwz      r22, 0x30(r1)
    lwz      r23, 0x34(r1)
    lwz      r24, 0x38(r1)
    lwz      r25, 0x3c(r1)
    lwz      r26, 0x40(r1)
    lwz      r27, 0x44(r1)
    lwz      r28, 0x48(r1)
    lwz      r29, 0x4c(r1)
    lwz      r30, 0x50(r1)
    lwz      r31, 0x54(r1)
    lfd      f14, 0x58(r1)
    lfd      f15, 0x60(r1)
    lfd      f16, 0x68(r1)
    lfd      f17, 0x70(r1)
    lfd      f18, 0x78(r1)
    lfd      f19, 0x80(r1)
    lfd      f20, 0x88(r1)
    lfd      f21, 0x90(r1)
    lfd      f22, 0x98(r1)
    lfd      f23, 0xa0(r1)
    lfd      f24, 0xa8(r1)
    lfd      f25, 0xb0(r1)
    lfd      f26, 0xb8(r1)
    lfd      f27, 0xc0(r1)
    lfd      f28, 0xc8(r1)
    lfd      f29, 0xd0(r1)
    lfd      f30, 0xd8(r1)
    lfd      f31, 0xe0(r1)
    addi     r1, r1, 0xe8
    blr
#endif // clang-format on
}

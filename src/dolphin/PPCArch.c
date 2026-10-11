#include <dolphin.h>
#include <dolphin/base/PPCArch.h>

// clang-format off
asm u32 PPCMfmsr() {
    nofralloc
    mfmsr r3
    blr    
}
// clang-format on

// clang-format off
asm void PPCMtmsr(register u32 newMSR) {
    nofralloc
    mtmsr newMSR
    blr
}
// clang-format on

// clang-format off
asm u32 PPCOrMsr(register u32 value) {
    nofralloc
    mfmsr r4
    or value, r4, value
    blr
}
// clang-format on

// clang-format off
asm u32 PPCAndMsr(register u32 value) {
    nofralloc
    mfmsr r4
    and value, r4, value
    blr
}
// clang-format on

// clang-format off
asm u32 PPCAndCMsr(register u32 value) {
    nofralloc
    mfmsr r4
    andc value, r4, value
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfhid0() {
    nofralloc
    mfspr r3, HID0
    blr
}
// clang-format on

// clang-format off
asm void PPCMthid0(register u32 newHID0) {
    nofralloc
    mtspr HID0, newHID0
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfhid1() {
    nofralloc
    mfspr r3, HID1
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfl2cr() {
    nofralloc
    mfspr r3, L2CR
    blr
}
// clang-format on

// clang-format off
asm void PPCMtl2cr(register u32 newL2cr) {
    nofralloc
    mtspr L2CR, newL2cr
    blr
}
// clang-format on

// clang-format off
asm void PPCMtdec(register u32 newDec) {
    nofralloc
    mtdec newDec
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfdec() {
    nofralloc
    mfdec r3
    blr
}
// clang-format on

// clang-format off
asm void PPCSync() {
    nofralloc
    sc
    blr
}
// clang-format on

// clang-format off
asm void PPCEieio() {
    nofralloc
    mfmsr r5
    rlwinm r6, r5, 0, 17, 15
    mtmsr r6
    mfspr r3, HID0
    ori r4, r3, 0x8
    mtspr HID0, r4
    isync
    eieio
    isync
    mtspr HID0, r3
    mtmsr r5
    isync
    blr
}
// clang-format on

// clang-format off
asm void PPCHalt() {
    nofralloc
    sync
loop:
    nop
    li r3, 0
    nop
    b loop
}
// clang-format on

// clang-format off
asm u32 PPCMfmmcr0() {
    nofralloc
    mfspr r3, MMCR0
    blr
}
// clang-format on

// clang-format off
asm void PPCMtmmcr0(register u32 newMmcr0) {
    nofralloc
    mtspr MMCR0, newMmcr0
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfmmcr1() {
    nofralloc
    mfspr r3, MMCR1
    blr
}
// clang-format on

// clang-format off
asm void PPCMtmmcr1(register u32 newMmcr1) {
    nofralloc
    mtspr MMCR1, newMmcr1
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfpmc1() {
    nofralloc
    mfspr r3, PMC1
    blr
}
// clang-format on

// clang-format off
asm void PPCMtpmc1(register u32 newPmc1) {
    nofralloc
    mtspr PMC1, newPmc1
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfpmc2() {
    nofralloc
    mfspr r3, PMC2
    blr
}
// clang-format on

// clang-format off
asm void PPCMtpmc2(register u32 newPmc2) {
    nofralloc
    mtspr PMC2, newPmc2
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfpmc3() {
    nofralloc
    mfspr r3, PMC3
    blr
}
// clang-format on

// clang-format off
asm void PPCMtpmc3(register u32 newPmc3) {
    nofralloc
    mtspr PMC3, newPmc3
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfpmc4() {
    nofralloc
    mfspr r3, PMC4
    blr
}
// clang-format on

// clang-format off
asm void PPCMtpmc4(register u32 newPmc4) {
    nofralloc
    mtspr PMC4, newPmc4
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfsia() {
    nofralloc
    mfspr r3, SIA
    blr
}
// clang-format on

// clang-format off
asm void PPCMtsia(register u32 newSia) {
    nofralloc
    mtspr SIA, newSia
    blr
} // clang-format on

u32 PPCMffpscr() {
    union FpscrUnion m;

    // clang-format off
    asm {
        mffs fp31
        stfd fp31, m.f;
    }
    // clang-format on

    return m.u.fpscr;
}

void PPCMtfpscr(register u32 newFPSCR) {
    union FpscrUnion m;

    // clang-format off
    asm {
        li    r4, 0
        stw   r4, m.u.fpscr_pad;
        stw   newFPSCR, m.u.fpscr
        lfd   fp31, m.f
        mtfsf 0xff, fp31
    }
    // clang-format on
}

// clang-format off
asm u32 PPCMfhid2() {
    nofralloc
    mfspr r3, HID2
    blr
}
// clang-format on

// clang-format off
asm void PPCMthid2(register u32 newhid2) {
    nofralloc
    mtspr HID2, newhid2
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfwpar() {
    nofralloc
    sync
    mfspr r3, WPAR
    blr
}
// clang-format on

// clang-format off
asm void PPCMtwpar(register u32 newwpar) {
    nofralloc
    mtspr WPAR, newwpar
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfdmaU() {
    nofralloc
    mfspr r3, DMA_U
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfdmaL() {
    nofralloc
    mfspr r3, DMA_L
    blr
}
// clang-format on

// clang-format off
asm void PPCMtdmaU(register u32 newdmau) {
    nofralloc
    mtspr DMA_U, newdmau
    blr
}
// clang-format on

// clang-format off
asm void PPCMtdmaL(register u32 newdmal) {
    nofralloc
    mtspr DMA_L, newdmal
    blr
}
// clang-format on

// clang-format off
asm u32 PPCMfpvr() {
    nofralloc
    mfspr r3, PVR
    blr
}
// clang-format on

void PPCEnableSpeculation(void) {
    PPCMthid0(PPCMfhid0() & ~HID0_SPD);
}

void PPCDisableSpeculation(void) {
    PPCMthid0(PPCMfhid0() | HID0_SPD);
}

// clang-format off
asm void PPCSetFpIEEEMode() {
	nofralloc
	mtfsb0 29
	blr
}
// clang-format on

// clang-format off
asm void PPCSetFpNonIEEEMode() {
	nofralloc
	mtfsb1 29
	blr
}
// clang-format on

#ifndef _PPC_REG_H
#define _PPC_REG_H

#include <dolphin/types.h>
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

typedef struct Default_PPC {
    /* 0x0 */ u32 GPR[32];
    /* 0x80 */ u32 PC;
    /* 0x84 */ u32 LR;
    /* 0x88 */ u32 CR;
    /* 0x8C */ u32 CTR;
    /* 0x90 */ u32 XER;
} Default_PPC;
CHECK_SIZEOF(Default_PPC, 0x94);

typedef struct Float_PPC {
    /* 0x0 */ u64 FPR[32];
    /* 0x100 */ u64 FPSCR;
    /* 0x108 */ u64 FPECR;
} Float_PPC;
CHECK_SIZEOF(Float_PPC, 0x110);

typedef struct Extended1_PPC_6xx_7xx {
    /* 0x0 */ u32 SR[16];
    /* 0x40 */ u32 TBL;
    /* 0x44 */ u32 TBU;
    /* 0x48 */ u32 HID0_;
    /* 0x4C */ u32 HID1;
    /* 0x50 */ u32 MSR;
    /* 0x54 */ u32 PVR;
    /* 0x58 */ u32 IBAT0U;
    /* 0x5C */ u32 IBAT0L;
    /* 0x60 */ u32 IBAT1U;
    /* 0x64 */ u32 IBAT1L;
    /* 0x68 */ u32 IBAT2U;
    /* 0x6C */ u32 IBAT2L;
    /* 0x70 */ u32 IBAT3U;
    /* 0x74 */ u32 IBAT3L;
    /* 0x78 */ u32 DBAT0U;
    /* 0x7C */ u32 DBAT0L;
    /* 0x80 */ u32 DBAT1U;
    /* 0x84 */ u32 DBAT1L;
    /* 0x88 */ u32 DBAT2U;
    /* 0x8C */ u32 DBAT2L;
    /* 0x90 */ u32 DBAT3U_;
    /* 0x94 */ u32 DBAT3L_;
    /* 0x98 */ u32 DMISS;
    /* 0x9C */ u32 DCMP;
    /* 0xA0 */ u32 HASH1;
    /* 0xA4 */ u32 HASH2;
    /* 0xA8 */ u32 IMISS;
    /* 0xAC */ u32 ICMP;
    /* 0xB0 */ u32 RPA;
    /* 0xB4 */ u32 SDR1;
    /* 0xB8 */ u32 DAR;
    /* 0xBC */ u32 DSISR;
    /* 0xC0 */ u32 SPRG0;
    /* 0xC4 */ u32 SPRG1;
    /* 0xC8 */ u32 SPRG2;
    /* 0xCC */ u32 SPRG3;
    /* 0xD0 */ u32 DEC;
    /* 0xD4 */ u32 IABR;
    /* 0xD8 */ u32 EAR;
    /* 0xDC */ u32 DABR;
    /* 0xE0 */ u32 PMC1;
    /* 0xE4 */ u32 PMC2;
    /* 0xE8 */ u32 PMC3;
    /* 0xEC */ u32 PMC4;
    /* 0xF0 */ u32 SIA;
    /* 0xF4 */ u32 MMCR0;
    /* 0xF8 */ u32 MMCR1;
    /* 0xFC */ u32 THRM1;
    /* 0x100 */ u32 THRM2;
    /* 0x104 */ u32 THRM3;
    /* 0x108 */ u32 ICTC;
    /* 0x10C */ u32 L2CR;
    /* 0x110 */ u32 UMMCR2;
    /* 0x114 */ u32 UBAMR;
    /* 0x118 */ u32 UMMCR0;
    /* 0x11C */ u32 UPMC1;
    /* 0x120 */ u32 UPMC2;
    /* 0x124 */ u32 USIA;
    /* 0x128 */ u32 UMMCR1;
    /* 0x12C */ u32 UPMC3;
    /* 0x130 */ u32 UPMC4;
    /* 0x134 */ u32 USDA;
    /* 0x138 */ u32 MMCR2;
    /* 0x13C */ u32 BAMR;
    /* 0x140 */ u32 SDA;
    /* 0x144 */ u32 MSSCR0;
    /* 0x148 */ u32 MSSCR1;
    /* 0x14C */ u32 PIR;
    /* 0x150 */ u32 exceptionID;
    /* 0x154 */ u32 GQR[8];
    /* 0x174 */ u32 HID_G;
    /* 0x178 */ u32 WPAR;
    /* 0x17C */ u32 DMA_U_;
    /* 0x180 */ u32 DMA_L_;
} Extended1_PPC_6xx_7xx;
CHECK_SIZEOF(Extended1_PPC_6xx_7xx, 0x184);

typedef struct Extended2_PPC_6xx_7xx {
    u32 PSR[32][2];
} Extended2_PPC_6xx_7xx;
CHECK_SIZEOF(Extended2_PPC_6xx_7xx, 0x100);

typedef struct ProcessorState_PPC_6xx_7xx {
    Default_PPC Default;
    Float_PPC Float;
    Extended1_PPC_6xx_7xx Extended1;
    Extended2_PPC_6xx_7xx Extended2;
    u32 transport_handler_saved_ra;
} ProcessorState_PPC_6xx_7xx;
CHECK_SIZEOF(ProcessorState_PPC_6xx_7xx, 0x430);

typedef ProcessorState_PPC_6xx_7xx ProcessorState_PPC;

#define SPR_XER 1
#define SPR_LR 8
#define SPR_CTR 9
#define SPR_DSISR 18
#define SPR_DAR 19
#define SPR_DEC 22
#define SPR_SDR1 25
#define SPR_SRR0 26
#define SPR_SRR1 27
#define SPR_SPRG0 272
#define SPR_SPRG1 273
#define SPR_SPRG2 274
#define SPR_SPRG3 275
#define SPR_EAR 282
#define SPR_TBL 284
#define SPR_TBU 285
#define SPR_PVR 287
#define SPR_IBAT0U 528
#define SPR_IBAT0L 529
#define SPR_IBAT1U 530
#define SPR_IBAT1L 531
#define SPR_IBAT2U 532
#define SPR_IBAT2L 533
#define SPR_IBAT3U 534
#define SPR_IBAT3L 535
#define SPR_IBAT4U 560
#define SPR_IBAT4L 561
#define SPR_IBAT5U 562
#define SPR_IBAT5L 563
#define SPR_IBAT6U 564
#define SPR_IBAT6L 565
#define SPR_IBAT7U 566
#define SPR_IBAT7L 567
#define SPR_DBAT0U 536
#define SPR_DBAT0L 537
#define SPR_DBAT1U 538
#define SPR_DBAT1L 539
#define SPR_DBAT2U 540
#define SPR_DBAT2L 541
#define SPR_DBAT3U 542
#define SPR_DBAT3L 543
#define SPR_DBAT4U 568
#define SPR_DBAT4L 569
#define SPR_DBAT5U 570
#define SPR_DBAT5L 571
#define SPR_DBAT6U 572
#define SPR_DBAT6L 573
#define SPR_DBAT7U 574
#define SPR_DBAT7L 575
#define SPR_GQR0 912
#define SPR_GQR1 913
#define SPR_GQR2 914
#define SPR_GQR3 915
#define SPR_GQR4 916
#define SPR_GQR5 917
#define SPR_GQR6 918
#define SPR_GQR7 919
#define SPR_HID2 920
#define SPR_WPAR 921
#define SPR_DMA_U 922
#define SPR_DMA_L 923
#define SPR_UMMCR0 936
#define SPR_UPMC1 937
#define SPR_UPMC2 938
#define SPR_USIA 939
#define SPR_UMMCR1 940
#define SPR_UPMC3 941
#define SPR_UPMC4 942
#define SPR_USDA 943
#define SPR_MMCR0 952
#define SPR_PMC1 953
#define SPR_PMC2 954
#define SPR_SIA 955
#define SPR_MMCR1 956
#define SPR_PMC3 957
#define SPR_PMC4 958
#define SPR_SDA 959
#define SPR_HID0 1008
#define SPR_HID1 1009
#define SPR_IABR 1010
#define SPR_HID4 1011
#define SPR_DABR 1013
#define SPR_L2CR 1017
#define SPR_ICTC 1019
#define SPR_THRM1 1020
#define SPR_THRM2 1021
#define SPR_FPECR 1022

// PPC exceptions
// 0x000 is reserved
#define PPC_SystemReset 0x100
#define PPC_MachineCheck 0x200
#define PPC_DataStorage 0x300
#define PPC_InstructionStorage 0x400
#define PPC_ExternalInterrupt 0x500
#define PPC_Alignment 0x600
#define PPC_Program 0x700
#define PPC_FloatingPointUnavailable 0x800
#define PPC_Decrementer 0x900
// 0xA00-0xB00 are reserved
#define PPC_SystemCall 0xC00
#define PPC_Trace 0xD00
#define PPC_FloatingPointAssist 0xE00
#define PPC_PerformanceMonitor 0xF00
#define PPC_InstructionAddressBreakpoint 0x1300
// 0x1400-0x2F00 are reserved, but TRK uses some
#define PPC_SystemManagementInterrupt 0x1400
#define PPC_ThermalManagementInterrupt 0x1700
#define PPC_1800Exception 0x1800
#define PPC_1900Exception 0x1900
#define PPC_1A00Exception 0x1A00
#define PPC_1B00Exception 0x1B00
#define PPC_1C00Exception 0x1C00
#define PPC_1D00Exception 0x1D00
#define PPC_1E00Exception 0x1E00
#define PPC_1F00Exception 0x1F00
#define PPC_2000Exception 0x2000

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif

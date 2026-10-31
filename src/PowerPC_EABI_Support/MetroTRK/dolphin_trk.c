#include "MetroTRK/trk.h"
#include <dolphin/ar.h>
#include <dolphin/os.h>

#define EXCEPTIONMASK_ADDR 0x80000044

static u32 lc_base;

static u32 TRK_ISR_OFFSETS[15] = {PPC_SystemReset,
                                  PPC_MachineCheck,
                                  PPC_DataStorage,
                                  PPC_InstructionStorage,
                                  PPC_ExternalInterrupt,
                                  PPC_Alignment,
                                  PPC_Program,
                                  PPC_FloatingPointUnavailable,
                                  PPC_Decrementer,
                                  PPC_SystemCall,
                                  PPC_Trace,
                                  PPC_PerformanceMonitor,
                                  PPC_InstructionAddressBreakpoint,
                                  PPC_SystemManagementInterrupt,
                                  PPC_ThermalManagementInterrupt};

__declspec(section ".init") void __TRK_reset(void) {
    OSResetSystem(FALSE, 0, FALSE);
}

void EnableMetroTRKInterrupts(void) {
    EnableEXI2Interrupts();
}

u32 TRKTargetTranslate(u32 val) {
    if (val >= lc_base && val < lc_base + 0x4000) {
        if (gTRKCPUState.Extended1.DBAT3U_ & 3)
            return val;
    }

    if (val >= 0x7E000000 && val <= 0x80000000) {
        return val;
    }

    return (val & 0x3FFFFFFF) | 0x80000000;
}

void TRK_copy_vector(u32 offset) {
    u32 dest = TRKTargetTranslate(offset);
    TRK_memcpy((void *)dest, (void *)(gTRKInterruptVectorTable + offset), 0x100);
    TRK_flush_cache(dest, 0x100);
}

void __TRK_copy_vectors(void) {
    u32 exception_mask_addr = lc_base;
    u32 *isrOffsetPtr;
    int i;
    u32 exception_mask;

    if (exception_mask_addr <= 0x44 && exception_mask_addr + 0x4000 > 0x44 &&
        gTRKCPUState.Extended1.DBAT3U_ & 3) {
        exception_mask_addr = 0x44;
    } else {
        exception_mask_addr = EXCEPTIONMASK_ADDR;
    }

    i = 0;
    exception_mask = *(u32 *)exception_mask_addr;
    isrOffsetPtr = TRK_ISR_OFFSETS;

    do {
        if ((exception_mask & (1 << i)) && i != 4) {
            TRK_copy_vector(isrOffsetPtr[i]);
        }

        i++;
    } while (i <= 14);
}

DSError TRKInitializeTarget(void) {
    gTRKState.isStopped = TRUE;
    gTRKState.msr = __TRK_get_MSR();
    lc_base = 0xE0000000;
    return DS_NoError;
}

#define __dcbi(a, b) asm { dcbi a, b }
#define __dcbfASM(a, b) asm { dcbf a, b }

void TRK__read_aram(register int mem_addr, register u32 aram_addr, size_t *length) {
    u32 err;
    int i;
    register int counter;
    u16 r;
    u32 x;
    u32 size;

    if (aram_addr < 0x4000 || aram_addr + *length > 0x8000000) {
        return;
    }

    x = aram_addr & ~0x1F;
    size = *length + (aram_addr & 0x1F);
    size = OSRoundUp32B(size);
    counter = 0;

    for (i = 0; i < size; i += 0x20) {
        __dcbi(counter, mem_addr);
        counter += 0x20;
    }

    do {
        err = ARGetDMAStatus();
    } while (err);

    r = __ARGetInterruptStatus();
    __ARClearInterrupt();

    ARStartDMA(1, mem_addr, x, size);

    while (!__ARGetInterruptStatus()) {
    }

    if (!r) {
        __ARClearInterrupt();
    }
}

void TRK__write_aram(register int mem_addr, register u32 aram_addr, size_t *length) {
    u8 buff[32] ATTRIBUTE_ALIGN(32);
    u32 err;
    register int count = mem_addr;
    register u32 bf;
    u32 aligned_addr;
    u32 size;
    u16 r;
    register u32 g;
    register int counter;
    u32 i;

    if (aram_addr < 0x4000 || aram_addr + *length > 0x8000000) {
        return;
    }

    aligned_addr = aram_addr & ~0x1f;
    counter = 0;
    size = *length + (aram_addr & 0x1f);
    size = OSRoundUp32B(size);

    for (i = 0; i < size; i += 0x20) {
        __dcbf((void *)counter, count);
        counter += 0x20;
    }

    do {
        err = ARGetDMAStatus();
    } while (err);

    r = __ARGetInterruptStatus();
    g = 0x8000000;

    counter = aram_addr & 0x1f;
    if (counter) {
        g = aligned_addr;
        bf = (u32)buff;
        __dcbi(r0, bf);
        __ARClearInterrupt();

        ARStartDMA(1, bf, aligned_addr, 0x20);

        while (!__ARGetInterruptStatus()) {
        }

        TRK_memcpy((void *)mem_addr, buff, counter);
        __dcbfASM(r0, mem_addr);
    }

    aram_addr += *length;
    counter = aram_addr & 0x1f;
    if (counter) {
        u32 val = aram_addr & ~0x1F;
        if (val != g) {
            bf = (u32)buff;
            __dcbi(r0, bf);
            __ARClearInterrupt();
            ARStartDMA(1, bf, val, 0x20);

            while (!__ARGetInterruptStatus()) {
            }
        }
        g = mem_addr + aram_addr;
        TRK_memcpy((void *)g, (void *)(buff + counter), 0x20 - counter);

        __dcbfASM(r0, g);
    }
    __sync();
    __ARClearInterrupt();
    ARStartDMA(0, mem_addr, aligned_addr, size);
    if (!r) {
        while (!__ARGetInterruptStatus()) {
        }

        __ARClearInterrupt();
    }
}

asm void InitMetroTRK(void) {
#ifdef __MWERKS__ // clang-format off
	nofralloc
	addi r1, r1, -4
	stw r3, 0(r1)
	lis r3, gTRKCPUState@h
	ori r3, r3, gTRKCPUState@l
	stmw r0, ProcessorState_PPC.Default.GPR(r3)
	lwz r4, 0(r1)
	addi r1, r1, 4
	stw r1, ProcessorState_PPC.Default.GPR[1](r3)
	stw r4, ProcessorState_PPC.Default.GPR[3](r3)
	mflr r4
	stw r4, ProcessorState_PPC.Default.LR(r3)
	stw r4, ProcessorState_PPC.Default.PC(r3)
	mfcr r4
	stw r4, ProcessorState_PPC.Default.CR(r3)
	//Turn off external interrupts (clear MSR[EE])
	mfmsr r4
	ori r3, r4, (1 << (31 - 16))
	xori r3, r3, (1 << (31 - 16))
	mtmsr r3
	mtsrr1 r4
	bl TRKSaveExtended1Block
	lis r3, gTRKCPUState@h
	ori r3, r3, gTRKCPUState@l
	lmw r0, ProcessorState_PPC.Default.GPR(r3)
	//Reset IABR and DABR
	li r0, 0
	mtspr  0x3f2, r0
	mtspr  0x3f5, r0
	//Switch to the debugger stack
	lis r1, _db_stack_addr@h
	ori r1, r1, _db_stack_addr@l
	mr r3, r5
	bl InitMetroTRKCommTable
	/*
	InitMetroTRKCommTable returns 1 for an unknown hardware id or when
	the selected GDEV or AMC DDH transport reports a stub (Hu_IsStub,
	AMC_IsStub; in this build Hu_IsStub returns 0 and AMC_IsStub 1);
	BBA always returns 0. On 1, take the failure path below; otherwise
	start TRK.
	*/
	cmpwi r3, 1
	bne initCommTableSuccess
	/*
	BUG: r3 still holds InitMetroTRKCommTable's return value (1), not
	gTRKCPUState, so the LR and GPR restore below read relative to address 1
	and return to a garbage code address.
	*/
	lwz r4, ProcessorState_PPC.Default.LR(r3)
	mtlr r4
	lmw r0, ProcessorState_PPC.Default.GPR(r3)
	blr
initCommTableSuccess:
	b TRK_main
	blr
#endif // clang-format on
}

asm void InitMetroTRK_BBA(void) {
#ifdef __MWERKS__ // clang-format off
	nofralloc
	addi r1, r1, -4
	stw r3, 0(r1)
	lis r3, gTRKCPUState@h
	ori r3, r3, gTRKCPUState@l
	stmw r0, ProcessorState_PPC.Default.GPR(r3)
	lwz r4, 0(r1)
	addi r1, r1, 4
	stw r1, ProcessorState_PPC.Default.GPR[1](r3)
	stw r4, ProcessorState_PPC.Default.GPR[3](r3)
	mflr r4
	stw r4, ProcessorState_PPC.Default.LR(r3)
	stw r4, ProcessorState_PPC.Default.PC(r3)
	mfcr r4
	stw r4, ProcessorState_PPC.Default.CR(r3)
	//Turn on external interrupts
	mfmsr r4
	ori r3, r4, (1 << (31 - 16))
	mtmsr r3
	mtsrr1 r4
	bl TRKSaveExtended1Block
	lis r3, gTRKCPUState@h
	ori r3, r3, gTRKCPUState@l
	lmw r0, ProcessorState_PPC.Default.GPR(r3)
	//Reset IABR and DABR
	li r0, 0
	mtspr  0x3f2, r0
	mtspr 0x3f5, r0
	//Switch to the debugger stack
	lis r1, _db_stack_addr@h
	ori r1, r1, _db_stack_addr@l
	li r3, 2
	bl InitMetroTRKCommTable //Initialize comm table as BBA hardware
	/*
	InitMetroTRKCommTable always returns 0 for the BBA id, so the failure
	path below is not taken in practice.
	*/
	cmpwi r3, 1
	bne initCommTableSuccess
	/*
	BUG: r3 still holds InitMetroTRKCommTable's return value (1), not
	gTRKCPUState, so the LR and GPR restore below read relative to address 1
	and return to a garbage code address.
	*/
	lwz r4, ProcessorState_PPC.Default.LR(r3)
	mtlr r4
	lmw r0, ProcessorState_PPC.Default.GPR(r3)
	blr
initCommTableSuccess:
	b TRK_main
	blr
#endif // clang-format on
}

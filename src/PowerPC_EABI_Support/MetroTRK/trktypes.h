#ifndef _METROTRK_TRKTYPES_H
#define _METROTRK_TRKTYPES_H

#include <dolphin/types.h>
#include <dolphin/os.h>
#include "MetroTRK/trkenum.h"
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/////// TRK STRUCTS AND TYPES //////
// UART error type (also typedef'd in the MSL UART.h).
typedef int UARTError;

// Function types for DB communications.
typedef int (*DBCommFunc)(void);
typedef int (*DBCommInitFunc)(void *, __OSInterruptHandler);
typedef int (*DBCommReadFunc)(u8 *, int);
typedef int (*DBCommWriteFunc)(const u8 *, int);

// Message buffer ID type.
typedef int MessageBufferID;

// Nub event ID type.
typedef u32 NubEventID;

// Size of message buffer.
#define TRKMSGBUF_SIZE (0x800 + 0x80)

// Struct for sending and receiving messages (size 0x890).
typedef struct MessageBuffer {
    /* 0x00 */ u32 _00; // unknown
    /* 0x04 */ BOOL isInUse;
    /* 0x08 */ u32 length;
    /* 0x0C */ u32 position;
    /* 0x10 */ u8 data[TRKMSGBUF_SIZE];
} MessageBuffer;
CHECK_SIZEOF(MessageBuffer, 0x890);

// Struct for storing DB communication functions (size 0x28).
typedef struct DBCommTable {
    /* 0x00 */ DBCommInitFunc initialize_func;
    /* 0x04 */ DBCommFunc init_interrupts_func;
    /* 0x08 */ DBCommFunc shutdown_func;
    /* 0x0C */ DBCommFunc peek_func;
    /* 0x10 */ DBCommReadFunc read_func;
    /* 0x14 */ DBCommWriteFunc write_func;
    /* 0x18 */ DBCommFunc open_func;
    /* 0x1C */ DBCommFunc close_func;
    /* 0x20 */ DBCommFunc pre_continue_func;
    /* 0x24 */ DBCommFunc post_stop_func;
} DBCommTable;
CHECK_SIZEOF(DBCommTable, 0x28);

// Struct for information on DS versions (kernel and protocol) (size 0x4)
typedef struct DSVersions {
    /* 0x00 */ u8 kernelMajor;
    /* 0x01 */ u8 kernelMinor;
    /* 0x02 */ u8 protocolMajor;
    /* 0x03 */ u8 protocolMinor;
} DSVersions;
CHECK_SIZEOF(DSVersions, 0x4);

// Struct for packet information (size 0x8).
typedef struct TRKPacketSeq {
    /* 0x00 */ u16 _00;   // unknown
    /* 0x02 */ u8 _02[6]; // unknown
} TRKPacketSeq;
CHECK_SIZEOF(TRKPacketSeq, 0x8);

// Struct for receiving packets from serial poll (size 0x14).
typedef struct TRKFramingState {
    /* 0x00 */ MessageBufferID msgBufID;
    /* 0x04 */ MessageBuffer *buffer;
    /* 0x08 */ ReceiverState receiveState;
    /* 0x0C */ BOOL isEscape;
    /* 0x10 */ u8 fcsType;
} TRKFramingState;
CHECK_SIZEOF(TRKFramingState, 0x14);

// Size of a command/reply packet header (CommandReply); payloads start after it.
#define TRKMSG_HEADER_SIZE 0x40

// Command reply information (size 0x40).
typedef struct CommandReply {
    /* 0x00 */ u32 _00;
    /* 0x04 */ union {
        u8 b;
        MessageCommandID m;
    } commandID; // use MessageCommandID enum
    /* 0x08 */ union {
        u8 b;  // reply error (DSReplyError) in an ACK
        u32 w; // packet word: file handle in file packets, program counter in stop notifications
    } replyError;
    /* 0x0C */ u32 _0C;
    /* 0x10 */ u8 _10[0x30]; // unknown
} CommandReply;
CHECK_SIZEOF(CommandReply, 0x40);

// Nub event information (size 0xC).
typedef struct TRKEvent {
    /* 0x00 */ NubEventType eventType;
    /* 0x04 */ NubEventID eventID;
    /* 0x08 */ MessageBufferID msgBufID;
} TRKEvent;
CHECK_SIZEOF(TRKEvent, 0xC);

// Event queue (size 0x28).
typedef struct TRKEventQueue {
    /* 0x00 */ u8 _00[4]; // unknown
    /* 0x04 */ int count; // number of events in queue
    /* 0x08 */ int next;  // next event in `events` to handle (0 or 1)
    /* 0x0C */ TRKEvent events[2];
    /* 0x24 */ NubEventID eventID; // ID to assign next event, min 0x100
} TRKEventQueue;
CHECK_SIZEOF(TRKEventQueue, 0x28);

// Struct for state information (size 0xA4).
typedef struct TRKState {
    /* 0x00 */ u32 gpr[32];
    /* 0x80 */ u32 lr;
    /* 0x84 */ u32 ctr;
    /* 0x88 */ u32 xer;
    /* 0x8C */ u32 msr;
    /* 0x90 */ u32 dar;
    /* 0x94 */ u32 dsisr;
    /* 0x98 */ BOOL isStopped;
    /* 0x9C */ BOOL inputActivated;
    /* 0xA0 */ void *inputPendingPtr;
} TRKState;
CHECK_SIZEOF(TRKState, 0xA4);

typedef struct ProcessorRestoreFlags_PPC {
    u8 TBR;
    u8 DEC;
    u8 linker_padding[0x9 - 0x2];
} ProcessorRestoreFlags_PPC;
CHECK_SIZEOF(ProcessorRestoreFlags_PPC, 0x9);

////////////////////////////////////

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif

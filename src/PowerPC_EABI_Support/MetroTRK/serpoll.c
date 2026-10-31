#include "MetroTRK/trk.h"

static TRKFramingState gTRKFramingState;
void *gTRKInputPendingPtr;

MessageBufferID TRKTestForPacket(void) {
    u8 payloadBuf[TRKMSGBUF_SIZE];
    u8 packetBuf[TRKMSG_HEADER_SIZE];
    int bufID;
    MessageBuffer *msg;
    MessageBufferID result;

    if (TRKPollUART() <= 0) {
        return -1;
    }

    result = TRKGetFreeBuffer(&bufID, &msg);

    MWTRACE(4, "TestForPacket : FreeBuffer is  %ld\n", result);

    TRKSetBufferPosition(msg, 0);
    if (TRKReadUARTN(packetBuf, TRKMSG_HEADER_SIZE) == UART_NoError) {
        int readSize;

        TRKAppendBuffer_ui8(msg, packetBuf, TRKMSG_HEADER_SIZE);
        readSize = ((u32 *)packetBuf)[0] - TRKMSG_HEADER_SIZE;
        result = bufID;
        if (readSize > 0) {
            MWTRACE(1, "Reading payload %ld bytes\n", readSize);
            if (TRKReadUARTN(payloadBuf, ((u32 *)packetBuf)[0] - TRKMSG_HEADER_SIZE) ==
                UART_NoError) {
                TRKAppendBuffer_ui8(msg, payloadBuf, ((u32 *)packetBuf)[0]);
            } else {
                MWTRACE(8, "TestForPacket : Invalid size of packet hdr.size\n");
                TRKReleaseBuffer(result);
                result = -1;
            }
        }
    } else {
        MWTRACE(8, "TestForPacket : Invalid size of packet\n");
        TRKReleaseBuffer(result);
        result = -1;
    }

    MWTRACE(1, "TestForPacket returning %ld\n", result);
    return result;
}

void TRKGetInput(void) {
    MessageBufferID id = TRKTestForPacket();
    if (id != -1) {
        TRKEvent event;
        TRKGetBuffer(id);
        TRKConstructEvent(&event, NUBEVENT_Request);
        event.msgBufID = id;
        gTRKFramingState.msgBufID = -1;
        TRKPostEvent(&event);
    }
}

void TRKProcessInput(int bufferIdx) {
    TRKEvent event;

    TRKConstructEvent(&event, NUBEVENT_Request);
    event.msgBufID = bufferIdx;
    gTRKFramingState.msgBufID = -1;
    TRKPostEvent(&event);
}

DSError TRKInitializeSerialHandler(void) {
    gTRKFramingState.msgBufID = -1;
    gTRKFramingState.receiveState = DSRECV_Wait;
    gTRKFramingState.isEscape = FALSE;

    MWTRACE(1, "TRK_Packet_Header \t    %ld bytes\n", 0x40);
    MWTRACE(1, "TRK_CMD_ReadMemory     %ld bytes\n", 0x40);
    MWTRACE(1, "TRK_CMD_WriteMemory    %ld bytes\n", 0x40);
    MWTRACE(1, "TRK_CMD_Connect \t    %ld bytes\n", 0x40);
    MWTRACE(1, "TRK_CMD_ReplyAck\t    %ld bytes\n", 0x40);
    MWTRACE(1, "TRK_CMD_ReadRegisters\t%ld bytes\n", 0x40);

    return DS_NoError;
}

DSError TRKTerminateSerialHandler(void) {
    return DS_NoError;
}

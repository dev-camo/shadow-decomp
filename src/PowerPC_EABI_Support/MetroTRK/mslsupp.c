#include "trk.h"

static DSIOResult __read_file(u32 handle, u8 *buffer, size_t *count, void (*idle_fn)(void));
static DSIOResult __write_file(u32 handle, u8 *buffer, size_t *count, void (*idle_fn)(void));
static DSIOResult __close_file(u32 handle);
static DSIOResult __access_file(u32 handle, u8 *buffer, size_t *count, void (*idle_fn)(void),
                                MessageCommandID cmd);

int __read_console(u32 handle, u8 *buffer, size_t *count, void (*idle_fn)(void)) {
    if (GetUseSerialIO() == 0) {
        return DS_IOError;
    }
    return __read_file(DS_Stdin, buffer, count, idle_fn);
}

int __TRK_write_console(u32 handle, u8 *buffer, size_t *count, void (*idle_fn)(void)) {
    if (GetUseSerialIO() == 0) {
        return DS_IOError;
    }
    return __write_file(DS_Stdout, buffer, count, idle_fn);
}

int __close_console(u32 handle) {
    return __close_file(handle);
}

static DSIOResult __read_file(u32 handle, u8 *buffer, size_t *count, void (*idle_fn)(void)) {
    return __access_file(handle, buffer, count, idle_fn, DSMSG_ReadFile);
}

static DSIOResult __write_file(u32 handle, u8 *buffer, size_t *count, void (*idle_fn)(void)) {
    return __access_file(handle, buffer, count, idle_fn, DSMSG_WriteFile);
}

static DSIOResult __close_file(u32 handle) {
    u32 result;

    if (GetTRKConnected() == DS_NoError) {
        return DS_IOError;
    }

    result = TRKCloseFile(DSMSG_CloseFile, handle);

    switch ((u8)result) {
    case DS_IONoError:
        return DS_IONoError;
    case DS_IOEOF:
        return DS_IOEOF;
    }

    return DS_IOError;
}

static DSIOResult __access_file(u32 handle, u8 *buffer, size_t *count, void (*idle_fn)(void),
                                MessageCommandID cmd) {
    size_t count_temp;
    u32 result;

    if (GetTRKConnected() == DS_NoError) {
        return DS_IOError;
    }

    count_temp = *count;
    result = TRKAccessFile(cmd, handle, &count_temp, buffer);
    *count = count_temp;

    switch ((u8)result) {
    case DS_IONoError:
        return DS_IONoError;
    case DS_IOEOF:
        return DS_IOEOF;
    }

    return DS_IOError;
}

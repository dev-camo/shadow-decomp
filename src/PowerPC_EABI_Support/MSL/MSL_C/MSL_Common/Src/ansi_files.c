#include "alloc.h"
#include "ansi_files.h"
#include "file_io.h"
#include "critical_regions.h"

#include "global.h"

static unsigned char stdin_buff[0x100];

static unsigned char stdout_buff[0x100];

static unsigned char stderr_buff[0x100];

files __files = {
    {
        0,              /* handle */
        0,              /* open_mode */
        __read,         /* io_mode */
        _IOLBF,         /* buffer_mode */
        __console_file, /* file_kind */
        /* file_orientation through char_buffer_overflow: */
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        {0, 0},             /* ungetc_buffer */
        {0, 0},             /* ungetc_wide_buffer */
        0,                  /* position */
        stdin_buff,         /* buffer */
        sizeof(stdin_buff), /* buffer_size */
        stdin_buff,         /* buffer_ptr */
        /* buffer_length, buffer_alignment, save_buffer_length, buffer_position: */
        0,
        0,
        0,
        0,
        NULL,             /* position_fn */
        __read_console,   /* read_fn */
        __write_console,  /* write_fn */
        __close_console,  /* close_fn */
        NULL,             /* idle_fn */
        &__files._stdout, /* next_file */
    },
    {
        1,              /* handle */
        0,              /* open_mode */
        __write,        /* io_mode */
        _IOLBF,         /* buffer_mode */
        __console_file, /* file_kind */
        /* file_orientation through char_buffer_overflow: */
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        {0, 0},              /* ungetc_buffer */
        {0, 0},              /* ungetc_wide_buffer */
        0,                   /* position */
        stdout_buff,         /* buffer */
        sizeof(stdout_buff), /* buffer_size */
        stdout_buff,         /* buffer_ptr */
        /* buffer_length, buffer_alignment, save_buffer_length, buffer_position: */
        0,
        0,
        0,
        0,
        NULL,             /* position_fn */
        __read_console,   /* read_fn */
        __write_console,  /* write_fn */
        __close_console,  /* close_fn */
        NULL,             /* idle_fn */
        &__files._stderr, /* next_file */
    },
    {
        2,              /* handle */
        0,              /* open_mode */
        __write,        /* io_mode */
        _IONBF,         /* buffer_mode */
        __console_file, /* file_kind */
        /* file_orientation through char_buffer_overflow: */
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        {0, 0},              /* ungetc_buffer */
        {0, 0},              /* ungetc_wide_buffer */
        0,                   /* position */
        stderr_buff,         /* buffer */
        sizeof(stderr_buff), /* buffer_size */
        stderr_buff,         /* buffer_ptr */
        /* buffer_length, buffer_alignment, save_buffer_length, buffer_position: */
        0,
        0,
        0,
        0,
        NULL,            /* position_fn */
        __read_console,  /* read_fn */
        __write_console, /* write_fn */
        __close_console, /* close_fn */
        NULL,            /* idle_fn */
        &__files.empty,  /* next_file */
    },
    {
        0, /* handle */
        0, /* open_mode */
        0, /* io_mode */
        0, /* buffer_mode */
        0, /* file_kind */
        /* file_orientation through char_buffer_overflow: */
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        {0, 0}, /* ungetc_buffer */
        {0, 0}, /* ungetc_wide_buffer */
        0,      /* position */
        NULL,   /* buffer */
        0,      /* buffer_size */
        NULL,   /* buffer_ptr */
        /* buffer_length, buffer_alignment, save_buffer_length, buffer_position: */
        0,
        0,
        0,
        0,
        NULL, /* position_fn */
        NULL, /* read_fn */
        NULL, /* write_fn */
        NULL, /* close_fn */
        NULL, /* idle_fn */
        NULL, /* next_file */
    },
};

void __close_all(void) {
    FILE *file = &__files._stdin;
    FILE *last_file;

    __begin_critical_region(stdin_access);

    while (file != NULL) {
        if (file->file_mode.file_kind != __closed_file) {
            fclose(file);
        }

        last_file = file;
        file = file->next_file;

        if (last_file->is_dynamically_allocated) {
            free(last_file);
        } else {
            last_file->file_mode.file_kind = __unavailable_file;
            if (file != NULL && file->is_dynamically_allocated) {
                last_file->next_file = NULL;
            }
        }
    }

    __end_critical_region(stdin_access);
}

unsigned int __flush_all(void) {
    unsigned int ret = 0;
    FILE *file = &__files._stdin;

    while (file) {
        if (file->file_mode.file_kind != __closed_file && fflush(file)) {
            ret = -1;
        }
        file = file->next_file;
    }

    return ret;
}

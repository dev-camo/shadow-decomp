/* CodeWarrior runtime exception-table fragments (MSL Gecko_ExceptionPPC.cp): registration of the
 * executable's exception index and the lookup of a PC's exception record. Nonmatching research. */
typedef struct ExceptionIndex {
    /* 0x00 */ unsigned int start;
    /* 0x04 */ unsigned int size_and_flags;
    /* 0x08 */ unsigned int exception_table;
} ExceptionIndex;

struct __eti_init_info {
    /* 0x00 */ ExceptionIndex *index_begin;
    /* 0x04 */ ExceptionIndex *index_end;
    /* 0x08 */ char *text_start;
    /* 0x0C */ unsigned int text_size;
};

/* registered exception-table fragment (names from MSL's Gecko_ExceptionPPC.cp) */
typedef struct ProcessInfo {
    /* 0x00 */ struct __eti_init_info *exception_info;
    /* 0x04 */ char *TOC;
    /* 0x08 */ int active;
} ProcessInfo;

typedef struct FragmentInfo {
    /* 0x00 */ void *exception_record;
    /* 0x04 */ char *exception_start;
    /* 0x08 */ void *exception_data;
    /* 0x0C */ unsigned int address_base;
    /* 0x10 */ void *relative_base;
    /* 0x14 */ char *toc;
    /* 0x18 */ unsigned int reserved_18;
    /* 0x1C */ unsigned int reserved_1C;
} FragmentInfo;

typedef struct FragmentSearchInfo {
    /* 0x00 */ ExceptionIndex *index_begin;
    /* 0x04 */ ExceptionIndex *index_end;
    /* USA uses paired words at sp+0x10/14 and sp+0x18/1c; consumers use 32-bit values. */
    /* These accesses do not prove the original semantic type was unsigned long long. */
    /* 0x08 */ unsigned long long address_base;
    /* 0x10 */ unsigned long long relative_base;
    /* 0x18 */ char *toc;
} FragmentSearchInfo;

typedef struct LongExceptionEntry {
    /* 0x00 */ unsigned int start;
    /* 0x04 */ unsigned short length;
    /* 0x06 */ unsigned short exception_offset;
} LongExceptionEntry;

typedef struct ShortExceptionEntry {
    /* 0x00 */ unsigned short start;
    /* 0x02 */ unsigned short end;
    /* 0x04 */ unsigned short exception_offset;
} ShortExceptionEntry;

static ProcessInfo fragmentinfo[1];

#if defined(VERSION_GUPJ8P)
#define FIND_EXCEPTION_FRAGMENT fn_803A2D0C
#elif defined(VERSION_GUPP8P)
#define FIND_EXCEPTION_FRAGMENT fn_803A371C
#else
#define FIND_EXCEPTION_FRAGMENT fn_803A26CC
#endif

/*
 * Observed parser boundaries: module iteration ends at text_size == 0, and module text ranges are
 * half-open. The index search masks 0x80000000 for the extent and uses that bit to choose an inline
 * table pointer or a relative_base offset; its broader format meaning is unknown. The current
 * comparisons accept both index endpoints. Header bit 3 selects the long-entry path; long records
 * begin at table+4, short records at table+2, and each list stops at start == 0. Both entry range
 * checks include their start and end values. Keep the numeric bits unnamed until their format is
 * established.
 */
void FIND_EXCEPTION_FRAGMENT(char *pc, FragmentInfo *info) {
    ProcessInfo *registration;
    struct __eti_init_info *module;
    ExceptionIndex *index;
    FragmentSearchInfo search;
    FragmentSearchInfo *search_info = &search;
    unsigned int pc_offset;
    unsigned int exception_offset;
    unsigned int long_entry_end;
    unsigned char *exception_table;
    LongExceptionEntry *long_entry;
    ShortExceptionEntry *short_entry;
    int found_module;
    int low;
    int high;
    int middle;

    registration = fragmentinfo;
    info->exception_record = 0;
    info->exception_data = 0;

    found_module = 0;
    if (registration->active != 0) {
        module = registration->exception_info;
        /* the exit test inside the loop stays closer to the target (87% vs 84% as a while condition) */
        while (1) {
            if (module->text_size == 0) {
                break;
            }

            if (pc >= module->text_start && pc < module->text_start + module->text_size) {
                search_info->index_begin = module->index_begin;
                search_info->index_end = module->index_end;
                search_info->address_base = 0;
                search_info->relative_base = 0;
                search_info->toc = registration->TOC;
                found_module = 1;
                break;
            }
            module++;
        }
    }

    if (found_module == 0) {
        return;
    }

    low = 0;
    high = search_info->index_end - search_info->index_begin;
    info->address_base = search_info->address_base;
    info->relative_base = (void *)(unsigned int)search_info->relative_base;
    info->toc = search_info->toc;
    pc_offset = (unsigned int)pc - search_info->address_base;
    /* This spelling gives MWCC the original single top-of-loop exit test. */
    for (;;) {
        if (low > high) {
            break;
        }
        middle = (low + high) / 2;
        index = search_info->index_begin + middle;
        if (pc_offset < index->start) {
            high = middle - 1;
        } else if (pc_offset > index->start + (index->size_and_flags & 0x7FFFFFFF)) {
            low = middle + 1;
        } else {
            info->exception_start = (char *)(search_info->address_base + index->start);
            if ((index->size_and_flags & 0x80000000) != 0) {
                exception_table = (unsigned char *)&index->exception_table;
            } else {
                exception_table =
                    (unsigned char *)search_info->relative_base + index->exception_table;
            }
            info->exception_record = exception_table;
            exception_offset = pc_offset - index->start;

            /* The original reloads the stored record pointer before reading its header. */
            if (((*(unsigned short *)info->exception_record >> 3) & 1) != 0) {
                long_entry = (LongExceptionEntry *)(exception_table + 4);

                while (long_entry->start != 0) {
                    long_entry_end = long_entry->start + long_entry->length * 4;
                    if (long_entry->start <= exception_offset &&
                        long_entry_end >= exception_offset) {
                        info->exception_data = exception_table + long_entry->exception_offset;
                        return;
                    }
                    long_entry++;
                }
            } else {
                short_entry = (ShortExceptionEntry *)(exception_table + 2);

                while (short_entry->start != 0) {
                    if (short_entry->start <= exception_offset &&
                        short_entry->end >= exception_offset) {
                        info->exception_data = exception_table + short_entry->exception_offset;
                        return;
                    }
                    short_entry++;
                }
            }
            return;
        }
    }
}

#undef FIND_EXCEPTION_FRAGMENT

void __unregister_fragment(int fragment_id) {
    if (fragment_id >= 0 && fragment_id < 1) {
        ProcessInfo *entry = &fragmentinfo[fragment_id];
        entry->exception_info = 0;
        entry->TOC = 0;
        entry->active = 0;
    }
}

/* loop form as in MSL's Gecko_ExceptionPPC.cp */
int __register_fragment(struct __eti_init_info *init_info, char *toc) {
    ProcessInfo *entry;
    int i;

    for (i = 0, entry = fragmentinfo; i < 1; ++i, ++entry) {
        if (entry->active == 0) {
            entry->exception_info = init_info;
            entry->TOC = toc;
            entry->active = 1;
            return i;
        }
    }

    return -1;
}

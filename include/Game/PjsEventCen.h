#ifndef GAME_PJSEVENTCEN_H
#define GAME_PJSEVENTCEN_H

#include "rw/rwcore.h"
#include "layout_check.h"

class UnkArchive_8004CA28;

/* ".CEN", the extension of the files that UnkCenFile_80329514 reads. The unrecovered event code
 * tests names against it too (fn_80170E5C). */
extern const char lbl_805F9CA8[];

/* A key of a .CEN file (placeholder name): a time and a position, read from four integers. The
 * RwV3d type is inferred from the member-by-member copy in Update. */
struct UnkCenKey_80329514 {
    /* 0x00 */ int time;
    /* 0x04 */ RwV3d position;
};
CHECK_SIZEOF(UnkCenKey_80329514, 0x10);

/* The reader of an event's .CEN file (placeholder name; the original class name is unknown and
 * the class has no RTTI). The task that holds one at 0x78 is TPjsEventActionScene by its RTTI
 * name: in the USA release (GUPE8P) the task's constructor at 0x80171A68 stores the vtable at
 * 0x8052BA24, whose type name is the string at 0x804C3728, and constructs the reader at
 * 0x80171AD4. The "PjsEvent" prefix of this header and its source file is taken from that
 * class. Method and member names are inferred. */
class UnkCenFile_80329514 {
  public:
    UnkCenFile_80329514();
    ~UnkCenFile_80329514();
    /* When the name of entry index of the archive has the .CEN extension, copies the name, asks
     * the archive to load the entry and to pass it to Parse, sets the state to 1 and returns
     * true. Otherwise it returns false. */
    bool Load(UnkArchive_8004CA28 *archive, int index);
    /* Finds the first key, the last one excluded, whose time is at or before time while the
     * next key's time is after it, and stores its index in lbl_805EF2B8 and its position in
     * lbl_80580224; a file without keys gives index 0 and a zero position. When no key matches
     * it restores lbl_805EF2B8 and returns false. Otherwise it chooses the base position
     * lbl_80580218, stores the sum of the two positions in lbl_80580230 and returns true.
     *
     * The base position, where "the record" is the record of fn_800CBAA4's object that the word
     * at 0xAC of lbl_8057D6A0 indexes:
     * - When lbl_805EF2A0 is 950, 970, 972, 974, 976 or 978: the record's member at 0xB4.
     * - Otherwise, when the word at 0x8 of fn_80074470's object is 6 or 14: the record's member
     *   at 0xA8 when the byte at 0x125 of lbl_8057D6A0 is set; else element 1 of the record's
     *   array at 0x60 when the byte at 0xB2 or the byte at 0xB1 is set; else the element that
     *   the word at 0xC4 indexes.
     * - Otherwise: position lbl_805EF2A4 of the points that fn_80170D8C's object refers to, or
     *   their first position when lbl_805EF2A4 is negative. When that object's pointer is null,
     *   lbl_80580218 keeps its value. */
    bool Update(float time);
    /* The archive's callback: it expects data to be the file's text and user the reader. The
     * archive passes a null data pointer when its read failed (UnkArchiveCallback_8004C358),
     * and Parse reads through data without testing it. It reads decimal integers up to the
     * first 'E', four per key (time, x, y, z), frees data and sets the state to 2. A number
     * counts only once a character that is neither a digit nor '-' follows it, so the last
     * number needs such a character before the 'E': "1 2 3 4E" leaves the fourth value unstored
     * and adds no key. */
    static void Parse(void *data, void *user);

    /* 0x00 */ char m_name[0x40];
    /* 0: constructed, 1: Load has requested the file, 2: Parse has read it. */
    /* 0x40 */ int m_state;
    /* 0x44 */ int m_keyCount;
    /* The task's next member starts at 0x180, which leaves room for twelve keys. Parse does not
     * check the count. */
    /* 0x48 */ UnkCenKey_80329514 m_keys[12];
};
CHECK_SIZEOF(UnkCenFile_80329514, 0x108);

#endif

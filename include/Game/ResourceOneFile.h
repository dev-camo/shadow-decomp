#ifndef GAME_RESOURCEONEFILE_H
#define GAME_RESOURCEONEFILE_H

#include "Game/UnkString_8004F888.h"
#include "layout_check.h"

struct UnkResourceOneFileRecord;
class UnkFile_8004CFB0;

/* Class names come from RTTI. Only the destructor interface used by these declarations is
 * represented; the other fourteen virtual methods are not called here. */
class Resource {
  public:
    virtual ~Resource();
};
CHECK_SIZEOF(Resource, 0x4);

class ResourceOneFile : public Resource {
  public:
    ResourceOneFile(const char *name, bool asynchronous);
    virtual ~ResourceOneFile();

    /* 0x04 */ int field_0x4;
    /* 0x08 */ UnkString_8004F888 field_0x8;
    /* 0x14 */ UnkResourceOneFileRecord *field_0x14;
    /* 0x18 */ int field_0x18;
    /* 0x1C */ unsigned int field_0x1C;
    /* 0x20 */ UnkFile_8004CFB0 *field_0x20;
    /* 0x24 */ int *field_0x24;
};
CHECK_SIZEOF(ResourceOneFile, 0x28);

#endif

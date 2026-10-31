#ifndef FN_8042_NAME_REGISTRY_H
#define FN_8042_NAME_REGISTRY_H

/* The name registry, byte-order and ARAM helpers at 0x80420C74-0x80421300 that the pack loader in
 * fn_80410630.c and the resource groups in fn_80411714.c call. The original names and types are
 * unknown; the descriptions are inferred from the code. */

/* A reference-counted table that maps a name to the identifiers registered under it (inferred).
 * Its layout is private to the registry functions declared below (fn_80420C74 to fn_804210AC). */
typedef struct NameRegistry NameRegistry;

#ifdef __cplusplus
extern "C" {
#endif

NameRegistry *fn_804210AC(void);
void fn_80421050(NameRegistry *registry);
/* Returns the identifier registered under the name, or 0. */
unsigned int fn_80420C74(NameRegistry *registry, const char *name);
/* Registers the identifier under the name, or adds a reference to an existing registration; returns
 * the identifier, or 0 when the registry or the name is missing. */
unsigned int fn_80420DA4(NameRegistry *registry, const char *name, unsigned int id);
/* Registers a new identifier under the name, or adds a reference to the name's first registration;
 * returns the identifier. */
unsigned int fn_80420EE0(NameRegistry *registry, const char *name);
/* Drops one reference to the identifier registered under the name. */
void fn_80420CE8(NameRegistry *registry, const char *name, unsigned int id);

/* Copy count 16-bit and 32-bit values from source to destination, reversing the bytes of each
 * (source may equal destination). */
void fn_8042118C(void *destination, const void *source, int count);
void fn_804210E4(void *destination, const void *source, int count);
/* Returns the byte-order setting (set by fn_804211D4) that the pack loader compares with a file's
 * own byte-order flag. */
int fn_804211CC(void);

/* Reads size bytes from ARAM into main memory (the 32-byte aligned copy that fn_80413318 makes). */
void fn_804211E4(void *destination, unsigned int aramAddress, int size);
/* Releases the ARAM block that starts at an address (called by fn_804129D4). */
void fn_80421300(unsigned int aramAddress);

#ifdef __cplusplus
}
#endif

#endif

/* CRI LSC critical section (lsc_crs.c). */

/* Inferred callee interfaces: neither body consumes incoming argument registers.
 * Original argument lists and return types are not established.
 */
extern void SJCRS_Unlock(void);
extern void SJCRS_Lock(void);

/* Inferred wrapper interfaces: several callers pass an opaque stack address.
 * It is unused here; the original type, full arity and return types are unknown.
 */
void LSC_UnlockCrs(void *unused_context) {
    SJCRS_Unlock();
}

void LSC_LockCrs(void *unused_context) {
    SJCRS_Lock();
}

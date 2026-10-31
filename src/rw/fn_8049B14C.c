/* RenderWare pipeline library: destroying packets. The original file name is unknown. */
#include "rw/rwcore.h"

/* Destroys a packet: frees the heap data of each of its clusters that has any and empties them,
 * and marks its pipeline's embedded packet unused */
void fn_8049B14C(RxPacket *packet) {
    RwUInt32 count;
    RxCluster *cluster = packet->clusters;

    packet->pipeline->embeddedPacketState = rxPKST_UNUSED;
    count = packet->numClusters;
    do {
        if (cluster->clusterRef != NULL) {
            if (cluster->data != NULL && !(cluster->flags & rxCLFLAGS_EXTERNAL)) {
                fn_8049EA2C(lbl_805F169C, cluster->data);
            }
            /* Matching workaround: the original clears flags and stride with one 32-bit store;
             * assigning the two 16-bit members separately (in either order, or chained) compiles
             * to two sth stores (measured, the function grows from 0xAC to 0xB0 bytes). Revisit
             * when the original's 32-bit access to the pair (member, union or struct copy) is
             * found. */
            *(RwUInt32 *)&cluster->flags = 0;
            cluster->data = NULL;
            cluster->numAlloced = 0;
            cluster->numUsed = 0;
            cluster->clusterRef = NULL;
        }
        cluster++;
    } while (--count != 0);
    packet->flags = 0;
}

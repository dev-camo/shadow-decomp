/* RenderWare pipeline library: the dependency pass of unlocking a pipeline. Working back from the
 * last node, each node collects the clusters that it and the nodes after it require, dropping
 * requirements that every path to them invalidates; the clusters that each node passes on to the
 * next are then traced into scopes, each scope is given a packet slot, and the slots,
 * the pipeline clusters and the embedded packet are laid out in the scratch space of the
 * pipeline's superblock. The original file name is unknown. */
#include "rw/rwcore.h"
#include <string.h>

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwID_COREPLUGIN;                                                         \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

typedef struct rxReqEntry rxReqEntry;
typedef struct rxScopeTrace rxScopeTrace;

/* A cluster that a node or the nodes after it need, or that one of its outputs passes on (name
 * inferred) */
struct rxReqEntry {
    /* 0x00 */ RxClusterDefinition *clusterDef;
    /* 0x04 */ RwInt32 rqdOrOpt;    /* rxCLREQ_* */
    /* 0x08 */ RwUInt32 numInsLeft; /* the node's inputs that do not yet invalidate it */
    /* 0x0C */ rxScopeTrace *scope;
    /* 0x10 */ rxReqEntry *nextInScope;
    /* 0x14 */ rxReq *req;           /* the list that holds the entry */
    /* 0x18 */ RwUInt32 outputMask;  /* the outputs that pass the cluster on, bit per output */
    /* 0x1C */ RwUInt32 slotIndex;   /* the packet slot of the cluster, -1 until one is given */
    /* 0x20 */ RxPipelineNode *node; /* the node whose requirement it is */
};
CHECK_SIZEOF(struct rxReqEntry, 0x24);

/* The requirements of a node: room for one entry per distinct cluster of the pipeline */
struct rxReq {
    /* 0x00 */ RwUInt32 numReqs;
    /* 0x04 */ RwUInt32 maxReqs;
    /* 0x08 */ RwUInt32 slotsUsed; /* the packet slots taken at the node, bit per slot */
    /* 0x0C */ rxReqEntry *reqs;
    /* 0x10 */ RxPipelineNode *node;
};
CHECK_SIZEOF(struct rxReq, 0x14);

/* A scope: requirements of successive nodes that carry the same cluster in the same packet slot.
 * Merged scopes are chained to the first one, which alone stays in the list of scopes (name
 * inferred). */
struct rxScopeTrace {
    /* 0x0 */ rxReqEntry *reqs;     /* chained through nextInScope */
    /* 0x4 */ rxScopeTrace *merged; /* the next scope merged into this one */
    /* 0x8 */ rxScopeTrace *next;   /* in the list of scopes */
    /* 0xC */ rxScopeTrace *parent; /* the scope this one was merged into */
};
CHECK_SIZEOF(struct rxScopeTrace, 0x10);

/* Called once for each distinct cluster definition of a list of scopes (name inferred). The one
 * callback here ignores its second argument, so its type is inferred. */
typedef void (*rxEnumClustersCallBack)(RxClusterDefinition *clusterDef, rxScopeTrace *scope,
                                       void *data);

/* Finds the entry for a cluster in a node's requirements; NULL if there is none */
rxReqEntry *fn_8049D408(rxReq *req, RxClusterDefinition *clusterDef) {
    RwUInt32 numReqs = req->numReqs;

    if (numReqs != 0) {
        rxReqEntry *entry = req->reqs;
        RwUInt32 i;

        for (i = 0; i < numReqs; i++) {
            if (entry->clusterDef == clusterDef) {
                return entry;
            }
            entry++;
        }
    }
    return NULL;
}

/* Removes an entry from a node's requirements, moving the last entry into its place */
void fn_8049D43C(rxReq *req, rxReqEntry *entry) {
    if (entry != &req->reqs[req->numReqs - 1]) {
        *entry = req->reqs[req->numReqs - 1];
    }
    req->numReqs--;
}

/* The index of a cluster among a node's clusters of interest; -1 if it is not one of them */
RwUInt32 fn_8049D4B8(RxIoSpec *io, RxClusterDefinition *clusterDef) {
    RwUInt32 i;

    for (i = 0; i < io->numClustersOfInterest; i++) {
        if (io->clustersOfInterest[i].clusterDef == clusterDef) {
            return i;
        }
    }
    return (RwUInt32)-1;
}

/* An entry of a node's requirements by index; NULL past the last one (name inferred) */
#define ReqGetEntry(req, index) ((index) < (req)->numReqs ? &(req)->reqs[index] : NULL)

/* One input of a node invalidates a cluster: when none of its inputs leaves the cluster valid any
 * more, the node no longer requires it, and neither do the nodes after it that only get the
 * cluster from this node */
void fn_8049D4FC(RxPipeline *pipeline, RxPipelineNode *node, RxClusterDefinition *clusterDef) {
    RwUInt32 i;
    rxReq *req = node->topSortData->req;
    RxIoSpec *io = &node->nodeDef->io;
    rxReqEntry *entry = fn_8049D408(req, clusterDef);

    if (entry != NULL) {
        if (--entry->numInsLeft == 0) {
            fn_8049D43C(node->topSortData->req, entry);
            for (i = 0; i < node->numOutputs; i++) {
                if (node->outputs[i] != (RwUInt32)-1) {
                    RxOutputSpec *output = &node->nodeDef->io.outputs[i];
                    RwUInt32 index = fn_8049D4B8(io, clusterDef);
                    RwInt32 valid;

                    if (index == (RwUInt32)-1) {
                        valid = output->allOtherClusters;
                    } else {
                        valid = output->outputClusters[index];
                    }
                    if (valid == rxCLVALID_NOCHANGE) {
                        fn_8049D4FC(pipeline, &pipeline->nodes[node->outputs[i]], clusterDef);
                    }
                }
            }
        }
    }
}

/* Creates the requirement list of a node in the scratch space; NULL when it runs out (name
 * inferred) */
static inline rxReq *ReqCreate(RwUInt32 maxReqs, RxPipelineNode *node) {
    rxReq *req = fn_8049B8BC(sizeof(rxReq));

    if (req != NULL) {
        req->reqs = fn_8049B8BC(maxReqs * sizeof(rxReqEntry));
        if (req->reqs != NULL) {
            req->maxReqs = maxReqs;
            req->numReqs = 0;
            req->node = node;
            req->slotsUsed = 0;
            return req;
        }
    }
    return NULL;
}

/* Adds a cluster to a node's requirements, or makes an existing entry required when the new
 * requirement is; returns the entry (name inferred) */
static inline rxReqEntry *ReqAddEntry(rxReq *req, RxClusterDefinition *clusterDef, RwInt32 rqdOrOpt,
                                      RwUInt32 numIns, RxPipelineNode *node) {
    rxReqEntry *entry = fn_8049D408(req, clusterDef);

    if (entry != NULL) {
        if (rqdOrOpt == rxCLREQ_REQUIRED) {
            entry->rqdOrOpt = rxCLREQ_REQUIRED;
            entry->node = node;
        }
        return entry;
    }

    entry = &req->reqs[req->numReqs];
    req->numReqs++;
    entry->clusterDef = clusterDef;
    entry->rqdOrOpt = rqdOrOpt;
    entry->numInsLeft = numIns;
    entry->nextInScope = NULL;
    entry->req = req;
    entry->scope = NULL;
    entry->outputMask = 0;
    entry->slotIndex = (RwUInt32)-1;
    entry->node = node;
    return entry;
}

/* From the last node back, gathers what each node and the nodes after it require. A cluster an
 * output leaves unchanged is required from the node itself; one the output invalidates no longer
 * reaches the next node through that input; one the output creates satisfies the requirement,
 * and an output that does not create a required cluster is an error. Returns 0 or the error
 * code. */
RwUInt32 fn_8049D934(RxPipeline *pipeline) {
    RxNodeDefinition *nodeDef;
    RwUInt32 count = pipeline->numNodes;
    RxPipelineNode *node = &pipeline->nodes[count - 1];

    do {
        RwUInt32 numClusters;
        rxReq *req;
        RwUInt32 i;
        RwUInt32 j;

        nodeDef = node->nodeDef;
        for (i = 0; i < nodeDef->io.numClustersOfInterest; i++) {
            RxClusterDefinition *clusterDef = nodeDef->io.clustersOfInterest[i].clusterDef;

            if (clusterDef == NULL) {
                /* the original passes no cluster index with this error */
                RWERROR((E_RX_DEP_NULLCLUSTERDEF, node->nodeDef->name));
                return E_RX_DEP_NULLCLUSTERDEF;
            }
            for (j = i + 1; j < nodeDef->io.numClustersOfInterest; j++) {
                RxClusterDefinition *other = nodeDef->io.clustersOfInterest[j].clusterDef;

                if (clusterDef == other) {
                    RWERROR((E_RX_DEP_DUPLICATECLUSTERDEFS, node->nodeDef->name, other->name));
                    return E_RX_DEP_DUPLICATECLUSTERDEFS;
                }
            }
        }

        numClusters = fn_8049B9A4(pipeline);
        req = ReqCreate(numClusters, node);
        node->topSortData->req = req;
        if (req == NULL) {
            RWERROR((E_RX_DEP_OUTOFMEMORY));
            return E_RX_DEP_OUTOFMEMORY;
        }

        for (i = 0; i < node->numOutputs; i++) {
            if (node->outputs[i] != (RwUInt32)-1) {
                RxPipelineNode *next = &pipeline->nodes[node->outputs[i]];
                RxOutputSpec *output = &node->nodeDef->io.outputs[i];

                for (j = 0; j < next->topSortData->req->numReqs; j++) {
                    rxReqEntry *nextEntry = ReqGetEntry(next->topSortData->req, j);
                    RxClusterDefinition *clusterDef = nextEntry->clusterDef;
                    RwUInt32 index = fn_8049D4B8(&nodeDef->io, clusterDef);
                    RwInt32 valid;

                    if (index == (RwUInt32)-1) {
                        valid = output->allOtherClusters;
                    } else {
                        valid = output->outputClusters[index];
                    }
                    if (valid == rxCLVALID_NOCHANGE) {
                        rxReqEntry *entry =
                            ReqAddEntry(node->topSortData->req, clusterDef, nextEntry->rqdOrOpt,
                                        node->topSortData->numIns, nextEntry->node);

                        if (entry == NULL) {
                            RWERROR((E_RX_DEP_OUTOFMEMORY));
                            return E_RX_DEP_OUTOFMEMORY;
                        }
                    } else if (nextEntry->rqdOrOpt == rxCLREQ_REQUIRED) {
                        if (valid != rxCLVALID_VALID) {
                            RWERROR((E_RX_DEP_DEPENDENCIESMISMATCH, nextEntry->clusterDef->name,
                                     nextEntry->node->nodeDef->name, node->nodeDef->name, i,
                                     output->name));
                            return E_RX_DEP_DEPENDENCIESMISMATCH;
                        }
                    } else if (valid == rxCLVALID_INVALID) {
                        fn_8049D4FC(pipeline, next, clusterDef);
                    }
                }
            }
        }

        for (i = 0; i < nodeDef->io.numClustersOfInterest; i++) {
            if (nodeDef->io.inputRequirements[i] != rxCLREQ_DONTWANT) {
                rxReqEntry *entry = ReqAddEntry(
                    node->topSortData->req, nodeDef->io.clustersOfInterest[i].clusterDef,
                    nodeDef->io.inputRequirements[i], node->topSortData->numIns, node);

                if (entry == NULL) {
                    RWERROR((E_RX_DEP_OUTOFMEMORY));
                    return E_RX_DEP_OUTOFMEMORY;
                }
            }
        }
        node--;
    } while (--count != 0);
    return 0;
}

/* From the first node on, adds the clusters that each node forces present or that its outputs
 * create, and marks in each entry the outputs that pass its cluster on. Returns 0 or the error
 * code. */
RwUInt32 fn_8049DE44(RxPipeline *pipeline) {
    RwUInt32 i;
    RxOutputSpec *output;
    RxNodeDefinition *nodeDef;
    RwUInt32 count;
    RwUInt32 j;
    RxPipelineNode *node;
    RxPipelineNode *next;

    count = pipeline->numNodes;
    node = pipeline->nodes;
    do {
        nodeDef = node->nodeDef;

        for (i = 0; i < nodeDef->io.numClustersOfInterest; i++) {
            RxClusterRef *clusterRef = &nodeDef->io.clustersOfInterest[i];

            if (clusterRef->forcePresent) {
                rxReqEntry *entry = ReqAddEntry(node->topSortData->req, clusterRef->clusterDef,
                                                rxCLREQ_DONTWANT, 1, node);

                if (entry == NULL) {
                    RWERROR((E_RX_DEP_OUTOFMEMORY));
                    return E_RX_DEP_OUTOFMEMORY;
                }
            }
        }

        for (i = 0; i < node->numOutputs; i++) {
            if (node->outputs[i] != (RwUInt32)-1) {
                next = &pipeline->nodes[node->outputs[i]];
                output = &node->nodeDef->io.outputs[i];

                for (j = 0; j < next->topSortData->req->numReqs; j++) {
                    rxReqEntry *nextEntry = ReqGetEntry(next->topSortData->req, j);
                    RxClusterDefinition *clusterDef = nextEntry->clusterDef;
                    RwUInt32 index = fn_8049D4B8(&nodeDef->io, clusterDef);
                    RwInt32 valid;
                    rxReqEntry *entry;

                    if (index == (RwUInt32)-1) {
                        valid = output->allOtherClusters;
                    } else {
                        valid = output->outputClusters[index];
                    }
                    if (valid != rxCLVALID_INVALID) {
                        if (valid != rxCLVALID_NOCHANGE) {
                            entry = ReqAddEntry(node->topSortData->req, clusterDef,
                                                rxCLREQ_DONTWANT, 1, node);
                            if (entry == NULL) {
                                RWERROR((E_RX_DEP_OUTOFMEMORY));
                                return E_RX_DEP_OUTOFMEMORY;
                            }
                        } else {
                            entry = fn_8049D408(node->topSortData->req, clusterDef);
                        }
                        if (entry != NULL) {
                            entry->outputMask |= 1 << i;
                        }
                    }
                }
            }
        }
        node++;
    } while (--count != 0);
    return 0;
}

/* Creates a scope in the scratch space and puts it at the head of the list; NULL when the
 * scratch space runs out (name inferred) */
static inline rxScopeTrace *ScopeTraceCreate(rxScopeTrace **scopes) {
    rxScopeTrace *scope = fn_8049B8BC(sizeof(rxScopeTrace));

    if (scope != NULL) {
        scope->merged = NULL;
        scope->reqs = NULL;
        scope->parent = NULL;
        scope->next = *scopes;
        *scopes = scope;
        return scope;
    }
    return NULL;
}

/* Adds a requirement to a scope (name inferred) */
static inline void ScopeTraceAddEntry(rxScopeTrace *scope, rxReqEntry *entry) {
    entry->nextInScope = scope->reqs;
    scope->reqs = entry;
}

/* Merges the scope of other into the scope of scope, unless they already are one scope; scopes
 * is the list of scopes (name inferred) */
static inline void ScopeTraceMerge(rxScopeTrace *scope, rxScopeTrace *other,
                                   rxScopeTrace **scopes) {
    while (scope->parent != NULL) {
        scope = scope->parent;
    }
    while (other->parent != NULL) {
        other = other->parent;
    }
    if (scope != other) {
        rxScopeTrace *last = scope;

        while (last->merged != NULL) {
            last = last->merged;
        }
        last->merged = other;
        other->parent = scope;

        /* unlink other from the list of scopes */
        while (*scopes != other) {
            scopes = &(*scopes)->next;
        }
        *scopes = (*scopes)->next;
    }
}

/* Gives every requirement a scope: an entry that the previous node passes on (an output keeps
 * the cluster valid) joins the scope of that node's entry. Returns 0 or the error code. */
RwUInt32 fn_8049E1A4(RxPipeline *pipeline, rxScopeTrace **scopes) {
    RwUInt32 count;
    RwUInt32 i;
    RwUInt32 j;
    RxPipelineNode *node;
    rxReqEntry *entry;
    RxPipelineNode *next;

    count = pipeline->numNodes;
    node = pipeline->nodes;
    do {
        for (j = 0; j < node->topSortData->req->numReqs; j++) {
            entry = ReqGetEntry(node->topSortData->req, j);

            if (entry->scope == NULL) {
                rxScopeTrace *scope = ScopeTraceCreate(scopes);

                entry->scope = scope;
                if (scope == NULL) {
                    RWERROR((E_RX_DEP_OUTOFMEMORY));
                    return E_RX_DEP_OUTOFMEMORY;
                }
                ScopeTraceAddEntry(entry->scope, entry);
            }
        }

        for (i = 0; i < node->numOutputs; i++) {
            if (node->outputs[i] != (RwUInt32)-1) {
                next = &pipeline->nodes[node->outputs[i]];
                for (j = 0; j < next->topSortData->req->numReqs; j++) {
                    rxReqEntry *nextEntry = ReqGetEntry(next->topSortData->req, j);

                    if (nextEntry->rqdOrOpt != rxCLREQ_DONTWANT) {
                        entry = fn_8049D408(node->topSortData->req, nextEntry->clusterDef);

                        if (entry != NULL && (entry->outputMask & (1 << i))) {
                            if (nextEntry->scope == NULL) {
                                ScopeTraceAddEntry(entry->scope, nextEntry);
                                nextEntry->scope = entry->scope;
                            } else {
                                ScopeTraceMerge(nextEntry->scope, entry->scope, scopes);
                            }
                        }
                    }
                }
            }
        }
        node++;
    } while (--count != 0);
    return 0;
}

/* Gives each scope the lowest packet slot that none of its nodes uses yet and sets the pipeline's
 * slot count. Returns 0. */
RwUInt32 fn_8049E438(RxPipeline *pipeline, rxScopeTrace **scopes) {
    RwUInt32 numSlots;
    rxScopeTrace *scope;
    RwUInt32 slot;
    RwUInt32 slotBit;
    rxScopeTrace *merged;
    RwUInt32 slotsUsed;
    rxReqEntry *entry;

    scope = *scopes;
    numSlots = 0;
    while (scope != NULL) {
        slotsUsed = 0;
        merged = scope;
        do {
            for (entry = merged->reqs; entry != NULL; entry = entry->nextInScope) {
                slotsUsed |= entry->req->slotsUsed;
            }
            merged = merged->merged;
        } while (merged != NULL);

        slot = 0;
        while (slotsUsed & 1) {
            slotsUsed >>= 1;
            slot++;
        }
        if (slot >= numSlots) {
            numSlots = slot + 1;
        }

        slotBit = 1 << slot;
        merged = scope;
        do {
            for (entry = merged->reqs; entry != NULL; entry = entry->nextInScope) {
                entry->slotIndex = slot;
                entry->req->slotsUsed |= slotBit;
            }
            merged = merged->merged;
        } while (merged != NULL);

        scope = scope->next;
    }
    pipeline->packetNumClusterSlots = numSlots;
    return 0;
}

/* Whether scope is the first in the list of scopes with its cluster definition (name inferred) */
static inline RwBool ScopeIsFirstOfCluster(rxScopeTrace *scopes, rxScopeTrace *scope) {
    rxScopeTrace *other = scopes;

    while (other != scope) {
        if (other->reqs->clusterDef == scope->reqs->clusterDef) {
            break;
        }
        other = other->next;
    }
    return other == scope;
}

/* Adds a pipeline cluster for a cluster definition in the scratch space; data points to the
 * first pipeline cluster, set by the first call */
void fn_8049E4F4(RxClusterDefinition *clusterDef, rxScopeTrace *scope, void *data) {
    RxPipelineCluster **first = data;
    RxPipelineCluster *cluster = fn_8049B92C(sizeof(RxPipelineCluster));

    if (*first == NULL) {
        *first = cluster;
    }
    cluster->clusterRef = clusterDef;
    cluster->creationAttributes = clusterDef->defaultAttributes;
}

/* Calls callBack once for each distinct cluster definition of the scopes (name inferred) */
static inline void EnumPipelineClusters(rxScopeTrace *scopes, rxEnumClustersCallBack callBack,
                                        void *data) {
    rxScopeTrace *scope;

    for (scope = scopes; scope != NULL; scope = scope->next) {
        if (ScopeIsFirstOfCluster(scopes, scope) && callBack != NULL) {
            callBack(scope->reqs->clusterDef, scope, data);
        }
    }
}

/* Lays out the packet tables in the scratch space: the embedded packet, the pipeline clusters,
 * each node's slot cluster table, slot continuity masks, cluster of interest slots and private
 * data, and the pipeline's input requirements sorted by cluster definition. Returns 0. */
RwUInt32 fn_8049E54C(RxPipeline *pipeline, rxScopeTrace *scopes) {
    RwUInt32 numInputRequirements = 0;
    RwUInt32 numClusters;
    RxPipelineCluster *clusters;
    rxScopeTrace *scope;
    void *top;
    void *bottom;
    RxPipelineNode *entryNode;
    RwUInt32 i;
    RwUInt32 j;

    /* the entry point is the first node once the nodes are sorted */
    entryNode = pipeline->nodes;
    for (j = 0; j < entryNode->topSortData->req->numReqs; j++) {
        rxReqEntry *entry = ReqGetEntry(entryNode->topSortData->req, j);

        if (entry->rqdOrOpt == rxCLREQ_REQUIRED || entry->rqdOrOpt == rxCLREQ_OPTIONAL) {
            numInputRequirements++;
        }
    }

    numClusters = 0;
    for (scope = scopes; scope != NULL; scope = scope->next) {
        if (ScopeIsFirstOfCluster(scopes, scope)) {
            numClusters++;
        }
    }

    /* clear what is left of the scratch space */
    top = fn_8049B8BC(0);
    bottom = fn_8049B92C(0);
    memset(bottom, 0, (RwUInt8 *)top - (RwUInt8 *)bottom);

    pipeline->embeddedPacket =
        fn_8049B92C(sizeof(RxPacket) + (pipeline->packetNumClusterSlots - 1) * sizeof(RxCluster));
    clusters = NULL;
    pipeline->embeddedPacket->numClusters = pipeline->packetNumClusterSlots;
    pipeline->embeddedPacket->pipeline = pipeline;
    pipeline->embeddedPacketState = rxPKST_PACKETLESS;
    EnumPipelineClusters(scopes, fn_8049E4F4, &clusters);

    for (i = 0; i < pipeline->numNodes; i++) {
        RxPipelineNode *node = &pipeline->nodes[i];
        RwUInt32 continuity;
        RwUInt32 k;

        if (pipeline->packetNumClusterSlots != 0) {
            node->slotClusterRefs =
                fn_8049B92C(pipeline->packetNumClusterSlots * sizeof(RxPipelineCluster *));
        }
        node->slotsContinue = fn_8049B92C((pipeline->packetNumClusterSlots + 1) * sizeof(RwUInt32));
        if (node->nodeDef->io.numClustersOfInterest != 0) {
            node->inputToClusterSlot =
                fn_8049B92C(node->nodeDef->io.numClustersOfInterest * sizeof(RwUInt32));
        }
        if (node->nodeDef->pipelineNodePrivateDataSize != 0) {
            node->privateData = fn_8049B92C((node->nodeDef->pipelineNodePrivateDataSize + 3) & ~3);
        }

        /* the first continuity mask is of the outputs that pass on every cluster */
        continuity = (RwUInt32)-1;
        for (j = 0; j < node->topSortData->req->numReqs; j++) {
            RxPipelineCluster *cluster = NULL;
            rxReqEntry *entry = ReqGetEntry(node->topSortData->req, j);

            for (k = 0; k < numClusters; k++) {
                if (clusters[k].clusterRef == entry->clusterDef) {
                    cluster = &clusters[k];
                    break;
                }
            }
            node->slotClusterRefs[entry->slotIndex] = cluster;
            node->slotsContinue[entry->slotIndex + 1] = entry->outputMask;
            continuity &= entry->outputMask;
        }
        node->slotsContinue[0] = continuity;

        for (j = 0; j < node->nodeDef->io.numClustersOfInterest; j++) {
            node->inputToClusterSlot[j] = (RwUInt32)-1;
            for (k = 0; k < pipeline->packetNumClusterSlots; k++) {
                if (node->slotClusterRefs[k] != NULL &&
                    node->slotClusterRefs[k]->clusterRef ==
                        node->nodeDef->io.clustersOfInterest[j].clusterDef) {
                    node->inputToClusterSlot[j] = k;
                    break;
                }
            }
        }
    }

    if (numInputRequirements != 0) {
        pipeline->inputRequirements =
            fn_8049B92C(numInputRequirements * sizeof(RxPipelineRequiresCluster));
    }
    numInputRequirements = 0;
    entryNode = pipeline->nodes;
    for (j = 0; j < entryNode->topSortData->req->numReqs; j++) {
        rxReqEntry *entry = ReqGetEntry(entryNode->topSortData->req, j);

        if (entry->rqdOrOpt == rxCLREQ_REQUIRED || entry->rqdOrOpt == rxCLREQ_OPTIONAL) {
            RxPipelineRequiresCluster *requirement =
                &pipeline->inputRequirements[numInputRequirements++];

            requirement->clusterDef = entry->clusterDef;
            requirement->rqdOrOpt = entry->rqdOrOpt;
            requirement->slotIndex = entry->slotIndex;
        }
    }
    pipeline->numInputRequirements = numInputRequirements;
    fn_8049F5B4(pipeline->inputRequirements, pipeline->numInputRequirements,
                sizeof(RxPipelineRequiresCluster), 0, 0, (RwUInt32)-1);
    return 0;
}

RwUInt32 fn_8049E9AC(RxPipeline *pipeline) {
    rxScopeTrace *scopes = NULL;
    RwUInt32 result;

    result = fn_8049D934(pipeline);
    if (result == 0) {
        result = fn_8049DE44(pipeline);
        if (result == 0) {
            result = fn_8049E1A4(pipeline, &scopes);
            if (result == 0) {
                result = fn_8049E438(pipeline, &scopes);
                if (result == 0) {
                    result = fn_8049E54C(pipeline, scopes);
                }
            }
        }
    }
    return result;
}

/* RenderWare pipeline library: editing pipelines. Locking a pipeline expands its nodes into
 * fixed-size slots of its superblock so that nodes can be added and connected; unlocking sorts
 * the nodes, works out which clusters flow where and packs the superblock again. The original
 * file name is unknown. */
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

/* The scratch the dependency pass takes per node: a requirement list of 0x14 bytes and, per
 * cluster, a requirement of 0x24 bytes and 0x10 bytes more (inferred from the dependency pass
 * and the size the superblock is grown to) */
#define rxDEPREQUIREMENTLISTSIZE 0x14
#define rxDEPREQUIREMENTSIZE 0x24
#define rxDEPCLUSTERSCRATCHSIZE 0x10

/* The size of a locked pipeline's superblock: a slot per node for the node, its output array and
 * its top-sort data */
#define rxLOCKEDSUPERBLOCKSIZE()                                                                   \
    (sizeof(RxPipelineNode) * RXPIPELINEGLOBAL(maxNodesPerPipe) +                                  \
     sizeof(RwUInt32) * RXNODEMAXOUTPUTS * RXPIPELINEGLOBAL(maxNodesPerPipe) +                     \
     sizeof(RxPipelineNodeTopSortData) * RXPIPELINEGLOBAL(maxNodesPerPipe))

/* The scratch space of an unlocking pipeline: the free part of its superblock, taken from the top
 * down (fn_8049B8BC) and from the bottom up (fn_8049B92C) */
typedef struct PipelineScratch {
    /* 0x0 */ RwUInt8 *top;
    /* 0x4 */ RwUInt8 *bottom;
} PipelineScratch;
CHECK_SIZEOF(PipelineScratch, 0x8);

PipelineScratch lbl_805EDF58 = {NULL, NULL};

/* The state of the topological sort of an unlocking pipeline's nodes */
typedef struct PipelineTopSortState {
    /* 0x0 */ RxPipeline *pipeline;
    /* 0x4 */ RwUInt32 numSorted; /* the next node slot to fill */
} PipelineTopSortState;
CHECK_SIZEOF(PipelineTopSortState, 0x8);

void *fn_8049B8BC(RwUInt32 size) {
    size = (size + 3) & ~3;
    lbl_805EDF58.top -= size;
    if (lbl_805EDF58.top < lbl_805EDF58.bottom) {
        lbl_805EDF58.top += size;
        RWERROR((E_RW_NOMEM, size));
        return NULL;
    }
    return lbl_805EDF58.top;
}

void *fn_8049B92C(RwUInt32 size) {
    size = (size + 3) & ~3;
    lbl_805EDF58.bottom += size;
    if (lbl_805EDF58.bottom > lbl_805EDF58.top) {
        lbl_805EDF58.bottom -= size;
        RWERROR((E_RW_NOMEM, size));
        return NULL;
    }
    return lbl_805EDF58.bottom - size;
}

RwUInt32 fn_8049B9A4(RxPipeline *pipeline) {
    RwUInt32 previous;
    RwUInt32 next;
    RwUInt32 numClusters;
    RwUInt32 i;
    RwUInt32 j;

    numClusters = 0;
    next = 0;

    /* count the distinct cluster definitions by finding them in increasing address order */
    for (;;) {
        previous = next;
        next = (RwUInt32)-1;
        for (i = 0; i < pipeline->numNodes; i++) {
            RxNodeDefinition *nodeDef = pipeline->nodes[i].nodeDef;

            for (j = 0; j < nodeDef->io.numClustersOfInterest; j++) {
                RwUInt32 clusterDef = (RwUInt32)nodeDef->io.clustersOfInterest[j].clusterDef;

                if (clusterDef > previous && clusterDef < next) {
                    next = clusterDef;
                }
            }
        }
        if (next == (RwUInt32)-1) {
            break;
        }
        numClusters++;
    }
    return numClusters;
}

/* Moves a pipeline's superblock to a new allocation of newSize bytes and rebases the pointers
 * into it */
static inline RwBool PipelineSuperBlockRealloc(RxPipeline *pipeline, RwUInt32 newSize) {
    void *oldSuperBlock = pipeline->superBlock;
    void *newSuperBlock = RwRealloc(
        oldSuperBlock, newSize, rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_GLOBAL | rwID_PIPEMODULE);

    if (newSuperBlock != NULL) {
        RwUInt32 numNodes = pipeline->numNodes;
        RwInt32 offset = (RwUInt8 *)newSuperBlock - (RwUInt8 *)oldSuperBlock;
        RwUInt32 i;

        pipeline->superBlock = newSuperBlock;
        pipeline->superBlockSize = newSize;
        pipeline->nodes = pipeline->superBlock;
        if (pipeline->embeddedPacket != NULL) {
            pipeline->embeddedPacket = (RxPacket *)((RwUInt8 *)pipeline->embeddedPacket + offset);
        }
        if (pipeline->inputRequirements != NULL) {
            pipeline->inputRequirements =
                (RxPipelineRequiresCluster *)((RwUInt8 *)pipeline->inputRequirements + offset);
        }
        for (i = 0; i < numNodes; i++) {
            if (pipeline->nodes[i].outputs != NULL) {
                pipeline->nodes[i].outputs =
                    (RwUInt32 *)((RwUInt8 *)pipeline->nodes[i].outputs + offset);
            }
            if (pipeline->nodes[i].slotClusterRefs != NULL) {
                pipeline->nodes[i].slotClusterRefs =
                    (RxPipelineCluster **)((RwUInt8 *)pipeline->nodes[i].slotClusterRefs + offset);
            }
            if (pipeline->nodes[i].slotsContinue != NULL) {
                pipeline->nodes[i].slotsContinue =
                    (RwUInt32 *)((RwUInt8 *)pipeline->nodes[i].slotsContinue + offset);
            }
            if (pipeline->nodes[i].privateData != NULL) {
                pipeline->nodes[i].privateData = (RwUInt8 *)pipeline->nodes[i].privateData + offset;
            }
            if (pipeline->nodes[i].inputToClusterSlot != NULL) {
                pipeline->nodes[i].inputToClusterSlot =
                    (RwUInt32 *)((RwUInt8 *)pipeline->nodes[i].inputToClusterSlot + offset);
            }
            if (pipeline->nodes[i].topSortData != NULL) {
                pipeline->nodes[i].topSortData =
                    (RxPipelineNodeTopSortData *)((RwUInt8 *)pipeline->nodes[i].topSortData +
                                                  offset);
            }
        }
    } else {
        RWERROR((E_RW_NOMEM, newSize));
        return FALSE;
    }
    return TRUE;
}

/* Gives the nodes of a locked pipeline their output array and top-sort slots, copying the nodes
 * of source first when it is another pipeline; FALSE if an initialisation data copy fails */
RwBool fn_8049BA30(RxPipeline *pipeline, RxPipeline *source) {
    RxPipelineNodeTopSortData *topSortData;
    RwUInt32 *outputs;
    RwInt32 i;

    if (pipeline != source) {
        for (i = source->numNodes - 1; i >= 0; i--) {
            memcpy(&pipeline->nodes[i], &source->nodes[i], sizeof(RxPipelineNode));
            pipeline->nodes[i].slotClusterRefs = NULL;
            pipeline->nodes[i].slotsContinue = NULL;
            pipeline->nodes[i].privateData = NULL;
            pipeline->nodes[i].inputToClusterSlot = NULL;
            if (pipeline->nodes[i].initializationDataSize != 0) {
                pipeline->nodes[i].initializationData =
                    RwMalloc(pipeline->nodes[i].initializationDataSize,
                             rwMEMHINTDUR_EVENT | rwID_PIPEMODULE);
                if (pipeline->nodes[i].initializationData == NULL) {
                    RWERROR((E_RW_NOMEM, pipeline->nodes[i].initializationDataSize));
                    return FALSE;
                }
                memcpy(pipeline->nodes[i].initializationData, source->nodes[i].initializationData,
                       pipeline->nodes[i].initializationDataSize);
            }
        }
        pipeline->numNodes = source->numNodes;
    }

    outputs = (RwUInt32 *)&pipeline->nodes[RXPIPELINEGLOBAL(maxNodesPerPipe)];
    for (i = source->numNodes - 1; i >= 0; i--) {
        pipeline->nodes[i].outputs = &outputs[i * RXNODEMAXOUTPUTS];
        if (source->nodes[i].outputs != NULL) {
            memcpy(pipeline->nodes[i].outputs, source->nodes[i].outputs,
                   RXNODEMAXOUTPUTS * sizeof(RwUInt32));
        }
    }

    topSortData =
        (RxPipelineNodeTopSortData *)&outputs[RXNODEMAXOUTPUTS * RXPIPELINEGLOBAL(maxNodesPerPipe)];
    for (i = 0; (RwUInt32)i < source->numNodes; i++) {
        topSortData[i].numIns = 0;
        topSortData[i].numInsVisited = 0;
        topSortData[i].req = NULL;
        pipeline->nodes[i].topSortData = &topSortData[i];
    }
    return TRUE;
}

/* Creates a node of nodeDef in the next node slot of a locked pipeline; FALSE if the definition
 * has too many outputs or clusters of interest */
RwBool fn_8049BC4C(RxPipeline *pipeline, RxPipelineNode *node, RxNodeDefinition *nodeDef) {
    RwBool result;
    RwUInt32 numOutputs = nodeDef->io.numOutputs;

    result = TRUE;

    memset(node, 0, sizeof(RxPipelineNode));
    if (numOutputs > RXNODEMAXOUTPUTS) {
        RWERROR((E_RX_NODETOOMANYOUTPUTS));
        result = FALSE;
    }
    if (nodeDef->io.numClustersOfInterest > RXNODEMAXCLUSTERSOFINTEREST) {
        RWERROR((E_RX_NODETOOMANYCLUSTERSOFINTEREST));
        result = FALSE;
    }
    /* the output count is the value tested against the pipeline's node limit */
    if (numOutputs >= RXPIPELINEGLOBAL(maxNodesPerPipe)) {
        RWERROR((E_RX_PIPELINETOOMANYNODES));
        result = FALSE;
    }

    if (result) {
        RwUInt32 *outputs;
        RwUInt32 i;
        RxPipelineNodeTopSortData *topSortData;

        /* the node's output array follows the node slots */
        outputs = (RwUInt32 *)(pipeline->nodes + RXPIPELINEGLOBAL(maxNodesPerPipe));
        outputs += pipeline->numNodes * RXNODEMAXOUTPUTS;
        node->outputs = outputs;
        node->numOutputs = numOutputs;
        for (i = 0; i < node->numOutputs; i++) {
            *outputs = (RwUInt32)-1;
            outputs++;
        }

        /* the node's top-sort slot follows the node slots and their output arrays */
        topSortData =
            (RxPipelineNodeTopSortData *)(pipeline->nodes + RXPIPELINEGLOBAL(maxNodesPerPipe));
        topSortData =
            (RxPipelineNodeTopSortData *)((RwUInt32 *)topSortData +
                                          RXPIPELINEGLOBAL(maxNodesPerPipe) * RXNODEMAXOUTPUTS);
        topSortData += pipeline->numNodes;
        topSortData->numIns = 0;
        topSortData->numInsVisited = 0;
        topSortData->req = NULL;
        node->topSortData = topSortData;
        node->initializationData = NULL;
        node->initializationDataSize = 0;
        node->nodeDef = nodeDef;
        pipeline->numNodes++;
    }
    return result;
}

/* Moves node nodeIndex into the next sorted slot, then sorts each node it leads to once all of
 * that node's inputs have been sorted */
void fn_8049BDF0(PipelineTopSortState *state, RwUInt32 nodeIndex) {
    RwUInt32 i = state->numSorted; /* the slot to fill, then the output being followed */
    RxPipelineNode *node;

    if (i != nodeIndex) {
        RwUInt32 *slotOutputs = state->pipeline->nodes[i].outputs;
        RwUInt32 *nodeOutputs = state->pipeline->nodes[nodeIndex].outputs;
        RxPipelineNodeTopSortData *slotTopSortData;
        RxPipelineNodeTopSortData *nodeTopSortData;
        RxPipelineNodeTopSortData topSortTemp;
        RxPipelineNode nodeTemp;
        RwUInt32 k;

        /* swap the two nodes, leaving their output arrays and top-sort data with them */
        for (k = 0; k < RXNODEMAXOUTPUTS; k++) {
            RwUInt32 temp = slotOutputs[k];

            slotOutputs[k] = nodeOutputs[k];
            nodeOutputs[k] = temp;
        }
        state->pipeline->nodes[i].outputs = nodeOutputs;
        state->pipeline->nodes[nodeIndex].outputs = slotOutputs;

        slotTopSortData = state->pipeline->nodes[i].topSortData;
        nodeTopSortData = state->pipeline->nodes[nodeIndex].topSortData;
        topSortTemp = *slotTopSortData;
        *slotTopSortData = *nodeTopSortData;
        *nodeTopSortData = topSortTemp;
        state->pipeline->nodes[i].topSortData = nodeTopSortData;
        state->pipeline->nodes[nodeIndex].topSortData = slotTopSortData;

        nodeTemp = state->pipeline->nodes[i];
        state->pipeline->nodes[i] = state->pipeline->nodes[nodeIndex];
        state->pipeline->nodes[nodeIndex] = nodeTemp;

        /* the outputs that led to either node now lead to the other slot */
        for (k = 0; k < state->pipeline->numNodes; k++) {
            RwUInt32 l;
            RxPipelineNode *other = &state->pipeline->nodes[k];

            for (l = 0; l < other->numOutputs; l++) {
                if (other->outputs[l] == i) {
                    other->outputs[l] = nodeIndex;
                } else if (other->outputs[l] == nodeIndex) {
                    other->outputs[l] = i;
                }
            }
        }
    }

    node = &state->pipeline->nodes[state->numSorted];
    state->numSorted++;
    if (node->numOutputs != 0) {
        for (i = 0; i < node->numOutputs; i++) {
            RwUInt32 output = node->outputs[i];

            if (output != (RwUInt32)-1) {
                RxPipelineNode *target = &state->pipeline->nodes[output];

                target->topSortData->numInsVisited++;
                if (target->topSortData->numIns == target->topSortData->numInsVisited) {
                    fn_8049BDF0(state, output);
                }
            }
        }
    }
}

/* Counts, for each node of an unlocking pipeline, the outputs that lead to it */
static inline void PipelineCountInputs(RxPipeline *pipeline) {
    RxPipelineNode *node;
    RwUInt32 i;

    node = &pipeline->nodes[0];
    for (i = 0; i < pipeline->numNodes; i++) {
        if (node->nodeDef != NULL) {
            node->topSortData->numInsVisited = 0;
            node->topSortData->numIns = 0;
        }
        node++;
    }
    node = &pipeline->nodes[0];
    for (i = 0; i < pipeline->numNodes; i++) {
        if (node->nodeDef != NULL && node->numOutputs != 0) {
            RwUInt32 count = node->numOutputs;
            RwUInt32 *outputs = &node->outputs[0];

            do {
                if ((RwInt32)*outputs != -1) {
                    pipeline->nodes[*outputs].topSortData->numIns++;
                }
                outputs++;
            } while (--count);
        }
        node++;
    }
}

/* Sorts the nodes of an unlocking pipeline so that each comes before the nodes its outputs lead
 * to, starting from the entry point; NULL if the pipeline is fragmented or cyclic */
static inline RxPipeline *PipelineTopSort(RxPipeline *pipeline) {
    PipelineTopSortState state;
    RwUInt32 i;

    state.pipeline = pipeline;
    state.numSorted = 0;

    PipelineCountInputs(pipeline);

    if (pipeline->nodes[pipeline->entryPoint].topSortData->numIns != 0) {
        RWERROR((E_RX_INVALIDENTRYPOINT));
        return NULL;
    }
    for (i = 0; i < pipeline->numNodes; i++) {
        if (i != pipeline->entryPoint && pipeline->nodes[i].topSortData->numIns == 0) {
            RWERROR((E_RX_FRAGMENTEDPIPELINE));
            return NULL;
        }
    }

    fn_8049BDF0(&state, pipeline->entryPoint);
    for (i = 0; i < pipeline->numNodes; i++) {
        if (pipeline->nodes[i].topSortData->numIns !=
            pipeline->nodes[i].topSortData->numInsVisited) {
            RWERROR((E_RX_CYCLICPIPELINE));
            return NULL;
        }
    }
    pipeline->entryPoint = 0;
    return pipeline;
}

/* The size of the nodes of a pipeline and their output arrays once packed; every node is counted
 * with the output count of the first one */
static inline RwUInt32 PipelineCalcPackedSize(RxPipeline *pipeline) {
    RwUInt32 size;
    RwUInt32 i;

    size = pipeline->numNodes * sizeof(RxPipelineNode);
    for (i = 0; i < pipeline->numNodes; i++) {
        size += pipeline->nodes[0].numOutputs * sizeof(RwUInt32);
    }
    return size;
}

/* The size of what unlocking allocates from the bottom of the superblock and keeps: the cluster
 * tables, the per-node slot tables and private data, and the packet */
static inline RwUInt32 PipelineCalcPersistentSize(RxPipeline *pipeline, RwUInt32 numClusters) {
    RwUInt32 size = 0;
    RxPipelineNode *node;
    RwUInt32 i;

    size += numClusters * sizeof(RxPipelineCluster);
    size += numClusters * sizeof(RxPipelineRequiresCluster);
    size += pipeline->numNodes * numClusters * sizeof(RwUInt32);
    size += pipeline->numNodes * (numClusters + 1) * sizeof(RwUInt32);
    for (i = 0; i < pipeline->numNodes; i++) {
        node = &pipeline->nodes[i];
        if (node->nodeDef->pipelineNodePrivateDataSize != 0) {
            size += node->nodeDef->pipelineNodePrivateDataSize;
        }
        size += sizeof(RwUInt32) * node->nodeDef->io.numClustersOfInterest;
    }
    size += sizeof(RxPacket) + sizeof(RxCluster) * (numClusters - 1);
    return size;
}

RxPipeline *fn_8049C1E0(RxPipeline *pipeline) {
    if (pipeline != NULL && pipeline->locked) {
        if (pipeline->numNodes != 0) {
            RxPipelineNodeTopSortData *topSortDst;
            RxPipelineNodeTopSortData *topSortData;
            RwUInt32 numClusters;
            RwUInt32 startSize;
            RwUInt32 endSize;
            RwUInt32 lockedSize;
            RwUInt32 unlockedSize;
            RwUInt32 numOutputsTotal = 0;
            RwUInt32 *outputsDst;
            RwUInt32 *outputs;
            RwBool failed = FALSE;
            RwUInt32 numInitialised = 0;
            RwInt32 i;

            if (pipeline->entryPoint >= pipeline->numNodes ||
                pipeline->nodes[pipeline->entryPoint].nodeDef == NULL) {
                RWERROR((E_RX_INVALIDENTRYPOINT));
                return NULL;
            }

            /* the superblock must hold the locked layout while the nodes are sorted and the
             * unlocked layout afterwards: the nodes and their output arrays, the top-sort data,
             * the dependency pass's scratch, then what the pass allocates from the bottom */
            numClusters = fn_8049B9A4(pipeline);
            lockedSize = rxLOCKEDSUPERBLOCKSIZE();
            unlockedSize = sizeof(RxPipelineNodeTopSortData) * RXPIPELINEGLOBAL(maxNodesPerPipe);
            unlockedSize += PipelineCalcPackedSize(pipeline);
            unlockedSize += pipeline->numNodes * rxDEPREQUIREMENTLISTSIZE +
                            pipeline->numNodes * numClusters * rxDEPREQUIREMENTSIZE +
                            pipeline->numNodes * numClusters * rxDEPCLUSTERSCRATCHSIZE;
            unlockedSize += PipelineCalcPersistentSize(pipeline, numClusters);

            startSize = lockedSize;
            if (unlockedSize > startSize) {
                startSize = unlockedSize;
            }
            if (startSize > pipeline->superBlockSize &&
                !PipelineSuperBlockRealloc(pipeline, startSize)) {
                return NULL;
            }

            lbl_805EDF58.top = (RwUInt8 *)pipeline->superBlock + startSize;
            lbl_805EDF58.bottom = NULL;

            pipeline = PipelineTopSort(pipeline);
            if (pipeline == NULL) {
                return NULL;
            }

            /* move the top-sort data to the end of the superblock and the output arrays, trimmed,
             * behind the nodes */
            outputs = (RwUInt32 *)&pipeline->nodes[RXPIPELINEGLOBAL(maxNodesPerPipe)];
            topSortData =
                (RxPipelineNodeTopSortData *)(outputs +
                                              RXNODEMAXOUTPUTS * RXPIPELINEGLOBAL(maxNodesPerPipe));
            topSortData += pipeline->numNodes - 1;
            topSortDst = (RxPipelineNodeTopSortData *)((RwUInt8 *)pipeline->superBlock + startSize);
            topSortDst--;
            for (i = pipeline->numNodes - 1; i >= 0; i--) {
                memcpy(topSortDst, topSortData, sizeof(RxPipelineNodeTopSortData));
                pipeline->nodes[i].topSortData = topSortDst;
                topSortData--;
                topSortDst--;
            }

            outputsDst = (RwUInt32 *)&pipeline->nodes[pipeline->numNodes];
            for (i = 0; (RwUInt32)i < pipeline->numNodes; i++) {
                if (pipeline->nodes[i].numOutputs == 0) {
                    pipeline->nodes[i].outputs = NULL;
                } else {
                    memcpy(outputsDst, outputs, pipeline->nodes[i].numOutputs * sizeof(RwUInt32));
                    pipeline->nodes[i].outputs = outputsDst;
                }
                outputs += RXNODEMAXOUTPUTS;
                outputsDst += pipeline->nodes[i].numOutputs;
                numOutputsTotal += pipeline->nodes[i].numOutputs;
            }

            lbl_805EDF58.bottom = (RwUInt8 *)&outputs[numOutputsTotal];
            lbl_805EDF58.top = (RwUInt8 *)topSortData;

            if (fn_8049E9AC(pipeline) != 0) {
                return NULL;
            }

            /* give back the unused end of the superblock, then initialise the nodes */
            endSize = lbl_805EDF58.bottom - (RwUInt8 *)pipeline->superBlock;
            if (!PipelineSuperBlockRealloc(pipeline, endSize)) {
                goto relock;
            }
            for (i = 0; (RwUInt32)i < pipeline->numNodes; i++) {
                pipeline->nodes[i].topSortData = NULL;
            }

            for (i = pipeline->numNodes - 1; i >= 0; i--) {
                RxNodeDefinition *nodeDef = pipeline->nodes[i].nodeDef;
                RxPipelineNode *node = &pipeline->nodes[i];

                if (nodeDef->InputPipesCnt++ == 0 && nodeDef->nodeMethods.nodeInit != NULL &&
                    nodeDef->nodeMethods.nodeInit(nodeDef) == FALSE) {
                    numInitialised = (pipeline->numNodes - 1) - i;
                    failed = TRUE;
                    break;
                }
                if (nodeDef->nodeMethods.pipelineNodeInit != NULL &&
                    nodeDef->nodeMethods.pipelineNodeInit(node) == FALSE) {
                    nodeDef->InputPipesCnt--;
                    if (nodeDef->InputPipesCnt == 0 && nodeDef->nodeMethods.nodeTerm != NULL) {
                        nodeDef->nodeMethods.nodeTerm(nodeDef);
                    }
                    numInitialised = (pipeline->numNodes - 1) - i;
                    failed = TRUE;
                    break;
                }
            }

            if (failed == FALSE) {
                for (i = pipeline->numNodes - 1; i >= 0; i--) {
                    RxNodeDefinition *nodeDef = pipeline->nodes[i].nodeDef;
                    RxPipelineNode *node = &pipeline->nodes[i];

                    if (nodeDef->nodeMethods.pipelineNodeConfig != NULL &&
                        nodeDef->nodeMethods.pipelineNodeConfig(node, pipeline) == FALSE) {
                        numInitialised = pipeline->numNodes;
                        failed = TRUE;
                        break;
                    }
                }
            }

            if (failed != FALSE) {
                /* terminate the nodes that were initialised */
                for (i = pipeline->numNodes - numInitialised; (RwUInt32)i < pipeline->numNodes;
                     i++) {
                    RxNodeDefinition *nodeDef = pipeline->nodes[i].nodeDef;
                    RxPipelineNode *node = &pipeline->nodes[i];

                    if (nodeDef->nodeMethods.pipelineNodeTerm != NULL) {
                        nodeDef->nodeMethods.pipelineNodeTerm(node);
                    }
                    nodeDef->InputPipesCnt--;
                    if (nodeDef->InputPipesCnt == 0 && nodeDef->nodeMethods.nodeTerm != NULL) {
                        nodeDef->nodeMethods.nodeTerm(nodeDef);
                    }
                }
                goto relock;
            }
        }

        pipeline->locked = FALSE;
        return pipeline;
    }

    if (pipeline == NULL) {
        RWERROR((E_RW_NULLP));
    } else {
        RWERROR((E_RX_UNLOCKEDPIPE));
    }
    return NULL;

relock:
    /* back to the locked layout */
    fn_8049BA30(pipeline, pipeline);
    return NULL;
}

RxPipeline *fn_8049CCB8(RxPipeline *pipeline) {
    RwUInt32 size;
    RwUInt32 i;

    if (!pipeline->locked) {
        size = rxLOCKEDSUPERBLOCKSIZE();

        if (pipeline->nodes != NULL) {
            if (size > pipeline->superBlockSize && !PipelineSuperBlockRealloc(pipeline, size)) {
                return NULL;
            }
            if (!fn_8049BA30(pipeline, pipeline)) {
                return NULL;
            }
        } else {
            pipeline->superBlock =
                RwMalloc(size, rwMEMHINTFLAG_RESIZABLE | rwMEMHINTDUR_EVENT | rwID_PIPEMODULE);
            if (pipeline->superBlock == NULL) {
                RWERROR((E_RW_NOMEM, size));
                return NULL;
            }
            pipeline->superBlockSize = size;
            pipeline->nodes = pipeline->superBlock;
        }

        /* terminate the nodes while they are edited */
        pipeline->locked = TRUE;
        if (pipeline->nodes != NULL) {
            for (i = 0; i < pipeline->numNodes; i++) {
                const RxNodeMethods *nodeMethods = &pipeline->nodes[i].nodeDef->nodeMethods;

                if (nodeMethods->pipelineNodeTerm != NULL) {
                    nodeMethods->pipelineNodeTerm(&pipeline->nodes[i]);
                }
                if (--pipeline->nodes[i].nodeDef->InputPipesCnt == 0 &&
                    nodeMethods->nodeTerm != NULL) {
                    nodeMethods->nodeTerm(pipeline->nodes[i].nodeDef);
                }
                pipeline->nodes[i].slotClusterRefs = NULL;
            }
        }
    }
    return pipeline;
}

RxPipelineNode *fn_8049CFCC(RxPipeline *pipeline, const RwChar *name, RxPipelineNode *start,
                            RwInt32 *nodeIndex) {
    RwBool valid = FALSE;

    if (pipeline != NULL && name != NULL && pipeline->numNodes != 0) {
        valid = TRUE;
    }

    if (valid) {
        RxPipelineNode *node = pipeline->nodes;
        RwInt32 remaining = pipeline->numNodes;

        /* searches after start when it is given */
        if (start != NULL) {
            while (node != start && remaining > 0) {
                node++;
                remaining--;
            }
            node++;
            remaining--;
        }
        for (; remaining > 0; node++, remaining--) {
            if (node->nodeDef != NULL && rwstrcmp(node->nodeDef->name, name) == 0) {
                if (nodeIndex != NULL) {
                    *nodeIndex = pipeline->numNodes - remaining;
                }
                return node;
            }
        }
    }
    if (nodeIndex != NULL) {
        *nodeIndex = -1;
    }
    return NULL;
}

/* A node's output by index, or NULL if it has no such output */
static inline RwUInt32 *PipelineNodeOutput(RxPipelineNode *node, RwUInt32 index) {
    if (node != NULL && node->nodeDef != NULL && index < node->numOutputs) {
        return &node->outputs[index];
    }
    return NULL;
}

/* A node that can be connected to, or NULL */
static inline RxPipelineNode *PipelineNodeInput(RxPipelineNode *node) {
    if (node != NULL && node->nodeDef != NULL) {
        return node;
    }
    return NULL;
}

RxPipeline *fn_8049D0D0(RxPipeline *pipeline, RwUInt32 *firstIndex, RxNodeDefinition *nodeDef,
                        ...) {
    RwUInt32 first;
    RwUInt32 numDefs;
    RwUInt32 numCreated;
    RxNodeDefinition *def;
    RxPipelineNode *prev;
    va_list args;

    if (pipeline != NULL && pipeline->locked) {
        va_start(args, nodeDef);
        numDefs = 0;
        for (def = nodeDef; def != NULL; def = va_arg(args, RxNodeDefinition *)) {
            numDefs++;
        }
        va_end(args);

        if (numDefs != 0) {
            prev = NULL;
            first = pipeline->numNodes;
            if (first + numDefs > RXPIPELINEGLOBAL(maxNodesPerPipe)) {
                RWERROR((E_RX_PIPELINETOOMANYNODES));
                return NULL;
            }

            /* create the nodes and chain each one's first output to the next */
            va_start(args, nodeDef);
            numCreated = 0;
            for (def = nodeDef; def != NULL; def = va_arg(args, RxNodeDefinition *)) {
                RxPipelineNode *node = &pipeline->nodes[first + numCreated];

                if (!fn_8049BC4C(pipeline, node, def)) {
                    break;
                }
                if (prev != NULL && fn_8049D368(pipeline, PipelineNodeOutput(prev, 0),
                                                PipelineNodeInput(node)) == NULL) {
                    fn_8049B350(node, pipeline);
                    break;
                }
                prev = node;
                numCreated++;
            }
            va_end(args);

            if (numCreated == numDefs) {
                if (firstIndex != NULL) {
                    *firstIndex = first;
                }
                return pipeline;
            }

            /* destroy what was created */
            while (numCreated-- != 0) {
                fn_8049B350(&pipeline->nodes[numCreated + first], pipeline);
            }
        }
    } else if (pipeline == NULL) {
        RWERROR((E_RW_NULLP));
    } else {
        RWERROR((E_RX_UNLOCKEDPIPE));
    }
    return NULL;
}

/* The index of a node of a pipeline, or -1 if it is not one of its nodes */
static inline RwUInt32 PipelineNodeIndex(RxPipeline *pipeline, RxPipelineNode *node) {
    RwUInt32 index = node - pipeline->nodes;

    if (&pipeline->nodes[index] == node && index < pipeline->numNodes) {
        return index;
    }
    return (RwUInt32)-1;
}

/* Connects an unconnected output of a node of a locked pipeline to another of its nodes (inferred
 * name: adds a path); NULL on failure */
RxPipeline *fn_8049D368(RxPipeline *pipeline, RwUInt32 *output, RxPipelineNode *input) {
    if (pipeline != NULL && pipeline->locked && output != NULL && *output == (RwUInt32)-1 &&
        input != NULL && input->nodeDef != NULL) {
        RwUInt32 index = PipelineNodeIndex(pipeline, input);

        if (index != (RwUInt32)-1) {
            *output = index;
            return pipeline;
        }
    }
    return NULL;
}

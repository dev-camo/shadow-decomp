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
        RWERROR((E_RW_NOMEM));
        return NULL;
    }
    return lbl_805EDF58.top;
}

void *fn_8049B92C(RwUInt32 size) {
    size = (size + 3) & ~3;
    lbl_805EDF58.bottom += size;
    if (lbl_805EDF58.bottom > lbl_805EDF58.top) {
        lbl_805EDF58.bottom -= size;
        RWERROR((E_RW_NOMEM));
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
    RwInt32 i;
    RwUInt32 maxNodes;
    RwUInt32 *outputs;
    RwUInt32 *outputArrays;
    RxPipelineNodeTopSortData *topSortData;

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

    maxNodes = RXPIPELINEGLOBAL(maxNodesPerPipe);
    outputArrays = (RwUInt32 *)(pipeline->nodes + maxNodes);
    outputs = outputArrays + (source->numNodes - 1) * RXNODEMAXOUTPUTS;
    for (i = source->numNodes - 1; i >= 0; i--) {
        pipeline->nodes[i].outputs = outputs;
        if (source->nodes[i].outputs != NULL) {
            memcpy(pipeline->nodes[i].outputs, source->nodes[i].outputs,
                   RXNODEMAXOUTPUTS * sizeof(RwUInt32));
        }
        outputs -= RXNODEMAXOUTPUTS;
    }

    topSortData = (RxPipelineNodeTopSortData *)(outputArrays + RXPIPELINEGLOBAL(maxNodesPerPipe) *
                                                                   RXNODEMAXOUTPUTS);
    for (i = 0; (RwUInt32)i < source->numNodes; i++) {
        topSortData->numIns = 0;
        topSortData->numInsVisited = 0;
        topSortData->req = NULL;
        pipeline->nodes[i].topSortData = topSortData;
        topSortData++;
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
    RwUInt32 i;
    RwUInt32 slot = state->numSorted;
    RxPipelineNode *node;

    if (slot != nodeIndex) {
        RwUInt32 *slotOutputs = state->pipeline->nodes[slot].outputs;
        RwUInt32 *nodeOutputs = state->pipeline->nodes[nodeIndex].outputs;
        RxPipelineNodeTopSortData *slotTopSortData;
        RxPipelineNodeTopSortData *nodeTopSortData;
        RxPipelineNodeTopSortData topSortTemp;
        RxPipelineNode nodeTemp;
        RwUInt32 j;

        /* swap the two nodes, leaving their output arrays and top-sort data with them */
        for (j = 0; j < RXNODEMAXOUTPUTS; j++) {
            RwUInt32 temp = slotOutputs[j];

            slotOutputs[j] = nodeOutputs[j];
            nodeOutputs[j] = temp;
        }
        state->pipeline->nodes[slot].outputs = nodeOutputs;
        state->pipeline->nodes[nodeIndex].outputs = slotOutputs;

        slotTopSortData = state->pipeline->nodes[slot].topSortData;
        nodeTopSortData = state->pipeline->nodes[nodeIndex].topSortData;
        topSortTemp = *slotTopSortData;
        *slotTopSortData = *nodeTopSortData;
        *nodeTopSortData = topSortTemp;
        state->pipeline->nodes[slot].topSortData = nodeTopSortData;
        state->pipeline->nodes[nodeIndex].topSortData = slotTopSortData;

        nodeTemp = state->pipeline->nodes[slot];
        state->pipeline->nodes[slot] = state->pipeline->nodes[nodeIndex];
        state->pipeline->nodes[nodeIndex] = nodeTemp;

        /* the outputs that led to either node now lead to the other slot */
        for (i = 0; i < state->pipeline->numNodes; i++) {
            RwUInt32 k;
            RxPipelineNode *other = &state->pipeline->nodes[i];

            for (k = 0; k < other->numOutputs; k++) {
                if (other->outputs[k] == slot) {
                    other->outputs[k] = nodeIndex;
                } else if (other->outputs[k] == nodeIndex) {
                    other->outputs[k] = slot;
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

/* Sorts the nodes of an unlocking pipeline so that each comes before the nodes its outputs lead
 * to, starting from the entry point; NULL if the pipeline is fragmented or cyclic */
static inline RxPipeline *PipelineTopSort(RxPipeline *pipeline) {
    PipelineTopSortState state;
    RwUInt32 i;
    RxPipelineNode *node;

    state.pipeline = pipeline;
    state.numSorted = 0;

    for (i = 0, node = pipeline->nodes; i < pipeline->numNodes; i++, node++) {
        if (node->nodeDef != NULL) {
            node->topSortData->numInsVisited = 0;
            node->topSortData->numIns = 0;
        }
    }
    for (i = 0, node = pipeline->nodes; i < pipeline->numNodes; i++, node++) {
        if (node->nodeDef != NULL && node->numOutputs != 0) {
            RwUInt32 *output = node->outputs;
            RwUInt32 count = node->numOutputs;

            do {
                if ((RwInt32)*output != -1) {
                    pipeline->nodes[*output].topSortData->numIns++;
                }
                output++;
            } while (--count != 0);
        }
    }

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

RxPipeline *fn_8049C1E0(RxPipeline *pipeline) {
    if (pipeline != NULL && pipeline->locked) {
        if (pipeline->numNodes != 0) {
            RwUInt32 numInitialised = 0;
            RwBool failed = FALSE;
            RwUInt32 numOutputsTotal = 0;
            RwUInt32 numClusters;
            RwUInt32 lockedSize;
            RwUInt32 unlockedSize;
            RwUInt32 size;
            RwUInt32 maxNodes;
            RwInt32 i;
            RxPipelineNode *node;

            if (pipeline->entryPoint >= pipeline->numNodes ||
                pipeline->nodes[pipeline->entryPoint].nodeDef == NULL) {
                RWERROR((E_RX_INVALIDENTRYPOINT));
                return NULL;
            }

            /* the superblock must hold the locked layout while the nodes are sorted and the
             * unlocked layout, with its packet and cluster tables, afterwards; the separate
             * products and the order of the two running totals follow the original */
            numClusters = fn_8049B9A4(pipeline);
            maxNodes = RXPIPELINEGLOBAL(maxNodesPerPipe);
            lockedSize = maxNodes * sizeof(RxPipelineNode) +
                         (maxNodes * RXNODEMAXOUTPUTS * sizeof(RwUInt32) +
                          maxNodes * sizeof(RxPipelineNodeTopSortData));
            unlockedSize = pipeline->numNodes * sizeof(RxPipelineNode);
            for (i = 0; (RwUInt32)i < pipeline->numNodes; i++) {
                /* the original counts the first node's outputs for every node */
                unlockedSize += pipeline->nodes->numOutputs * sizeof(RwUInt32);
            }
            unlockedSize = maxNodes * sizeof(RxPipelineNodeTopSortData) + unlockedSize;
            size = numClusters * sizeof(RxPipelineCluster) +
                   numClusters * sizeof(RxPipelineRequiresCluster);
            unlockedSize = pipeline->numNodes * numClusters * rxDEPREQUIREMENTSIZE +
                           pipeline->numNodes * numClusters * rxDEPCLUSTERSCRATCHSIZE +
                           unlockedSize;
            size += pipeline->numNodes * numClusters * sizeof(RwUInt32);
            unlockedSize = pipeline->numNodes * rxDEPREQUIREMENTLISTSIZE + unlockedSize;
            size += pipeline->numNodes * (numClusters + 1) * sizeof(RwUInt32);
            for (i = 0, node = pipeline->nodes; (RwUInt32)i < pipeline->numNodes; i++, node++) {
                if (node->nodeDef->pipelineNodePrivateDataSize != 0) {
                    size += node->nodeDef->pipelineNodePrivateDataSize;
                }
                size += node->nodeDef->io.numClustersOfInterest * sizeof(RwUInt32);
            }
            size = (numClusters - 1) * sizeof(RxCluster) + size + sizeof(RxPacket);
            unlockedSize += size;
            size = lockedSize;
            if (unlockedSize > lockedSize) {
                size = unlockedSize;
            }

            if (size > pipeline->superBlockSize && !PipelineSuperBlockRealloc(pipeline, size)) {
                return NULL;
            }

            lbl_805EDF58.bottom = NULL;
            lbl_805EDF58.top = (RwUInt8 *)pipeline->superBlock + size;

            pipeline = PipelineTopSort(pipeline);
            if (pipeline == NULL) {
                return NULL;
            }

            /* move the top-sort data to the end of the superblock and the output arrays, trimmed,
             * behind the nodes */
            {
                RxPipelineNodeTopSortData *topSortDst;
                RxPipelineNodeTopSortData *topSortSrc;
                RwUInt32 *outputsDst;
                RwUInt32 *outputsSrc;
                RwUInt32 j;

                topSortDst = (RxPipelineNodeTopSortData *)((RwUInt8 *)pipeline->superBlock + size);
                topSortDst--;
                maxNodes = RXPIPELINEGLOBAL(maxNodesPerPipe);
                outputsSrc = (RwUInt32 *)(pipeline->nodes + maxNodes);
                topSortSrc =
                    (RxPipelineNodeTopSortData *)(outputsSrc + maxNodes * RXNODEMAXOUTPUTS);
                topSortSrc += pipeline->numNodes - 1;
                for (i = pipeline->numNodes - 1; i >= 0; i--) {
                    memcpy(topSortDst, topSortSrc, sizeof(RxPipelineNodeTopSortData));
                    pipeline->nodes[i].topSortData = topSortDst;
                    topSortSrc--;
                    topSortDst--;
                }

                outputsDst = (RwUInt32 *)(pipeline->nodes + pipeline->numNodes);
                for (j = 0; j < pipeline->numNodes; j++) {
                    if (pipeline->nodes[j].numOutputs == 0) {
                        pipeline->nodes[j].outputs = NULL;
                    } else {
                        memcpy(outputsDst, outputsSrc,
                               pipeline->nodes[j].numOutputs * sizeof(RwUInt32));
                        pipeline->nodes[j].outputs = outputsDst;
                    }
                    outputsSrc += RXNODEMAXOUTPUTS;
                    numOutputsTotal += pipeline->nodes[j].numOutputs;
                    outputsDst += pipeline->nodes[j].numOutputs;
                }

                lbl_805EDF58.top = (RwUInt8 *)topSortSrc;
                lbl_805EDF58.bottom = (RwUInt8 *)(outputsSrc + numOutputsTotal);
            }

            if (fn_8049E9AC(pipeline) != 0) {
                return NULL;
            }

            /* give back the unused end of the superblock, then initialise the nodes */
            if (!PipelineSuperBlockRealloc(pipeline,
                                           lbl_805EDF58.bottom - (RwUInt8 *)pipeline->superBlock)) {
                goto relock;
            }
            {
                RwUInt32 j;

                for (j = 0; j < pipeline->numNodes; j++) {
                    pipeline->nodes[j].topSortData = NULL;
                }

                for (i = pipeline->numNodes - 1; i >= 0; i--) {
                    RxPipelineNode *initNode = &pipeline->nodes[i];
                    RxNodeDefinition *nodeDef = initNode->nodeDef;

                    if (nodeDef->InputPipesCnt++ == 0 && nodeDef->nodeMethods.nodeInit != NULL &&
                        !nodeDef->nodeMethods.nodeInit(nodeDef)) {
                        failed = TRUE;
                        numInitialised = pipeline->numNodes - 1 - i;
                        break;
                    }
                    if (nodeDef->nodeMethods.pipelineNodeInit != NULL &&
                        !nodeDef->nodeMethods.pipelineNodeInit(initNode)) {
                        nodeDef->InputPipesCnt--;
                        if (nodeDef->InputPipesCnt == 0 && nodeDef->nodeMethods.nodeTerm != NULL) {
                            nodeDef->nodeMethods.nodeTerm(nodeDef);
                        }
                        failed = TRUE;
                        numInitialised = pipeline->numNodes - 1 - i;
                        break;
                    }
                }

                if (!failed) {
                    for (i = pipeline->numNodes - 1; i >= 0; i--) {
                        RxPipelineNodeConfigFn config =
                            pipeline->nodes[i].nodeDef->nodeMethods.pipelineNodeConfig;

                        if (config != NULL && !config(&pipeline->nodes[i], pipeline)) {
                            numInitialised = pipeline->numNodes;
                            failed = TRUE;
                            break;
                        }
                    }
                }

                if (failed) {
                    /* terminate the nodes that were initialised */
                    for (j = pipeline->numNodes - numInitialised; j < pipeline->numNodes; j++) {
                        RxNodeDefinition *nodeDef = pipeline->nodes[j].nodeDef;

                        if (nodeDef->nodeMethods.pipelineNodeTerm != NULL) {
                            nodeDef->nodeMethods.pipelineNodeTerm(&pipeline->nodes[j]);
                        }
                        nodeDef->InputPipesCnt--;
                        if (nodeDef->InputPipesCnt == 0 && nodeDef->nodeMethods.nodeTerm != NULL) {
                            nodeDef->nodeMethods.nodeTerm(nodeDef);
                        }
                    }
                    goto relock;
                }
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
    RwUInt32 maxNodes;
    RwUInt32 size;
    RxNodeDefinition *nodeDef;
    RwUInt32 i;

    if (!pipeline->locked) {
        maxNodes = RXPIPELINEGLOBAL(maxNodesPerPipe);
        size = maxNodes * sizeof(RxPipelineNode) + maxNodes * RXNODEMAXOUTPUTS * sizeof(RwUInt32) +
               maxNodes * sizeof(RxPipelineNodeTopSortData);

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
                nodeDef = pipeline->nodes[i].nodeDef;

                if (nodeDef->nodeMethods.pipelineNodeTerm != NULL) {
                    nodeDef->nodeMethods.pipelineNodeTerm(&pipeline->nodes[i]);
                }
                if (--pipeline->nodes[i].nodeDef->InputPipesCnt == 0 &&
                    nodeDef->nodeMethods.nodeTerm != NULL) {
                    nodeDef->nodeMethods.nodeTerm(pipeline->nodes[i].nodeDef);
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

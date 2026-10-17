/* RenderWare pipeline library: opening and closing the library, and creating, executing and
 * destroying pipelines. The original file name is unknown. */
#include "rw/rwcore.h"
#include <string.h>

#define rwMEMHINTDUR_EVENT 0x30000
#define rwMEMHINTDUR_GLOBAL 0x40000

/* The pipeline module's ID */
#define rwID_PIPEMODULE 0x409

/* The core's error plugin ID (inferred) */
#define rwCOREERRORPLUGIN 1

/* Reports an error of the core; args is the parenthesized error code and its arguments */
#define RWERROR(args)                                                                              \
    do {                                                                                           \
        RwError error_;                                                                            \
                                                                                                   \
        error_.pluginID = rwCOREERRORPLUGIN;                                                       \
        error_.errorCode = fn_8047DC28 args;                                                       \
        fn_8047DB84(&error_);                                                                      \
    } while (0)

RxExecutionContext lbl_805E3260; /* the pipeline being executed */
RwFreeList lbl_805E3278;         /* the storage of the pipeline free list */

RwUInt32 lbl_805EDF48 = 0x1000; /* the size the execution heap is created with */
RwUInt32 lbl_805EDF4C = 64;     /* the most nodes a pipeline can have */
RwInt32 lbl_805EDF50 = 64;      /* pipelines per free list block */
RwInt32 lbl_805EDF54 = 1;       /* free list blocks allocated up front */

RxHeap *lbl_805F169C;
RwBool lbl_805F1698; /* whether the library is open */

/* Closes the pipeline library */
RwBool fn_8049B1F8(void) {
    if (lbl_805F1698) {
        fn_8047FCDC(RXPIPELINEGLOBAL(pipesFreeList));
        RXPIPELINEGLOBAL(pipesFreeList) = NULL;
        fn_8049F0CC(lbl_805F169C);
        lbl_805F169C = NULL;
        lbl_805F1698 = FALSE;
    }
    return TRUE;
}

/* Opens the pipeline library: creates the execution heap and the pipeline free list and sets the
 * default render states */
RwBool fn_8049B258(void) {
    if (!lbl_805F1698) {
        lbl_805F169C = fn_8049F168(lbl_805EDF48);
        if (lbl_805F169C == NULL) {
            return FALSE;
        }

        RXPIPELINEGLOBAL(pipesFreeList) =
            fn_8047FC04(sizeof(RxPipeline), lbl_805EDF50, sizeof(RwUInt32), lbl_805EDF54,
                        &lbl_805E3278, rwMEMHINTDUR_GLOBAL | rwID_PIPEMODULE);
        if (RXPIPELINEGLOBAL(pipesFreeList) == NULL) {
            fn_8049F0CC(lbl_805F169C);
            lbl_805F169C = NULL;
            return FALSE;
        }

        RXPIPELINEGLOBAL(maxNodesPerPipe) = lbl_805EDF4C;
        fn_8049F2A4(&RXPIPELINEGLOBAL(defaultRenderState));
        RXPIPELINEGLOBAL(allPipelines).link.prev = NULL;
        RXPIPELINEGLOBAL(allPipelines).link.next = NULL;
        lbl_805F1698 = TRUE;
        return TRUE;
    }
    return FALSE;
}

RxPipelineNode *fn_8049B350(RxPipelineNode *node, RxPipeline *pipeline) {
    if (!pipeline->locked) {
        /* terminate the pipeline node, and the node definition after its last pipeline */
        if (node->nodeDef->nodeMethods.pipelineNodeTerm != NULL) {
            node->nodeDef->nodeMethods.pipelineNodeTerm(node);
        }
        node->nodeDef->InputPipesCnt--;
        if (node->nodeDef->InputPipesCnt == 0) {
            if (node->nodeDef->nodeMethods.nodeTerm != NULL) {
                node->nodeDef->nodeMethods.nodeTerm(node->nodeDef);
            }
            if (node->nodeDef->editable) {
                RwFree(node->nodeDef);
                node->nodeDef = NULL;
            }
        }
        if (node->initializationData != NULL) {
            RwFree(node->initializationData);
            node->initializationData = NULL;
            node->initializationDataSize = 0;
        }
        memset(node, 0, sizeof(RxPipelineNode));
    } else {
        RwInt32 index;
        RwUInt32 i;

        if (node->initializationData != NULL) {
            RwFree(node->initializationData);
            node->initializationData = NULL;
            node->initializationDataSize = 0;
        }
        if (node->nodeDef->InputPipesCnt == 0 && node->nodeDef->editable) {
            RwFree(node->nodeDef);
            node->nodeDef = NULL;
        }

        /* close the gap in the node array and in the output and top-sort arrays behind it */
        index = node - pipeline->nodes;
        if (index < pipeline->numNodes - 1) {
            RwUInt32 maxNodes = RXPIPELINEGLOBAL(maxNodesPerPipe);
            RwUInt32 *outputs = (RwUInt32 *)(pipeline->nodes + maxNodes) + index * RXNODEMAXOUTPUTS;
            RwUInt32 *nextOutputs = outputs + RXNODEMAXOUTPUTS;
            RxPipelineNodeTopSortData *topSortData;
            RxPipelineNodeTopSortData *nextTopSortData;

            for (i = index; i < pipeline->numNodes - 1; i++) {
                memcpy(outputs, nextOutputs, RXNODEMAXOUTPUTS * sizeof(RwUInt32));
                outputs = nextOutputs;
                nextOutputs += RXNODEMAXOUTPUTS;
            }

            /* the top-sort entries move from the first one on, not from the node's own */
            maxNodes = RXPIPELINEGLOBAL(maxNodesPerPipe);
            topSortData = (RxPipelineNodeTopSortData *)((RwUInt32 *)(pipeline->nodes + maxNodes) +
                                                        maxNodes * RXNODEMAXOUTPUTS);
            nextTopSortData = topSortData + 1;
            for (i = index; i < pipeline->numNodes - 1; i++) {
                memcpy(topSortData, nextTopSortData, sizeof(RxPipelineNodeTopSortData));
                topSortData = nextTopSortData;
                nextTopSortData++;
            }

            for (i = index; i < pipeline->numNodes - 1; i++) {
                memcpy(&pipeline->nodes[i], &pipeline->nodes[i + 1], sizeof(RxPipelineNode));
                pipeline->nodes[i].outputs -= RXNODEMAXOUTPUTS;
                pipeline->nodes[i].topSortData--;
            }

            /* outputs to the removed node now lead nowhere; later nodes moved down one */
            for (i = 0; i < pipeline->numNodes - 1; i++) {
                RwUInt32 output;
                RwUInt32 j;

                for (j = 0; j < pipeline->nodes[i].numOutputs; j++) {
                    output = pipeline->nodes[i].outputs[j];

                    if (output >= index) {
                        if (index == output) {
                            pipeline->nodes[i].outputs[j] = (RwUInt32)-1;
                        } else {
                            pipeline->nodes[i].outputs[j] = output - 1;
                        }
                    }
                }
            }
        }
    }
    pipeline->numNodes--;
    return node;
}

RxHeap *fn_8049B66C(void) {
    return lbl_805F169C;
}

RxPipeline *fn_8049B674(RxPipeline *pipeline, void *data, RwBool heapReset) {
    RwUInt32 exitCode;

    if (heapReset) {
        RxHeapReset(lbl_805F169C);
    }

    lbl_805E3260.exitCode = TRUE;
    lbl_805E3260.pipeline = pipeline;
    lbl_805E3260.params.dataParam = data;
    lbl_805E3260.params.heap = lbl_805F169C;
    pipeline->embeddedPacketState = rxPKST_PACKETLESS;
    exitCode =
        pipeline->nodes[0].nodeDef->nodeMethods.nodeBody(&pipeline->nodes[0], &lbl_805E3260.params);
    if (exitCode == FALSE) {
        lbl_805E3260.exitCode = exitCode;
    }

    /* destroy a packet the nodes left behind */
    if (pipeline->embeddedPacketState > rxPKST_UNUSED) {
        pipeline->embeddedPacketState = rxPKST_INUSE;
        fn_8049B14C(pipeline->embeddedPacket);
    }

    lbl_805E3260.params.dataParam = NULL;
    lbl_805E3260.pipeline = NULL;
    lbl_805E3260.params.heap = NULL;
    if (lbl_805E3260.exitCode) {
        return pipeline;
    }
    return NULL;
}

RxPipeline *fn_8049B76C(void) {
    RxPipeline *pipeline =
        RwFreeListAlloc(RXPIPELINEGLOBAL(pipesFreeList), rwMEMHINTDUR_EVENT | rwID_PIPEMODULE);

    if (pipeline != NULL) {
        memset(pipeline, 0, sizeof(RxPipeline));
        pipeline->locked = FALSE;
        return pipeline;
    }
    RWERROR((E_RW_NOMEM, sizeof(RxPipeline)));
    return NULL;
}

void fn_8049B800(RxPipeline *pipeline) {
    if (pipeline != NULL) {
        RwUInt32 numNodes;
        RwUInt32 i;
        RxPipelineNode *node;

        node = pipeline->nodes;
        numNodes = pipeline->numNodes;
        for (i = 0; i < numNodes; i++) {
            fn_8049B350(node, pipeline);
            node++;
        }
        pipeline->nodes = NULL;
        if (pipeline->superBlock != NULL) {
            RwFree(pipeline->superBlock);
            pipeline->superBlock = NULL;
            pipeline->superBlockSize = 0;
        }
        RwFreeListFree(RXPIPELINEGLOBAL(pipesFreeList), pipeline);
    }
}

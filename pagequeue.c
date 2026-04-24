/** pagequeue.c
 * ===========================================================
 * Name: Ben Wong, _2027
 * Section: CS483 / M3
 * Project: PEX3 - Page Replacement Simulator
 * Purpose: Implementation of the PageQueue ADT — a doubly-linked
 *          list for LRU page replacement.
 *          Head = LRU (eviction end), Tail = MRU end.
 * =========================================================== */
#include <stdio.h>
#include <stdlib.h>

#include "pagequeue.h"

/**
 * @brief Create and initialize a page queue with a given capacity
 */
PageQueue *pqInit(unsigned int maxSize) {
    // TODO: malloc a PageQueue, set head and tail to NULL,
    //       size to 0, maxSize to maxSize, and return the pointer
    PageQueue *pq = (PageQueue *) malloc(sizeof(PageQueue));
    if (pq == NULL) {
        return NULL;
    }

    pq->head = NULL;
    pq->tail = NULL;
    pq->size = 0;
    pq->maxSize = maxSize;

    return pq;
}

/**
 * @brief Access a page in the queue (simulates a memory reference)
 */
long pqAccess(PageQueue *pq, unsigned long pageNum) {
    // TODO: Search the queue for pageNum (suggest searching tail->head
    //       so you naturally count depth from the MRU end).
    //
    // HIT path (page found at depth d):
    //   - Remove the node from its current position and re-insert
    //     it at the tail (most recently used).
    //   - Return d.
    //
    // MISS path (page not found):
    //   - Allocate a new node for pageNum and insert it at the tail.
    //   - If size now exceeds maxSize, evict the head node (free it).
    //   - Return -1.
    long depth = 0;
    int found = 0;
    PqNode *curr = pq->tail;

    while (curr != NULL && found == 0) {
        if (curr->pageNum == pageNum) {
            found = 1;
        } else {
            depth++;
            curr = curr->prev;
        }
    }

    if (found == 1) {
        if (curr == pq->tail) {
            return depth;
        }

        PqNode *before = curr->prev;
        PqNode *after = curr->next;

        if (before != NULL) {
            before->next = after;
        } else {
            pq->head = after;
        }

        if (after != NULL) {
            after->prev = before;
        } else {
            pq->tail = before;
        }

        curr->prev = pq->tail;
        curr->next = NULL;
        if (pq->tail != NULL) {
            pq->tail->next = curr;
        } else {
            pq->head = curr;
        }
        pq->tail = curr;

        return depth;
    }

    PqNode *newNode = (PqNode *) malloc(sizeof(PqNode));
    if (newNode == NULL) {
        return -1;
    }

    newNode->pageNum = pageNum;
    newNode->next = NULL;
    newNode->prev = NULL;

    if (pq->tail == NULL) {
        pq->head = newNode;
        pq->tail = newNode;
    } else {
        newNode->prev = pq->tail;
        pq->tail->next = newNode;
        pq->tail = newNode;
    }

    pq->size = pq->size + 1;

    if (pq->size > pq->maxSize) {
        PqNode *victim = pq->head;
        if (victim != NULL) {
            PqNode *nextHead = victim->next;
            pq->head = nextHead;
            if (nextHead != NULL) {
                nextHead->prev = NULL;
            } else {
                pq->tail = NULL;
            }
            free(victim);
            pq->size = pq->size - 1;
        }
    }

    return -1;
}

/**
 * @brief Free all nodes in the queue and reset it to empty
 */
void pqFree(PageQueue *pq) {
    // TODO: Walk from head to tail, free each node, then free
    //       the PageQueue struct itself.
    if (pq == NULL) {
        return;
    }

    PqNode *curr = pq->head;
    while (curr != NULL) {
        PqNode *next = curr->next;
        free(curr);
        curr = next;
    }

    free(pq);
}

/**
 * @brief Print queue contents to stderr for debugging
 */
void pqPrint(PageQueue *pq) {
    // TODO (optional): Print each page number from head to tail,
    //                  marking which is head and which is tail.
    //                  Useful for desk-checking small traces.
}

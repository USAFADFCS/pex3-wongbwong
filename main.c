/** main.c
 * ===========================================================
 * Name: Benjamin Wong, 2027
 * Section: CS483 / M3
 * Project: PEX3 - Page Replacement Simulator
 * Purpose: Reads a BYU binary memory trace file and simulates
 *          LRU page replacement to measure fault rates across
 *          varying frame allocations.
 * Documentation: linux man pages
 * =========================================================== */
#include <stdio.h>
#include <stdlib.h>
#include "byutr.h"
#include "pagequeue.h"

#define PROGRESS_INTERVAL 100000  // print status every N accesses

int main(int argc, char **argv) {
    FILE *ifp = NULL;
    unsigned long numAccesses = 0;
    p2AddrTr traceRecord;

    // Validate command-line arguments: trace_file frame_size
    if (argc != 3) {
        fprintf(stderr, "usage: %s input_byutr_file frame_size\n", argv[0]);
        fprintf(stderr, "\nframe_size:\n\t1: 512 bytes\n\t2: 1KB\n\t3: 2KB\n\t4: 4KB\n");
        exit(1);
    }

    // Open the binary trace file
    ifp = fopen(argv[1], "rb");
    if (ifp == NULL) {
        fprintf(stderr, "cannot open %s for reading\n", argv[1]);
        exit(1);
    }

    // Parse and validate frame size selection
    int menuOption = atoi(argv[2]);
    if (menuOption < 1 || menuOption > 4) {
        fprintf(stderr, "invalid frame size option: %s (must be 1-4)\n", argv[2]);
        fclose(ifp);
        exit(1);
    }

    // Map menu option to page geometry
    int offsetBits = 0;
    int maxFrames = 0;
    switch (menuOption) {
        case 1:
            offsetBits = 9;   // 512-byte pages
            maxFrames = 8192;
            break;
        case 2:
            offsetBits = 10;  // 1KB pages
            maxFrames = 4096;
            break;
        case 3:
            offsetBits = 11;  // 2KB pages
            maxFrames = 2048;
            break;
        case 4:
            offsetBits = 12;  // 4KB pages
            maxFrames = 1024;
            break;
    }

    fprintf(stderr, "Frame size option %d: %d offset bits, %d max frames, algorithm=LRU\n",
            menuOption, offsetBits, maxFrames);

    // TODO: Create your PageQueue (call pqInit, which returns a pointer)
    //       and allocate the faults[] array.  faults[f] will hold the
    //       total number of page faults that occur when f frames are
    //       available.  Use calloc so all entries start at zero.
    PageQueue *pq = pqInit((unsigned int) maxFrames);
    unsigned long *faults = calloc((size_t) (maxFrames + 1), sizeof(unsigned long));
    if (pq == NULL || faults == NULL) {
        fprintf(stderr, "failed memory allocation \n");
        if (pq != NULL) {
            pqFree(pq);
        }
        if (faults != NULL) {
            free(faults);
        }
        fclose(ifp);
        exit(1);
    }

    // Process each memory access from the trace file
    while (!feof(ifp)) {
        fread(&traceRecord, sizeof(p2AddrTr), 1, ifp);

        // Extract page number by shifting off the offset bits
        unsigned long pageNum = traceRecord.addr >> offsetBits;
        numAccesses++;

        // Print progress indicator to stderr every PROGRESS_INTERVAL accesses
        // (also prints the last page number seen — useful for early debugging)
        if ((numAccesses % PROGRESS_INTERVAL) == 0) {
            fprintf(stderr, "%lu samples read, last page: %lu\r", numAccesses, pageNum);
        }


        // TODO: Call pqAccess() to simulate this memory reference.
        //       It returns:
        //         -1      -> page was NOT in the queue (fault for ALL frame counts)
        //         d >= 0  -> page was at depth d from the MRU end
        //                    (fault for any allocation with fewer than d+1 frames)
        //
        //       Update faults[] accordingly.
        long depth = pqAccess(pq, pageNum);
        if (depth < 0) {
            int frameCount = 1;
            while (frameCount <= maxFrames) {
                unsigned long oldValue = faults[frameCount];
                unsigned long newValue = oldValue + 1;
                faults[frameCount] = newValue;
                frameCount++;
            }
        } else {
            int frameCount = 1;
            while (frameCount <= maxFrames) {
                if (frameCount <= depth) {
                    unsigned long oldValue = faults[frameCount];
                    unsigned long newValue = oldValue + 1;
                    faults[frameCount] = newValue;
                }
                frameCount++;
            }
        }
    }

    fprintf(stderr, "\n%lu total accesses processed\n", numAccesses);

    // Output CSV results to stdout (redirect with > to create a .csv file)
    printf("Total Accesses:,%lu\n", numAccesses);
    printf("Frames,Missees,Miss Rate\n");

    // TODO: Loop from frame count 1 to maxFrames and print each row:
    //       printf("%d,%lu,%f\n", frameCount, faults[frameCount],
    //              (double)faults[frameCount] / (double)numAccesses);
    int frameCount = 1;
    while (frameCount <= maxFrames) {
        printf("%d,%lu,%f\n", frameCount, faults[frameCount],
               (double) faults[frameCount] / (double) numAccesses);
        frameCount++;
    }

    // TODO: Free your PageQueue and the faults[] array,
    //       then close the file.
    pqFree(pq);
    free(faults);
    fclose(ifp);

    return 0;
}

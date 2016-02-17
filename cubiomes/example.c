/**
 * example.c
 *
 * Author: Cubitect
 * Created on: 17 Feb 2016
 * Licence: GPLv3
 */

#include <stdio.h>

#include "finders.h"

/**
 * An example using the getQuadHutSeedList() function to print out a list of quad witch hut seeds.
 */
int main(int argc, char **argv)
{
    long qhseeds[0x10000];
    int regionRange = 3, quality = 2;
    long baseSeed = 0, totSeedsFound = 0;
    Pos p[4];
    int len, i;

    initBiomes();

    if(argc > 1)
        sscanf(argv[1], "%ld", &baseSeed);
    if(argc > 2)
        sscanf(argv[2], "%d", &regionRange);
    if(argc > 3)
        sscanf(argv[3], "%d", &quality);

    printf("Starting baseSeed: %ld   Range: %d   Allowed border chunks: %d\n", baseSeed, regionRange*512, quality);

    // Spam stdout with quad witch hut seeds ;P
    for(; totSeedsFound < (1L<<48); baseSeed++)
    {
    	// Find the next bunch of quad-witch-hut seeds. May take a few seconds for every call.
        baseSeed = getQuadHutSeedList(qhseeds, &len, p, baseSeed, regionRange, quality);

        fprintf(stdout, "The following seeds have a quad witch hut with coordinates:\n");
        for(i = 0; i < 4; i++)
        	fprintf(stdout, "(x=%d,z=%d)\n", p[i].x, p[i].z);

        for(i = 0; i < len; i++)
        {
            fprintf(stdout, "%ld\n", qhseeds[i]);
        }

        totSeedsFound += len;
    }

    return 0;
}


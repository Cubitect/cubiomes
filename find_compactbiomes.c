/**
 * This example program finds seeds that contain all major biomes within 1024
 * blocks of the origin. These seeds are very rare, with only about 1 in every
 * 100 million. For a speed reference: on my 4GHz CPU it takes about 9 minutes
 * to check 1E9 seeds using a single thread.
 */

#include "finders.h"
#include "generator.h"

#define SEED_BUF_LEN 0x10000


struct compactinfo_t
{
    long seedStart, seedEnd;
};

void *searchCompactBiomesThread(void *data)
{
    struct compactinfo_t info = *(struct compactinfo_t *)data;

    long *seeds = (long *) malloc(sizeof(*seeds)*SEED_BUF_LEN);
    long i, s, scnt;

    for(s = info.seedStart; s < info.seedEnd; s += SEED_BUF_LEN)
    {
        if(s + SEED_BUF_LEN > info.seedEnd)
            scnt = info.seedEnd - s;
        else
            scnt = SEED_BUF_LEN;


        for(i = 0; i < scnt; i++)
        {
            seeds[i] = s + i;
        }

        scnt = filterAllTempCats(seeds, seeds, scnt, 0, 0);

        // The biomes really shouldn't be further out than 1024 blocks.
        scnt = filterAllMajorBiomes(seeds, seeds, scnt, -4, -4, 8, 8);

        for(i = 0; i < scnt; i++)
        {
            printf("%ld\n", seeds[i]);
        }
        fflush(stdout);
    }

    free(seeds);

    return NULL;
}


int main(int argc, char *argv[])
{
    initBiomes();

    long seedStart, seedEnd;
    uint threads, t;

    if(argc > 3)
    {
        if(sscanf(argv[1], "%ld", &seedStart) != 1) seedStart = 0;
        if(sscanf(argv[2], "%ld", &seedEnd) != 1) seedEnd = 1000000000L;
        if(sscanf(argv[3], "%u", &threads) != 1) threads = 1;
    }

    pthread_t threadID[threads];
    struct compactinfo_t info[threads];

    for(t = 0; t < threads; t++)
    {
        long seedCnt = (seedEnd - seedStart) / threads;
        info[t].seedStart = seedStart + seedCnt * t;
        info[t].seedEnd = seedStart + seedCnt * (t+1) + 1;
    }
    info[threads-1].seedEnd = seedEnd;

    for(t = 0; t < threads; t++)
    {
        pthread_create(&threadID[t], NULL, searchCompactBiomesThread, (void*)&info[t]);
    }

    for(t = 0; t < threads; t++)
    {
        pthread_join(threadID[t], NULL);
    }

    return 0;
}







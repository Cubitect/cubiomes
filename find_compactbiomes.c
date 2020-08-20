#include "finders.h"
#include "generator.h"


struct compactinfo_t
{
    int64_t seedStart, seedEnd;
    unsigned int range;
    BiomeFilter filter;
    int withHut, withMonument;
    int minscale;
};


#ifdef USE_PTHREAD
static void *searchCompactBiomesThread(void *data)
#else
static DWORD WINAPI searchCompactBiomesThread(LPVOID data)
#endif
{
    struct compactinfo_t info = *(struct compactinfo_t *)data;
    int ax = -info.range, az = -info.range;
    int w = 2*info.range, h = 2*info.range;
    int64_t s;

    int mcversion = MC_1_14;
    LayerStack g;
    setupGenerator(&g, mcversion);
    int *cache = allocCache(&g.layers[L_VORONOI_ZOOM_1], w, h);

    for (s = info.seedStart; s != info.seedEnd; s++)
    {
        if (!hasAllTemps(&g, s, 0, 0))
            continue;

        if (checkForBiomes(&g, L_VORONOI_ZOOM_1, cache, s, ax, az, w, h, info.filter, 1) > 0)
        {
            int x, z;
            if (info.withHut)
            {
                int r = info.range / SWAMP_HUT_CONFIG.regionSize;
                for (z = -r; z < r; z++)
                {
                    for (x = -r; x < r; x++)
                    {
                        Pos p;
                        p = getStructurePos(SWAMP_HUT_CONFIG, s, x, z, NULL);
                        if (isViableStructurePos(Swamp_Hut, mcversion, &g, s, p.x, p.z))
                            goto L_hut_found;
                    }
                }
                continue;
                L_hut_found:;
            }
            if (info.withMonument)
            {
                int r = info.range / MONUMENT_CONFIG.regionSize;
                for (z = -r; z < r; z++)
                {
                    for (x = -r; x < r; x++)
                    {
                        Pos p;
                        p = getStructurePos(MONUMENT_CONFIG, s, x, z, NULL);
                        if (isViableStructurePos(Monument, mcversion, &g, s, p.x, p.z))
                            goto L_monument_found;
                    }
                }
                continue;
                L_monument_found:;
            }

            printf("%" PRId64 "\n", s);
            fflush(stdout);
        }
    }

    free(cache);

#ifdef USE_PTHREAD
    pthread_exit(NULL);
#endif
    return 0;
}


int main(int argc, char *argv[])
{
    initBiomes();

    int64_t seedStart, seedEnd;
    unsigned int threads, t, range;
    BiomeFilter filter;
    int withHut, withMonument;
    int minscale;

    // arguments
    if (argc <= 1)
    {
        printf( "find_compactbiomes [seed_start] [seed_end] [threads] [range]\n"
                "\n"
                "  seed_start    starting seed for search [long, default=0]\n"
                "  end_start     end seed for search [long, default=-1]\n"
                "  threads       number of threads to use [uint, default=1]\n"
                "  range         search range (in blocks) [uint, default=1024]\n");
        exit(1);
    }
    if (argc <= 1 || sscanf(argv[1], "%" PRId64, &seedStart) != 1) seedStart = 0;
    if (argc <= 2 || sscanf(argv[2], "%" PRId64, &seedEnd) != 1) seedEnd = -1;
    if (argc <= 3 || sscanf(argv[3], "%u", &threads) != 1) threads = 1;
    if (argc <= 4 || sscanf(argv[4], "%u", &range) != 1) range = 1024;

    // TODO: set up a customisable biome filter
    filter = setupBiomeFilter(BIOMES_L13_OCEAN_MIX_4,
                sizeof(BIOMES_L13_OCEAN_MIX_4)/sizeof(int));
    minscale = 1; // terminate search at this layer scale
    // TODO: simple structure filter
    withHut = 0;
    withMonument = 0;

    printf("Starting search through seeds %" PRId64 " to %" PRId64", using %u threads.\n"
           "Search radius = %u.\n", seedStart, seedEnd, threads, range);

    thread_id_t threadID[threads];
    struct compactinfo_t info[threads];

    // store thread information
    uint64_t seedCnt = ((uint64_t)seedEnd - (uint64_t)seedStart) / threads;
    for (t = 0; t < threads; t++)
    {
        info[t].seedStart = (int64_t)(seedStart + seedCnt * t);
        info[t].seedEnd = (int64_t)(seedStart + seedCnt * (t+1));
        info[t].range = range;
        info[t].filter = filter;
        info[t].withHut = withHut;
        info[t].withMonument = withMonument;
        info[t].minscale = minscale;
    }
    info[threads-1].seedEnd = seedEnd;

    // start threads
#ifdef USE_PTHREAD

    for (t = 0; t < threads; t++)
    {
        pthread_create(&threadID[t], NULL, searchCompactBiomesThread, (void*)&info[t]);
    }

    for (t = 0; t < threads; t++)
    {
        pthread_join(threadID[t], NULL);
    }

#else

    for (t = 0; t < threads; t++)
    {
        threadID[t] = CreateThread(NULL, 0, searchCompactBiomesThread, (LPVOID)&info[t], 0, NULL);
    }

    WaitForMultipleObjects(threads, threadID, TRUE, INFINITE);

#endif

    return 0;
}







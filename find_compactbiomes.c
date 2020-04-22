#include "finders.h"
#include "generator.h"


struct compactinfo_t
{
    int64_t seedStart, seedEnd;
    unsigned int range;
    BiomeFilter filter;
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

    LayerStack g = setupGenerator(MC_1_14);
    int *cache = allocCache(&g.layers[L_VORONOI_ZOOM_1], w, h);

    for (s = info.seedStart; s < info.seedEnd; s++)
    {
        if (checkForBiomes(&g, cache, s, ax, az, w, h, info.filter, 1))
        {
            int x, z;
            int has_hut = 0, has_monument = 0;
            for (z = -2; z < 2; z++)
            {
                for (x = -2; x < 2; x++)
                {
                    Pos p;
                    p = getStructurePos(SWAMP_HUT_CONFIG, s, x, z);
                    if (isViableFeaturePos(Swamp_Hut, g, cache, p.x, p.z))
                        has_hut = 1;
                    p = getLargeStructurePos(MONUMENT_CONFIG, s, x, z);
                    if (isViableOceanMonumentPos(g, cache, p.x, p.z))
                        has_monument = 1;
                }
            }

            if (!has_hut || !has_monument)
                continue;

            printf("%ld\n", s);
            fflush(stdout);
        }
    }

    freeGenerator(g);
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

    // arguments
    if (argc <= 0)
    {
        printf( "find_compactbiomes [seed_start] [seed_end] [threads] [range]\n"
                "\n"
                "  seed_start    starting seed for search [long, default=0]\n"
                "  end_start     end seed for search [long, default=1e8]\n"
                "  threads       number of threads to use [uint, default=1]\n"
                "  range         search range (in blocks) [uint, default=1024]\n");
        exit(1);
    }
    if (argc <= 1 || sscanf(argv[1], "%" PRId64, &seedStart) != 1) seedStart = 0;
    if (argc <= 2 || sscanf(argv[2], "%" PRId64, &seedEnd) != 1) seedEnd = 100000000LL;
    if (argc <= 3 || sscanf(argv[3], "%u", &threads) != 1) threads = 1;
    if (argc <= 4 || sscanf(argv[4], "%u", &range) != 1) range = 1024;

    // TODO: set up a customisable biome filter
    filter = setupBiomeFilter(BIOMES_L13_OCEAN_MIX_4,
                sizeof(BIOMES_L13_OCEAN_MIX_4)/sizeof(int));

    printf("Starting search through seeds %" PRId64 " to %" PRId64", using %u threads.\n"
           "Search radius = %u.\n", seedStart, seedEnd, threads, range);

    thread_id_t threadID[threads];
    struct compactinfo_t info[threads];

    // store thread information
    for (t = 0; t < threads; t++)
    {
        int64_t seedCnt = (seedEnd - seedStart) / threads;
        info[t].seedStart = seedStart + seedCnt * t;
        info[t].seedEnd = seedStart + seedCnt * (t+1) + 1;
        info[t].range = range;
        info[t].filter = filter;
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







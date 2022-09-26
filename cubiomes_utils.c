#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#include "finders.h"
#include "generator.h"
#include "util.h"

char *struct2str(int structType);

/* EXAMPLE CODE
    //Set version and seed
    int mc = MC_1_19;
    uint64_t seed = 686689767;

    //Apply seed and dimension
    Generator g;
    setupGenerator(&g, mc, 0);
    applySeed(&g, DIM_OVERWORLD, seed);

    //Center point to generate from, this example being X: 0, Z: 0 in the overworld. 
    int x = 0, z = 0;

    //The size of the map. Will generate from the center of the x/z coordinates.
    int width = 120*8, height = 120*8; //960x960

    //How many pixels per biome block. This example generates a 960x960 image. 
    //If the pixelScale was 4, it would be 3840x3840.
    int pixelScale = 1;

    //Scale of the map, 1 being the smallest.
    int scale = 1;

    //Depth (Y) of the biome generation.
    int depth = 100;

    //Generates a .ppm file with the given params.
    createMap(g, width, height, scale, depth, x, z, pixelScale);
*/ 
void createMap(Generator g, int sizeX, int sizeY, int scale, int depth, int x, int y, int pixelScale)
{
    Range r;
    r.scale = scale;
    r.sx = sizeX, r.sz = sizeY;
    r.x = x - (sizeX / 2), r.z = y - (sizeY / 2);
    r.y = depth, r.sy = 1;

    int* biomeIds = allocCache(&g, r);
    genBiomes(&g, biomeIds, r);

    int imgWidth = pixelScale * r.sx, imgHeight = pixelScale * r.sz;
    unsigned char biomeColors[256][3];
    initBiomeColors(biomeColors);
    unsigned char *rgb = (unsigned char*)malloc(3 * imgWidth * imgHeight);
    biomesToImage(rgb, biomeColors, biomeIds, r.sx, r.sz, pixelScale, 2);

    char fileName[100];
    sprintf(fileName, "map_%" PRId64 ".ppm", g.seed);

    savePPM(fileName, rgb, imgWidth, imgHeight);
    printf("Image saved");

    free(biomeIds);
    free(rgb);
}

/* EXAMPLE CODE
    //Set version and seed
    int mc = MC_1_19;
    uint64_t seed = 686689767;

    //Apply seed and dimension
    Generator g;
    setupGenerator(&g, mc, 0);
    applySeed(&g, DIM_OVERWORLD, seed);

    //Center point to search from, this example being X: 0, Z: 0. 
    int x = 0, z = 0;

    //The amount of strongholds you want to search for.
    int limit = 5;

    //This method returns a string containing the X/Z coordinates, and the distance in blocks it is 
    //from the given X/Z coordinates.
    char *result = findStrongholds(g, x, z, limit);

    //Iterate through the strongholds
    char *token = strtok(result, "\n");

    if(token ==  NULL) 
    {   //Either no strongholds were found, or there was a config error
        printf("Error: %s", result);
    }

    while (token != NULL)
    {   //Prints out the coordinate data and distance
        printf("Stronghold (X,Z,Distance): %s\n", token);
        token = strtok(NULL, "\n");
    }
*/
char *findStrongholds(Generator g, int x, int z, int limit)
{
    char *dest = malloc(sizeof(char) * (limit*100));

    StrongholdIter sh;
    initFirstStronghold(&sh, g.mc, g.seed);
    int i;
    for (i = 1; i <= limit; i++)
    {
        if (nextStronghold(&sh, &g) <= 0) break;

        char str[35];
        //Get the diagonal distance to structure from the given coordinates
        int distance = sqrt(pow((x - sh.pos.x), 2) + pow((z - sh.pos.z), 2));
        
        sprintf(str, "%d,%d,%d\n", sh.pos.x, sh.pos.z, distance);
        strcat(dest, str);
    }
    if ((dest[0] >= '0' && dest[0] <= '9') || dest[0] == '-')
    {
         return dest;
    }
    else
    {
        return dest + 3;
    }
}
    
/* EXAMPLE CODE
    //Set version and seed
    int mc = MC_1_19;
    uint64_t seed = 686689767;

    //Apply seed and dimension
    Generator g;
    setupGenerator(&g, mc, 0);
    applySeed(&g, DIM_OVERWORLD, seed);

    //Center point to search from, this example being X: 0, Z: 0 in the overworld. 
    int x = 0, z = 0;

    //Search range. This example will search in a 2000 block range around the given X/Z coordinates.
    int range = 2000;

    //Structure to search for.
    int structureType = Village;
    
    //This method returns a string containing the X/Z coordinates, and the distance in blocks it is 
    //from the given X/Z coordinates.
    char *result =  findStructures(structureType, g, range, x, z);

    //Iterate through the structures that were found
    char *token = strtok(result, "\n");
    if(token ==  NULL) 
    {   //Either no structures were found, or there was a config error
        printf("Error: %s", result);
    }

    while (token != NULL)
    {   //Prints out the coordinate data and distance
        printf("%s (X,Z,Distance): %s\n", struct2str(structureType), token);
        token = strtok(NULL, "\n");
    }
*/
char *findStructures(int structureType, Generator g, int range, int x, int z)
{
	char *dest = malloc(sizeof(char) * 1000);

    SurfaceNoise sn;
    if (structureType == End_City)
        initSurfaceNoiseEnd(&sn, g.seed);

    StructureConfig sconf;
    if (!getStructureConfig(structureType, g.mc, &sconf))
        return "Config failed, are you sure this structure is supported in this version?";

    int x0 = x - range;
    int z0 = z - range;
    int x1 = x + range;
    int z1 = z + range;

    // segment area into structure regions
    double blocksPerRegion = sconf.regionSize * 16.0;
    int rx0 = (int)floor(x0 / blocksPerRegion);
    int rz0 = (int)floor(z0 / blocksPerRegion);
    int rx1 = (int)ceil(x1 / blocksPerRegion);
    int rz1 = (int)ceil(z1 / blocksPerRegion);
    int i, j;
    for (j = rz0; j <= rz1; j++)
    {
        for (i = rx0; i <= rx1; i++)
        { // check the structure generation attempt in region (i, j)
            Pos pos;
            if (!getStructurePos(structureType, g.mc, g.seed, i, j, &pos))
                continue; // this region is not suitable
            if (pos.x < x0 || pos.x > x1 || pos.z < z0 || pos.z > z1)
                continue; // structure is outside the specified area
            if (!isViableStructurePos(structureType, &g, pos.x, pos.z, 0))
                continue; // biomes are not viable
            if (structureType == End_City)
            { // end cities have a dedicated terrain checker
                if (!isViableEndCityTerrain(&g.en, &sn, pos.x, pos.z))
                    continue;
            }
            else if (g.mc >= MC_1_18)
            { // some structures in 1.18+ depend on the terrain
                if (!isViableStructureTerrain(structureType, &g, pos.x, pos.z))
                    continue;
            }
            char str[80];

            int distance = sqrt(pow((x - pos.x), 2) + pow((z - pos.z), 2));
            sprintf(str, "%d,%d,%d\n", pos.x, pos.z, distance);
            strcat(dest, str);
        }
    }
    if ((dest[0] >= '0' && dest[0] <= '9') || dest[0] == '-')
    {
        return dest;
    }
    else
    {
        return dest + 1;
    }
}

void findBiome(int biome, Generator g, int range, int x, int y, int z){}

/* Structure name to string */
char *struct2str(int structType)
{
    switch (structType)
    {
    case Feature: return "Feature"; break;
    case Desert_Pyramid: return "Desert_Pyramid"; break;
    case Jungle_Temple: return "Jungle_Temple"; break;
    case Swamp_Hut: return "Swamp_Hut"; break;
    case Igloo: return "Igloo"; break;
    case Village: return "Village"; break;
    case Ocean_Ruin: return "Ocean_Ruin"; break;
    case Shipwreck: return "Shipwreck"; break;
    case Monument: return "Monument"; break;
    case Mansion: return "Mansion"; break;
    case Outpost: return "Outpost"; break;
    case Ruined_Portal: return "Ruined_Portal"; break;
    case Ruined_Portal_N: return "Ruined_Portal_N"; break;
    case Ancient_City: return "Ancient_City"; break;
    case Treasure: return "Treasure"; break;
    case Mineshaft: return "Mineshaft"; break;
    case Fortress: return "Fortress"; break;
    case Bastion: return "Bastion"; break;
    case End_City: return "End_City"; break;
    case End_Gateway: return "End_Gateway"; break;
    case FEATURE_NUM: return "FEATURE_NUM"; break;
    default: return "Unknown Structure Type";
    }
    return "-";
}

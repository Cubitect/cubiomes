#include "finders.h"
#include <stdio.h>
#include <math.h>
#include <locale.h>
#include <windows.h>
#include <fstream>
#include <string>
#include <iostream>

#define NUM_STRONGHOLDS 128 // Define the maximum number of strongholds

// Tableau des noms des structures
const char* structureNames[] = {
    "Elément du terrain",
    "Pyramide du désert",
    "Temple de la jungle",
    "Cabane de sorcière",
    "Iglou",
    "Village",
    "Ruine océanique",
    "Epave",
    "Monument sous-marin",
    "Manoir",
    "Avant-poste",
    "Portail en ruine",
    "Portail en ruine (Nether)",
    "Cité ancienne",
    "Trésor",
    "Mine abandonnée",
    "Puit du désert",
    "Géode",
    "Forteresse",
    "Vestige de bastion",
    "Cité de l'End",
    "Passerelle de l'End",
    "Ile de l'End",
    "Ruine de sentier",
    "Chambre des épreuves"
};

// Fonction pour obtenir l'icône en fonction du type de structure
const char* getStructureIcon(int structureType)
{
    switch (structureType)
    {
        case Ancient_City: return "skull";
        case Bastion: return "building";
        case Desert_Pyramid: return "temple";
        case Desert_Well: return "beer";
        case End_City: return "house";
        case End_Gateway: return "portal";
        case End_Island: return "pirateflag";
        case Feature: return "default";
        case Fortress: return "tower";
        case Geode: return "diamond";
        case Igloo: return "walk";
        case Jungle_Temple: return "temple";
        case Mansion: return "bighouse";
        case Mineshaft: return "minecart";
        case Monument: return "temple";
        case Ocean_Ruin: return "tower";
        case Outpost: return "lighthouse";
        case Ruined_Portal: return "portal";
        case Ruined_Portal_N: return "portal";
        case Shipwreck: return "anchor";
        case Swamp_Hut: return "door";
        case Trail_Ruins: return "factory";
        case Treasure: return "chest";
        case Trial_Chambers: return "key";
        case Village: return "house";
        default: return "pin";
    }
}

// Fonction pour lire la seed à partir du fichier server.properties
uint64_t getSeedFromProperties(const std::string& filePath)
{
    std::ifstream file(filePath);
    std::string line;
    uint64_t seed = 0;

    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            if (line.find("level-seed") != std::string::npos)
            {
                size_t pos = line.find("=");
                if (pos != std::string::npos)
                {
                    std::string seedStr = line.substr(pos + 1);
                    seed = std::stoll(seedStr);
                    break;
                }
            }
        }
        file.close();
    }
    else
    {
        std::cerr << "Impossible d'ouvrir le fichier " << filePath << std::endl;
    }

    return seed;
}

// Fonction pour trouver des structures dans une zone spécifiée
void findStructures(FILE *file, int structureType, int mc, int dim, uint64_t seed,
    int x0, int z0, int x1, int z1)
{
    // Initialisation du générateur de biomes
    Generator g;
    setupGenerator(&g, mc, 0);
    applySeed(&g, dim, seed);

    // Configuration de la structure
    StructureConfig sconf;
    if (!getStructureConfig(structureType, mc, &sconf))
        return; // Version ou structure incorrecte

    // Segmentation de la zone en régions de structure
    double blocksPerRegion = sconf.regionSize * 16.0;
    int rx0 = (int) floor(x0 / blocksPerRegion);
    int rz0 = (int) floor(z0 / blocksPerRegion);
    int rx1 = (int) ceil(x1 / blocksPerRegion);
    int rz1 = (int) ceil(z1 / blocksPerRegion);
    int i, j;

    // Parcours des régions de structure
    for (j = rz0; j <= rz1; j++)
    {
        for (i = rx0; i <= rx1; i++)
        {   
            // Vérification de la tentative de génération de structure dans la région (i, j)
            Pos pos;
            if (!getStructurePos(structureType, mc, seed, i, j, &pos))
                continue; // Cette région n'est pas adaptée
            if (pos.x < x0 || pos.x > x1 || pos.z < z0 || pos.z > z1)
                continue; // La structure est en dehors de la zone spécifiée
            if (!isViableStructurePos(structureType, &g, pos.x, pos.z, 0))
                continue; // Les biomes ne sont pas viables
            else if (mc >= MC_1_18)
            {   
                // Certaines structures dans 1.18+ dépendent du terrain
                if (!isViableStructureTerrain(structureType, &g, pos.x, pos.z))
                    continue;
            }
            // Affichage des coordonnées de la structure trouvée
            // printf("Structure %s trouvée à : %d, %d (icon: %s)\n", structureNames[structureType], pos.x, pos.z, getStructureIcon(structureType));
            // Écriture des coordonnées de la structure trouvée dans le fichier CSV
            fprintf(file, "%s,%d,%d,%s\n", structureNames[structureType], pos.x, pos.z, getStructureIcon(structureType));
        }
    }
}

// Fonction pour rechercher les Strongholds
void findStrongholds(FILE *file, int mc, int dim, uint64_t seed)
{
    // Initialisation de l'itérateur pour les Strongholds
    StrongholdIter sh;
    Pos pos;

    // Initialisation du générateur pour les vérifications de biomes
    Generator g;
    setupGenerator(&g, mc, 0);
    applySeed(&g, dim, seed);

    // Initialisation du premier Stronghold
    initFirstStronghold(&sh, mc, seed);

    // Parcours des Strongholds
    do
    {
        // Vérification des biomes et obtention de la position exacte
        if (nextStronghold(&sh, &g))
        {
            pos = sh.pos;
            // Affichage des coordonnées du Stronghold trouvé
            // printf("Stronghold trouvé à : %d, %d (icon: gear)\n", pos.x, pos.z);
            // Écriture des coordonnées du Stronghold dans le fichier CSV
            fprintf(file, "Fort,%d,%d,gear\n", pos.x, pos.z);
        }
    } while (sh.index < NUM_STRONGHOLDS); // Continue jusqu'à ce que tous les Strongholds soient parcourus
}

// Fonction principale
int main()
{
    // Configuration de la locale pour utiliser UTF-8
    setlocale(LC_ALL, "fr_FR.UTF-8");
    // Configuration de la console pour utiliser UTF-8
    SetConsoleOutputCP(CP_UTF8);

    // Chemin vers le fichier server.properties
    std::string filePath = "../../Serveur-Minecraft/server.properties";

    // Lire la seed à partir du fichier server.properties
    uint64_t seed = getSeedFromProperties(filePath);

    // Vérifier si la seed a été correctement lue
    if (seed == 0)
    {
        std::cerr << "Erreur : Seed non trouvée dans le fichier server.properties" << std::endl;
        return 1;
    }

    // Définition du rayon de recherche
    int r = 20000;

    // Variables pour la version de Minecraft et la dimension
    int mcVersion = MC_1_21_3;
    int dimension = DIM_OVERWORLD;

    // Liste des types de structures à rechercher
    int structures[] = {
        Ancient_City,
        // Bastion,
        Desert_Pyramid,
        Desert_Well,
        // End_City,
        // End_Gateway,
        // End_Island,
        // Feature, // for locations of temple generation attempts pre 1.13
        // Fortress,
        Geode,
        Igloo,
        Jungle_Temple,
        Mansion,
        Mineshaft,
        Monument,
        Ocean_Ruin,
        Outpost,
        Ruined_Portal,
        // Ruined_Portal_N,
        Shipwreck,
        Swamp_Hut,
        Trail_Ruins,
        Treasure,
        Trial_Chambers,
        Village
    };

    // Ouverture du fichier CSV pour écrire les résultats
    FILE *file = fopen("export_markers_dynmap.csv", "w");
    if (!file)
    {
        perror("Échec de l'ouverture du fichier");
        return 1;
    }

    // Écriture de l'en-tête du fichier CSV
    fprintf(file, "structureType,x,z,icon\n");

    // Recherche des Strongholds
    printf("Recherche des forts (seed : %lld, version : %d, dimension : %d)...\n", seed, mcVersion, dimension);
    findStrongholds(file, mcVersion, dimension, seed);

    // Recherche des autres structures
    printf("Recherche des structures dans un rayon de %d blocs (seed : %lld, version : %d, dimension : %d)...\n", r, seed, mcVersion, dimension);
    for (int i = 0; i < sizeof(structures) / sizeof(structures[0]); i++)
    {
        findStructures(file, structures[i], mcVersion, dimension, seed, -r, -r, +r, +r);
    }

    // Fermeture du fichier CSV
    fclose(file);

    return 0;
}
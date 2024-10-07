#include "biomenoise.h"

#include "tables/btree18.h"
#include "tables/btree192.h"
#include "tables/btree19.h"
#include "tables/btree20.h"
#include "tables/btree213.h"

BiomeTree g_btree[MC_NEWEST - MC_1_18 + 1] =
{
    // MC_1_18_2
    { btree18_steps, &btree18_param[0][0], btree18_nodes, btree18_order,
        sizeof(btree18_nodes) / sizeof(uint64_t) },
    // MC_1_19_2
    { btree192_steps, &btree192_param[0][0], btree192_nodes, btree192_order,
        sizeof(btree192_nodes) / sizeof(uint64_t) },
    // MC_1_19_4
    { btree19_steps, &btree19_param[0][0], btree19_nodes, btree19_order,
        sizeof(btree19_nodes) / sizeof(uint64_t) },
    // MC_1_20_6
    { btree20_steps, &btree20_param[0][0], btree20_nodes, btree20_order,
        sizeof(btree20_nodes) / sizeof(uint64_t) },
    // MC_1_21_2
    { btree20_steps, &btree20_param[0][0], btree20_nodes, btree20_order,
        sizeof(btree20_nodes) / sizeof(uint64_t) },
    // MC_1_21_3
    { btree213_steps, &btree213_param[0][0], btree213_nodes, btree213_order,
        sizeof(btree213_nodes) / sizeof(uint64_t) },
};


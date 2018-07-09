#!/bin/bash
make

# ===========================================================================
# Basic search
# ===========================================================================
./multifinder -s 7B -e 10B | sort -bn > /tmp/multifinder_basic.txt
echo '[91m'
diff /tmp/multifinder_basic.txt test/golden_basic.txt
echo '(B[m'

# ===========================================================================
# Basic search with threads
# ===========================================================================
mkdir /tmp/multifinder_thread
./multifinder -e 20B -t 4 -o /tmp/multifinder_thread
cat /tmp/multifinder_thread/*.txt | sort -bn > /tmp/multifinder_thread.txt
echo '[91m'
diff /tmp/multifinder_thread.txt test/golden_thread.txt
echo '(B[m'

# ===========================================================================
# All biomes near spawn
# ===========================================================================
./multifinder -s 24B -e 25B --all_biomes | \
    sort -bn > /tmp/multifinder_all.txt
echo '[91m'
diff /tmp/multifinder_all.txt test/golden_all.txt
echo '(B[m'

# ===========================================================================
# Rare spawn chunks biome
# ===========================================================================
./multifinder -s 59B -e 60B --spawn_biomes=flower_forest | \
    sort -bn > /tmp/multifinder_spawn.txt
echo '[91m'
diff /tmp/multifinder_spawn.txt test/golden_spawn.txt
echo '(B[m'

# ===========================================================================
# Plentiful mushroom biomes
# ===========================================================================
./multifinder -s 26B -e 27B --plentiful_biome=mushroom --plentifulness=20 | \
    sort -bn > /tmp/multifinder_plentiful.txt
echo '[91m'
diff /tmp/multifinder_plentiful.txt test/golden_plentiful.txt
echo '(B[m'

# ===========================================================================
# Ocean monument near quad huts
# ===========================================================================
./multifinder -s 27B -e 28B --monument_distance=6 | \
    sort -bn > /tmp/multifinder_monument.txt
echo '[91m'
diff /tmp/multifinder_monument.txt test/golden_monument.txt
echo '(B[m'

# ===========================================================================
# Woodland mansions
# ===========================================================================
./multifinder -s 24B -e 25B --woodland_mansions=2 | \
    sort -bn > /tmp/multifinder_mansions.txt
echo '[91m'
diff /tmp/multifinder_mansions.txt test/golden_mansions.txt
echo '(B[m'

#rm -r /tmp/multifinder*

# ===========================================================================
# Strongholds
# ===========================================================================
./multifinder -s 13B -e 14B --stronghold_distance=150 | \
    sort -bn > /tmp/multifinder_stronghold.txt
echo '[91m'
diff /tmp/multifinder_stronghold.txt test/golden_stronghold.txt
echo '(B[m'

rm -r /tmp/multifinder*

# TODO: file append
# TODO: radii

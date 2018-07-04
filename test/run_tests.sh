#!/bin/bash

# Basic output
./multifinder -e 10B | sort -bn > /tmp/multifinder_all.txt
diff /tmp/multifinder_all.txt test/golden_all.txt

# TODO: start / end seed

# Basic output with threads
mkdir /tmp/multifinder_thread
./multifinder -e 20B -t 4 -o /tmp/multifinder_thread
cat /tmp/multifinder_thread/*.txt | sort -bn > /tmp/multifinder_thread.txt
diff /tmp/multifinder_thread.txt test/golden_thread.txt

# TODO: append

# TODO: all biomes
# TODO: plentiful biomes
# TODO: spawn biomes

# TODO: monuments
# TODO: strongholds
# TODO: woodland mansions

# TODO: radii

# Ocean monument near quad huts search
mkdir /tmp/multifinder_monument
./multifinder -e 1T -t 12 -m 4 -o /tmp/multifinder_monument
cat /tmp/multifinder_monument/*.txt | sort -bn > /tmp/multifinder_monument.txt
diff /tmp/multifinder_monument.txt test/golden_monument.txt

rm -r /tmp/multifinder*

# cubiomes

Cubiomes is a standalone library, written in C, that mimics the Minecraft biome and feature generation. 
It is intended as a powerful tool to devise very fast, custom seed finding applications and large scale map viewers.


### Audience

You should be familiar with the C programming language, also a basic understanding of the Minecraft biome generation process would be helpful. 
A POSIX environment is required to compile the finders library and examples, but the core generator library may also work on other platforms.


### Documentation

There is a reference document for the generator layers which contains a summary for most generator layers and their function within the generation process.


### Example

There are two example programs in this repository which can be compiled using the makefile provided.


#### Finding Quad-Witch-Huts at a Specific Location

This classic type of finder uses several optimisations reguarding positioning of temples in the world. One of which allows you to specify the exact region (512x512) position about which the quad-hut should generate, without affecting the performance. For example:

`./find_quadhut 0 0`

will start a search with a regional positioning around the origin. (Actually the huts will be positioned in regions (-1,-1) to (0,0) this way.) 

To my knowlege, as of the time of writing, this is fastest single-thread quad-hut-finder out there. However, note that the current implementation of the biome finding optimisations causes the finder to miss some seeds (< 2%) in favour for speed.


#### Finding Compact Biome Seeds

This finder searches for seeds that contain all major biome types within 1024 blocks of the origin. These seeds are very rare and it might take a moment for the finder to yield any. The commandline arguments are:

`./find_compactbiomes [starting_seed] [end_seed] [threads]`


### Cool Seeds

If you are looking to get the "Adventuring Time" achievment you might consider one of the following seeds. All of these seeds have all 36 required biomes within less than 700 blocks from the origin (rarity: less than 1 in 100 billion): 

| Seed          | All biome radius |
|---------------|------------------|
| 1065757415811 | 612              | 
| -880424771806 | 644              | 
| -9896963610   | 664              | 
| -78538250832  | 686              | 
| 1251759844332 | 692              | 








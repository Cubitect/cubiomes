#ifndef UTIL_H_
#define UTIL_H_


#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Loads a list of seeds from a file. The seeds should be written as decimal
 * ASCII numbers separated by newlines.
 * @fnam: file path
 * @scnt: number of valid seeds found in the file, which is also the number of
 *        elements in the returned buffer
 *
 * Return a pointer to a dynamically allocated seed list.
 */
uint64_t *loadSavedSeeds(const char *fnam, uint64_t *scnt);


/// convert between version enum and text
const char* mc2str(int mc);
int str2mc(const char *s);

/// get the resource id name for a biome (for versions 1.13+)
const char *biome2str(int mc, int id);

/// get the resource id name for a structure
const char *struct2str(int stype);

/// initialize a biome colormap with some defaults
void initBiomeColors(unsigned char biomeColors[256][3]);
void initBiomeTypeColors(unsigned char biomeColors[256][3]);

/* Attempts to parse a biome-color mappings from a text buffer.
 * The parser makes one attempt per line and is not very picky regarding a
 * combination of biomeID/name with a color, represented as either a single
 * number or as a triplet in decimal or as hex (preceeded by 0x or #).
 * Returns the number of successfully mapped biome ids
 */
int parseBiomeColors(unsigned char biomeColors[256][3], const char *buf);

int biomesToImage(unsigned char *pixels,
        unsigned char biomeColors[256][3], const int *biomes,
        const unsigned int sx, const unsigned int sy,
        const unsigned int pixscale, const int flip);

/* Save the pixel buffer (e.g. from biomesToImage) to the given path as an PPM
 * image file. Returns 0 if successful, or -1 if the file could not be opened,
 * or 1 if not all the pixel data could be written to the file.
 */
int savePPM(const char* path, const unsigned char *pixels,
        const unsigned int sx, const unsigned int sy);

#ifdef __cplusplus
}
#endif

#endif

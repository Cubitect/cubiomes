#ifndef UTIL_H_
#define UTIL_H_

#ifdef __cplusplus
extern "C"
{
#endif

/// convert between version enum and text
const char* mc2str(int mc);
int str2mc(const char *s);

/// get the resource id name for a biome (for versions 1.13+)
const char *biome2str(int mc, int id);

/// initialize a biome colormap with some defaults
void initBiomeColors(unsigned char biomeColors[256][3]);
void initBiomeTypeColors(unsigned char biomeColors[256][3]);

/// Attempts to parse a biome-color mappings from a text buffer.
/// The parser makes one attempt per line and is not very picky regarding a
/// combination of biomeID/name with a color, represented as either a single
/// number or as a triplet in decimal or as hex (preceeded by 0x or #).
/// Returns the number of successfully mapped biome ids
int parseBiomeColors(unsigned char biomeColors[256][3], const char *buf);

int biomesToImage(unsigned char *pixels,
        unsigned char biomeColors[256][3], const int *biomes,
        const unsigned int sx, const unsigned int sy,
        const unsigned int pixscale, const int flip);

int savePPM(const char* path, const unsigned char *pixels,
        const unsigned int sx, const unsigned int sy);

#ifdef __cplusplus
}
#endif

#endif

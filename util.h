#ifndef UTIL_H_
#define UTIL_H_

#ifdef __cplusplus
extern "C"
{
#endif

void initBiomeColours(unsigned char biomeColours[256][3]);
void initBiomeTypeColours(unsigned char biomeColours[256][3]);

int biomesToImage(unsigned char *pixels, 
        const unsigned char biomeColours[256][3], const int *biomes, 
        const unsigned int sx, const unsigned int sy, 
        const unsigned int pixscale, const int flip);

int savePPM(const char* path, const unsigned char *pixels, 
        const unsigned int sx, const unsigned int sy);

#ifdef __cplusplus
}
#endif

#endif

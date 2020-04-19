/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>

struct texture_s
{
    uint8_t width, height;
    uint8_t *pixels;
};

enum
{
    TEXTURE_SOURCE_TEXT_1,     /* TEXT1.DTA*/
    TEXTURE_SOURCE_PALAT_001,  /* PALAT.001*/
    TEXTURE_SOURCE_PALAT_002,  /* PALAT.002*/
    TEXTURE_SOURCE_ANIMS,      /* ANIMS.DTA*/
};

/* Loads and returns the texture at the given index in the given texture source.
 * For instance, to load the 4th texture in PALAT.002, you'd call this function
 * like so: ktexture_load_texture(3, TEXTURE_SOURCE_PALAT_002).*/
struct texture_s ktexture_load_texture(const unsigned idx, const int sourceEnum);

#endif

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

// Loads all textures into memory, etc.
void ktexture_initialize_textures(void);

// Frees up any texture memory allocated by ktexture_initialize_textures(), etc.
void ktexture_release_textures(void);

// Returns the prop texture at the given index.
struct texture_s* ktexture_prop_texture(unsigned propTextureIdx);

// Returns the PALA texture at the given index.
struct texture_s* ktexture_pala_texture(unsigned palaTextureIdx);

#endif

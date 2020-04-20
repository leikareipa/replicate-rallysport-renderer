/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "generic_stack.h"
#include "texture.h"
#include "file.h"

#if __DMC__
    #define snprintf _snprintf
#endif

static struct kelpo_generic_stack_s *PALA_TEXTURES;
static struct kelpo_generic_stack_s *PROP_TEXTURES;

static struct texture_s load_from_pala(const unsigned textureIdx, const unsigned palaIdx)
{
    assert((palaIdx < 2) && "PALA index out of bounds.");

    struct texture_s tex;
    tex.width = 16;
    tex.height = 16;
    tex.pixels = malloc(tex.width * tex.height);

    char filename[20];
    snprintf(filename, 20, "PALAT.00%c", ('1' + palaIdx));

    const file_handle_t palaHandle = kfile_open_file(filename, "rb");
    kfile_seek((textureIdx * 256), palaHandle);
    kfile_read_byte_array(tex.pixels, (tex.width * tex.height), palaHandle);
    kfile_close_file(palaHandle);

    return tex;
}

// Loads the texture at the given index in the TEXT1.DTA file. In case of error,
// the pixel data pointer of the retuned texture will be set to NULL.
static struct texture_s load_from_text(const unsigned textureIdx)
{
    struct texture_s tex;

    const file_handle_t rallyeHandle = kfile_open_file("RALLYE.EXE", "rb");
    const file_handle_t textHandle = kfile_open_file("TEXT1.DTA", "rb");

    assert((kfile_file_size(textHandle) == 32768) && "Unexpected file size for TEXT1.DTA.");

    kfile_seek((123614 + (textureIdx * 10)), rallyeHandle);

    // The first 2 bytes of the one-after-last prop texture entry are 0xFFFF.
    {
        uint16_t word;
        kfile_read_byte_array((uint8_t*)&word, 2, rallyeHandle);

        if (word == 0xffff)
        {
            tex.pixels = NULL;
            return tex;
        }

        kfile_jump(-2, rallyeHandle);
    }

    kfile_read_byte_array(&tex.width, 1, rallyeHandle);
    kfile_jump(1, rallyeHandle);
    kfile_read_byte_array(&tex.height, 1, rallyeHandle);
    kfile_jump(1, rallyeHandle);

    tex.width /= 2;
    tex.height /= 2;
    tex.pixels = malloc(tex.width * tex.height);

    kfile_jump(2, rallyeHandle);

    // Offset of this texture in the texture atlas of TEXT1.DTA.
    unsigned xOffset = 0;
    unsigned yOffset = 0;
    kfile_read_byte_array((uint8_t*)&xOffset, 1, rallyeHandle);
    kfile_read_byte_array((uint8_t*)&yOffset, 1, rallyeHandle);
    yOffset *= 2;

    // Copy this texture's data from the TEXT1.DTA texture atlas.
    for (unsigned y = 0; y < tex.height; y++)
    {
        kfile_seek((xOffset + (yOffset + y) * 128), textHandle);

        for (unsigned x = 0; x < tex.width; x++)
        {
            kfile_read_byte_array(&tex.pixels[x + y * tex.width], 1, textHandle);
        }
    }

    kfile_close_file(rallyeHandle);
    kfile_close_file(textHandle);

    return tex;
}

struct texture_s* ktexture_prop_texture(const unsigned propTextureIdx)
{
    return (struct texture_s*)kelpo_generic_stack__at(PROP_TEXTURES, propTextureIdx);
}

void ktexture_release_textures(void)
{
    kelpo_generic_stack__free(PROP_TEXTURES);

    return;
}

void ktexture_initialize_textures(void)
{
    PROP_TEXTURES = kelpo_generic_stack__create(23, sizeof(struct texture_s));
    PALA_TEXTURES = kelpo_generic_stack__create(255, sizeof(struct texture_s));

    // Load all prop textures.
    for (struct texture_s texture = load_from_text(PROP_TEXTURES->count); texture.pixels; texture = load_from_text(PROP_TEXTURES->count))
    {
        kelpo_generic_stack__push_copy(PROP_TEXTURES, &texture);
    }

    return;
}
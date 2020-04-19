/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "texture.h"
#include "file.h"

#if __DMC__
    #define snprintf _snprintf
#endif

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

static struct texture_s load_from_text(const unsigned textureIdx)
{
    struct texture_s tex;

    const file_handle_t rallyeHandle = kfile_open_file("RALLYE.EXE", "rb");
    const file_handle_t textHandle = kfile_open_file("TEXT1.DTA", "rb");

    assert((kfile_file_size(textHandle) == 32768) && "Unexpected file size for TEXT1.DTA.");

    kfile_seek((123614 + (textureIdx * 10)), rallyeHandle);

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

struct texture_s ktexture_load_texture(const unsigned idx, const int sourceEnum)
{
    switch (sourceEnum)
    {
        case TEXTURE_SOURCE_PALAT_001: return load_from_pala(idx, 0);
        case TEXTURE_SOURCE_PALAT_002: return load_from_pala(idx, 1);
        case TEXTURE_SOURCE_TEXT_1: return load_from_text(idx);
        default:
        {
            struct texture_s nullTexture;
            memset(&nullTexture, 0, sizeof(struct texture_s));

            return nullTexture;
        };
    }
}
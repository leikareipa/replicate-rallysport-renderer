/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "file.h"
#include "mesh.h"
#include "ground.h"
#include "renderer.h"
#include "polygon.h"

#if MSDOS
    #if __DMC__ // Digital Mars C/C++.
        #include <bios.h>
        #include <dos.h>
    #else
        #include <bios.h>
        #include <dos.h>
    #endif
#else // We'll render into an SDL surface if not on DOS.
    #include <SDL2/SDL.h>

    static SDL_Window *sdlWindow;
    static SDL_Renderer *sdlRenderer;
    static SDL_Texture *sdlTexture;

    // Color indices in the render buffer point to RGB values in this palette.
    static uint8_t PALETTE[256][3];
#endif

// A pixel buffer we'll do all rendering into. This is later copied into video
// memory.
static uint8_t *RENDER_BUFFER;
#define VRAM_XY(x, y) RENDER_BUFFER[(x) + (y) * 320]

#define LERP(a, b, weight) ((a) + ((weight) * ((b) - (a))))

static unsigned CURRENT_VIDEO_MODE = VIDEO_MODE_TEXT;

static struct vertex_s CAMERA_POS = {300, -900, -5};

static const unsigned GRAPHICS_MODE_WIDTH = 320;
static const unsigned GRAPHICS_MODE_HEIGHT = 200;

#include "polytrnf.c"
#include "polyfill.c"

static int current_video_mode(void)
{
    #if MSDOS
        union REGS regs;
        regs.h.ah = 0xf;
        regs.h.al = 0;

        int86(0x10, &regs, &regs);

        return regs.h.al;
    #else
        return CURRENT_VIDEO_MODE;
    #endif
}

void krender_initialize(void)
{
    RENDER_BUFFER = malloc(GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT);

    ktexture_initialize_textures();
    kmesh_initialize_meshes();
    kground_initialize_ground("MAASTO.001");

    krender_enter_grapics_mode();
    krender_clear_surface();

    return;
}

void krender_release(void)
{
    kmesh_release_meshes();
    ktexture_release_textures();
    kground_release_ground();

    free(RENDER_BUFFER);

    krender_enter_text_mode();

    return;
}

void krender_flip_surface(void)
{
    #if MSDOS
        // Wait for vsync.
        while ((inp(0x03da)  & 0x08)) _asm{nop};
        while (!(inp(0x03da) & 0x08)) _asm{nop};

        // Copy into VGA mode 13h video memory.
        memcpy((uint8_t*)0xA0000000L, RENDER_BUFFER, (GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT));
    #else
        static uint8_t *scratch = 0;
        if (!scratch)
        {
            scratch = malloc(GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT * 4);
        }

        for (unsigned i = 0; i < (GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT); i++)
        {
            scratch[(i * 4) + 0] = PALETTE[RENDER_BUFFER[i]][0];
            scratch[(i * 4) + 1] = PALETTE[RENDER_BUFFER[i]][1];
            scratch[(i * 4) + 2] = PALETTE[RENDER_BUFFER[i]][2];
            scratch[(i * 4) + 3] = 255;
        }

        SDL_UpdateTexture(sdlTexture, NULL, scratch, (GRAPHICS_MODE_WIDTH * 4));
        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
        SDL_RenderPresent(sdlRenderer);
    #endif

    return;
}

void krender_use_palette(const unsigned paletteIdx)
{
    assert((CURRENT_VIDEO_MODE == VIDEO_MODE_GRAPHICS) && "Can only set the palette while in graphics mode.");

    assert((paletteIdx < 5) && "Palette index out of bounds.");

    const file_handle_t rallyeHandle = kfile_open_file("RALLYE.EXE", "rb");

    // Seek to the start of the palette block.
    // (There are 32 colors per palette, and 3 color channels (rgb) per color.)
    const uint32_t paletteByteOffs = (131798 + (paletteIdx * 32 * 3));
    kfile_seek(paletteByteOffs, rallyeHandle);

    // Read in all 32 primary colors of the palette.
    for (unsigned i = 0; i < 32; i++)
    {
        uint8_t color[3];

        kfile_read_byte_array(color, 3, rallyeHandle);

        #ifdef MSDOS
            outp(0x03c8, i);
            outp(0x03c9, color[0]);
            outp(0x03c9, color[1]);
            outp(0x03c9, color[2]);
        #else
            PALETTE[i][0] = (color[0] * 4);
            PALETTE[i][1] = (color[1] * 4);
            PALETTE[i][2] = (color[2] * 4);
        #endif
    }

    kfile_close_file(rallyeHandle);

    return;
}

void krender_move_camera(void)
{
    CAMERA_POS.x += 8;

    return;
}

void krender_clear_surface(void)
{
    switch (CURRENT_VIDEO_MODE)
    {
        case VIDEO_MODE_GRAPHICS:
        {
            memset(RENDER_BUFFER, 0, (GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT));
            break;
        }
        case VIDEO_MODE_TEXT:
        {
            /* TODO.*/
            break;
        }
        default: assert(0 && "Unknown video mode.");
    }

    return;
}

void krender_draw_mesh(const struct mesh_s *const mesh, int doTransform)
{
    static struct polygon_s poly = {0};
    if (!poly.verts)
    {
        poly.verts = malloc(sizeof(struct polygon_s) * 10);
    }

    for (unsigned i = 0; i < mesh->numPolys; i++)
    {
        poly.color = mesh->polys[i].color;
        poly.numVerts = mesh->polys[i].numVerts;
        poly.texture = mesh->polys[i].texture;
        memcpy(poly.verts, mesh->polys[i].verts, sizeof(struct vertex_s) * mesh->polys[i].numVerts);

        if (doTransform)
        {
            krender_transform_poly(&poly);
        }
        fill_poly(&poly);
    }
    
    return;
}

int krender_enter_grapics_mode(void)
{
    if (CURRENT_VIDEO_MODE == VIDEO_MODE_GRAPHICS)
    {
        return 1;
    }

    #if MSDOS
        union REGS regs;
        regs.h.ah = 0;
        regs.h.al = VIDEO_MODE_GRAPHICS;

        int86(0x10, &regs, &regs);

        CURRENT_VIDEO_MODE = current_video_mode();
    #else
        sdlWindow = SDL_CreateWindow("Rally-Sport render test", 0, 0, 1280, 800, 0);
        sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
        sdlTexture = SDL_CreateTexture(sdlRenderer,
                                       SDL_PIXELFORMAT_ABGR8888,
                                       SDL_TEXTUREACCESS_STREAMING,
                                       GRAPHICS_MODE_WIDTH,
                                       GRAPHICS_MODE_HEIGHT);

        CURRENT_VIDEO_MODE = VIDEO_MODE_GRAPHICS;
    #endif

    return (CURRENT_VIDEO_MODE == VIDEO_MODE_GRAPHICS);
}

int krender_enter_text_mode(void)
{
    #if MSDOS
        union REGS regs;
        regs.h.ah = 0;
        regs.h.al = VIDEO_MODE_TEXT;

        int86(0x10, &regs, &regs);

        CURRENT_VIDEO_MODE = current_video_mode();
    #else 
        SDL_DestroyWindow(sdlWindow);
        SDL_DestroyRenderer(sdlRenderer);
        SDL_DestroyTexture(sdlTexture);

        CURRENT_VIDEO_MODE = VIDEO_MODE_TEXT;
    #endif

    return (CURRENT_VIDEO_MODE == VIDEO_MODE_TEXT);
}

unsigned krender_current_video_mode(void)
{
    return CURRENT_VIDEO_MODE;
}

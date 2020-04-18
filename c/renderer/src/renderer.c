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
#include "renderer.h"
#include "polygon.h"

#if __DMC__
    /* Digital Mars C/C++.*/
    #include <bios.h>
    #include <dos.h>
#elif __GNUC__
    /* Assume this is a modern version of GCC that should just ignore the DOS stuff.*/
    union REGS{struct{int ah; int al;} h;};
    #define int86(x,y,z) (void)(x); (void)(y); (void)(z);
#else
    #include <bios.h>
    #include <dos.h>
#endif

/* Pointer to the beginning of video memory in VGA mode 13h.*/
static uint8_t *const VRAM = (uint8_t*)0xA0000000L;

#define LERP(a, b, weight) ((a) + ((weight) * ((b) - (a))))

/* Indexes into the X,Y element in video memory in VGA mode 13h.*/
#define VRAM_XY(x, y) VRAM[(unsigned)(x) + (unsigned)(y) * 320]

static unsigned CURRENT_VIDEO_MODE = VIDEO_MODE_TEXT;

static struct vertex_s CAMERA_POS = {160, 100, 0};

static const unsigned GRAPHICS_MODE_WIDTH = 320;
static const unsigned GRAPHICS_MODE_HEIGHT = 200;

#include "polytrnf.c"
#include "polyfill.c"

static int current_video_mode(void)
{
    union REGS regs;
    regs.h.ah = 0xf;
    regs.h.al = 0;

    int86(0x10, &regs, &regs);

    return regs.h.al;
}

void krender_clear_screen(void)
{
    switch (CURRENT_VIDEO_MODE)
    {
        case VIDEO_MODE_GRAPHICS:
        {
            memset(VRAM, 0, (GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT));
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

void krender_draw_test_pattern(struct polygon_s *const poly)
{
    switch (CURRENT_VIDEO_MODE)
    {
        case VIDEO_MODE_TEXT:
        {
            printf("TEST PATTERN.\n");
            break;
        }
        case VIDEO_MODE_GRAPHICS:
        {
            transform_poly(poly);
            fill_poly(poly);
            break;
        }
    }

    return;
}

int krender_enter_grapics_mode(void)
{
    union REGS regs;
    regs.h.ah = 0;
    regs.h.al = VIDEO_MODE_GRAPHICS;

    int86(0x10, &regs, &regs);

    CURRENT_VIDEO_MODE = current_video_mode();

    return (CURRENT_VIDEO_MODE == VIDEO_MODE_GRAPHICS);
}

int krender_enter_text_mode(void)
{
    union REGS regs;
    regs.h.ah = 0;
    regs.h.al = VIDEO_MODE_TEXT;

    int86(0x10, &regs, &regs);

    CURRENT_VIDEO_MODE = current_video_mode();

    return (CURRENT_VIDEO_MODE == VIDEO_MODE_TEXT);
}

unsigned krender_current_video_mode(void)
{
    return CURRENT_VIDEO_MODE;
}

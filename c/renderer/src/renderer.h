/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#ifndef RENDERER_H
#define RENDERER_H

enum
{
    VIDEO_MODE_GRAPHICS = 0x13,
    VIDEO_MODE_TEXT = 0x3
};

/* Places the display in VGA video mode 13h. Returns true on success; false
 * otherwise.*/
int krender_enter_grapics_mode(void);

/* Places the display a text-compatible VGA video mode. Returns true on success;
 * false otherwise.*/
int krender_enter_text_mode(void);

/* Wipes the screen to blank.*/
void krender_clear_screen(void);

/* Temporary function. Renders something to the screen to allow the user to
 * verify that the renderer is working.*/
void krender_draw_test_pattern(void);

unsigned krender_current_video_mode(void);

#endif

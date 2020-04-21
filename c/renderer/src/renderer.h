/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#ifndef RENDERER_H
#define RENDERER_H

struct polygon_s;
struct mesh_s;

enum
{
    VIDEO_MODE_GRAPHICS = 0x13,
    VIDEO_MODE_TEXT = 0x3
};

void krender_move_camera(void);

// Places the display in VGA video mode 13h. Returns true on success; false
// otherwise.
int krender_enter_grapics_mode(void);

// Copies the current contents of the render buffer onto the display (e.g.
// into video memory in DOS).
void krender_flip_surface(void);

// Apply the given Rally-Sport palette.
void krender_use_palette(const unsigned paletteIdx);

// Places the display a text-compatible VGA video mode. Returns true on success;
// false otherwise.
int krender_enter_text_mode(void);

// Wipes the screen to blank.
void krender_clear_surface(void);

// Renders the given mesh.
void krender_draw_mesh(const struct mesh_s *const mesh, int doTransform);

// Prepare the render surface for drawing. In DOS, this means entering VGA mode
// 13h.
void krender_initialize(void);

// Release the render surface. In DOS, this means leaving the graphics video
// mode and entering text mode.
void krender_release(void);

// Transforms the given polygon into screen space.
void krender_transform_poly(struct polygon_s *const poly);

unsigned krender_current_video_mode(void);

#endif

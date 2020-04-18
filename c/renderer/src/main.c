/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#include <stdio.h>
#include "renderer.h"
#include "polygon.h"

int main(void)
{
    struct polygon_s poly = kpolygon_create_polygon(4);

    poly.verts[0].x = -50;
    poly.verts[0].y = 0;
    poly.verts[0].z = 2;
    poly.verts[0].color = 0x0a;
    poly.verts[1].x = 50;
    poly.verts[1].y = 0;
    poly.verts[1].z = 2;
    poly.verts[1].color = 0x0a;
    poly.verts[2].x = 50;
    poly.verts[2].y = 0;
    poly.verts[2].z = 1;
    poly.verts[2].color = 0x0d;
    poly.verts[3].x = -50;
    poly.verts[3].y = 0;
    poly.verts[3].z = 1;
    poly.verts[3].color = 0x0d;

    krender_enter_grapics_mode();
    krender_clear_screen();
    krender_draw_test_pattern(&poly);

    kpolygon_release_polygon(&poly);
    
    return 0;
}

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

    poly.verts[0].x = 150;
    poly.verts[0].y = 70;
    poly.verts[0].color = 22;
    poly.verts[1].x = 100;
    poly.verts[1].y = 50;
    poly.verts[1].color = 22;
    poly.verts[2].x = 170;
    poly.verts[2].y = 150;
    poly.verts[2].color = 29;
    poly.verts[3].x = 70;
    poly.verts[3].y = 120;
    poly.verts[3].color = 29;

    krender_enter_grapics_mode();
    krender_clear_screen();
    krender_draw_test_pattern(&poly);

    kpolygon_release_polygon(&poly);
    
    return 0;
}

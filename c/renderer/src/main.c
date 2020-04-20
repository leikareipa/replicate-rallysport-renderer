/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "mesh.h"
#include "renderer.h"
#include "polygon.h"

int main(void)
{
    krender_initialize();
    krender_use_palette(0);

    time_t startTime = time(NULL);
    unsigned numFrames = 0;

    while ((time(NULL) - startTime < 6))
    {
        krender_clear_surface();
        krender_draw_mesh(kmesh_prop_mesh(14));
        krender_draw_mesh(kmesh_prop_mesh(7));
        krender_draw_mesh(kmesh_prop_mesh(3));
        krender_draw_mesh(kmesh_prop_mesh(1));
        krender_flip_surface();
        
        krender_move_camera();
        
        numFrames++;
    }

    krender_release();

    printf("~%f FPS\n", (numFrames / (float)(time(NULL) - startTime)));

    return 0;
}

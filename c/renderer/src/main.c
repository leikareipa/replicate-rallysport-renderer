/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "mesh.h"
#include "ground.h"
#include "renderer.h"
#include "polygon.h"

int main(void)
{
    krender_initialize();
    krender_use_palette(0);

    time_t startTime = time(NULL);
    unsigned numFrames = 0;

    while ((time(NULL) - startTime < 3))
    {
        krender_clear_surface();
        krender_draw_mesh(kground_ground_mesh(), 0);
        krender_draw_mesh(kmesh_prop_mesh(PROP_TYPE_STONE_ARCH), 1);
        krender_flip_surface();
        
        krender_move_camera();
        
        numFrames++;
    }

    printf("%d FPS\n", (int)round(numFrames / (float)(time(NULL) - startTime)));
    getchar();

    krender_release();

    return 0;
}

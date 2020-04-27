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
#include "generic_stack.h"
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

    while ((time(NULL) - startTime) < 6)
    {
        krender_clear_surface();
        
        // Move the camera, for testing purposes.
        {
            static float px = 10;
            static float pz = 10;
        
            kground_update_ground_mesh(px, pz);
            px += 0.25;
        }

        // Render the ground.
        {
            const struct kelpo_generic_stack_s *const groundMeshes = kground_ground_meshes();

            for (unsigned i = 0; i < groundMeshes->count; i++)
            {
                krender_draw_mesh(kelpo_generic_stack__at(groundMeshes, i), 1);
            }
        }

        krender_flip_surface();

        numFrames++;
    }

    printf("%d FPS\n", (int)round(numFrames / (float)(time(NULL) - startTime)));
    getchar();

    krender_release();

    return 0;
}

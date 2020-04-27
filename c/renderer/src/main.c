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
#include "common/genstack.h"
#include "assets/mesh.h"
#include "assets/ground.h"
#include "renderer/renderer.h"
#include "renderer/polygon.h"

int main(void)
{
    ktexture_initialize_textures();
    kmesh_initialize_meshes();
    kground_initialize_ground(3);
    krender_initialize();
    krender_use_palette(0);

    time_t startTime = time(NULL);
    unsigned numFrames = 0;

    while ((time(NULL) - startTime) < 6)
    {
        krender_clear_surface();
        
        // Move the camera, for testing purposes.
        {
            static float px = 1;
            static float pz = 1;
        
            kground_update_ground_mesh(px, pz);
            pz += 0.25;
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

    printf("~%d FPS\n", (int)round(numFrames / (float)(time(NULL) - startTime)));
    getchar();

    kmesh_release_meshes();
    ktexture_release_textures();
    kground_release_ground();
    krender_release();

    return 0;
}

/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "renderer.h"
#include "ground.h"
#include "mesh.h"
#include "file.h"

// For each corner in a track's surface its height value.
static int *HEIGHTMAP;
static unsigned HEIGHTMAP_WIDTH; // In units of tiles.
static unsigned HEIGHTMAP_HEIGHT;

// A polygonal, renderable mesh representing the ground.
static struct mesh_s GROUND_MESH;
static unsigned GROUND_MESH_WIDTH = 15; // In units of tiles.
static unsigned GROUND_MESH_HEIGHT = 12;

int kground_width(void)
{
    return HEIGHTMAP_WIDTH;
}

int kground_height(void)
{
    return HEIGHTMAP_HEIGHT;
}

const struct mesh_s* kground_ground_mesh(void)
{
    return &GROUND_MESH;
}

void kground_update_ground_mesh(void)
{
    for (unsigned z = 0; z < GROUND_MESH_HEIGHT; z++)
    {
        for (unsigned x = 0; x < GROUND_MESH_WIDTH; x++)
        {
            const int vertX = ((x * 128) - ((GROUND_MESH_WIDTH/2+1) * 128));
            const int vertZ = (0 - (z * 128));

            struct polygon_s *const groundPoly = &GROUND_MESH.polys[x + z * GROUND_MESH_WIDTH];

            // Top left.
            groundPoly->verts[0].x = vertX;
            groundPoly->verts[0].y = 0;
            groundPoly->verts[0].z = (vertZ / 256.0);

            // Top right.
            groundPoly->verts[1].x = vertX + 128;
            groundPoly->verts[1].y = 0;
            groundPoly->verts[1].z = (vertZ / 256.0);

            // Front right.
            groundPoly->verts[2].x = vertX + 128;
            groundPoly->verts[2].y = 0;
            groundPoly->verts[2].z = ((vertZ+128) / 256.0);

            // Front left.
            groundPoly->verts[3].x = vertX;
            groundPoly->verts[3].y = 0;
            groundPoly->verts[3].z = ((vertZ+128) / 256.0);

            // Pre-transform to screen coordinates.
            krender_transform_poly(groundPoly);
        }
    }
}

void kground_initialize_ground(const char *const filename)
{
    GROUND_MESH.numPolys = (GROUND_MESH_WIDTH * GROUND_MESH_HEIGHT);
    GROUND_MESH.polys = malloc(sizeof(struct polygon_s) * GROUND_MESH.numPolys);
    for (unsigned i = 0; i < GROUND_MESH.numPolys; i++)
    {
        GROUND_MESH.polys[i] = kpolygon_create_polygon(4);
        
        GROUND_MESH.polys[i].texture = ktexture_pala_texture(3);
    }

    // Import the Rally-Sport heightmap.
    {
        const file_handle_t maastoHandle = kfile_open_file(filename, "rb");

        HEIGHTMAP_WIDTH = HEIGHTMAP_HEIGHT = sqrt(kfile_file_size(maastoHandle) / 2);
        assert(((HEIGHTMAP_WIDTH == 64) || (HEIGHTMAP_WIDTH == 128)) && "Unsupported heightmap dimensions.");

        HEIGHTMAP = malloc(sizeof(*HEIGHTMAP) * HEIGHTMAP_WIDTH * HEIGHTMAP_HEIGHT);

        for (unsigned i = 0; i < (HEIGHTMAP_WIDTH * HEIGHTMAP_HEIGHT); i++)
        {
            uint8_t word[2];

            kfile_read_byte_array(word, 2, maastoHandle);

            // More than -255 below ground level.
            if (word[1] == 1)            
            {
                HEIGHTMAP[i] = (-256 - word[0]);
            }
            // Above ground when word[1] == 255, otherwise below ground level.
            else                    
            {
                HEIGHTMAP[i] = (word[1] - word[0]);
            }
        }

        kfile_close_file(maastoHandle);
    }

    kground_update_ground_mesh();

    return;
}

void kground_release_ground(void)
{
    free(HEIGHTMAP);
    free(GROUND_MESH.polys);

    return;
}

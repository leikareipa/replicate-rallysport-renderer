/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "generic_stack.h"
#include "vector.h"
#include "renderer.h"
#include "ground.h"
#include "mesh.h"
#include "file.h"

struct track_prop_s
{
    struct vector_s position;
    int type;
};

#define GROUND_TILE_WIDTH 128
#define GROUND_TILE_HEIGHT 128

// For each corner in a track's surface its height value.
static int16_t *HEIGHTMAP;
static unsigned HEIGHTMAP_WIDTH; // In units of tiles.
static unsigned HEIGHTMAP_HEIGHT;

// For each tile on the track its texture index. Note that the tilemap has the
// same dimensions as the heightmap.
static uint8_t *TILEMAP;

// We'll make the heightmap data into quads that can be included in the ground
// mesh.
static struct polygon_s *HEIGHTMAP_QUADS;
#define NUM_HEIGHTMAP_QUADS_X 23 // In units of tiles.
#define NUM_HEIGHTMAP_QUADS_Z 24

// A polygonal, renderable mesh representing the ground; including the heightmap
// quads, any visible props, etc.
static struct mesh_s GROUND_MESH;
static struct kelpo_generic_stack_s *GROUND_MESH_POLYS;

static struct kelpo_generic_stack_s *GROUND_MESHES;

#define MAX_NUM_PROPS 14
static uint16_t NUM_PROPS = 0;
static struct track_prop_s* PROPS[MAX_NUM_PROPS];

// Vertices representing ground meshes will be offset by these amounts. This
// is done to center the meshes on the screen when rendered.
static struct vector_s GROUND_OFFSET = {-(((NUM_HEIGHTMAP_QUADS_X / 2) - 1) * GROUND_TILE_WIDTH), 0, -700};

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

const struct kelpo_generic_stack_s* kground_ground_meshes(void)
{
    return GROUND_MESHES;
}

void kground_update_ground_mesh(const int viewOffsX, const int viewOffsZ)
{
    kelpo_generic_stack__clear(GROUND_MESH_POLYS);
    kelpo_generic_stack__clear(GROUND_MESHES);

    // Add ground tiles - including any billboards.
    {
        for (int z = 0; z < NUM_HEIGHTMAP_QUADS_Z; z++)
        {
            for (int x = 0; x < NUM_HEIGHTMAP_QUADS_X; x++)
            {
                const int tileX = (x + viewOffsX);
                const int tileY = (z + viewOffsZ);

                // Center the mesh on screen.
                const int vertX = ((x * GROUND_TILE_WIDTH) + GROUND_OFFSET.x);
                const int vertZ = ((-z * GROUND_TILE_HEIGHT) + GROUND_OFFSET.z);

                struct polygon_s *const groundPoly = &HEIGHTMAP_QUADS[x + z * NUM_HEIGHTMAP_QUADS_X];

                // Back left.
                groundPoly->verts[0].x = vertX;
                groundPoly->verts[0].y = -HEIGHTMAP[tileX + tileY * HEIGHTMAP_WIDTH];
                groundPoly->verts[0].z = vertZ;

                // Back right.
                groundPoly->verts[1].x = (vertX + GROUND_TILE_WIDTH);
                groundPoly->verts[1].y = -HEIGHTMAP[(tileX + 1) + tileY * HEIGHTMAP_WIDTH];
                groundPoly->verts[1].z = vertZ;

                // Front left.
                groundPoly->verts[2].x = vertX;
                groundPoly->verts[2].y = -HEIGHTMAP[tileX + (tileY - 1) * HEIGHTMAP_WIDTH];
                groundPoly->verts[2].z = (vertZ + GROUND_TILE_HEIGHT);

                // Front right.
                groundPoly->verts[3].x = (vertX + GROUND_TILE_WIDTH);
                groundPoly->verts[3].y = -HEIGHTMAP[(tileX + 1) + (tileY - 1) * HEIGHTMAP_WIDTH];
                groundPoly->verts[3].z = (vertZ + GROUND_TILE_HEIGHT);

                groundPoly->texture = ktexture_pala_texture(TILEMAP[tileX + (tileY - 1) * HEIGHTMAP_WIDTH]);

                kelpo_generic_stack__push_copy(GROUND_MESH_POLYS, groundPoly);
            }
        }

        GROUND_MESH.x = GROUND_MESH.y = GROUND_MESH.z = 0;
        GROUND_MESH.numPolys = GROUND_MESH_POLYS->count;
        GROUND_MESH.polys = GROUND_MESH_POLYS->data;

        kelpo_generic_stack__push_copy(GROUND_MESHES, &GROUND_MESH);
    }

    // Add props.
    for (unsigned i = 0; i < NUM_PROPS; i++)
    {
        //                                                                        v-- Negate the camera's offset.
        const int meshX = (PROPS[i]->position.x - ((viewOffsX * GROUND_TILE_WIDTH) - GROUND_OFFSET.x));
        const int meshZ = (PROPS[i]->position.z + (viewOffsZ * GROUND_TILE_HEIGHT) + GROUND_OFFSET.z);
        const int meshY = (PROPS[i]->position.y
                           ? PROPS[i]->position.y
                           : -HEIGHTMAP[((int)PROPS[i]->position.x / GROUND_TILE_WIDTH) + (-(int)PROPS[i]->position.z / GROUND_TILE_HEIGHT) * HEIGHTMAP_WIDTH]);
        
        // If the prop isn't visible on screen, don't add it.
        if ((meshZ > (GROUND_OFFSET.z + GROUND_TILE_HEIGHT)) ||
            (meshZ < (GROUND_OFFSET.z - ((NUM_HEIGHTMAP_QUADS_Z + 3) * GROUND_TILE_HEIGHT))) ||
            (meshX < (GROUND_OFFSET.x - (3 * GROUND_TILE_WIDTH))) ||
            (meshX > (GROUND_OFFSET.x + ((NUM_HEIGHTMAP_QUADS_X + 3) * GROUND_TILE_WIDTH))))
        {
            continue;
        }

        struct mesh_s propMesh = kmesh_prop_mesh(PROPS[i]->type, meshX, meshY, meshZ);

        kelpo_generic_stack__push_copy(GROUND_MESHES, &propMesh);
    }

    return;
}

void kground_initialize_ground(const unsigned groundIdx)
{
    assert((groundIdx <= 8) && "Ground index out of bounds.");

    GROUND_MESH.x = GROUND_MESH.y = GROUND_MESH.z = 0;

    GROUND_MESH_POLYS = kelpo_generic_stack__create(100, sizeof(struct polygon_s));
    GROUND_MESHES = kelpo_generic_stack__create(15, sizeof(struct mesh_s));

    HEIGHTMAP_QUADS = malloc(sizeof(*HEIGHTMAP_QUADS) * NUM_HEIGHTMAP_QUADS_X * NUM_HEIGHTMAP_QUADS_Z);
    for (unsigned i = 0; i < (NUM_HEIGHTMAP_QUADS_X * NUM_HEIGHTMAP_QUADS_Z); i++)
    {
        HEIGHTMAP_QUADS[i] = kpolygon_create_polygon(4);
    }

    // Import the Rally-Sport heightmap.
    {
        char filename[20];
        sprintf(filename, "MAASTO.00%c", ('1' + groundIdx));

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

    // Import the Rally-Sport tilemap.
    {
        char filename[20];
        sprintf(filename, "VARIMAA.00%c", ('1' + groundIdx));

        const file_handle_t varimaaHandle = kfile_open_file(filename, "rb");
        assert((sqrt(kfile_file_size(varimaaHandle)) == HEIGHTMAP_WIDTH) && "Invalid tilemap dimensions.");

        TILEMAP = malloc(HEIGHTMAP_WIDTH * HEIGHTMAP_HEIGHT);
        kfile_read_byte_array(TILEMAP, (HEIGHTMAP_WIDTH * HEIGHTMAP_HEIGHT), varimaaHandle);

        kfile_close_file(varimaaHandle);
    }

    // Load prop locations.
    {
        const file_handle_t rallyeHandle = kfile_open_file("RALLYE.EXE", "rb");
        uint32_t propHeaderByteOffset = 0;

        switch (groundIdx)
        {
            case 0: propHeaderByteOffset = 0x15109l; break;
            case 1: propHeaderByteOffset = 0x1518fl; break;
            case 2: propHeaderByteOffset = 0x1519dl; break;
            case 3: propHeaderByteOffset = 0x151cfl; break;
            case 4: propHeaderByteOffset = 0x15279l; break;
            case 5: propHeaderByteOffset = 0x15293l; break;
            case 6: propHeaderByteOffset = 0x152a1l; break;
            case 7: propHeaderByteOffset = 0x1533fl; break;
            default: assert(0 && "Invalid track index."); break;
        }

        kfile_seek(propHeaderByteOffset, rallyeHandle);

        kfile_read_byte_array((uint8_t*)&NUM_PROPS, 2, rallyeHandle);

        assert((NUM_PROPS <= MAX_NUM_PROPS) && "The number of track props would overflow.");

        for (unsigned i = 0; i < NUM_PROPS; i++)
        {
            uint16_t coordinateByteOffset = 0;
            uint16_t indexByteOffset = 0;
            uint16_t posX = 0, posY = 0, posZ = 0;

            PROPS[i] = malloc(sizeof(struct track_prop_s));
   
            kfile_read_byte_array((uint8_t*)&coordinateByteOffset, 2, rallyeHandle);
            kfile_read_byte_array((uint8_t*)&indexByteOffset, 2, rallyeHandle);

            kfile_jump(2, rallyeHandle);

            kfile_read_byte_array((uint8_t*)&posX, 2, rallyeHandle);
            kfile_read_byte_array((uint8_t*)&posZ, 2, rallyeHandle);
            kfile_read_byte_array((uint8_t*)&posY, 2, rallyeHandle);

            PROPS[i]->position.x = posX;
            PROPS[i]->position.y = ((posY == 0xffff)? 0 : (255 - (posY + (GROUND_TILE_WIDTH / 2))));
            PROPS[i]->position.z = -posZ;

            // Determine the prop's type from the byte offsets to its 3d model data.
            if      (coordinateByteOffset == 0xd02c && indexByteOffset == 0xd088) PROPS[i]->type = PROP_TYPE_TREE;
            else if (coordinateByteOffset == 0x47e2 && indexByteOffset == 0x4c98) PROPS[i]->type = PROP_TYPE_WIRE_FENCE;
            else if (coordinateByteOffset == 0x47e2 && indexByteOffset == 0x4d98) PROPS[i]->type = PROP_TYPE_HORSE_FENCE;
            else if (coordinateByteOffset == 0x4766 && indexByteOffset == 0x4c20) PROPS[i]->type = PROP_TYPE_TRAFFIC_SIGN_80;
            else if (coordinateByteOffset == 0x4766 && indexByteOffset == 0x4c5c) PROPS[i]->type = PROP_TYPE_TRAFFIC_SIGN_EXCLAMATION;
            else if (coordinateByteOffset == 0x4aba && indexByteOffset == 0x4aec) PROPS[i]->type = PROP_TYPE_STONE_POST;
            else if (coordinateByteOffset == 0x4932 && indexByteOffset == 0x49e4) PROPS[i]->type = PROP_TYPE_LARGE_ROCK;
            else if (coordinateByteOffset == 0x498e && indexByteOffset == 0x49e4) PROPS[i]->type = PROP_TYPE_SMALL_ROCK;
            else if (coordinateByteOffset == 0x4466 && indexByteOffset == 0x4560) PROPS[i]->type = PROP_TYPE_LARGE_BILLBOARD;
            else if (coordinateByteOffset == 0x44e6 && indexByteOffset == 0x4660) PROPS[i]->type = PROP_TYPE_SMALL_BILLBOARD;
            else if (coordinateByteOffset == 0x48ee && indexByteOffset == 0x4b7c) PROPS[i]->type = PROP_TYPE_BUILDING;
            else if (coordinateByteOffset == 0x4324 && indexByteOffset == 0x434a) PROPS[i]->type = PROP_TYPE_UTIL_POLE_1;
            else if (coordinateByteOffset == 0x4324 && indexByteOffset == 0x439c) PROPS[i]->type = PROP_TYPE_UTIL_POLE_2;
            else if (coordinateByteOffset == 0x5488 && indexByteOffset == 0x5502) PROPS[i]->type = PROP_TYPE_STARTING_LINE;
            else if (coordinateByteOffset == 0x50d2 && indexByteOffset == 0x51c4) PROPS[i]->type = PROP_TYPE_STONE_STARTING_LINE;
            else if (coordinateByteOffset == 0x4ff2 && indexByteOffset == 0x51e0) PROPS[i]->type = PROP_TYPE_STONE_ARCH;
            else
            {
                assert(0 && "Invalid prop type.");
            }
        }

        kfile_close_file(rallyeHandle);
    }

    kground_update_ground_mesh(3, 22);

    return;
}

void kground_release_ground(void)
{
    free(HEIGHTMAP);
    free(TILEMAP);
    free(HEIGHTMAP_QUADS);
    kelpo_generic_stack__free(GROUND_MESH_POLYS);
    kelpo_generic_stack__free(GROUND_MESHES);

    return;
}

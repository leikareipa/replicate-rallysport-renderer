/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 * Represents a view (or "ground view") of a Rally-Sport track, including its
 * surface mesh (based on the track's MAASTO and VARIMAA data), any track props
 * (like trees and billboards), etc.
 * 
 * The view is realized as a set of polygonal meshes, returned by
 * kground_ground_meshes().
 * 
 */

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "common/genstack.h"
#include "common/file.h"
#include "renderer/vector.h"
#include "renderer/renderer.h"
#include "assets/ground.h"
#include "assets/mesh.h"

struct track_prop_s
{
    struct vector_s position;
    int type;
};

// The number of surface tiles vertically and horizontally to display in the
// ground view.
#define GROUND_VIEW_WIDTH 23
#define GROUND_VIEW_HEIGHT 24

// The dimensions, in world units, of a single tile in the surface mesh.
#define SURFACE_MESH_TILE_WIDTH 128
#define SURFACE_MESH_TILE_HEIGHT 128

// The height value of each corner point in the surface mesh.
static int16_t *HEIGHTMAP;
static unsigned HEIGHTMAP_WIDTH; // In tile units.
static unsigned HEIGHTMAP_HEIGHT;

// The PALAT texture index of each surface mesh tile.
static uint8_t *TILEMAP;
static unsigned TILEMAP_WIDTH; // In tile units.
static unsigned TILEMAP_HEIGHT;

// The meshes that constitute the ground view.
static struct kelpo_generic_stack_s *GROUND_VIEW_MESHES;

// Pre-allocated memory for building the surface mesh into.
static struct polygon_s *SURFACE_MESH_POLY_CACHE;

// All props on the given Rally-Sport track. Note that only those props that are
// visible in the current view will be included with its meshes.
#define MAX_NUM_PROPS 14 // How many props a track can have, at most.
static uint16_t NUM_PROPS = 0; // How many props this track has. Must be a 2-byte variable.
static struct track_prop_s* PROPS[MAX_NUM_PROPS];

// The vertices of the ground view meshes will be offset by this amount on the
// XYZ axes, so as to properly center them on the screen when rendered.
static struct vector_s GROUND_VIEW_SCREEN_OFFSET = {-(((GROUND_VIEW_WIDTH / 2) - 1) * SURFACE_MESH_TILE_WIDTH),
                                                    0,
                                                    -700};

// Convenience macro for querying the heightmap's value at the given XY
// coordinates, with bounds-checking on the coordinate values.
#define HEIGHT_AT(x, y) (((x) < 0) || ((x) > HEIGHTMAP_WIDTH) ||\
                         ((y) < 0) || ((y) >= HEIGHTMAP_HEIGHT))? 0 : -HEIGHTMAP[(x) + (y) * HEIGHTMAP_WIDTH]

// Convenience macro for querying the tilemap's value at the given XY
// coordinates, with bounds-checking on the coordinate values.
#define TILE_AT(x, y)   (((x) < 0) || ((x) > HEIGHTMAP_WIDTH) ||\
                         ((y) < 0) || ((y) >= HEIGHTMAP_HEIGHT))? 0 : TILEMAP[(x) + (y) * HEIGHTMAP_WIDTH]

// Figures out which spectator billboard texture should be drawn at the given
// track tile coordinates.
static unsigned spectator_billboard_idx(const unsigned x, const unsigned y)
{
    const unsigned firstSpectatorTexIdx = 236;      // Index of the first PALA representing a (standing) spectator.
    const unsigned lastSpectatorTexIdx = 239;       // Index of the last PALA representing a (standing) spectator. Assumes consecutive arrangement.
    const unsigned numSkins = 4;
    const unsigned sameRows = ((HEIGHTMAP_WIDTH == 128)? 16 : 32); // The game will repeat the same pattern of variants on the x axis this many times.

    const unsigned yOffs = (y / sameRows) % numSkins;
    const unsigned texOffs = ((x + (numSkins - 1)) + (yOffs * (numSkins - 1))) % numSkins;

    const unsigned textureIdx = (firstSpectatorTexIdx + texOffs);

    assert(((textureIdx >= firstSpectatorTexIdx) &&
            (textureIdx <= lastSpectatorTexIdx)) &&
           "Was going to return a spectator texture out of bounds. Not good.");

    return textureIdx;
}

int kground_width(void)
{
    return HEIGHTMAP_WIDTH;
}

int kground_height(void)
{
    return HEIGHTMAP_HEIGHT;
}

const struct kelpo_generic_stack_s* kground_ground_meshes(void)
{
    return GROUND_VIEW_MESHES;
}

void kground_update_ground_mesh(const int viewOffsX, const int viewOffsZ)
{
    kelpo_generic_stack__clear(GROUND_VIEW_MESHES);

    // Add surface tiles.
    {
        unsigned numPolys = 0;

        for (int z = 0; z < GROUND_VIEW_HEIGHT; z++)
        {
            for (int x = 0; x < GROUND_VIEW_WIDTH; x++)
            {
                const int tileX = (x + viewOffsX);
                const int tileY = (z + viewOffsZ);

                // Center the mesh on screen.
                const int vertX = ((x * SURFACE_MESH_TILE_WIDTH) + GROUND_VIEW_SCREEN_OFFSET.x);
                const int vertZ = ((-z * SURFACE_MESH_TILE_HEIGHT) + GROUND_VIEW_SCREEN_OFFSET.z);

                struct polygon_s *const groundPoly = &SURFACE_MESH_POLY_CACHE[numPolys++];

                // Back left.
                groundPoly->verts[0].x = vertX;
                groundPoly->verts[0].y = HEIGHT_AT(tileX, tileY);
                groundPoly->verts[0].z = vertZ;

                // Back right.
                groundPoly->verts[1].x = (vertX + SURFACE_MESH_TILE_WIDTH);
                groundPoly->verts[1].y = HEIGHT_AT((tileX + 1), tileY);
                groundPoly->verts[1].z = vertZ;

                // Front left.
                groundPoly->verts[2].x = vertX;
                groundPoly->verts[2].y = HEIGHT_AT(tileX, (tileY - 1));
                groundPoly->verts[2].z = (vertZ + SURFACE_MESH_TILE_HEIGHT);

                // Front right.
                groundPoly->verts[3].x = (vertX + SURFACE_MESH_TILE_WIDTH);
                groundPoly->verts[3].y = HEIGHT_AT((tileX + 1), (tileY - 1));
                groundPoly->verts[3].z = (vertZ + SURFACE_MESH_TILE_HEIGHT);

                const unsigned palaIdx = TILE_AT(tileX, (tileY - 1));
                groundPoly->texture = ktexture_pala_texture(palaIdx);

                // Add a billboard tile, if any.
                {
                    unsigned billboardPalaIdx = 0;

                    switch (palaIdx)
                    {
                        // Spectators.
                        case 240:
                        case 241:
                        case 242: billboardPalaIdx = spectator_billboard_idx(tileX, (tileY - 1)); break;

                        // Shrubs.
                        case 243: billboardPalaIdx = 208; break;
                        case 244: billboardPalaIdx = 209; break;
                        case 245: billboardPalaIdx = 210; break;

                        // Small poles.
                        case 246:
                        case 247: billboardPalaIdx = 211; break;
                        case 250: billboardPalaIdx = 212; break;

                        // Bridge.
                        case 248:
                        case 249: billboardPalaIdx = 177; break;

                        // No billboard.
                        default: break;
                    }

                    if (billboardPalaIdx)
                    {
                        struct polygon_s *const billboardPoly = &SURFACE_MESH_POLY_CACHE[numPolys++];
                        const int height = HEIGHT_AT(tileX, tileY);

                        // Bridge tile.
                        if (billboardPalaIdx == 177)
                        {
                            // Back left.
                            billboardPoly->verts[0].x = vertX;
                            billboardPoly->verts[0].y = 0;
                            billboardPoly->verts[0].z = vertZ;

                            // Back right.
                            billboardPoly->verts[1].x = (vertX + SURFACE_MESH_TILE_WIDTH);
                            billboardPoly->verts[1].y = 0;
                            billboardPoly->verts[1].z = vertZ;

                            // Front left.
                            billboardPoly->verts[2].x = vertX;
                            billboardPoly->verts[2].y = 0;
                            billboardPoly->verts[2].z = (vertZ + SURFACE_MESH_TILE_HEIGHT);

                            // Front right.
                            billboardPoly->verts[3].x = (vertX + SURFACE_MESH_TILE_WIDTH);
                            billboardPoly->verts[3].y = 0;
                            billboardPoly->verts[3].z = (vertZ + SURFACE_MESH_TILE_HEIGHT);
                        }
                        // Other billboards.
                        else
                        {
                            // Top left.
                            billboardPoly->verts[0].x = vertX;
                            billboardPoly->verts[0].y = (height - SURFACE_MESH_TILE_HEIGHT);
                            billboardPoly->verts[0].z = vertZ;

                            // Top right.
                            billboardPoly->verts[1].x = (vertX + SURFACE_MESH_TILE_WIDTH);
                            billboardPoly->verts[1].y = (height - SURFACE_MESH_TILE_HEIGHT);
                            billboardPoly->verts[1].z = vertZ;

                            // Bottom left.
                            billboardPoly->verts[2].x = vertX;
                            billboardPoly->verts[2].y = height;
                            billboardPoly->verts[2].z = vertZ;

                            // Bottom right.
                            billboardPoly->verts[3].x = (vertX + SURFACE_MESH_TILE_WIDTH);
                            billboardPoly->verts[3].y = height;
                            billboardPoly->verts[3].z = vertZ;
                        }

                        billboardPoly->texture = ktexture_pala_texture(billboardPalaIdx);
                    }
                }
            }
        }

        struct mesh_s heightmapMesh;

        heightmapMesh.x = heightmapMesh.y = heightmapMesh.z = 0;
        heightmapMesh.numPolys = numPolys;
        heightmapMesh.polys = SURFACE_MESH_POLY_CACHE;

        kelpo_generic_stack__push_copy(GROUND_VIEW_MESHES, &heightmapMesh);
    }

    // Add props.
    for (unsigned i = 0; i < NUM_PROPS; i++)
    {
        const int meshX = (PROPS[i]->position.x - ((viewOffsX * SURFACE_MESH_TILE_WIDTH) - GROUND_VIEW_SCREEN_OFFSET.x));
        const int meshZ = (PROPS[i]->position.z + (viewOffsZ * SURFACE_MESH_TILE_HEIGHT) + GROUND_VIEW_SCREEN_OFFSET.z);
        const int meshY = (PROPS[i]->position.y
                           ? PROPS[i]->position.y
                           : HEIGHT_AT(((int)PROPS[i]->position.x / SURFACE_MESH_TILE_WIDTH), (-(int)PROPS[i]->position.z / SURFACE_MESH_TILE_HEIGHT)));
        
        // If the prop isn't within the view frustum, don't add it.
        if ((meshZ > (GROUND_VIEW_SCREEN_OFFSET.z + (1 * SURFACE_MESH_TILE_HEIGHT))) ||
            (meshZ < (GROUND_VIEW_SCREEN_OFFSET.z - ((GROUND_VIEW_HEIGHT + 3) * SURFACE_MESH_TILE_HEIGHT))) ||
            (meshX < (GROUND_VIEW_SCREEN_OFFSET.x - (3 * SURFACE_MESH_TILE_WIDTH))) ||
            (meshX > (GROUND_VIEW_SCREEN_OFFSET.x + ((GROUND_VIEW_WIDTH + 3) * SURFACE_MESH_TILE_WIDTH))))
        {
            continue;
        }

        struct mesh_s propMesh = kmesh_prop_mesh(PROPS[i]->type, meshX, meshY, meshZ);

        kelpo_generic_stack__push_copy(GROUND_VIEW_MESHES, &propMesh);
    }

    return;
}

void kground_initialize_ground(const unsigned groundIdx)
{
    assert((groundIdx <= 8) && "Ground index out of bounds.");

    GROUND_VIEW_MESHES = kelpo_generic_stack__create((MAX_NUM_PROPS + 1), sizeof(struct mesh_s));

    // Pre-allocate memory for as many surface polygons as we're going to need
    // at most. Since each surface tile (a quad polygon) can optionally have a
    // billboard tile (e.g. a spectator), we need to allocate twice the size
    // of the ground view.
    {
        const unsigned maxNumSurfacePolys = (2 * GROUND_VIEW_WIDTH * GROUND_VIEW_HEIGHT);

        SURFACE_MESH_POLY_CACHE = malloc(sizeof(*SURFACE_MESH_POLY_CACHE) * maxNumSurfacePolys);

        for (unsigned i = 0; i < maxNumSurfacePolys; i++)
        {
            SURFACE_MESH_POLY_CACHE[i] = kpolygon_create_polygon(4);
        }
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

        TILEMAP_WIDTH = HEIGHTMAP_WIDTH;
        TILEMAP_HEIGHT = HEIGHTMAP_HEIGHT;
        TILEMAP = malloc(TILEMAP_WIDTH * TILEMAP_HEIGHT);
        kfile_read_byte_array(TILEMAP, (TILEMAP_WIDTH * TILEMAP_HEIGHT), varimaaHandle);

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
            PROPS[i]->position.y = ((posY == 0xffff)? 0 : (255 - (posY + (SURFACE_MESH_TILE_WIDTH * 2))));
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
    free(SURFACE_MESH_POLY_CACHE);
    kelpo_generic_stack__free(GROUND_VIEW_MESHES);

    for (unsigned i = 0; i < NUM_PROPS; i++)
    {
        free(PROPS[i]);
    }

    return;
}

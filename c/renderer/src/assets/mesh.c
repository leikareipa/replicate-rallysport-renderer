/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 * Handles loading and serving Rally-Sport's mesh assets.
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "common/genstack.h"
#include "common/file.h"
#include "assets/mesh.h"

static struct mesh_s *PROP_MESHES;

struct mesh_s load_prop_mesh(const int propType)
{
    struct mesh_s mesh;
    const file_handle_t rallyeHandle = kfile_open_file("RALLYE.EXE", "rb");
    struct kelpo_generic_stack_s *polyStack = kelpo_generic_stack__create(0, sizeof(struct polygon_s));

    // Byte offsets in RALLYE.EXE where the corresponding data begins.
    uint32_t vertexCoordsOffs = 0;
    uint32_t vertexIndicesOffs = 0;

    switch (propType)
    {
        case PROP_TYPE_TREE:                     vertexCoordsOffs = 0xd02c; vertexIndicesOffs = 0xd088; break;
        case PROP_TYPE_WIRE_FENCE:               vertexCoordsOffs = 0x47e2; vertexIndicesOffs = 0x4c98; break;
        case PROP_TYPE_HORSE_FENCE:              vertexCoordsOffs = 0x47e2; vertexIndicesOffs = 0x4d98; break;
        case PROP_TYPE_TRAFFIC_SIGN_80:          vertexCoordsOffs = 0x4766; vertexIndicesOffs = 0x4c20; break;
        case PROP_TYPE_TRAFFIC_SIGN_EXCLAMATION: vertexCoordsOffs = 0x4766; vertexIndicesOffs = 0x4c5c; break;
        case PROP_TYPE_STONE_ARCH:               vertexCoordsOffs = 0x4ff2; vertexIndicesOffs = 0x51e0; break;
        case PROP_TYPE_STONE_POST:               vertexCoordsOffs = 0x4aba; vertexIndicesOffs = 0x4aec; break;
        case PROP_TYPE_LARGE_ROCK:               vertexCoordsOffs = 0x4932; vertexIndicesOffs = 0x49e4; break;
        case PROP_TYPE_SMALL_ROCK:               vertexCoordsOffs = 0x498e; vertexIndicesOffs = 0x49e4; break;
        case PROP_TYPE_LARGE_BILLBOARD:          vertexCoordsOffs = 0x4466; vertexIndicesOffs = 0x4560; break;
        case PROP_TYPE_SMALL_BILLBOARD:          vertexCoordsOffs = 0x44e6; vertexIndicesOffs = 0x4660; break;
        case PROP_TYPE_BUILDING:                 vertexCoordsOffs = 0x48ee; vertexIndicesOffs = 0x4b7c; break;
        case PROP_TYPE_UTIL_POLE_1:              vertexCoordsOffs = 0x4324; vertexIndicesOffs = 0x434a; break;
        case PROP_TYPE_UTIL_POLE_2:              vertexCoordsOffs = 0x4324; vertexIndicesOffs = 0x439c; break;
        case PROP_TYPE_STARTING_LINE:            vertexCoordsOffs = 0x5488; vertexIndicesOffs = 0x5502; break;
        case PROP_TYPE_STONE_STARTING_LINE:      vertexCoordsOffs = 0x50d2; vertexIndicesOffs = 0x51c4; break;
        default: assert(0 && "Unknown prop type."); break;
    }

    // Append the starting offset of RALLYE.EXE's data segment.
    vertexCoordsOffs += 70560;
    vertexIndicesOffs += 70560;

    // Load vertex coordinates.
    struct vertex_s *vertexCoords = NULL;
    {
        kfile_seek(vertexCoordsOffs, rallyeHandle);

        uint16_t numCoords = 0;    
        kfile_read_byte_array((uint8_t*)&numCoords, sizeof(numCoords), rallyeHandle);
        vertexCoords = malloc(sizeof(*vertexCoords) * numCoords);

        for (int i = 0; i < numCoords; i++)
        {
            int16_t coords[3];
            kfile_read_byte_array((uint8_t*)coords, (sizeof(*coords) * 3), rallyeHandle);
            
            vertexCoords[i].x = coords[0];
            vertexCoords[i].y = coords[1];
            vertexCoords[i].z = coords[2];
        }
    }

    assert(vertexCoords && "Failed to properly load prop mesh vertex coordinates.");

    // Load polygons.
    kfile_seek(vertexIndicesOffs, rallyeHandle);
    for (;;)
    {
        // The first 2 bytes of the one-after-last polygon entry are 0xFFFF.
        {
            uint16_t word;
            kfile_read_byte_array((uint8_t*)&word, 2, rallyeHandle);

            if (word == 0xffff)
            {
                break;
            }

            kfile_jump(-2, rallyeHandle);
        }

        uint16_t fillStyle = 0;
        kfile_read_byte_array((uint8_t*)&fillStyle, 2, rallyeHandle);

        // Skip some bytes whose purpose we don't know.
        kfile_jump(8, rallyeHandle);

        // Get the polygon's vertices.
        uint16_t numVerts = 0;
        uint16_t *vertexIndices = NULL;
        {
            kfile_read_byte_array((uint8_t*)&numVerts, 2, rallyeHandle);

            // Read in the vertex indices.
            vertexIndices = malloc(sizeof(*vertexIndices) * numVerts);
            {
                // First index.
                kfile_read_byte_array((uint8_t*)&vertexIndices[0], sizeof(*vertexIndices), rallyeHandle);

                // Rest of the indices.
                for (int i = 0; i < (numVerts - 1); i++)
                {
                    kfile_read_byte_array((uint8_t*)&vertexIndices[i+1], sizeof(*vertexIndices), rallyeHandle);
                    kfile_jump(2, rallyeHandle);
                }

                // Skip the last index, which just connects to the first index.
                kfile_jump(2, rallyeHandle);
            }
        }

        // Construct the polygon.
        struct polygon_s poly = kpolygon_create_polygon(numVerts);
        {
            poly.visible = 1;
            
            // Solid color without a texture.
            if (fillStyle < 32)
            {
                poly.texture = NULL;
                poly.color = fillStyle;
            }
            // A texture without a solid color.
            else
            {
                poly.color = 0;
                poly.texture = ktexture_prop_texture((fillStyle - 32) % 128);
            }

            for (int i = 0; i < numVerts; i++)
            {
                poly.verts[i] = vertexCoords[vertexIndices[i]];
            }
        }

        kelpo_generic_stack__push_copy(polyStack, &poly);

        free(vertexIndices);
    }

    // Copy into the mesh the polygons we've constructed.
    mesh.numPolys = polyStack->count;
    mesh.polys = malloc(sizeof(struct polygon_s) * polyStack->count);
    memcpy(mesh.polys, polyStack->data, sizeof(struct polygon_s) * polyStack->count);

    free(vertexCoords);
    kelpo_generic_stack__free(polyStack);
    kfile_close_file(rallyeHandle);

    return mesh;
}

struct mesh_s kmesh_prop_mesh(const int propType, const float x, const float y, const float z)
{
    assert(((propType >= 0) && (propType <= PROP_TYPE_COUNT)) && "Accessing props out of bounds.");

    struct mesh_s mesh = PROP_MESHES[propType];
    mesh.x = x;
    mesh.y = y;
    mesh.z = z;

    return mesh;
}

void kmesh_initialize_meshes(void)
{
    PROP_MESHES = malloc(sizeof(*PROP_MESHES) * PROP_TYPE_COUNT);

    for (unsigned i = 0; i < PROP_TYPE_COUNT; i++)
    {
        PROP_MESHES[i] = load_prop_mesh(i);
    }
    
    return;
}

void kmesh_release_meshes(void)
{
    free(PROP_MESHES);
    
    return;
}

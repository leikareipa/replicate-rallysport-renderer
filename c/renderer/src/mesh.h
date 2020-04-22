/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#ifndef MODEL_H
#define MODEL_H

#include "polygon.h"

struct mesh_s
{
    unsigned numPolys;
    struct polygon_s *polys;

    // The mesh's world position. These values will be added to copies of the
    // mesh's polygon vertex values at render-time.
    float x, y, z;
};

enum
{
    PROP_TYPE_TREE,
    PROP_TYPE_WIRE_FENCE,
    PROP_TYPE_HORSE_FENCE,
    PROP_TYPE_TRAFFIC_SIGN_80,
    PROP_TYPE_TRAFFIC_SIGN_EXCLAMATION,
    PROP_TYPE_STONE_ARCH,
    PROP_TYPE_STONE_POST,
    PROP_TYPE_LARGE_ROCK,
    PROP_TYPE_SMALL_ROCK,
    PROP_TYPE_LARGE_BILLBOARD,
    PROP_TYPE_SMALL_BILLBOARD,
    PROP_TYPE_BUILDING,
    PROP_TYPE_UTIL_POLE_1,
    PROP_TYPE_UTIL_POLE_2,
    PROP_TYPE_STARTING_LINE,
    PROP_TYPE_STONE_STARTING_LINE,

    // Must be last.
    PROP_TYPE_COUNT
};

// Returns the polygon mesh of the prop of the given type (e.g. PROP_TYPE_TREE)
// positioned at the given XYZ world coordinates.
struct mesh_s kmesh_prop_mesh(const int propType, const float x, const float y, const float z);

void kmesh_initialize_meshes(void);

void kmesh_release_meshes(void);

#endif

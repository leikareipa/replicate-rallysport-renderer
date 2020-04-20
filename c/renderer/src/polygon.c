/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#include <stdlib.h>
#include "polygon.h"
#include "vertex.h"

struct polygon_s kpolygon_create_polygon(const uint16_t numVerts)
{
    struct polygon_s poly;
    
    poly.numVerts = numVerts;
    poly.verts = calloc((poly.numVerts + 1), sizeof(struct vertex_s));

    return poly;
}

void kpolygon_release_polygon(struct polygon_s *const polygon)
{
    free(polygon->verts);
    polygon->verts = NULL;
    polygon->numVerts = 0;

    return;
}

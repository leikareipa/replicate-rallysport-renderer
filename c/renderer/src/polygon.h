/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#ifndef POLYGON_H
#define POLYGON_H

#include "texture.h"
#include "vertex.h"

struct polygon_s
{
    unsigned numVerts;
    struct vertex_s *verts;

    uint8_t color; /* Palette index.*/
    struct texture_s *texture;
};

struct polygon_s kpolygon_create_polygon(const unsigned numVerts);

void kpolygon_release_polygon(struct polygon_s *const polygon);

#endif

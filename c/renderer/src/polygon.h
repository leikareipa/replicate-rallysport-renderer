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
    uint16_t numVerts;
    struct vertex_s *verts;

    uint8_t color; // As a palette index.
    struct texture_s *texture;

    // If 0, the polygon won't be rendered.
    uint8_t visible;
};

struct polygon_s kpolygon_create_polygon(const uint16_t numVerts);

void kpolygon_release_polygon(struct polygon_s *const polygon);

#endif

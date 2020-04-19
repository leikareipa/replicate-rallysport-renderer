/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 * Handles filling (rasterizing) polygons into the current rendere buffer.
 * 
 * NOTE: This file expects to be #included in renderer.c.
 * 
 */

#include <stdlib.h>
#include <math.h>
#include "texture.h"

/* The maximum number of vertices per polygon we support.*/
#define MAX_VERTEX_COUNT 16

/* Initialize increments for vertical interpolation.*/
static void init_lerp_deltas(float *const deltaX,
                             const int dir,
                             const unsigned vertexIdx,
                             const struct vertex_s *const verts)
{
    /* Horizontal edges.*/
    if (verts[vertexIdx].y == verts[vertexIdx + dir].y)
    {
        *deltaX = 0;
    }
    else
    {
        const float height = (verts[vertexIdx + dir].y - verts[vertexIdx].y);

        *deltaX = ((verts[vertexIdx + dir].x - verts[vertexIdx].x) / height);
    }

    return;
}

/* Initialize values to be interpolated vertically.*/
static void init_lerp_values(float *const x,
                             const unsigned vertexIdx,
                             const struct vertex_s *const verts)
{
    *x = verts[vertexIdx].x;

    return;
}

int compareY(const void *a, const void *b)
{
    if (((struct vertex_s*)a)->y <  ((struct vertex_s*)b)->y)
    {
        return -1;
    }
    else if (((struct vertex_s*)a)->y == ((struct vertex_s*)b)->y)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/* Sorts the polygon's vertices in counter-clockwise order, starting from the
 * top (lowest Y) and winding around the polygon back to the top.*/
static unsigned sort_vertices_ccw(struct polygon_s *poly)
{
    unsigned i;
    unsigned numLeftVerts = 0;
    unsigned numRightVerts = 0;
    static struct vertex_s leftVerts[MAX_VERTEX_COUNT];
    static struct vertex_s rightVerts[MAX_VERTEX_COUNT];

    assert((poly->numVerts < MAX_VERTEX_COUNT) && "Too many vertices.");

    if (!poly->numVerts)
    {
        return 0;
    }

    /* Sort verts by Y coordinate from lowest to highest (top to bottom on the
     * screen).*/
    qsort(poly->verts, poly->numVerts, sizeof(poly->verts[0]), compareY);
    
    struct vertex_s *const topVert = &poly->verts[0];
    struct vertex_s *const bottomVert = &poly->verts[poly->numVerts - 1];
    const unsigned polyHeight = (bottomVert->y - topVert->y);

    leftVerts[numLeftVerts++] = *topVert;

    for (i = 1; i < (poly->numVerts - 1); i++)
    {
        const float lr = LERP(topVert->x,
                              bottomVert->x,
                              ((poly->verts[i].y - topVert->y) / (bottomVert->y - topVert->y)));

        if (poly->verts[i].x < lr)
        {
            leftVerts[numLeftVerts++] = poly->verts[i];
        }
        else
        {
            rightVerts[numRightVerts++] = poly->verts[i];
        }
    }

    leftVerts[numLeftVerts++] = *bottomVert;
    
    for (i = 0; i < numLeftVerts; i++)
    {
        poly->verts[i] = leftVerts[i];
    }

    for (i = 0; i < numRightVerts; i++)
    {
        poly->verts[i + numLeftVerts] = rightVerts[numRightVerts - i - 1];
    }

    return polyHeight;
}

void fill_poly(struct polygon_s *const poly)
{
    /// Texture created for temporary testing.
    const struct texture_s texture = ktexture_load_texture(16, TEXTURE_SOURCE_TEXT_1);

    if (!poly->numVerts)
    {
        return;
    }

    const unsigned polyHeight = sort_vertices_ccw(poly);

    /* Complete the vertex loop by connecting an extra vertex at the end to
     * the beginning. This simplifies rendering.*/
    poly->verts[poly->numVerts] = poly->verts[0];

    unsigned y = poly->verts[0].y;
    unsigned leftVertIdx = 0;
    unsigned rightVertIdx = poly->numVerts;

    /* Vertical interpolation deltas.*/
    float deltaStartX, deltaEndX;
    float textureVDelta = (texture.height / (float)polyHeight);

    /* Vertical interpolated values.*/
    float startX, endX;
    float textureV = 0;

    init_lerp_deltas(&deltaStartX, 1, leftVertIdx, poly->verts);
    init_lerp_deltas(&deltaEndX, -1, rightVertIdx, poly->verts);

    init_lerp_values(&startX, leftVertIdx, poly->verts);
    init_lerp_values(&endX, rightVertIdx, poly->verts);

    /* Fill.*/
    for (;;)
    {
        if (y >= GRAPHICS_MODE_HEIGHT)
        {
            break;
        }
        
        if (y == poly->verts[leftVertIdx + 1].y)
        {
            leftVertIdx++;
            init_lerp_deltas(&deltaStartX, 1, leftVertIdx, poly->verts);
            init_lerp_values(&startX, leftVertIdx, poly->verts);
        }

        if (y == poly->verts[rightVertIdx - 1].y)
        {
            rightVertIdx--;
            init_lerp_deltas(&deltaEndX, -1, rightVertIdx, poly->verts);
            init_lerp_values(&endX, rightVertIdx, poly->verts);
        }

        /* When we reach the bottom of the polygon, we're done.*/
        if (y >= (polyHeight + poly->verts[0].y))
        {
            break;
        }

        /* Fill the current raster line.*/
        if ((y >= 0) && (endX > startX))
        {
            const float lineWidth = (endX - startX + 1);

            /* Horizontal interpolated values.*/
            float textureU = 0;

            /* Horizontal interpolation deltas.*/
            float deltaTextureU = (texture.width / lineWidth);

            for (unsigned x = startX; x < endX; x++)
            {
                VRAM_XY(x, y) = texture.pixels[(unsigned)(floor(textureU) + (texture.height - floor(textureV) - 1) * texture.width)];

                /* Increment horizontal deltas.*/
                textureU += deltaTextureU;
            }
        }

        /* Increment vertical deltas.*/
        startX += deltaStartX;
        endX += deltaEndX;
        textureV += textureVDelta;

        y++;
    }

    /* Visually indicate the locations of the poly's vertices. For debugging
     * reasons.*/
   /* for (y = 0; y < poly->numVerts; y++)
    {
        VRAM_XY(poly->verts[y].x, poly->verts[y].y) = y%10+10;
    }*/

    return;
}

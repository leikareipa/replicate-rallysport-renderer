/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 * Transforms polygons.
 * 
 * NOTE: This file expects to be #included in renderer.c.
 * 
 */

#include <math.h>

// Perspective division to a vanishing point at the top center of the screen
// (e.g. to x=160, y=0 in VGA mode 13h).
void krender_transform_poly(struct polygon_s *const poly)
{
    const float screenWidthHalf = (GRAPHICS_MODE_WIDTH / 2);

    for (unsigned i = 0; i < poly->numVerts; i++)
    {
        poly->verts[i].x = round(screenWidthHalf + ((CAMERA_POS.x + poly->verts[i].x - screenWidthHalf) / (CAMERA_POS.z + poly->verts[i].z)));
        poly->verts[i].y = round((CAMERA_POS.y + poly->verts[i].y) / (CAMERA_POS.z + poly->verts[i].z));
    }

    // Find whether at least one of the polygon's transformed vertices is inside
    // the view frustum.
    poly->visible = 0;
    for (unsigned i = 0; i < poly->numVerts; i++)
    {
        if (((poly->verts[i].x >= 0) && (poly->verts[i].x < GRAPHICS_MODE_WIDTH)) &&
            ((poly->verts[i].y >= 0) && (poly->verts[i].y < GRAPHICS_MODE_HEIGHT)))
        {
            poly->visible = 1;
            break;
        }
    }
    
    return;
}

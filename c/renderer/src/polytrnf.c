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
        poly->verts[i].x = ceil(screenWidthHalf + ((CAMERA_POS.x + poly->verts[i].x - screenWidthHalf) / (CAMERA_POS.z + poly->verts[i].z)));
        poly->verts[i].y = ceil((CAMERA_POS.y + poly->verts[i].y) / (CAMERA_POS.z + poly->verts[i].z));
    }
    
    return;
}

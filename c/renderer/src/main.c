/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include "renderer.h"
#include "polygon.h"

#include "../bin/gate.c"

int main(void)
{
    krender_enter_grapics_mode();
    krender_clear_screen();

    /* Render a test model.*/
    {
        struct polygon_s *model = gate_model();

        for (int i = gateModelPolyCount-1; i >= 0; i--)
        {
            krender_draw_test_pattern(&model[i]);
            kpolygon_release_polygon(&model[i]);
        }

        free(model);
    }
    
    return 0;
}

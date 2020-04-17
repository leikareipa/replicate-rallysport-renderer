/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#include <stdio.h>
#include "renderer.h"

int main(void)
{
    krender_enter_grapics_mode();
    krender_clear_screen();
    krender_draw_test_pattern();
    
    return 0;
}

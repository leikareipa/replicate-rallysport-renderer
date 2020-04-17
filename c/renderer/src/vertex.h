/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#ifndef VERTEX_H
#define VERTEX_H

#include <stdint.h>

struct vertex_s
{
    float x, y, z;
    uint8_t color; /* Palette index.*/
};

#endif

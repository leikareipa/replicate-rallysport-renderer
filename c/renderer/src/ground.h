/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Render test for replicating Rally-Sport's rendering.
 * 
 */

#ifndef GROUND_H
#define GROUND_H

int kground_width(void);

int kground_height(void);

const struct kelpo_generic_stack_s* kground_ground_meshes(void);

void kground_update_ground_mesh(const int viewOffsX, const int viewOffsZ);

void kground_initialize_ground(const unsigned groundIdx);

void kground_release_ground(void);

#endif

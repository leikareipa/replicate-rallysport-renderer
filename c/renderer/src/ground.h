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

const struct mesh_s* kground_ground_mesh(void);

void kground_update_ground_mesh(void);

void kground_initialize_ground(const char *const filename);

void kground_release_ground(void);

#endif

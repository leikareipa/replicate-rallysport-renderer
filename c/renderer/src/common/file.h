/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <stdio.h>

typedef unsigned file_handle_t;

file_handle_t kfile_open_file(const char *const filename, const char *const mode);

void kfile_close_file(const file_handle_t handle);

void kfile_seek(const uint32_t pos, const file_handle_t handle);

void kfile_jump(const int32_t posDelta, const file_handle_t handle);

uint32_t kfile_file_size(const file_handle_t handle);

FILE* kfile_exposed_file_handle(file_handle_t handle);

void kfile_read_byte_array(uint8_t *dst, const size_t numBytes, const file_handle_t handle);

void kfile_create_directory(const char *const name, const int warnIfExists);

void kfile_seek(const uint32_t pos, const file_handle_t handleId);

int kfile_getline(const file_handle_t handle, char *const dst, const size_t maxLen);

void kfile_append_contents(const file_handle_t fhSrc, const file_handle_t fhDst);

long kfile_position(const file_handle_t handle);

void kfile_rewind_file(const file_handle_t handle);

void kfile_flush_file(const file_handle_t handle);

void kfile_fill(const unsigned char byte, const unsigned long len, const file_handle_t handle);

void kfile_write_byte_array(const unsigned char *const src, const unsigned long len, const file_handle_t handle);

void kfile_write_string(const char *const str, const file_handle_t handle);

#endif

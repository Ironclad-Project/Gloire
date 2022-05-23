#pragma once

#include <stddef.h>
#include <stdint.h>

uint32_t *open_bmp_to_array(const char *path, size_t *size, size_t *width,
                            size_t *height);

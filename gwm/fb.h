#pragma once

#include <stdint.h>
#include <stddef.h>

void draw_background(uint32_t *buffer, uint32_t color);
void draw_pixel(uint32_t *buffer, int x, int y, uint32_t color);
void draw_character(uint32_t *buffer, int fbx, int fby, char c, uint32_t fg, uint32_t bg);
void draw_string(uint32_t *buffer, int x, int y, const char *str, size_t char_count, uint32_t fg, uint32_t bg);

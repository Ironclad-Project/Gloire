#pragma once

#include <stddef.h>

#define FONT_HEIGHT 16
#define FONT_WIDTH  8

extern char font[];

size_t get_char_size();
size_t get_char_index(char c);

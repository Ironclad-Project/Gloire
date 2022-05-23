#pragma once

#include <stdint.h>
#include <stddef.h>

struct framebuffer {
    int       backing_fd;
    uint32_t *background;
    uint32_t *memory;
    size_t    pixel_width;
    size_t    pixel_height;
    size_t    pitch;
    size_t    pixel_size;
    size_t    linear_size;
};

struct framebuffer *create_framebuffer_from_fd(int fd);
void refresh_to_backend(struct framebuffer *fb);
void draw_background(struct framebuffer *fb);
void draw_pixel(struct framebuffer *fb, int x, int y, uint32_t color);
void draw_rectangle(struct framebuffer *fb, int x, int y, int x_end, int y_end, uint32_t color);
void draw_character(struct framebuffer *fb, int fbx, int fby, char c, uint32_t fg, uint32_t bg);
void draw_string(struct framebuffer *fb, int x, int y, const char *str, size_t char_count, uint32_t fg, uint32_t bg);

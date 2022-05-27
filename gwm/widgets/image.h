#pragma once

#include <fb.h>

struct image {
    uint32_t *image_data;
    size_t    image_size;
    size_t    image_width;
    size_t    image_height;
};

struct image *init_image(const char *path);
void draw_image(struct image *im, struct framebuffer *fb, int start_x, int start_y, int width_x, int width_y);
void destroy_image(struct image *im);

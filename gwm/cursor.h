#pragma once

#include <fb.h>

struct cursor {
    int x_position;
    int y_position;
};

struct cursor *create_cursor(void);
void update_cursor(struct cursor *c, struct framebuffer *fb, int x_variation, int y_variation);
void draw_cursor(struct cursor *c, struct framebuffer *fb);

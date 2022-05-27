#pragma once

#include <fb.h>

struct textbox {
    const char *text;
};

struct textbox *init_textbox(const char *text);
void draw_textbox(struct textbox *tx, struct framebuffer *fb, int start_x, int start_y, int width_x, int width_y);
void destroy_textbox(struct textbox *tx);

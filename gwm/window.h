#pragma once

#include <stdint.h>
#include <fb.h>

#define WINDOW_NAME_LEN 64
#define WINDOW_TEXT_LEN 40

struct window {
    char name[WINDOW_NAME_LEN];
    int top_corner_x;
    int top_corner_y;
    int length_x;
    int length_y;
    const char *children_text[WINDOW_TEXT_LEN];
};

struct window *create_window(const char *name);
void add_text(struct window *win, const char *text);
void draw_window(struct window *win, struct framebuffer *fb);

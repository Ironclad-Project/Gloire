#pragma once

#include <stdint.h>
#include <fb.h>

#define WINDOW_NAME_LEN 64
#define WINDOW_TEXT_LEN 40

struct window {
    char name[WINDOW_NAME_LEN];
    int is_focus;
    int top_corner_x;
    int top_corner_y;
    int length_x;
    int length_y;
    const char *children_text[WINDOW_TEXT_LEN];
};

struct window *create_window(const char *name);
void add_text(struct window *win, const char *text);
int pixel_is_in_window_bar(struct window *win, int x, int y);
int pixel_is_in_window(struct window *win, int x, int y);
void move_window(struct window *win, int x_variation, int y_variation);
void focus_window(struct window *win);
void unfocus_window(struct window *win);
void draw_window(struct window *win, struct framebuffer *fb);

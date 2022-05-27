#pragma once

#include <stdint.h>
#include <widgets/widget.h>
#include <fb.h>

#define WINDOW_NAME_LEN 64
#define WINDOW_CHILDREN_LEN 40

struct window {
    char name[WINDOW_NAME_LEN];
    int is_focus;
    int is_packed;
    int top_corner_x;
    int top_corner_y;
    int length_x;
    int length_y;
    struct widget *children[WINDOW_CHILDREN_LEN];
};

struct window *create_window(const char *name);
void add_child(struct window *win, struct widget *wid);
int pixel_is_in_window_bar(struct window *win, int x, int y);
int pixel_is_in_window(struct window *win, int x, int y);
void move_window(struct window *win, int x_variation, int y_variation, int start_x, int start_y, int max_x, int max_y);
void pack_window(struct window *win);
void focus_window(struct window *win);
void unfocus_window(struct window *win);
void draw_window(struct window *win, struct framebuffer *fb);
void destroy_window(struct window *win);

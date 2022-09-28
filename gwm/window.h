#pragma once

#include <stdint.h>
#include <widgets/widget.h>
#include <fb.h>

#define WINDOW_NAME_LEN     64
#define WINDOW_CHILDREN_LEN 20

struct window {
    char name[WINDOW_NAME_LEN];
    int is_focus;
    int top_corner_x;
    int top_corner_y;
    int length_x;
    int length_y;
    struct widget *children[WINDOW_CHILDREN_LEN];
};

enum window_click_action {
    WINDOW_NOT_TOUCHED,   // The click didnt land in the window.
    WINDOW_CONTENT_CLICK, // The click landed inside the window.
    WINDOW_BAR_CLICK,     // The click landed on the window bar.
    WINDOW_PLEASE_CLOSE   // The click told the window to be closed.
};

struct window *create_window(const char *name);
void add_child(struct window *win, struct widget *wid);
enum window_click_action click_window(struct window *win, int x, int y);
void move_window(struct window *win, int x_variation, int y_variation,
    int start_x, int start_y, int max_x, int max_y);
void focus_window(struct window *win);
void unfocus_window(struct window *win);
void draw_window(struct window *win, struct framebuffer *fb);
void destroy_window(struct window *win);

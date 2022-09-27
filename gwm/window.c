#include <window.h>
#include <stdlib.h>
#include <string.h>
#include <font.h>
#include <fb.h>

#define TITLEBAR_FONT_COLOR     0xffffff
#define TITLEBAR_FOCUS_COLOR    0xff8888
#define TITLEBAR_NO_FOCUS_COLOR 0xaaaaaa
#define WINDOW_BACKGROUND_COLOR 0x888888
#define WINDOW_BORDERS_COLOR    0xffffff

struct window *create_window(const char *name) {
    struct window *ret = malloc(sizeof(struct window));
    strncpy(ret->name, name, WINDOW_NAME_LEN - 1);
    ret->is_focus     = 0;
    ret->is_packed    = 0;
    ret->top_corner_x = 100;
    ret->top_corner_y = 100;
    ret->length_x     = 570;
    ret->length_y     = 300;
    memset(ret->children, 0, sizeof(ret->children));
    return ret;
}

void add_child(struct window *win, struct widget *wid) {
    for (int i = 0; i < WINDOW_CHILDREN_LEN; i++) {
        if (win->children[i] == NULL) {
            win->children[i] = wid;
            return;
        }
    }
}

enum window_click_action click_window(struct window *win, int x, int y) {
    if (x >= win->top_corner_x && x <= win->top_corner_x + win->length_x &&
        y >= win->top_corner_y - FONT_HEIGHT && y <= win->top_corner_y) {
        return WINDOW_BAR_CLICK;
    } else if (x >= win->top_corner_x && x <= win->top_corner_x + win->length_x &&
        y >= win->top_corner_y - FONT_HEIGHT && y <= win->top_corner_y + win->length_y) {
        return WINDOW_CONTENT_CLICK;
    } else {
        return WINDOW_NOT_TOUCHED;
    }
}

void move_window(struct window *win, int x_variation, int y_variation, int start_x, int start_y, int max_x, int max_y) {
    win->top_corner_x += x_variation;
    win->top_corner_y += y_variation;

    if (win->top_corner_x + win->length_x < start_x + 20) {
        win->top_corner_x = start_x + 20 - win->length_x;
    } else if (win->top_corner_x + 20 > max_x) {
        win->top_corner_x = max_x - 20;
    }

    if (win->top_corner_y <= start_y + FONT_HEIGHT) {
        win->top_corner_y = start_y + FONT_HEIGHT;
    } else if (win->top_corner_y > max_y - FONT_HEIGHT) {
        win->top_corner_y = max_y - FONT_HEIGHT;
    }
}

void pack_window(struct window *win) {
    win->is_packed = 1;
}

void focus_window(struct window *win) {
    win->is_focus = 1;
}

void unfocus_window(struct window *win) {
    win->is_focus = 0;
}

void draw_window(struct window *win, struct framebuffer *fb) {
    const int start_x = win->top_corner_x;
    const int final_x = win->top_corner_x + win->length_x;
    const int start_y = win->top_corner_y;
    const int final_y = win->top_corner_y + win->length_y;

    // Draw the window bar.
    const uint32_t title_color = win->is_focus ? TITLEBAR_FOCUS_COLOR : TITLEBAR_NO_FOCUS_COLOR;
    draw_rectangle(fb, start_x, start_y - FONT_HEIGHT, final_x, start_y, title_color);

    // Draw the rest of the window background.
    draw_rectangle(fb, start_x, start_y, final_x, final_y, WINDOW_BACKGROUND_COLOR);

    // Draw window title.
    size_t char_count = strlen(win->name);
    if (char_count * FONT_WIDTH > win->length_x) {
        char_count = win->length_x / FONT_WIDTH;
    }
    draw_string(fb, start_x, start_y - FONT_HEIGHT, win->name, char_count, TITLEBAR_FONT_COLOR, title_color);

    // Draw borders.
    for (int i = -FONT_HEIGHT; i <= win->length_y; i++) {
        draw_pixel(fb, start_x - 1, start_y + i, WINDOW_BORDERS_COLOR);
        draw_pixel(fb, final_x,     start_y + i, WINDOW_BORDERS_COLOR);
    }
    for (int i = 0; i < win->length_x; i++) {
        draw_pixel(fb, start_x + i, start_y - FONT_HEIGHT, WINDOW_BORDERS_COLOR);
        draw_pixel(fb, start_x + i, final_y, WINDOW_BORDERS_COLOR);
    }

    // Count the children so we can divide the window real state in rows.
    int child_count = 0;
    for (int i = 0; i < WINDOW_CHILDREN_LEN; i++) {
        if (win->children[i] != NULL) {
            child_count++;
        }
    }
    if (child_count == 0) {
        return;
    }

    const size_t step   = win->length_y / child_count;
    size_t current_y    = start_y;
    size_t taken_away_y = 0;
    for (int i = 0; i < WINDOW_CHILDREN_LEN; i++) {
        if (win->children[i] != NULL) {
            draw_widget(win->children[i], fb, start_x, current_y, win->length_x, step + taken_away_y);
            //  TODO: Breaks encapsulation for ease of implementation
            // (and lazyness).
            if (win->is_packed && win->children[i]->type == WIDGET_IMAGE) {
                current_y    += win->children[i]->image->image_height;
                taken_away_y = (current_y - start_y) % step;
            } else {
                current_y += step;
            }
        }
    }
}

void destroy_window(struct window *win) {
    for (int i = 0; i < WINDOW_CHILDREN_LEN; i++) {
        if (win->children[i] != NULL) {
            destroy_widget(win->children[i]);
        }
    }

    free(win);
}

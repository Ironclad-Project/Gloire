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
    strncpy(ret->name, name, WINDOW_NAME_LEN);
    ret->is_focus     = 0;
    ret->top_corner_x = 100;
    ret->top_corner_y = 100;
    ret->length_x     = 570;
    ret->length_y     = 300;
    memset(ret->children_text, 0, sizeof(ret->children_text));
    return ret;
}

void add_text(struct window *win, const char *text) {
    for (int i = 0; i < WINDOW_TEXT_LEN; i++) {
        if (win->children_text[i] == NULL) {
            win->children_text[i] = text;
            return;
        }
    }
}

int pixel_is_in_window_bar(struct window *win, int x, int y) {
    if (x >= win->top_corner_x && x <= win->top_corner_x + win->length_x &&
        y >= win->top_corner_y && y <= win->top_corner_y + FONT_HEIGHT) {
        return 1;
    } else {
        return 0;
    }
}

int pixel_is_in_window(struct window *win, int x, int y) {
    if (x >= win->top_corner_x && x <= win->top_corner_x + win->length_x &&
        y >= win->top_corner_y && y <= win->top_corner_y + FONT_HEIGHT + win->length_y) {
        return 1;
    } else {
        return 0;
    }
}

void move_window(struct window *win, int x_variation, int y_variation) {
    win->top_corner_x += x_variation;
    win->top_corner_y += y_variation;
    if (win->top_corner_x < 0) {
        win->top_corner_x = 0;
    }
    if (win->top_corner_y < 0) {
        win->top_corner_y = 0;
    }
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
    draw_rectangle(fb, start_x, start_y, final_x, start_y + FONT_HEIGHT, title_color);

    // Draw the rest of the window background.
    draw_rectangle(fb, start_x, start_y + FONT_HEIGHT, final_x, final_y, WINDOW_BACKGROUND_COLOR);

    // Draw window title.
    size_t char_count = strlen(win->name);
    if (char_count * FONT_WIDTH > win->length_x) {
        char_count = win->length_x / FONT_WIDTH;
    }
    draw_string(fb, start_x, start_y, win->name, char_count, TITLEBAR_FONT_COLOR, title_color);

    // Draw borders.
    for (int i = 0; i <= win->length_y; i++) {
        draw_pixel(fb, start_x - 1, start_y + i, WINDOW_BORDERS_COLOR);
        draw_pixel(fb, final_x,     start_y + i, WINDOW_BORDERS_COLOR);
    }
    for (int i = 0; i < win->length_x; i++) {
        draw_pixel(fb, start_x + i, start_y, WINDOW_BORDERS_COLOR);
        draw_pixel(fb, start_x + i, final_y, WINDOW_BORDERS_COLOR);
    }

    // Draw text.
    size_t current_y = start_y + FONT_HEIGHT;
    for (int i = 0; i < WINDOW_TEXT_LEN; i++) {
        if (win->children_text[i] != NULL) {
            size_t char_count = strlen(win->children_text[i]);
            draw_string(fb, start_x, current_y, win->children_text[i], char_count, TITLEBAR_FONT_COLOR, WINDOW_BACKGROUND_COLOR);
        }
        current_y += FONT_HEIGHT;
        if (current_y >= final_y) {
            break;
        }
    }
}

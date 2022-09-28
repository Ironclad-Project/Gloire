#pragma once

#include <widgets/image.h>
#include <widgets/textbox.h>
#include <fb.h>

enum widget_type {
    WIDGET_IMAGE,
    WIDGET_TEXTBOX
};

struct widget {
    int center;
    enum widget_type type;
    union {
        struct image   *image;
        struct textbox *textbox;
    };
};

struct widget *create_image(const char *path, int center);
struct widget *create_textbox(const char *text, int center);
void draw_widget(struct widget *wid, struct framebuffer *fb, int start_x, int start_y, int width_x, int width_y);
void destroy_widget(struct widget *wid);

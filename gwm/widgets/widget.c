#include <widgets/widget.h>
#include <stdlib.h>

struct widget *create_image(const char *path, int center) {
    struct widget *ret = malloc(sizeof(struct widget));
    ret->center = center;
    ret->type   = WIDGET_IMAGE;
    ret->image  = init_image(path);
    return ret;
}

struct widget *create_textbox(const char *text, int center) {
    struct widget *ret = malloc(sizeof(struct widget));
    ret->center  = center;
    ret->type    = WIDGET_TEXTBOX;
    ret->textbox = init_textbox(text);
    return ret;
}

void draw_widget(struct widget *wid, struct framebuffer *fb, int start_x, int start_y, int width_x, int width_y) {
    switch (wid->type) {
        case WIDGET_IMAGE:
            draw_image(wid->image, fb, start_x, start_y, width_x, width_y, wid->center);
            break;
        case WIDGET_TEXTBOX:
            draw_textbox(wid->textbox, fb, start_x, start_y, width_x, width_y, wid->center);
            break;
    }
}

void destroy_widget(struct widget *wid) {
    switch (wid->type) {
        case WIDGET_IMAGE:   destroy_image(wid->image);     break;
        case WIDGET_TEXTBOX: destroy_textbox(wid->textbox); break;
    }

    free(wid);
}

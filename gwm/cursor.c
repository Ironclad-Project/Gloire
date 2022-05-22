#include <cursor.h>
#include <stdlib.h>

const uint32_t X = 0xffffff;
const uint32_t B = 0x000000;
const uint32_t o = -1;

#define CURSOR_HEIGHT 21
#define CURSOR_WIDTH  15

static const uint32_t cursor[] = {
    X, o, o, o, o, o, o, o, o, o, o, o, o, o, o,
    X, X, o, o, o, o, o, o, o, o, o, o, o, o, o,
    X, B, X, o, o, o, o, o, o, o, o, o, o, o, o,
    X, B, B, X, o, o, o, o, o, o, o, o, o, o, o,
    X, B, B, B, X, o, o, o, o, o, o, o, o, o, o,
    X, B, B, B, B, X, o, o, o, o, o, o, o, o, o,
    X, B, B, B, B, B, X, o, o, o, o, o, o, o, o,
    X, B, B, B, B, B, B, X, o, o, o, o, o, o, o,
    X, B, B, B, B, B, B, B, X, o, o, o, o, o, o,
    X, B, B, B, B, B, B, B, B, X, o, o, o, o, o,
    X, B, B, B, B, B, B, B, B, B, X, o, o, o, o,
    X, B, B, B, B, B, B, B, B, B, B, X, o, o, o,
    X, B, B, B, B, B, B, B, B, B, B, B, X, o, o,
    X, B, B, B, B, B, B, B, B, B, B, B, B, X, o,
    X, B, B, B, B, B, X, X, X, X, X, X, X, X, X,
    X, B, B, B, B, X, o, o, o, o, o, o, o, o, o,
    X, B, B, B, X, o, o, o, o, o, o, o, o, o, o,
    X, B, B, X, o, o, o, o, o, o, o, o, o, o, o,
    X, B, X, o, o, o, o, o, o, o, o, o, o, o, o,
    X, X, o, o, o, o, o, o, o, o, o, o, o, o, o,
    X, o, o, o, o, o, o, o, o, o, o, o, o, o, o
};

struct cursor *create_cursor(void) {
    struct cursor *ret = malloc(sizeof(struct cursor));
    ret->x_position = 500;
    ret->y_position = 500;
    return ret;
}

void update_cursor(struct cursor *c, struct framebuffer *fb, int x_variation, int y_variation) {
    if (c->x_position + x_variation < 0) {
        c->x_position = 0;
    } else if (c->x_position + x_variation >= fb->pixel_width) {
        c->x_position = fb->pixel_width - 1;
    } else {
        c->x_position += x_variation;
    }

    if (c->y_position + y_variation < 0) {
        c->y_position = 0;
    } else if (c->y_position + y_variation >= fb->pixel_height) {
        c->y_position = fb->pixel_height - 1;
    } else {
        c->y_position += y_variation;
    }
}

void draw_cursor(struct cursor *c, struct framebuffer *fb) {
    for (int x = 0; x < CURSOR_WIDTH; x++) {
        for (int y = 0; y < CURSOR_HEIGHT; y++) {
            const uint32_t px = cursor[y * CURSOR_WIDTH + x];
            if (px != o) {
                draw_pixel(fb, c->x_position + x, c->y_position + y, px);
            }
        }
    }
}

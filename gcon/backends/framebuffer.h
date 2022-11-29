#ifndef _TERM_FRAMEBUFFER_H
#define _TERM_FRAMEBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../term.h"

#define FBTERM_FONT_GLYPHS 256

struct fbterm_char {
    uint32_t c;
    uint32_t fg;
    uint32_t bg;
};

struct fbterm_queue_item {
    size_t x, y;
    struct fbterm_char c;
};

struct fbterm_context {
    struct term_context term;

    size_t font_width;
    size_t font_height;
    size_t glyph_width;
    size_t glyph_height;

    size_t font_scale_x;
    size_t font_scale_y;

    size_t offset_x, offset_y;

    volatile uint32_t *framebuffer;
    size_t pitch;
    size_t width;
    size_t height;
    size_t bpp;

    size_t font_bits_size;
    uint8_t *font_bits;
    size_t font_bool_size;
    bool *font_bool;

    uint32_t ansi_colours[8];
    uint32_t ansi_bright_colours[8];
    uint32_t default_fg, default_bg;

    size_t canvas_size;
    uint32_t *canvas;

    size_t grid_size;
    size_t queue_size;
    size_t map_size;

    struct fbterm_char *grid;

    struct fbterm_queue_item *queue;
    size_t queue_i;

    struct fbterm_queue_item **map;

    uint32_t text_fg;
    uint32_t text_bg;
    bool cursor_status;
    size_t cursor_x;
    size_t cursor_y;

    uint32_t saved_state_text_fg;
    uint32_t saved_state_text_bg;
    size_t saved_state_cursor_x;
    size_t saved_state_cursor_y;

    size_t old_cursor_x;
    size_t old_cursor_y;
};

struct term_context *fbterm_init(
    void *(*_malloc)(size_t),
    uint32_t *framebuffer, size_t width, size_t height, size_t pitch,
    uint32_t *canvas,
    uint32_t *ansi_colours, uint32_t *ansi_bright_colours,
    uint32_t *default_bg, uint32_t *default_fg,
    void *font, size_t font_width, size_t font_height, size_t font_spacing,
    size_t font_scale_x, size_t font_scale_y,
    size_t margin
);

#ifdef __cplusplus
}
#endif

#endif

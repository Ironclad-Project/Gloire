#include <widgets/textbox.h>
#include <font.h>
#include <stdlib.h>
#include <string.h>

#define TITLEBAR_FONT_COLOR     0xffffff
#define WINDOW_BACKGROUND_COLOR 0x888888

struct textbox *init_textbox(const char *text) {
    struct textbox *ret = malloc(sizeof(struct textbox));
    ret->text = text;
    return ret;
}

void draw_textbox(struct textbox *tx, struct framebuffer *fb, int start_x,
    int start_y, int width_x, int width_y, int center)
{
    const size_t char_count    = strlen(tx->text);
    const size_t char_per_line = width_x / FONT_WIDTH;
    const size_t line_count    = width_y / FONT_HEIGHT;

    size_t last_start        = 0;
    size_t current_line_char = 0;
    size_t current_line      = 0;
    for (size_t i = 0; i < char_count; i++) {
        if (current_line == line_count) {
            return;
        }
        if (current_line_char == char_per_line) {
            draw_string(fb, start_x, start_y + (current_line * FONT_HEIGHT), tx->text + last_start, char_per_line, TITLEBAR_FONT_COLOR, WINDOW_BACKGROUND_COLOR);
            current_line_char = 0;
            current_line += 1;
            last_start = i;
        } else {
            current_line_char++;
        }

        if (tx->text[i] == '\n' && last_start == i) {
            current_line_char = 0;
            current_line += 1;
            last_start = i + 1;
        } else if (tx->text[i] == '\n') {
            size_t line_start_x;
            if (center) {
                line_start_x = (width_x / 2) - (((i - last_start) / 2) * FONT_WIDTH);
            } else {
                line_start_x = 0;
            }
            line_start_x += start_x;
            draw_string(fb, line_start_x, start_y + (current_line * FONT_HEIGHT), tx->text + last_start, i - last_start, TITLEBAR_FONT_COLOR, WINDOW_BACKGROUND_COLOR);
            current_line_char = 0;
            current_line += 1;
            last_start = i + 1;
        }
    }

    size_t line_start_x;
    if (center) {
        line_start_x = (width_x / 2) - (((char_count - last_start) / 2) * FONT_WIDTH);
    } else {
        line_start_x = 0;
    }
    line_start_x += start_x;
    draw_string(fb, line_start_x, start_y + (current_line * FONT_HEIGHT), tx->text + last_start, char_count - last_start, TITLEBAR_FONT_COLOR, WINDOW_BACKGROUND_COLOR);
}

void destroy_textbox(struct textbox *tx) {
    free(tx);
}

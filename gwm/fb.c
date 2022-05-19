#include <fb.h>
#include <font.h>
#include <sys/ironclad.h>

extern struct fb_dimensions dimensions;
extern uint64_t pixel_size;
extern uint64_t linear_size;

static int bittest(int var, int index) {
    return (var >> index) & 1U;
}

void draw_background(uint32_t *buffer, uint32_t color) {
    for (uint64_t i = 0; i < pixel_size; i++) {
        buffer[i] = color;
    }
}

void draw_pixel(uint32_t *buffer, int x, int y, uint32_t color) {
    if (x >= dimensions.width || y >= dimensions.height || x < 0 || y < 0) {
        return;
    }
    int position = x + (dimensions.pitch / sizeof (uint32_t)) * y;
    buffer[position] = color;
}

void draw_character(uint32_t *buffer, int fbx, int fby, char c, uint32_t fg, uint32_t bg) {
    const size_t index = get_char_index(c);
    for (int i = 0; i < FONT_HEIGHT; i++) {
        int current_line = FONT_WIDTH;
        for (int y = 0; y < FONT_WIDTH; y++) {
            uint32_t output = bittest(font[index + i], --current_line) ? fg : bg;
            draw_pixel(buffer, y + fbx, i + fby, output);
        }
    }
}

void draw_string(uint32_t *buffer, int x, int y, const char *str, size_t char_count, uint32_t fg, uint32_t bg) {
    for (int i = 0; i < char_count; i++) {
        draw_character(buffer, x + (i * FONT_WIDTH), y, str[i], fg, bg);
    }
}


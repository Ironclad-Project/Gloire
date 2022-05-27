#include <fb.h>
#include <font.h>
#include <sys/ironclad.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

static int bittest(int var, int index) {
    return (var >> index) & 1U;
}

struct framebuffer *create_framebuffer_from_fd(int fd) {
    // Fetch the dimensions.
    struct ironclad_fb_dimensions dimensions;
    if (ioctl(fd, IOCTL_FB_DIMENSIONS, &dimensions) == -1) {
        return NULL;
    }

    // Allocate the memory representation and get the current one for
    // background purposes.
    size_t pixel_size    = dimensions.height * (dimensions.pitch / sizeof(uint32_t));
    size_t linear_size   = pixel_size * sizeof(uint32_t);
    uint32_t *antibuffer = malloc(linear_size);
    uint32_t *current_fb = malloc(linear_size);
    if (antibuffer == NULL || current_fb == NULL) {
        return NULL;
    }

    // Read the background.
    lseek(fd, 0, SEEK_SET);
    read(fd, current_fb, pixel_size);

    // Allocate the final object and return it.
    struct framebuffer *ret = malloc(sizeof(struct framebuffer));
    if (ret == NULL) {
        return NULL;
    }
    ret->backing_fd   = fd;
    ret->background   = current_fb;
    ret->memory       = antibuffer;
    ret->pixel_width  = dimensions.width;
    ret->pixel_height = dimensions.height;
    ret->pitch        = dimensions.pitch;
    ret->pixel_size   = pixel_size;
    ret->linear_size  = linear_size;

    return ret;
}

void refresh_to_backend(struct framebuffer *fb) {
    lseek(fb->backing_fd, 0, SEEK_SET);
    write(fb->backing_fd, fb->memory, fb->pixel_size);
}

void draw_background(struct framebuffer *fb) {
    for (uint64_t i = 0; i < fb->pixel_size; i++) {
        fb->memory[i] = fb->background[i];
    }
}

void draw_pixel(struct framebuffer *fb, int x, int y, uint32_t color) {
    if (x >= fb->pixel_width || y >= fb->pixel_height || x < 0 || y < 0) {
        return;
    }
    fb->memory[x + (fb->pitch / sizeof (uint32_t)) * y] = color;
}

void draw_rectangle(struct framebuffer *fb, int x, int y, int x_end, int y_end, uint32_t color) {
    for (int i = x; i < x_end; i++) {
        for (int j = y; j < y_end; j++) {
            draw_pixel(fb, i, j, color);
        }
    }
}

void draw_character(struct framebuffer *fb, int fbx, int fby, char c, uint32_t fg, uint32_t bg) {
    const size_t index = get_char_index(c);
    for (int i = 0; i < FONT_HEIGHT; i++) {
        int current_line = FONT_WIDTH;
        for (int y = 0; y < FONT_WIDTH; y++) {
            uint32_t output = bittest(font[index + i], --current_line) ? fg : bg;
            draw_pixel(fb, y + fbx, i + fby, output);
        }
    }
}

void draw_string(struct framebuffer *fb, int x, int y, const char *str, size_t char_count, uint32_t fg, uint32_t bg) {
    for (int i = 0; i < char_count; i++) {
        draw_character(fb, x + (i * FONT_WIDTH), y, str[i], fg, bg);
    }
}


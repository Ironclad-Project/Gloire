#include <fb.h>
#include <font.h>
#include <linux/fb.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

static int bittest(int var, int index) {
    return (var >> index) & 1U;
}

struct framebuffer *create_framebuffer_from_fd(int fd) {
    // Fetch fb info.
    struct fb_var_screeninfo var_info;
    struct fb_fix_screeninfo fix_info;
    if (ioctl(fd, FBIOGET_VSCREENINFO, &var_info) == -1) {
        return NULL;
    }
    if (ioctl(fd, FBIOGET_FSCREENINFO, &fix_info) == -1) {
        return NULL;
    }

    // Get dimensions.
    size_t pixel_size  = fix_info.smem_len / sizeof(uint32_t);
    size_t linear_size = pixel_size * sizeof(uint32_t);

    // Mmap the framebuffer into our mem_window.
    size_t aligned_size = (linear_size + 0x1000 - 1) & ~(0x1000 - 1);
    uint32_t *mem_window = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE,
       MAP_SHARED, fd, 0);
    if (mem_window == NULL) {
        return NULL;
    }

    // Allocate the memory representation and get the current one for
    // background purposes.
    uint32_t *antibuffer  = malloc(linear_size);
    uint32_t *frontbuffer = malloc(linear_size);
    uint32_t *background  = malloc(linear_size);
    if (antibuffer == NULL || frontbuffer == NULL || background == NULL) {
        return NULL;
    }

    // Load a background and put it in the buffers.
    for (size_t i = 0; i < pixel_size; i++) {
        background[i] = i % 7 == 0 ? 0x605885 : 0xC0C0C0;
    }
    memcpy(mem_window,  background, linear_size);
    memcpy(antibuffer,  background, linear_size);
    memcpy(frontbuffer, background, linear_size);

    // Allocate the final object and return it.
    struct framebuffer *ret = malloc(sizeof(struct framebuffer));
    if (ret == NULL) {
        return NULL;
    }
    ret->backing_fd    = fd;
    ret->background    = background;
    ret->antibuffer    = antibuffer;
    ret->frontbuffer   = frontbuffer;
    ret->memory_window = mem_window;
    ret->pixel_width   = var_info.xres;
    ret->pixel_height  = var_info.yres;
    ret->pitch         = fix_info.smem_len / var_info.yres;
    ret->pixel_size    = pixel_size;
    ret->linear_size   = linear_size;
    return ret;
}

void refresh_to_backend(struct framebuffer *fb) {
    // Compare changes with the front buffer and write to the window.
    for (uint64_t i = 0; i < fb->pixel_size; i++) {
        if (fb->frontbuffer[i] != fb->antibuffer[i]) {
            fb->frontbuffer[i]   = fb->antibuffer[i];
            fb->memory_window[i] = fb->antibuffer[i];
        }
    }
}

void draw_background(struct framebuffer *fb) {
    memcpy(fb->antibuffer, fb->background, fb->pixel_size * sizeof(uint32_t));
}

void draw_pixel(struct framebuffer *fb, int x, int y, uint32_t color) {
    if (x >= fb->pixel_width || y >= fb->pixel_height || x < 0 || y < 0) {
        return;
    }
    fb->antibuffer[x + (fb->pitch / sizeof (uint32_t)) * y] = color;
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

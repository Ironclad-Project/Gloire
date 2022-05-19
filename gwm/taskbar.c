#include <taskbar.h>
#include <stdlib.h>

#define TASKBAR_HEIGHT 20
#define TASKBAR_COLOR  0x888888

struct taskbar *create_taskbar(void) {
    struct taskbar *ret = malloc(sizeof(struct taskbar));
    ret->is_top = true;
    return ret;
}

void draw_taskbar(struct taskbar *bar, struct framebuffer *fb) {
    size_t start_y, end_y;
    if (bar->is_top) {
        start_y = 0;
        end_y   = TASKBAR_HEIGHT;
    } else {
        start_y = fb->pixel_height - TASKBAR_HEIGHT;
        end_y   = fb->pixel_height;
    }

    draw_rectangle(fb, 0, start_y, fb->pixel_width, end_y, TASKBAR_COLOR);
}

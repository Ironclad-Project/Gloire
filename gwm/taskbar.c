#include <taskbar.h>
#include <stdlib.h>
#include <bmp.h>

#define TASKBAR_HEIGHT 20
#define TASKBAR_COLOR  0x888888

struct taskbar *create_taskbar(void) {
    // Read the logo.
    size_t    logo_len, logo_width, logo_height;
    uint32_t *logo_arr = open_bmp_to_array("/etc/gwm-logo.bmp", &logo_len,
                                           &logo_width, &logo_height);
    if (logo_arr == NULL) {
        return NULL;
    }

    struct taskbar *ret = malloc(sizeof(struct taskbar));
    ret->is_top   = true;
    ret->logo_arr = logo_arr;
    ret->logo_len = logo_len;
    ret->logo_width = logo_width;
    ret->logo_height = logo_height;
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

    // Draw logo.
    for (int j = start_y; j < start_y + bar->logo_height; j++) {
        for (int i = 0; i < bar->logo_width; i++) {
            uint32_t color = bar->logo_arr[i + bar->logo_width * (bar->logo_height - j)];
            if (color != 0) {
                draw_pixel(fb, i, j, color);
            }
        }
    }
}

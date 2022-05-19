#pragma once

#include <stdbool.h>
#include <fb.h>

struct taskbar {
    bool is_top;
};

struct taskbar *create_taskbar(void);
void draw_taskbar(struct taskbar *bar, struct framebuffer *fb);

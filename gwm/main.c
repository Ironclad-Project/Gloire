#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <window.h>
#include <fb.h>
#include <taskbar.h>

#define BACKGROUND_COLOR 0xaaaaaa

static struct framebuffer *main_fb;
static struct taskbar     *taskbar;
static struct window      *win_array[20];

struct window *add_window(const char *name) {
    for (int i = 0; i < 20; i++) {
        if (win_array[i] == NULL) {
            win_array[i] = create_window(name);
            return win_array[i];
        }
    }
}

void refresh() {
    // Clear the background.
    draw_solid_background(main_fb, BACKGROUND_COLOR);

    // Write the windows.
    for (int i = 0; i < 20; i++) {
        if (win_array[i] != NULL) {
            draw_window(win_array[i], main_fb);
        }
    }

    // Write the taskbar.
    draw_taskbar(taskbar, main_fb);

    // Finally write.
    refresh_to_backend(main_fb);
}

int main() {
    // Open the framebuffer.
    int fb = open("/dev/bootmfb", O_RDONLY);
    if (fb == -1) {
        perror("Could not open the framebuffer");
        exit(1);
    }

    // Create a framebuffer from that one.
    main_fb = create_framebuffer_from_fd(fb);
    if (main_fb == NULL) {
        perror("Could not create main framebuffer");
        exit(1);
    }

    // Initialize desktop widgets and create the welcome window.
    taskbar = create_taskbar();
    struct window *win = add_window("Welcome!");
    add_text(win, "Gloire is an Ironclad distribution using mlibc and several GNU tools.");
    add_text(win, "");
    add_text(win, "To find command-line arguments and syscall documentation, there is");
    add_text(win, "documentation in Info and PDF formats in /usr/share/info and");
    add_text(win, "/usr/share/docs/ironclad");
    add_text(win, "");
    add_text(win, "Ironclad: https://github.com/streaksu/Ironclad");
    add_text(win, "Gloire:   https://github.com/streaksu/Gloire");
    add_text(win, "Mlibc:    https://github.com/managarm/mlibc");
    add_text(win, "");
    add_text(win, "Have a nice time around!");

    // Initial refresh and loop waiting for events.
    // TODO: Wait for events.
    refresh();
    for (;;);
}

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ironclad.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <window.h>
#include <font.h>
#include <fb.h>

struct fb_dimensions dimensions;
int fb;
uint64_t pixel_size;
uint64_t linear_size;
uint32_t *antibuffer;

#define BACKGROUND_COLOR 0xaaaaaa

void refresh() {
    // Clear the background.
    draw_background(antibuffer, BACKGROUND_COLOR);

    // Write a mock-window.
    struct window *win = create_window("Hello!");
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
    draw_window(win, antibuffer);

    // Finally write.
    write(fb, antibuffer, pixel_size);
}

int main() {
    // Open the framebuffer.
    fb = open("/dev/bootmfb", O_RDONLY);
    if (fb == -1) {
        perror("Could not open the framebuffer");
        exit(1);
    }

    // Fetch the dimensions.
    if (ioctl(fb, IOCTL_FB_DIMENSIONS, &dimensions) == -1) {
        perror("Could not fetch framebuffer details");
        exit(1);
    }

    // Create and fill the buffers.
    pixel_size  = dimensions.height * (dimensions.pitch / sizeof (uint32_t));
    linear_size = pixel_size * sizeof (uint32_t);
    antibuffer  = malloc(linear_size);
    if (antibuffer == NULL) {
        perror("Could not allocate");
        exit(1);
    }

    // Initial refresh and loop waiting for events.
    // TODO: Wait for events.
    refresh();
    for (;;);
}

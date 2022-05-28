#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <window.h>
#include <fb.h>
#include <taskbar.h>
#include <cursor.h>
#include <bmp.h>
#include <font.h>
#include <widgets/widget.h>
#include <sys/ironclad.h>
#include <sys/ioctl.h>

#define BACKGROUND_COLOR 0xaaaaaa

static struct framebuffer *main_fb;
static struct taskbar     *taskbar;
static struct cursor      *cursor;
static struct window      *win_array[20];

struct window *add_window(const char *name) {
    for (int i = 0; i < 20; i++) {
        if (win_array[i] == NULL) {
            win_array[i] = create_window(name);
            return win_array[i];
        }
    }

   return NULL;
}

void refresh() {
    // Clear the background.
    draw_background(main_fb);

    // Go thru the windows, check if we are clicking any, move and draw.
    for (int i = 0; i < 20; i++) {
        if (win_array[i] != NULL) {
            draw_window(win_array[i], main_fb);
        }
    }

    // Write the taskbar and cursor.
    draw_taskbar(taskbar, main_fb);
    draw_cursor(cursor, main_fb);

    // Finally write.
    refresh_to_backend(main_fb);
}

int main() {
    // Clear the screen and disable the cursor.
    printf("\e[2J\e[H\e[?25l");
    fflush(stdout);

    // Open the framebuffer.
    int fb = open("/dev/bootfb", O_RDWR);
    if (fb == -1) {
        perror("Could not open the framebuffer");
        exit(1);
    }

    // Open the mouse.
    int ps = open("/dev/ps2mouse", O_RDONLY);
    if (ps == -1) {
        perror("Could not open the mouse");
        exit(1);
    }

    // Set mouse modes.
    ioctl(ps, PS2MOUSE_2_1_SCALING,     0);
    ioctl(ps, PS2MOUSE_SET_RES,         3);
    ioctl(ps, PS2MOUSE_SET_SAMPLE_RATE, 100);

    // Create a framebuffer from that one.
    main_fb = create_framebuffer_from_fd(fb);
    if (main_fb == NULL) {
        perror("Could not create main framebuffer");
        exit(1);
    }

    // Initialize desktop widgets and create the welcome window.
    taskbar = create_taskbar();
    if (taskbar == NULL) {
        perror("Could not create taskbar");
        exit(1);
    }

    cursor = create_cursor();
    if (cursor == NULL) {
        perror("Could not create cursor");
        exit(1);
    }

    struct window *win = add_window("Welcome!");
    add_child(win, create_image("/etc/gwm-gloire.bmp"));
    add_child(win, create_textbox(
        "Gloire is an Ironclad distribution using mlibc and several GNU tools.\n"
        "\n"
        "To find command-line arguments and syscall documentation, there is\n"
        "documentation in Info and PDF formats in /usr/share/info and\n"
        "/usr/share/docs/ironclad\n"
        "\n"
        "Ironclad: https://github.com/streaksu/Ironclad\n"
        "Gloire:   https://github.com/streaksu/Gloire\n"
        "Mlibc:    https://github.com/managarm/mlibc\n"
        "\n"
        "Have a nice time around!"
    ));
    pack_window(win);

    // Initial refresh and loop waiting for mouse.
    for (;;) {
        refresh();

        struct ironclad_mouse_data data;
        read(ps, &data, sizeof(struct ironclad_mouse_data));

        size_t old_cursor_x = cursor->x_position;
        size_t old_cursor_y = cursor->y_position;
        update_cursor(cursor, main_fb, data.x_variation, data.y_variation);

        // If left-clicking, check if we are clicking any window bars and move.
        // We use the old cursor data because the new one will make it so
        // sometimes the user moves the cursor out when trying to move.
        if (data.is_left) {
            for (int i = 0; i < 20; i++) {
                if (win_array[i] != NULL) {
                    if (pixel_is_in_window(win_array[i], old_cursor_x, old_cursor_y)) {
                        focus_window(win_array[i]);
                        if (pixel_is_in_window_bar(win_array[i], old_cursor_x, old_cursor_y)) {
                            // FIXME: This hardcodes the thickness of the bar.
                            int start_y = 0;
                            int final_y = main_fb->pixel_height - FONT_HEIGHT;
                            if (taskbar->is_top) {
                                start_y = FONT_HEIGHT;
                                final_y = main_fb->pixel_height;
                            }
                            move_window(win_array[i], data.x_variation, data.y_variation, 0, start_y, main_fb->pixel_width, final_y);
                            break;
                        }
                    } else {
                        unfocus_window(win_array[i]);
                    }
                }
            }
        }

        // If right-clicking, remove the window if we are in the bar.
        if (data.is_right) {
            for (int i = 0; i < 20; i++) {
                if (win_array[i] != NULL) {
                    if (pixel_is_in_window_bar(win_array[i], cursor->x_position, cursor->y_position)) {
                        // FIXME: Uncommenting this with qemu-system-x86_64 -hda gloire.hdd -m 4G -smp 4 -enable-kvm
                        // makes ironclad die, uncomment this when fixed lol.
                        // destroy_window(win_array[i]);
                        win_array[i] = NULL;
                        goto done;
                    }
                }
            }

            struct window *new_window = add_window("Look at this cool cat");
            new_window->length_y = 700;
            new_window->top_corner_x = cursor->x_position;
            new_window->top_corner_y = cursor->y_position;
            add_child(new_window, create_image("/etc/gwm-jinx.bmp"));
        }
done:
    }
}

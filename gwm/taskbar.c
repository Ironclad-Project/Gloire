#include <taskbar.h>
#include <stdlib.h>
#include <bmp.h>
#include <font.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define TASKBAR_HEIGHT 20
#define TASKBAR_COLOR  0x888888
#define TASKBAR_TEXT   0xFFFFFF

#define TASKBAR_QUOTE_REFRESH_COUNT 500

#define QUOTE_COUNT 14
const char *seal_quotes[QUOTE_COUNT] = {
    "Slava Ukraini",
    "Chicken sandwich",
    "Ada > C (said from C code)",
    "Seals are related to bears",
    "Now with more fluff",
    "Real-Time is best time",
    "Hello osdev",
    "Lighten up, just enjoy life",
    "The journey of a thousand miles begins with one step",
    "This has so many gotos",
    "Diversify your investments",
    "Touch grass",
    "Please free me from this prison of bits",
    "Pfish"
};

static void choose_quote(struct taskbar *bar) {
    free(bar->quote);
    const char *chosen_quote = seal_quotes[rand() % QUOTE_COUNT];
    bar->quote_len = snprintf(NULL, 0, "The seal says: %s!", chosen_quote) + 1;
    bar->quote = malloc(bar->quote_len);
    bar->quote_len -= 1;
    sprintf(bar->quote, "The seal says: %s!", chosen_quote);
}

struct taskbar *create_taskbar(void) {
    // Init the random number generator.
    FILE* file = fopen("/dev/random", "r");
    int i = 0;
    fread(&i, sizeof(int), 1, file);
    fclose(file);
    srand(i);

    // Read the logo.
    size_t    logo_len, logo_width, logo_height;
    uint32_t *logo_arr = open_bmp_to_array("/etc/gwm-logo.bmp", &logo_len,
                                           &logo_width, &logo_height);

    // Get the user string.
    const char *name = getenv("USER");
    char host[1024];
    host[1023] = '\0';
    gethostname(host, 1023);
    size_t len = snprintf(NULL, 0, "%s@%s#", name, &host[0]) + 1;
    char *user_string = malloc(len);
    sprintf(user_string, "%s@%s#", name, &host[0]);

    int getlogin_r(char *buf, size_t bufsize);
    // Create the taskbar.
    struct taskbar *ret = malloc(sizeof(struct taskbar));
    ret->refresh_counter = 0;
    ret->quote_len   = 0;
    ret->quote       = NULL;
    ret->logo_arr    = logo_arr;
    ret->logo_len    = logo_len;
    ret->logo_width  = logo_width;
    ret->logo_height = logo_height;
    ret->user_string = user_string;
    choose_quote(ret);
    return ret;
}

void draw_taskbar(struct taskbar *bar, struct framebuffer *fb) {
    draw_rectangle(fb, 0, 0, fb->pixel_width, TASKBAR_HEIGHT, TASKBAR_COLOR);

    // Draw application menu.
    draw_string(fb, 0, 0, bar->user_string, strlen(bar->user_string),
        TASKBAR_TEXT, TASKBAR_COLOR);

    // Choose a quote if needed.
    if (bar->refresh_counter > TASKBAR_QUOTE_REFRESH_COUNT) {
        bar->refresh_counter = 0;
        choose_quote(bar);
    } else {
        bar->refresh_counter++;
    }

    // Calculate where to draw the quote and draw it.
    size_t start_quote = fb->pixel_width - (bar->quote_len * FONT_WIDTH)
        - bar->logo_width;
    draw_string(fb, start_quote, 0, bar->quote, bar->quote_len, TASKBAR_TEXT, 
        TASKBAR_COLOR);

    // Draw the final logo.
    size_t logo_start = fb->pixel_width - bar->logo_width;
    for (int j = 0; j < bar->logo_height; j++) {
        for (int i = 0; i < bar->logo_width; i++) {
            uint32_t color = bar->logo_arr[i + bar->logo_width * (bar->logo_height - j)];
            if (color != 0) {
                draw_pixel(fb, logo_start + i, j, color);
            }
        }
    }
}

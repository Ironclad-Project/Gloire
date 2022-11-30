#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <term.h>
#include <backends/framebuffer.h>
#include <stdlib.h>
#include <sys/mman.h>

char *const start_path = "/sbin/epoch";
char *const args[] = {start_path, "--init", NULL};

int main(void) {
    // Initialize the tty.
    struct fb_var_screeninfo var_info;
    struct fb_fix_screeninfo fix_info;
    int fb = open("/dev/fb0", O_RDWR);
    if (fb == -1) {
        perror("Could not open framebuffer");
        return 1;
    }

    if (ioctl(fb, FBIOGET_VSCREENINFO, &var_info) == -1) {
        perror("Could not fetch framebuffer properties");
        return 1;
    }
    if (ioctl(fb, FBIOGET_FSCREENINFO, &fix_info) == -1) {
        perror("Could not fetch framebuffer properties");
        return 1;
    }

    size_t pixel_size  = fix_info.smem_len / sizeof(uint32_t);
    size_t linear_size = pixel_size * sizeof(uint32_t);
    size_t aligned_size = (linear_size + 0x1000 - 1) & ~(0x1000 - 1);
    uint32_t *mem_window = mmap(
        NULL,
        aligned_size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fb,
        0
    );
    if (mem_window == NULL) {
        perror("Could not mmap framebuffer");
        return 1;
    }

    // Initialize the terminal.
    struct term_context *term = fbterm_init(
        malloc,
        mem_window,
        var_info.xres,
        var_info.yres,
        fix_info.smem_len / var_info.yres,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        69,
        420,
        1,
        1,
        1,
        0
   );

    // Open the ps2 keyboard for use as new stdin.
    int kb = open("/dev/ps2keyboard", O_RDONLY);
    if (kb == -1) {
        perror("Could not open keyboard");
        return 1;
    }

    // Create a pipe for stdout/stderr.
    int master_pty;
    int pty_spawner = open("/dev/ptmx", O_RDWR);
    if (pty_spawner == -1) {
      perror("Could not open ptmx");
      return 1;
    }

    if (ioctl(pty_spawner, 0, &master_pty) != 0) {
        perror("Could not create pty");
        return 1;
    }

    // Export some variables related to the TTY.
    putenv("TERM=linux");

    // Boot the child.
    int child = fork();
    if (child == 0) {
        // Replace std streams.
        int slave_pty;
        if (ioctl(master_pty, 0, &slave_pty) != 0) {
            perror("Could not create pty");
            return 1;
        }

        dup2(slave_pty, 0);
        dup2(slave_pty, 1);
        dup2(slave_pty, 2);
        execvp(start_path, args);
        perror("Could not start");
        return 1;
    }

    // Boot an input process.
    int input_child = fork();
    if (input_child == 0) {
        for (;;) {
            char input;
            read(kb, &input, 1);
            write(master_pty, &input, 1);
            sched_yield();
        }
    }

    // Catch what the child says.
    for (;;) {
        char output[512];
        ssize_t count = read(master_pty, &output, 512);
        for (ssize_t i = 0; i < count; i++) {
            if (output[i] == '\b') {
                term_write(term, "\b \b", 3);
            } else {
                term_write(term, &output[i], 1);
            }
        }
        sched_yield();
    }
}

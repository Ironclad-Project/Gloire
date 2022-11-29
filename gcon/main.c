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
    int stdout_pipes[2];
    if (pipe(stdout_pipes) != 0) {
        perror("Could not create pipes");
        return 1;
    }

    // Export some variables related to the TTY.
    putenv("TERM=linux");

    // Boot the child.
    int child = fork();
    if (child == 0) {
        // Replace std streams.
        close(stdout_pipes[0]);
        dup2(kb,           0);
        dup2(stdout_pipes[1], 1);
        dup2(stdout_pipes[1], 2);
        execvp(start_path, args);
        perror("Could not start");
        return 1;
    }

    // Catch what the child says.
    close(stdout_pipes[1]);
    for (;;) {
        char res[512];
        ssize_t count = read(stdout_pipes[0], &res, 512);
        if (count != -1) {
            term_write(term, res, count);
        }
    }
}

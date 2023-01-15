#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <math.h>

int main(int argc, char *argv[]) {
    int print_only_free  = 0;
    int print_only_used  = 0;
    int print_only_total = 0;

    char c;
    while ((c = getopt (argc, argv, "hvfut")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: showmem [options]");
                puts("");
                puts("-h        Print this help message");
                puts("-v        Print version information");
                puts("-f        Print only free memory (in MiB)");
                puts("-u        Print only used memory (in MiB)");
                puts("-t        Print only total memory (in MiB)");
                return 0;
            case 'v':
                puts("showmem");
                puts("Distributed with Gloire under the GPLv3");
                return 0;
            case 'f': print_only_free  = 1; break;
            case 'u': print_only_used  = 1; break;
            case 't': print_only_total = 1; break;
            default:
                fprintf(stderr, "Unknown option character '%c'\n", optopt);
                return 1;
        }
    }

    long page_size, free_pages, total_pages, ret, errno;
    SYSCALL1(26, 1);
    if (ret == -1) {
        return 1;
    } else {
        page_size = ret;
    }
    SYSCALL1(26, 4);
    if (ret == -1) {
        return 1;
    } else {
        free_pages = ret;
    }
    SYSCALL1(26, 5);
    if (ret == -1) {
        return 1;
    } else {
        total_pages = ret;
    }

    long free_mib  = (free_pages  * page_size) / 1000000;
    long total_mib = (total_pages * page_size) / 1000000;
    int width      = round(1 + log(total_mib) / log(10));
    if (print_only_free)       { printf("%lu\n", free_mib);             }
    else if (print_only_used)  { printf("%lu\n", total_mib - free_mib); }
    else if (print_only_total) { printf("%lu\n", total_mib);            }
    else {
        printf("Free physical memory:  %*luMiB\n", width, free_mib);
        printf("Used physical memory:  %*luMiB\n", width, total_mib - free_mib);
        printf("Total physical memory: %*luMiB\n", width, total_mib);
    }
}

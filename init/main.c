#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ironclad.h>

const char *ironclad_x86_ttydev = "/dev/stivale2tty";
const char *first_program       = "/usr/bin/bash";
const char *new_hostname        = "glorious_earthrise";

int main(void) {
    fflush(NULL);
    stdin  = fopen(ironclad_x86_ttydev, "r");
    stdout = fopen(ironclad_x86_ttydev, "w");
    stderr = fopen(ironclad_x86_ttydev, "w");
    dup2(fileno(stdin),  0);
    dup2(fileno(stdout), 1);
    dup2(fileno(stderr), 2);
    puts("init: Switched streams");

    printf("init: Switching hostname to %s\n", new_hostname);
    if (sethostname(new_hostname, strlen(new_hostname)) != 0) {
        perror("Could not set new hostname");
    }

    puts("init: Setting environment");
    setenv("HOME", "/root", 1);
    setenv("TERM", "linux", 1);
    setenv("PATH", "/usr/bin", 1);
    setenv("USER", "root", 1);
    setenv("LOGNAME", "root", 1);
    setenv("SHELL", "/bin/bash", 1);
    setenv("MAIL", "/var/mail", 1);
    setenv("XDG_RUNTIME_DIR", "/run", 1);

    printf("init: Executing %s\n", first_program);
    char * const args[] = {first_program, NULL};
    pid_t spawned = program_spawn(first_program, &args[0], environ);
    if (spawned == -1) {
        perror("Could not start program");
        return 1;
    }

    for (;;) {
        int status;
        wait(&status);
    }
}

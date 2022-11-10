#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ironclad.h>

const char *kbdev = "/dev/ps2keyboard";
const char *shell = "/usr/bin/bash";

int main(void) {
    fflush(NULL);
    stdin = fopen(kbdev, "r");
    dup2(fileno(stdin), 0);
    puts("serial_shim: Switched streams");

    puts("serial_shim: Setting environment");
    setenv("HOME", "/root", 1);
    setenv("TERM", "linux", 1);
    setenv("PATH", "/usr/bin", 1);
    setenv("USER", "root", 1);
    setenv("LOGNAME", "root", 1);
    setenv("SHELL", "/bin/bash", 1);
    setenv("MAIL", "/var/mail", 1);
    setenv("XDG_RUNTIME_DIR", "/run", 1);

    // Disable MAC by giving all rights.
    struct mac_filter filt;
    strcpy(filt.path, "/");
    filt.length = strlen(filt.path);
    filt.perms  = (uint8_t)-1;
    set_mac_capabilities((unsigned long)-1);
    add_mac_filter(&filt);
    lock_mac();

    printf("serial_shim: Executing %s\n", shell);
    char * const args[] = {shell, NULL};
    pid_t spawned = program_spawn(shell, &args[0], environ);
    if (spawned == -1) {
        perror("Could not start program");
        return 1;
    }

    for (;;) {
        int status;
        wait(&status);
    }
}

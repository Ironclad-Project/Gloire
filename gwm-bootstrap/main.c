#include <unistd.h>
#include <stdlib.h>

int main(void) {
    putenv("runlevel=desktop");
    execl("/sbin/epoch", "--init");
}

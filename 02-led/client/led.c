#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        printf("./led 0|1\n");
        return 1;
    }

    int sw = atoi(argv[1]) == 0 ? 0 : 1;


    int fd = open("/dev/myled", O_WRONLY);
    if (fd < 0) {
        printf("open led error\n");
        return 1;
    }

    if (sw) {
        write(fd, "on", 2);
    } else {
        write(fd, "off", 3);
    }

    close(fd);
    return 0;
}

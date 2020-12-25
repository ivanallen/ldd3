#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <errno.h>

const char* devices[] = {
    "/dev/myscullpipe0",
    "/dev/myscullpipe1",
    "/dev/myscullpipe2",
    "/dev/myscullpipe3"
};

char buffer[4096];

int main() {
    int i = 0;
    int r = 0;
    int m = 0;
    int n = 0;
    int fd[4] = { 0 };
    struct pollfd fds[4];

    char prompt[7] = "pipe0:";

    for (i = 0; i < 4; ++i) {
        fd[i] = open(devices[i], O_RDONLY);

        if (fd[i] < 0) {
            perror("open");
            return 1;
        }
    }


    for (i = 0; i < 4; ++i) {
        fds[i].fd = fd[i];
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }

    while (1) {
        write(STDOUT_FILENO, "----\n", 5);
        r = poll(fds, 4, -1);
        if (r < 0) return 2;

        for (i = 0; i < 4; ++i) {
            prompt[4] = '0' + i;
            if (fds[i].revents & POLLIN) {
                write(STDOUT_FILENO, prompt, 6);

                n = read(fds[i].fd, buffer, 4096);
                if (n >= 0) {
                    m = write(STDOUT_FILENO, buffer, n);
                }

                if (n < 0 || m < 0) {
                    return 3;
                }
            }
        }
    }

    return 0;
}

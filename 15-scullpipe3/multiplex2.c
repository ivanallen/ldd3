#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <fcntl.h>
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
    int epfd = 0;
    struct epoll_event events[4] = { 0 };
    uint32_t idx = 0;

    char prompt[7] = "pipe0:";

    for (i = 0; i < 4; ++i) {
        fd[i] = open(devices[i], O_RDONLY);

        if (fd[i] < 0) {
            perror("open");
            return 1;
        }
    }

    epfd = epoll_create(1);

    for (i = 0; i < 4; ++i) {
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.u32 = i;
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd[i], &event);
    }

    while (1) {
        write(STDOUT_FILENO, "----\n", 5);
        r = epoll_wait(epfd, events, 4, -1);

        if (r < 0) return 2;

        for (i = 0; i < r; ++i) {
            idx = events[i].data.u32;
            prompt[4] = '0' + idx;
            if (events[i].events & EPOLLIN) {
                write(STDOUT_FILENO, prompt, 6);

                n = read(fd[idx], buffer, 4096);
                if (n >= 0) {
                    m = write(STDOUT_FILENO, buffer, n);
                }

                if (n < 0 || m < 0) {
                    return 3;
                }
            }
        }
    }

    close(epfd);
    for (i = 0; i < 4; ++i) {
        close(fd[i]);
    }
    return 0;
}

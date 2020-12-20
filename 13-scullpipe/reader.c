#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

char buffer[4096];

int main(int argc, char **argv) {
	int delay = 1, n, m = 0;

    int scullpipe = open("/dev/myscullpipe0", O_RDONLY);

    if (scullpipe < 0) {
        return 1;
    }

	fcntl(scullpipe, F_SETFL, fcntl(scullpipe, F_GETFL) | O_NONBLOCK);

	while (1) {
		n = read(scullpipe, buffer, 4096);
		if (n >= 0) {
			m = write(STDOUT_FILENO, buffer, n);
        }

		if ((n < 0 || m < 0) && (errno != EAGAIN)) {
			break;
        }

        if (errno == EAGAIN) {
            perror("scullpipe0");
        }

		sleep(delay);
	}

    close(scullpipe);

    if (n < 0) {
        perror("scullpipe0");
        return 2;
    }

    return 0;
}

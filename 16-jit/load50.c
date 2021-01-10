#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int i, load = 50;

    if (argc == 2) {
        load = atoi(argv[1]);
    }

    printf("Bringing load to %i\n",load);

    for (i = 0; i < load; ++i)
        if (fork() == 0)
            break;

    while(1);
    return 0;
}

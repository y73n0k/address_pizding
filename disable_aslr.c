#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/personality.h>


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Please provide program to exec and its args\n");
        return 0;
    }

    if (personality(ADDR_NO_RANDOMIZE) == -1) {
        printf("Things went too bad DX: %s\n", strerror(errno));
        exit(1);
    }

    if (execvp(argv[1], argv + 1) == -1) {
        printf("%s: %s\n", argv[1], strerror(errno));
        exit(2);
    }

    return 0;
}

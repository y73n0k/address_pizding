#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>

#ifndef WORDLEN
#define WORDLEN 4
#endif

#ifndef SUBSTR
#define SUBSTR "libc.so"
#endif

#ifndef SKIPS
#define SKIPS 1
#endif


typedef union
{
    long val;
    char s[WORDLEN];
} l2b;  // long to string converter


char *pizding_string(pid_t pid, unsigned long long address) {
    // just dumping string from tracee's address 
    char *string = malloc(WORDLEN * sizeof(char));
    unsigned long long old_length, length = 0, capacity = WORDLEN, counter = 0;
    unsigned char finished = 0;
    l2b t;
    while (!finished) {
        old_length = length;
        t.val = ptrace(PTRACE_PEEKDATA, pid, address + WORDLEN * counter, 0);
        for (unsigned char i = 0; i < WORDLEN; ++i) {
            ++length;
            if (t.s[i] == 0) {
                finished = 1;
                break;
            }
        }

        if (length >= capacity) {
            capacity = 2 * length;
            string = realloc(string, sizeof(char) * capacity);
        }
        memcpy(string + old_length, t.s, WORDLEN);
        ++counter;
    }
    return string;
}


void step(pid_t pid, int *wait_status){
    // PTRACE_SYSCALL is catching entering and exiting from syscall
    ptrace(PTRACE_SYSCALL, pid, 0, 0);
    waitpid(pid, wait_status, 0);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Please provide program to exec and its args\n");
        return 0;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }

    if (pid == 0) {
        // run tracee
        ptrace(PTRACE_TRACEME, 0, NULL, 0);

        if (execvp(argv[1], argv + 1) == -1) {
            printf("%s: %s\n", argv[1], strerror(errno));
            exit(2);
        }

    } else {
        // tracer logic for finding library base
        int wait_status;
        struct user_regs_struct state;
        long fd;
        char found = 0;

        waitpid(pid, &wait_status, 0);

        if (!SKIPS) {
            ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL);
        } else {
            // PTRACE_O_TRACEEXEC ignores until execv syscall
            for (unsigned long long i = 0; i < SKIPS; ++i) {
                ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL | PTRACE_O_TRACEEXEC);
                ptrace(PTRACE_CONT, pid, 0, 0);
                waitpid(pid, &wait_status, 0);
            }
        }

        while (!WIFEXITED(wait_status)) {
            step(pid, &wait_status);

            ptrace(PTRACE_GETREGS, pid, 0, &state);
            if (!found) {
                // searching for openat syscall with path which contains SUBSTR
                if (state.orig_rax == SYS_openat) {
                    char *path = pizding_string(pid, state.rsi);
                    if (strstr(path, SUBSTR) != NULL) {
                        found = 1;
                        printf("FOUND: %s\n", path);
                        step(pid, &wait_status);
                        ptrace(PTRACE_GETREGS, pid, 0, &state);
                        fd = state.rax;  // openat returns fd, so save it to find mmap syscall with this fd
                    } else {
                        step(pid, &wait_status);
                    }
                    free(path);
                }
            } else {
                // find first mmap(NULL,...,...,...,fd,...) after opening path
                if (state.orig_rax == SYS_mmap) {
                    ptrace(PTRACE_GETREGS, pid, 0, &state);
                    if (state.r8 == fd && state.rdi == 0) {
                        step(pid, &wait_status);
                        ptrace(PTRACE_GETREGS, pid, 0, &state);
                        printf("AT: %llx\n", state.rax);  // mmap returns base of library
                        break;
                    }
                }
                step(pid, &wait_status);
            }
        }
    }
    return 0;
}

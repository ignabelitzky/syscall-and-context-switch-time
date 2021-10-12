#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <sched.h>

#define BUF_SIZE 10

/*
 * TO-DO
 * Figure it out how to calculate the time with gettimeofday between 2 processes
 * and where to put the funcion, also check how to make the child and the parent
 * run on the same cpu (check for sched_setaffinity()
 */

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("usage: %s <string>\n", argv[0]);
        exit(1);
    }

    struct timeval *time = malloc(sizeof(struct timeval));
    long int secs = 0, usecs = 0;

    int filedes[2];
    char buf[BUF_SIZE];
    ssize_t numRead = 0;

    cpu_set_t set;
    int parentCPU = 0, childCPU = 0;

    CPU_ZERO(&set);

    if(pipe(filedes) == -1) {
        fprintf(stderr, "Error in pipe.\n");
        exit(1);
    }
    switch(fork()) {
    case -1:
        fprintf(stderr, "Error in fork.\n");
        exit(1);
        break;
    case 0:
        CPU_SET(childCPU, &set);
        if(sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
            fprintf(stderr, "Error sched_setaffinity\n");
            exit(1);
        }
        if(close(filedes[1]) == -1) {
            fprintf(stderr, "Close error on child.\n");
            exit(1);
        }
        for(;;) {
            numRead = read(filedes[0], buf, BUF_SIZE);
            if(numRead == -1) {
                fprintf(stderr, "Error reading.\n");
                exit(1);
            }
            if(numRead == 0) {
                break;  // End of file
            }
            if(write(STDOUT_FILENO, buf, numRead) != numRead) {
                fprintf(stderr, "child - partial/failed write");
            }
        }
        write(STDOUT_FILENO, "\n", 1);

        gettimeofday(time, NULL);
        secs = time->tv_sec;
        usecs = time->tv_usec;
        printf("\nChild time: %ld,%.6ld\n", secs, usecs);
        free(time);

        if(close(filedes[0]) == -1) {
            fprintf(stderr, "Error closing.\n");
        }
        break;
    default:
        CPU_SET(parentCPU, &set);
        if(sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
            fprintf(stderr, "Error sched_setaffinity\n");
            exit(1);
        }
        if(close(filedes[0]) == -1) {
            fprintf(stderr, "Close error un parent.\n");
            exit(1);
        }
        if(write(filedes[1], argv[1], strlen(argv[1])) != (long int)strlen(argv[1])) {
            fprintf(stderr, "parent - partial/failed write");
        }
        if(close(filedes[1]) == -1) {
            fprintf(stderr, "Error closing.\n");
            exit(1);
        }
        wait(NULL);

        gettimeofday(time, NULL);
        secs = time->tv_sec;
        usecs = time->tv_usec;
        printf("\nParent time: %ld,%.6ld\n", secs, usecs);
        free(time);
        break;
    }
    return 0;
}

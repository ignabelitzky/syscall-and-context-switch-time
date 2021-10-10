#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define BUF_SIZE 10

/*
 * TO-DO
 * Figure it out how to calculate the time with gettimeofday between 2 processes
 * and where to put the funcion, also check how to make the child and the parent
 * run on the same cpu (check for sched_setaffinity()
 */

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("usage: program <argument>\n");
        exit(1);
    }
    int filedes[2];
    char buf[BUF_SIZE];
    ssize_t numRead = 0;

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
        if(close(filedes[0]) == -1) {
            fprintf(stderr, "Error closing.\n");
        }
        break;
    default:
        if(close(filedes[0]) == -1) {
            fprintf(stderr, "Close error un parent.\n");
            exit(1);
        }
        if(write(filedes[1], argv[1], strlen(argv[1])) != strlen(argv[1])) {
            fprintf(stderr, "parent - partial/failed write");
        }
        if(close(filedes[1]) == -1) {
            fprintf(stderr, "Error closing.\n");
            exit(1);
        }
        wait(NULL);
        break;
    }
    return 0;
}

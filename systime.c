#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

int main(void) {
    int rc = 0;
    struct timeval beginTime, endTime;
    long int secs = 0, usecs = 0;

    gettimeofday(&beginTime, NULL);
    rc = read(STDIN_FILENO, NULL, 0);
    gettimeofday(&endTime, NULL);
    if(rc == -1) {
        fprintf(stderr, "Error un read syscall.\n");
        exit(1);
    }

    secs = endTime.tv_sec - beginTime.tv_sec;
    if(endTime.tv_usec >= beginTime.tv_usec) {
        usecs = endTime.tv_usec - beginTime.tv_usec;
    } else {
        usecs = beginTime.tv_usec - endTime.tv_usec;
    }
    
    printf("\nTime of syscall: %ld,%.6ld seconds.\n\n", secs, usecs);
    return 0;
}

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <sched.h>

#define BUF_SIZE 50

void reverse(char s[]);
void itoa(int n, char s[]);

int main(void) {
    char *message = "\nTesting the time of a context Switch.\n\n";
    struct timeval *time = malloc(sizeof(struct timeval));
    long int secs = 0, usecs = 0;

    int filedes[6];
    char buf[BUF_SIZE];
    char sec_buf[BUF_SIZE];
    char usec_buf[BUF_SIZE];
    ssize_t numRead = 0;

    cpu_set_t set;
    int parentCPU = 0, childCPU = 0;

    CPU_ZERO(&set);
    
    for(int i = 0; i < 3; ++i) {
        if(pipe(filedes + 2 * i) == -1) {
            fprintf(stderr, "Error in pipe.\n");
            exit(1);
        }
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

        gettimeofday(time, NULL);
        secs = time->tv_sec;
        usecs = time->tv_usec;
        free(time);

        for(int i = 1; i <= 2; ++i) {
            if(close(filedes[i]) == -1) {
                fprintf(stderr, "Error closing filedes with index %d.\n", i);
                exit(1);
            }
        }
        if(close(filedes[4]) == -1) {
                fprintf(stderr, "Error closing filedes with index 4.\n");
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
                fprintf(stderr, "Child - partial/failed write.\n");
            }
        }
        write(STDOUT_FILENO, "\n", 1);
 
        itoa(secs, sec_buf);    // converts secs to string
        if(write(filedes[3], sec_buf, strlen(sec_buf)) != (long int)strlen(sec_buf)) {
            fprintf(stderr, "Child - partial/failed wirte.\n");
        }

        itoa(usecs, usec_buf);   // converts usecs to string
        if(write(filedes[5], usec_buf, strlen(usec_buf)) != (long int)strlen(usec_buf)) {
            fprintf(stderr, "Child - partial/failed write.\n");
        }

        if(close(filedes[0]) == -1) {
            fprintf(stderr, "Error closing filedes with index 0.\n");
        }
        if(close(filedes[3]) == -1) {
            fprintf(stderr, "Error closing filedes with index 3.\n");
        }
        if(close(filedes[5]) == -1) {
            fprintf(stderr, "Error closing filedes with index 5.\n");
        }
        break;
    default:
        CPU_SET(parentCPU, &set);
        if(sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
            fprintf(stderr, "Error sched_setaffinity\n");
            exit(1);
        }
        if(close(filedes[0]) == -1) {
            fprintf(stderr, "Error closing filedes with index 0 in parent\n");
            exit(1);
        }
        if(close(filedes[3]) == -1) {
            fprintf(stderr, "Error closing filedes with index 3 in parent.\n");
            exit(1);
        }
        if(close(filedes[5]) == -1) {
            fprintf(stderr, "Error closing filedes with index 5 in parent.\n");
            exit(1);
        }
        if(write(filedes[1], message, strlen(message)) != (long int)strlen(message)) {
            fprintf(stderr, "parent - partial/failed write");
        }
        if(close(filedes[1]) == -1) {
            fprintf(stderr, "Error closing.\n");
            exit(1);
        }
        for(;;) {
            numRead = read(filedes[2], sec_buf, BUF_SIZE);
            if(numRead == -1) {
                fprintf(stderr, "Error reading.\n");
                exit(1);
            }
            if(numRead == 0) {
                break; // End of file
            }
        }
        numRead = 0;
        long int childSecs = (long int)atoi(sec_buf);
        for(;;) {
            numRead = read(filedes[4], usec_buf, BUF_SIZE);
            if(numRead == -1) {
                fprintf(stderr, "Error reading.\n");
                exit(1);
            }
            if(numRead == 0) {
                break;  // End of file
            }
        }
        long int childMicroSecs = (long int)atoi(usec_buf);
        if(close(filedes[2]) == -1) {
            fprintf(stderr, "Error closing filedes with index 2.\n");
            exit(1);
        }
        if(close(filedes[4]) == -1) {
            fprintf(stderr, "Error closing filedes with index 4.\n");
            exit(1);
        }
        wait(NULL);

        gettimeofday(time, NULL);
        secs = time->tv_sec;
        usecs = time->tv_usec;
        free(time);

        long int differenceSecs = 0, differenceMicroSecs = 0;

        differenceSecs = secs >= childSecs? secs-childSecs:childSecs-secs; 
        differenceMicroSecs = usecs >= childMicroSecs?usecs-childMicroSecs:childMicroSecs-usecs;

        printf("Parent time: %ld,%.6ld seconds.\n", secs, usecs);
        printf("Child time: %ld,%.6ld seconds.\n", childSecs, childMicroSecs);
        printf("\nContext Switch Time: %ld,%.6ld seconds.\n\n", differenceSecs, differenceMicroSecs);
        break;
    }
    return 0;
}


void reverse(char s[])
{
    int i, j;
    char c;
    for(i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[])
{
    int i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while((n /= 10) > 0);
    s[i] = '\0';
    reverse(s);
}

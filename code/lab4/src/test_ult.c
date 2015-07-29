#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include "ult.h"
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 32 * 1

// int end_copying = 0;
long copyed_bytes = 0;
time_t start_time;
bool got_fish = false;

void threadA() {
    FILE *input = fopen("/dev/random", "r");
    FILE *dump = fopen("/dev/null", "wb");
    int input_fd = fileno(input);
    int dump_fd = fileno(dump);

    int buffer[BUF_SIZE]; // buffer is NOT clean here

    // while(true)
    // printf("Hello from A\n");
    int len;
    start_time = time(NULL);
    while (true) {

        // careful: anly write len, not BUF_SIZE because ult_read may return less bytes

        len = ult_read(input_fd, &buffer, BUF_SIZE);
        write(dump_fd, &buffer, len);
        copyed_bytes += len;

        // Clear the buffer again
        // memset(&buffer, 0, sizeof(buffer));
        ult_yield();
    }
    // printf("Stopping file dump. This was pretty fucking pointless!\n");
}

void readInput(char *buf, int *bufLen) {
    int i;
    char in[1];
    
    i = 0;
    in[0] = '\0';
    while(ult_read(STDIN_FILENO, &in, 1) && in[0] != '\n' && in[0] != EOF) {
        buf[i] = in[0];
        i++;
        if(i>*bufLen) {
            realloc(bufLen, *bufLen = *bufLen * 2);
        }
    }
    buf [i] = '\0';
}

void threadB() {
    int bufLen = 1024;
    char *buf = calloc(bufLen, sizeof(char));
    
    while (true) {
        printf(">");
        readInput(buf, &bufLen);
        
        if (strcmp(buf, "exit") == 0) {
            if(got_fish) {
                printf("Mach's gut, und danke für den Fisch!\n");
            } else {
                printf("bye\n");
            }
            
            free(buf);
            exit(0);
        } else if(strcmp(buf, "fish") == 0) {
            got_fish = true;
            printf("Danke\n");
        } else if(strcmp(buf, "stats") == 0) {
            time_t currTime = time(NULL);
            printf("Anzahl kopierter Bytes: %ld\nZeit seit start des Thread: %ld Sekunden\nDurchsatz: %ldBytes/Sekunde\n", copyed_bytes,  currTime -start_time, copyed_bytes / (currTime -start_time));
        } else {
            printf("Unbekannter input: %s\n", buf);
        }
        
        buf[0] = '\0';
        ult_yield();
    }
}

void myInit() {
    int cpid[2], i, status;
    printf("spawn A\n");
    fflush(stdout);
    cpid[0] = ult_spawn(threadA);
    printf("spawn B\n");
    fflush(stdout);
    cpid[1] = ult_spawn(threadB);

    for (i = 0; i < 2; i++) {
        printf("waiting for cpid[%d] = %d\n", i, cpid[i]);
        fflush(stdout);
        if (ult_waitpid(cpid[i], &status) == -1) {
            fprintf(stderr, "waitpid for %d failed\n", cpid[i]);
            ult_exit(-1);
        }
        printf("(status = %d)\n", status);
    }
    puts("ciao!");
    ult_exit(0);
}

int main() {
    printf("starting myInit\n");
    fflush(stdout);
    ult_init(myInit);
    exit(0);
}



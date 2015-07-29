// Include header files
#include <stdlib.h>
#include <stdbool.h>

#include "sar_lib.h"

// Define background job
typedef struct _backgroundjob
{
	struct _backgroundjob *next, *prev;
    int pid;
    bool finished;
    int result;
} BackgroundJob;

void bj_list();

// Construct background job
void bj_add(int pid);

// Wait for background job to terminate
BackgroundJob *bj_wait(int[]);

// Interupt wait
void bj_wait_interrupt();

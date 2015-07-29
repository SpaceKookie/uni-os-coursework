// Include header files
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

// Define command
typedef struct _call_cmd
{
    char *name;
    char **args;
    struct _call_cmd *pipe;
} Command;

// Define call
typedef struct
{
    Command *command;
    bool isBackground;
} Call;

// Construct call from string
Call *call_new(char*);

// Free call
void call_free(Call*);

// Run call and return last pid
int call_run(Call*);

// Get pid of the current foreground process
int call_getFgPid();

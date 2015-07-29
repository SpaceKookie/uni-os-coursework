// Include header file
#include "sar_shell.h"

void printPrompt()
{
    // Get current working directory
    char *cwd = getCWD();

    // Print prompt
    printf("[%s]$ ", cwd);

    // Free cwd string
    free(cwd);
}

// Parse array of pid strings
int *parsePids(char *pidStrs[])
{
    // Count arguments
    int pidCount = 0;
    int ps;

    for (ps = 0; pidStrs[ps] != NULL; ++ps)
    {
        pidCount++;
    }

    // Parse pids
    int *pids = malloc((pidCount + 1) * sizeof(int));

    for(ps = 0; pidStrs[ps] != NULL; ++ps)
    {
        pids[ps] = atoi(pidStrs[ps]);
        if (pids[ps] <= 0) { pids[ps] = -1; }
    }

    // Add zero termination
    pids[pidCount] = 0;

    // Return parsed pid array
    return pids;
}

// Wait for array of pids and print status info
void waitForPids(int pids[])
{
    // Wait for jobs and print their result
    BackgroundJob *job;
    while (job = bj_wait(pids))
    {
        // Print job info
        printf("Job %i done with status %i\n", job->pid, WEXITSTATUS(job->result));

        // Free the job
        free(job);
    }
}

// Handle signal
void sigIntHandler(int dummy)
{
    // Get foreground process id
    int fgPid = call_getFgPid();

    // Send kill signal if existent
    if (fgPid > 0)
    {
        kill(fgPid, SIGINT);
    }
    // Interrupt wait else
    else
    {
        bj_wait_interrupt();;

        if(!fork()) {
            exit(0);
        }
    }
}

// Everything starts right here
int main()
{
    // Setup signal handler
	signal(SIGINT, sigIntHandler);
    
    // Read, interprete and process commands
    while(true)
    {
        // Print prompt
        printPrompt();

        // Read input line
        char *input = readInput();
        
        // Validate input
        if(strlen(input)) {
            // Parse command
            Call *call = call_new(trimString(input));

            // Validate call
            if (call)
            {
                // Wait for processes
                if(!strcmp(call->command->name, "wait"))
                {
                    // Parse process ids from arguments
                    int *pids = parsePids(&call->command->args[1]);

                    // Wait for specified processes
                    waitForPids(pids);

                    // Free pid list
                    free(pids);
                }
                // Change directory
                else if(!strcmp(call->command->name, "cd")) {
                    chdir(call->command->args[1]);
                }
                // Exit shell
                else if(!strcmp(call->command->name, "exit")) {
                    exit(0);
                }
                // Execute program
                else
                {
                    // Run call
                    int pid = call_run(call);
                    if (call->isBackground)
                    {
                        printf("[%i]\n", pid);
                    }
                }
                // Free call information
                call_free(call);
            }
            // Print error on invalid call
            else
            {
                printf("Invalid command!\n");
            }
        }

        // Free input string
        free(input);
    }

    // Exit with success status
    return EXIT_SUCCESS;
}

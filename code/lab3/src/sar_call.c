// Include header file
#include "sar_call.h"

#define BUF_SIZE 64

int call_fg_pid = -1;

/*** Helper functions ***/

// Free command
void command_free(Command *cmd)
{
    // Do nothing on NULL pointer
    if (!cmd) { return; }

    // Iterate over and free arguments if existent
    if (cmd->args)
    {
        int i;
        for(i = 0; cmd->args[i] != NULL ; ++i)
        {
            free(cmd->args[i]);
        }

        // Free argument array
        free(cmd->args);
    }
    
    // Free piped command if existent
    if (cmd->pipe)
    {
        command_free(cmd->pipe);
    }

    // Free command finally
    free(cmd);
}

// Construct command from string starting at certain position
Command *command_new(char *cmdStr, int *c)
{
    // Allocate new command
    Command *command = malloc(sizeof(Command));
    command->pipe = NULL;

    // Calculate argument count
    int argc = getValidSeqCount(&cmdStr[*c]);

    // Return NULL if no valid sequences contained
    if (argc == 0)
    {
        command_free(command);
        return NULL;
    }

    // Allocate arguments
    command->args = malloc((argc + 1) * sizeof(char*));

    // Loop through characters and extract arguments until invalid character
    int currentArg = 0;
    int argStart = -1;
    while(true)
    {
        // Set start of sequence if valid character after space/start
        if (isValidCmdChar(cmdStr[*c]))
        {
            if (argStart == -1)
            {
                argStart = *c;
            }
        }
        // If space or end reached after valid character sequence, copy argument and prepare next
        else
        {
            if (argStart >= 0)
            {
                // Get length of current argument
                int argLen = *c - argStart;

                // Allocate and copy current argument
                char *arg = malloc(argLen+1);
                substrCpy(cmdStr, arg, argStart, argLen);
                arg[argLen] = '\0';

                command->args[currentArg] = arg;

                // Prepare next argument
                ++currentArg;
                argStart = -1;
            }

            // Stop on invalid character
            if (!isspace(cmdStr[*c]))
            {
                break;
            }
        }

        // Increase character counter
        ++*c;
    }

    // Terminate argument list with NULL
    command->args[argc] = NULL;
    
    // Point command name to first argument
    command->name = command->args[0];

    // Return constructed command<div></div>
    return command;
}

// Execute command
int command_exec(char *cmd, char **args, int inPipe, int *outPipe)
{
    // Prepare new pipe
    int fd[2];

    // Create pipe and handle errors
    if (pipe(fd))
    {
        printf("Pipe error!\n");
        exit(EXIT_FAILURE);
    }

    // Fork process
    int pid = fork();

    // Handle errors
    if (pid < 0)
    {
        printf("Fork error!\n");
        exit(EXIT_FAILURE);
    }
    
    // Execute program in child process
    else if (pid == 0)
    {
        // Connect pipe and close file descriptor from last command if requested
        if (inPipe >= 0)
        {
            dup2(inPipe, 0);
            close(inPipe);
        }
        // Connect pipe to next command if requested
        if (*outPipe >= 0)
        {
            dup2(fd[1], 1);
        }
        
        // Close unused file descriptors
        close(fd[0]);
        close(fd[1]);

        // Execute program
        execvp(cmd, args);

        // Terminate on error
        printf("Command execution error!\n");
        exit(EXIT_FAILURE);
    }
    // Clean up and return in main process
    else
    {
        // Set group id to zero
        setpgid(pid, 0);

        // Close file descriptors
        close(fd[1]);
        if (inPipe >= 0)
        {
            close(inPipe);
        }

        // Assign pipe for possible next command or close if not requested
        if (*outPipe >= 0)
        {
            *outPipe = fd[0];
        }
        else
        {
            close(fd[0]);
        }
    }

    // Return new process id
    return pid;
}

/*** Interface ***/

// Free call
void call_free(Call *call)
{
    // Free attached command if not NULL
    if (call->command)
    {
        command_free(call->command);
    }

    // Free call
    free(call);
}

// Construct call from string
Call *call_new(char *callStr)
{
    // Allocate new call
    Call *call = malloc(sizeof(Call));

    // Initialize character iterator
    int c = 0;

    // Interprete first command
    call->command = command_new(callStr, &c);
    call->isBackground = false;
    Command *lastCommand = call->command;

    // Return NULL if invalid command
    if (lastCommand == NULL)
    {
        call_free(call);
        return NULL;
    }

    // Interprete call string by iterating over characters
    while (callStr[c] != '\0')
    {
        // Pipe if valid
        if (callStr[c] == '|')
        {
            // Pipe starting at next character if valid command follows
            ++c;
            lastCommand->pipe = command_new(callStr, &c);
            if (lastCommand->pipe)
            {
                lastCommand = lastCommand->pipe;
            }
            // Return NULL else
            else
            {
                call_free(call);
                return NULL;
            }
        }
        // Set to background
        else if (callStr[c] == '&')
        {
            // Set background job if last character
            if (callStr[c+1] == '\0')
            {
                call->isBackground = true;
                break;
            }
            // Return NULL else
            else
            {
                call_free(call);
                return NULL;
            }
        }
        // Consume space
        else if (isspace(callStr[c]))
        {
            ++c;
        }
        // Return NULL on invalid character
        else
        {
            call_free(call);
            return NULL;
        }
    }
    
    // Return constructed call
    return call;
}

// Run call and return last pid
int call_run(Call *call)
{
    // Declare file descriptor to null device
    int devnull;

    // Set pipe according to background property
    int cmdIn;
    int cmdOut;

    if (call->isBackground)
    {
        cmdIn = devnull = open("/dev/null", O_WRONLY);
    }
    else { cmdIn = -1; }

    // Declare variable to store process id of last process
    int pid;

    // Loop through commands
    Command *current = call->command;
    while (current)
    {
        // Keep stdout of last process if not in background
        cmdOut = (current->pipe == NULL && !call->isBackground) ? -1 : 0;

        // Execute and pipe command
        pid = command_exec(current->name, current->args, cmdIn, &cmdOut);

        // Setup next commands input to current output
        cmdIn = cmdOut;

        // Continue with next command
        current = current->pipe;
    }

    // Bind last process output to null device if in background
    if (call->isBackground)
    {
        dup2(devnull, cmdOut);
        close(cmdOut);

        // Close null device descriptor
        close(devnull);
    }

    // Add background job
    bj_add(pid);

    // Wait for process if not in background
    if (!call->isBackground)
    {
        call_fg_pid = pid;
        
        int pidArray[] = {pid, 0};
        bj_wait(pidArray);

        call_fg_pid = -1;
    }

    // Return process id of last process
    return pid;
}

// Get pid of the current foreground process
int call_getFgPid()
{
    return call_fg_pid;
}

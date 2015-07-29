// Include header file
#include "sar_backgroundjob.h"

// Setup doubly linked list first and last elements
BackgroundJob *bj_first = NULL, *bj_last = NULL;
bool bj_interrupted = false;

/*** Helper functions ***/

// Find background job by pid
BackgroundJob *bj_find(int pid)
{
	// Search job with specified pid
	BackgroundJob *current = bj_first;
	while (current && current->pid != pid)
	{
		current = current->next;
	}

	// Return corresponding job or NULL
	return current;
}

// Remove job
void bj_remove(BackgroundJob *job)
{
	// Update pointers
	if (job->prev != NULL)	{ job->prev->next = job->next; }
	else					{ bj_first = job->next; }

	if (job->next != NULL)	{ job->next->prev = job->prev; }
	else					{ bj_last = job->prev; }

	job->next = job->prev = NULL;
}

/*** Interface ***/

void bj_list()
{
	BackgroundJob *current = bj_first;
	while (current)
	{
		printf("%i(%i) -> ", current->pid, current->finished);
		current = current->next;
	}
	printf("\n");
}

// Construct and insert background job
void bj_add(int pid)
{
	// Allocate new background job
	BackgroundJob *newJob = calloc(1, sizeof(BackgroundJob));

	// Setup properties
	newJob->pid = pid;
	newJob->finished = false;

	// Insert into list
	newJob->next = NULL;
	newJob->prev = bj_last;
	if (bj_last != NULL)	{ bj_last->next = newJob; }
	else					{ bj_first = newJob; }
	bj_last = newJob;
}

// Wait for background job contained in zero terminated pid array to terminate
BackgroundJob *bj_wait(int pids[])
{
	// Reset interrupt
	bj_interrupted = false;

	// Initialize remaining job counter
	int remainingJobs = 0;

	// Loop through pids to find already finished job
	int p;
	for (p = 0; pids[p] != 0; ++p)
	{
		// Skip if invalid pid
		if (pids[p] < 0) { continue; }

		// Find job with pid
		BackgroundJob *job = bj_find(pids[p]);

		// Continue check if valid job
		if (job)
		{
			if (job->finished)
			{
				// Remove job
				bj_remove(job);

				// Return terminated job
				return job;
			}
			else
			{
				remainingJobs++;	
			}
		}
		else
		{
			// Set pid to -1 if not a background job
			pids[p] = -1;
		}
	}

	// Return NULL if no remaining jobs
	if (!remainingJobs) { return NULL; }

	// Loop until background job terminated
	while (!bj_interrupted)
	{
		// Wait for child process to terminate
		int result = 0;
		int pid = wait(&result);

		// Handle errors
		if (pid == -1)
		{
			printf("Waiting error!\n");
			exit(EXIT_FAILURE);
		}

		// Search for job with current pid
		BackgroundJob *job = bj_find(pid);

		// Update job if existent and return if requested
		if (job)
		{
			// Update properties
			job->finished = true;
			job->result = result;

			// Check if waiting for current pid
			if (intIndexOf(pids, pid) != -1)
			{
				// Remove job
				bj_remove(job);

				// Return terminated job
				return job;
			}
		}
	}
}

// Interupt wait
void bj_wait_interrupt()
{
	bj_interrupted = true;
}

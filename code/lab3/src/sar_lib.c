// Include header file
#include "sar_lib.h"

// Copy substring
void substrCpy(char *source, char *target, int first, int count)
{
	// Copy characters
	int c;
	for (c = 0; c < count; ++c)
	{
		target[c] = source[first + c];
	}

	target[c] = '\0';
}

// Trim string in place and return pointer (You can't free() this pointer!)
char *trimString(char *str)
{
	int c;

	// Iterate until first non-space found
	for (c = 0; str[c] != '\0'; ++c)
	{
		if (!isspace(str[c])) { break; }
	}

	// Set pointer to found character
	str = &str[c];

	// Find last space
	int lastSpace = -1;
	for (c = 0; str[c] != '\0'; ++c)
	{
		// Store if first space after non-space
		if (isspace(str[c]))
		{
			if (lastSpace == -1)
			{
				lastSpace = c;
			}
		}
		// Reset if non-space
		else
		{
			lastSpace = -1;
		}
	}

	// Replace with \0 termination if spaces found
	if (lastSpace != -1)
	{
		str[lastSpace] = '\0';
	}
	
	// Return pointer to trimed string
	return str;
}

// Check whether characters can be used in command
bool isValidCmdChar(char c)
{
	return (33 <= c && c <= 126 && c != '&' && c != '|');
}

// Get count of valid sequences separated by spaces
int getValidSeqCount(char *str)
{
    // Initialize counter and helper variable
    int count = 0;
    bool inSeq = false;

    // Count sequences by iterating over characters
    int c = 0;
    while (true)
    {
        // Set currently in sequence if valid character
        if (isValidCmdChar(str[c]))
        {
            inSeq = true;
        }
        else
        {
        	// Increase counter on space or invalid character after sequence
        	if (inSeq)
            {
                ++count;
                inSeq = false;
            }

            // Break if invalid character
            if (!isspace(str[c]))
            {
            	break;
            }
        }

        // Increase character iterator
        ++c;
    }

    return count;
}

// Check if value contained in array
int intIndexOf(int array[], int value)
{
	// Search for value
	int i;
	for (i = 0; array[i] != 0; ++i)
	{
		// Return index if found
		if (array[i] == value) { return i; }
	}

	// Return -1 if not found
	return -1;
}

// Read input line
char *readInput()
{
	// Setup variables
    char *input;
    size_t inputLen = 0;
	ssize_t byteCount;

    // Read until non-empty input line
	do
	{
		// Read line and determin count of read characters
		byteCount = getline(&input, &inputLen, stdin);
	}
	while (!byteCount);

	// Replace linebreak with NULL termination
    input[byteCount-1] = '\0';

	// Return read string
    return input;
}

// Get current working directory
char *getCWD()
{
	// Allocate string memory
	char *dir = (char *) malloc(PATH_MAX);

	// Write current working directory to string
	getcwd(dir, PATH_MAX);

	// Return string
	return dir;
}

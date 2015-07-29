// Include header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <linux/limits.h>

// Copy substring
void substrCpy(char*, char*, int, int);

// Trim string in place and return pointer (You can't free() this pointer!)
char *trimString(char*);

// Check whether characters can be used in command
bool isValidCmdChar(char);

// Get count of alphanumeric character sequences separated by spaces
int getAlNumSeqCount(char*);

// Check if value contained in array
int intIndexOf(int[], int);

// Read input line
char *readInput();

// Allocate and fill empty string of given size
char *getEmptyString(int);

// Get current working directory
char *getCWD();

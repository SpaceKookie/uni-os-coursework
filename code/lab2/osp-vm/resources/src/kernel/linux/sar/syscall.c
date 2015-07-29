#include <linux/kernel.h>
#include <linux/syscalls.h>

#define PROMPTLEN 10

char prompt[] = "my_prompt_";

// 351
void setPrompt(char* newPrompt)
{

}

// 352
char* getPrompt()
{
	return prompt;
}

// 353
char* swapPrompt(char* newPrompt)
{

}
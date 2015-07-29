/** AUTHOR: (c) 2015 KATHARINA SABEL
 *  LICENSE: MIT PUBLIC LICENSE
 *  COMMENT: A VERY SIMPLE PROGRAM TO DEMONSTRATE THE FUNDAMENTAL WORKINGS
 *  		OF A BOOTLOADER FOR THE x86 ARCHITECTURE
 */

#ifndef _CODE16GCC_H_
#define _CODE16GCC_H_

/** This sets a prefix to run 32bit code in 16bit mode */
asm(".code16gcc\n");
#endif

/* This jumps to the main method */
asm("jmpl $0, $main");

/* Utility character print function */
void printChar(char c)
{
	asm(
		"mov $0x0e, %%ah;"
		"int $0x10;"::"al"(c)
	);
}

/* Utility character read function */
char getChar(void)
{
	char c;
	asm("mov $0x00, %%ax;"::);
	asm("int $0x16":"=al"(c):/* EMPTY */);

	return c;
}

/* Reboot utility function */
void reboot(void)
{
	asm("int $0x19");
}

/* Utility function to print a string */
void printString(char str[], int size)
{
	int i;
	for(i = 0; i <= size; i++){
		printChar(str[i]);
	}
}

/** Utility function to set a newline */
void newLine(void)
{
	printChar('\n');
	printChar('\r');
}

/** Main function */
void main(void)
{	
	printString("Hello!", 6);
	newLine();
	while(1)
	{
		int counter;
		char passphrase[] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
		
		int inputLength = 0;

		/** Checks for 8 character passwords */
		for(counter = 0; counter < 8; counter++)
		{
			char c = getChar();
			printChar('*');
			if(c == '\r') break;
			inputLength++;
			passphrase[counter] = c;
		}

		newLine();
		printString(passphrase, 8);
		newLine();
		
		if(inputLength == 0) break;
	}

	printString("Reboot!", 7);
	newLine();
	reboot();
	asm("jmp .");
}






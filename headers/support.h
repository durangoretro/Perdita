#include "globals.h"

#ifndef SUPPORT_H
#define SUPPORT_H

void usage(char name[]);// help with command line options
void stat(void);		// display processor status
void stack_stat(void);	// display stack status
void dump(word dir);	// display 16 bytes of memory
void full_dump(void);    // Dump full memory space and registers to file
void load_dump(const char name[]); // Loads memory and registers from dump file

#endif

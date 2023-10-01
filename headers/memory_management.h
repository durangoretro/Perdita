#include <time.h>

#include "globals.h"
#include "vps.h"
#include "vdu.h"

#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H

/* memory management */
	byte peek(word dir);			// read memory or I/O
	void poke(word dir, byte v);	// write memory or I/O
	inline void push(byte b)		{ poke(0x100 + s--, b); }		// standard stack ops
	inline byte pop(void)			{ return peek(++s + 0x100); }
	void randomize(void);

#endif

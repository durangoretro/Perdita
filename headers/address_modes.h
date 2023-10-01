#include "globals.h"
#include "memory_management.h"

#ifndef ADDRESS_MODES_H
#define ADDRESS_MODES_H

	word am_a(void);		// absolute
	word am_ax(int*);		// absolute indexed X, possible penalty
	word am_ay(int*);		// absolute indexed Y, possible penalty
	inline byte am_zx(void)		{ return (peek(pc++) + x) & 255; }	// ZeroPage indexed X
	inline byte am_zy(void)		{ return (peek(pc++) + y) & 255; }	// ZeroPage indexed Y (rare)
	word am_ix(void);		// pre-indexed indirect X (rare)
	word am_iy(int*);		// indirect post-indexed Y, possible penalty
	word am_iz(void);		// indirect (CMOS only)
	inline word am_ai(void)		{ word j=am_a(); return peek(j)  |(peek(j+1)  <<8); }	// absolute indirect, not broken
	inline word am_aix(void)		{ word j=am_a(); return peek(j+x)|(peek(j+x+1)<<8); }	// absolute pre-indexed indirect (CMOS only)
	void rel(int*);			// relative branches, possible penalty

#endif

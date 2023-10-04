#include "address_modes.h"

/* *** addressing modes *** */
/* absolute */
word am_a(void) {
	word pt = peek(pc) | (peek(pc+1) <<8);
	pc += 2;

	return pt;
}

/* absolute indexed X */
word am_ax(int *bound) {
	word ba = am_a();		// pick base address and skip operand
	word pt = ba + x;		// add offset
	*bound = ((pt & 0xFF00)==(ba & 0xFF00))?0:1;	// check page crossing

	return pt;
}

/* absolute indexed Y */
word am_ay(int *bound) {
	word ba = am_a();		// pick base address and skip operand
	word pt = ba + y;		// add offset
	*bound = ((pt & 0xFF00)==(ba & 0xFF00))?0:1;	// check page crossing

	return pt;
}

/* indirect */
word am_iz(void) {
	word pt = peek(peek(pc)) | (peek((peek(pc)+1)&255)<<8);	// EEEEEEEK
	pc++;

	return pt;
}

/* indirect post-indexed */
word am_iy(int *bound) {
	word ba = am_iz();		// pick base address and skip operand
	word pt = ba + y;		// add offset
	*bound = ((pt & 0xFF00)==(ba & 0xFF00))?0:1;	// check page crossing

	return pt;
}

/* pre-indexed indirect */
word am_ix(void) {
	word pt = (peek((peek(pc)+x)&255)|(peek((peek(pc)+x+1)&255)<<8));	// EEEEEEK
	pc++;

	return pt;
}

/* relative branch */
void rel(int *bound) {
	byte off = peek(pc++);	// read offset and skip operand
	word old = pc;

	pc += off;
	pc -= (off & 128)?256:0;						// check negative displacement

	*bound = ((old & 0xFF00)==(pc & 0xFF00))?0:1;	// check page crossing
}


#include "globals.h"

#ifndef OPCODES_H
#define OPCODES_H

void bits_nz(byte b);	// set N&Z flags

void asl(byte *d);		// shift left
void lsr(byte *d);		// shift right
void rol(byte *d);		// rotate left
void ror(byte *d);		// rotate right

void adc(byte d);		// add to A with carry
void sbc(byte d);		// subtract from A with borrow, 6502-style
void cmp(byte reg, byte d);		// compare, based on subtraction result

#endif

#include "opcodes.h"

/* *** opcode assistants *** */
/* compute usual N & Z flags from value */
void bits_nz(byte b) {
	p &= 0b01111101;		// pre-clear N & Z
	p |= (b & 128);			// set N as bit 7
	p |= (b==0)?2:0;		// set Z accordingly
}

/* ASL, shift left */
void asl(byte *d) {
	p &= 0b11111110;		// clear C
	p |= ((*d) & 128) >> 7;	// will take previous bit 7
	(*d) <<= 1;				// EEEEEEEEK
	bits_nz(*d);
}

/* LSR, shift right */
void lsr(byte *d) {
	p &= 0b11111110;		// clear C
	p |= (*d) & 1;			// will take previous bit 0
	(*d) >>= 1;				// eeeek
	bits_nz(*d);
}

/* ROL, rotate left */
void rol(byte *d) {
	byte tmp = (p & 1);		// keep previous C

	p &= 0b11111110;		// clear C
	p |= ((*d) & 128) >> 7;	// will take previous bit 7
	(*d) <<= 1;				// eeeeeek
	(*d) |= tmp;			// rotate C
	bits_nz(*d);
}

/* ROR, rotate right */
void ror(byte *d) {
	byte tmp = (p & 1)<<7;	// keep previous C (shifted)

	p &= 0b11111110;		// clear C
	p |= (*d) & 1;			// will take previous bit 0
	(*d) >>= 1;				// eeeek
	(*d) |= tmp;			// rotate C
	bits_nz(*d);
}

/* ADC, add with carry */
void adc(byte d) {
	byte old = a;
	word big = a, high;

	big += d;				// basic add... but check for Decimal mode!
	big += (p & 1);			// add with Carry (A was computer just after this)

	if (p & 0b00001000) {						// Decimal mode!
		high = (old & 0x0F)+(d & 0x0F);			// compute carry-less LSN
		if (((big & 0x0F) > 9)||(high  0x10))) {		// LSN overflow? was 'a' instead of 'big'
			big += 6;											// get into next decade
		}
		if (((big & 0xF0) > 0x90)||(big & 256)) {				// MSN overflow?
			big += 0x60;						// correct it
		}
	}
	a = big & 255;			// placed here trying to correct Carry in BCD mode

	if (big & 256)			p |= 0b00000001;	// set Carry if needed
	else					p &= 0b11111110;
	if ((a&128)^(old&128))	p |= 0b01000000;	// set oVerflow if needed
	else					p &= 0b10111111;
	bits_nz(a);									// set N & Z as usual
}

/* SBC, subtract with borrow */ //EEEEEEEEEEEEEEEEK
void sbc(byte d) {
	byte old = a;
	word big = a;

	big += ~d;				// basic subtract, 6502-style... but check for Decimal mode!
	big += (p & 1);			// add with Carry
	
	if (p & 0b00001000) {						// Decimal mode!
		if ((big & 0x0F) > 9) {					// LSN overflow?
			big -= 6;								// get into next decade *** check
		}
		if ((big & 0xF0) > 0x90) {				// MSN overflow?
			big -= 0x60;							// correct it
		}
	}
	a = big & 255;			// same as ADC

	if (big & 256)			p &= 0b11111110;	// set Carry if needed EEEEEEEEEEEEK, is this OK?
	else					p |= 0b00000001;
	if ((a&128)^(old&128))	p |= 0b01000000;	// set oVerflow if needed
	else					p &= 0b10111111;
	bits_nz(a);									// set N & Z as usual
}

/* CMP/CPX/CPY compare register to memory */
void cmp(byte reg, byte d) {
	word big = reg;

	big -= d;				// apparent subtract, always binary

	if (big & 256)			p &= 0b11111110;	// set Carry if needed (note inversion)
	else					p |= 0b00000001;
	bits_nz(reg - d);							// set N & Z as usual
}


#include "globals.h"
#include "memory_management.h"
#include "interrupt.h"

/* *** interrupt support *** */
/* acknowledge interrupt and save status */
void intack(void) {
	push(pc >> 8);							// stack standard status
	push(pc & 255);
	push(p);

	p |= 0b00000100;						// set interrupt mask
	p &= 0b11110111;						// and clear Decimal mode (CMOS only)
	dec = 0;

	cont += 7;								// interrupt acknowledge time
}

/* reset CPU, like !RES signal */
void reset(void) {
	pc = peek(0xFFFC) | peek(0xFFFD)<<8;	// RESET vector

	if (ver > 1)	printf(" RESET: PC=>%04X\n", pc);

	p &= 0b11110111;						// CLD on 65C02
	p |= 0b00110100;						// these always 1, includes SEI
	dec = 0;								// per CLD above
	mem[0xDFA0] = 0;						// interrupt gets disabled on RESET!
	mem[0xDFB0] = 0;						// ...and so does BUZZER
	gamepads[0] = 0;						// Reset gamepad 1 register
	gamepads[1] = 0;						// Reset gamepad 2 register

	cont = 0;								// reset global cycle counter?
}

/* emulate !NMI signal */
void nmi(void) {
	intack();								// acknowledge and save

	pc = peek(0xFFFA) | peek(0xFFFB)<<8;	// NMI vector
	if (ver > 1)	printf(" NMI: PC=>%04X\n", pc);
}

/* emulate !IRQ signal */
void irq(void) {
	if (!(p & 4)) {								// if not masked...
		p &= 0b11101111;						// clear B, as this is IRQ!
		intack();								// acknowledge and save
		p |= 0b00010000;						// retrieve current status

		pc = peek(0xFFFE) | peek(0xFFFF)<<8;	// IRQ/BRK vector
		if (ver > 1)	printf(" IRQ: PC=>%04X\n", pc);
	}
}


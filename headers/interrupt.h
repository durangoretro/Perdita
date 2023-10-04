#ifndef INTERRUPT_H
#define INTERRUPT_H

	void intack(void);		// save status for interrupt acknowledge
	void reset(void);		// RESET & hardware interrupts
	void nmi(void);
	void irq(void);

#endif

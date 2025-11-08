#include <stdlib.h>
#include <time.h>

#include "memory_management.h"

/* *** memory management *** */
/* read from memory or I/O */
byte peek(word dir) {
	byte d = 0;					// supposed floating databus value?

	if (dir>=0xDF80 && dir<=0xDFFF) {	// *** I/O ***
		if (dir<=0xDF87) {				// video mode (high nibble readable)
			d = mem[0xDF80] | 0x0F;		// assume RGB mode and $FF floating value
		} else if (dir<=0xDF8F) {		// sync flags
			d = mem[0xDF88];
		} else if (dir==0xDF93) {		// Read from VSP
			if (mem[0xDF94]==PSV_FREAD) {
				if (!feof(psv_file)) {
					d = fgetc(psv_file);				// get char from input file
					if (ver)	printf("(%d)", d);		// DEBUG transmitted char
				} else {
					d = 0;				// NULL means EOF
					printf(" Done reading file\n");
					fclose(psv_file);	// eeeeek
					psv_file = NULL;
					mem[0xDF94] = PSV_DISABLE;			// no further actions
				}
			}							// cannot read anything if disabled, default d=0 means EOF anyway
			mem[0xDF93] = d;			// cache value
		} else if (dir==0xDF9B && emulate_minstrel) {	// Minstrel keyboard port EEEEEK
			switch(mem[0xDF9B]) {
				case 1: return minstrel_keyboard[0];
				case 2: return minstrel_keyboard[1];
				case 4: return minstrel_keyboard[2];
				case 8: return minstrel_keyboard[3];
				case 16: return minstrel_keyboard[4];
				case 32: return 0x2C;
			}
									// no separate if-else is needed because of the default d value
									// ...and $DF9B could be used by another device
		} else if (dir<=0xDF9F) {	// expansion port
			d = mem[dir];			// *** is this OK?
		} else if (dir<=0xDFBF) {	// interrupt control and beeper are NOT readable and WILL be corrupted otherwise
			if (ver)	printf("\n*** Reading from Write-only ports at $%04X ***\n", pc);
			if (safe)	run = 0;
		} else {					// cartridge I/O
			d = mem[dir];			// *** is this OK?
		}
	} else {
		d = mem[dir];				// default memory read, either RAM or ROM
	}

	return d;
}

/* write to memory or I/O */
void poke(word dir, byte v) {
	if (dir<=0x7FFF) {			// 32 KiB static RAM
		mem[dir] = v;
		if ((dir & 0x6000) == screen) {			// VRAM area
			scr_dirty = 1;		// screen access detected, thus window must be updated!
		}
	} else if (dir>=0xDF80 && dir<=0xDFFF) {	// *** I/O ***
		if (dir<=0xDF87) {		// video mode?
			mem[0xDF80] = v;	// canonical address
			screen = (v & 0b00110000) << 9;		// screen switching
			scr_dirty = 1;		// note window should be updated when changing modes!
		} else if (dir<=0xDF8F) {				// sync flags not writable!
			if (ver)	printf("\n*** Writing to Read-only ports at $%04X ***\n", pc);
			if (safe)	run = 0;
		} else if (dir==0xDF93) { // virtual serial port at $df93
			vps_run(dir, v);
		} else if (dir==0xDF94) { // virtual serial port config at $df94
			vps_config(dir, v);
		} else if (dir==0xDF9C) { // gamepad 1 at $df9c
			if (ver>2)	printf("Latch gamepads\n");
			gamepads_latch[0] = gamepads[0];
			gamepads_latch[1] = gamepads[1];
			gp_shift_counter = 0;
		} else if (dir==0xDF9D) { // gamepads 2 at $df9d 
			if (ver>2)	printf("Shift gamepads\n");
			if(++gp_shift_counter == 8) {
				mem[0xDF9C]=~gamepads_latch[0];
				mem[0xDF9D]=~gamepads_latch[1];
			}
			else {
				mem[0xDF9C]=0;
				mem[0xDF9D]=0;
			}
		} else if (dir<=0xDF9F) {	// expansion port?
			mem[dir] = v;			// *** is this OK?
		} else if (dir<=0xDFAF) {	// interrupt control?
			mem[0xDFA0] = v;		// canonical address, only D0 matters
			scr_dirty = 1;			// note window might be updated when changing the state of the LED!
		} else if (dir<=0xDFBF) {	// beeper?
			mem[0xDFB0] = v;		// canonical address, only D0 matters
			sample_audio(sample_nr, (v&1)?255:0);	// generate audio sample
		} else {
			mem[dir] = v;		// otherwise is cartridge I/O *** anything else?
		}
	} else {					// any other address is ROM, thus no much sense writing there?
		if (ver)	printf("\n*** Writing to ROM at $%04X ***\n", pc);
		if (safe)	run=0;
	}
}

/* *** randomize memory contents *** */
void randomize(void) {
	int i;

	srand(time(NULL));
	for (i=0; i<32768; i++)		mem[i]=rand() & 255;	// RAM contents
	mem[0xDF80] = rand() & 255;							// random video mode at powerup
}



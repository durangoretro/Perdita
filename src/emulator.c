#include "emulator.h"

void run_emulation (int ready) {
	int cyc=0, it=0;		// instruction and interrupt cycle counter
	int ht=0;				// horizontal counter
	int line=0;				// line count for vertical retrace flag
	int stroke;				// received keystroke
	clock_t next;			// delay counter
	clock_t sleep_time;		// delay time
	clock_t min_sleep;		// for peek performance evaluation
	clock_t render_start;	// for SDL/GPU performance evaluation
	clock_t render_time;
	clock_t max_render;
	long frames = 0;		// total elapsed frames (for performance evaluation)
	long ticks = 0;			// total added microseconds of DELAY
	long us_render = 0;		// total microseconds of rendering
	long skip = 0;			// total skipped frames
	printf("[F1=STOP, F2=NMI, F3=IRQ, F4=RESET, F5=PAUSE, F6=DUMP, F7=STEP, F8=CONT, F9=LOAD]\n");
	if (graf)	init_vdu();
	if(!ready) {
                reset();				// ready to start!
        }

	next=clock()+19906;		// set delay counter, assumes CLOCKS_PER_SEC is 1000000!
	min_sleep = 19906;		// EEEEEEK
	max_render = 0;

	while (run) {
/* execute current opcode */
		cyc = exec();		// count elapsed clock cycles for this instruction
		cont += cyc;		// add last instruction cycle count
		sample_nr += cyc;	// advance audio sample cursor (at CPU rate!)
		it += cyc;			// advance interrupt counter
		ht += cyc;			// both get slightly out-of-sync during interrupts, but...
/* check horizontal counter for HSYNC flag and count lines for VSYNC */
		if (ht >= 98) {
			ht -= 98;
			line++;
			if (line >= 312) {
				line = 0;						// 312-line field limit
				sample_nr -= 30576;				// refresh audio sample
				sample_audio(sample_nr, old_v);
				frames++;
				render_start = clock();
				if (graf && scr_dirty)	vdu_draw_full();	// seems worth updating screen every VSYNC
				render_time = clock()-render_start;
				us_render += render_time;					// compute rendering time
				if (render_time > max_render)	max_render = render_time;
/* make a suitable delay for speed accuracy */
				if (!fast) {
					sleep_time=next-clock();
					ticks += sleep_time;		// for performance measurement
					if (sleep_time < min_sleep)		min_sleep = sleep_time;		// worse performance so far
					if(sleep_time>0) {
						usleep(sleep_time);		// should be accurate enough
					} else {
						skip++;
						if (ver) {
							printf("!");		// not enough CPU power!
						}
					}
					next=clock()+19906;			// set next frame time (more like 19932)
				}
			}
			mem[0xDF88] &= 0b10111111;			// replace bit 6 (VSYNC)...
			mem[0xDF88] |= (line&256)>>2;		// ...by bit 8 of line number (>=256)
		}
		mem[0xDF88] &= 0b01111111;		// replace bit 7 (HSYNC)...
		mem[0xDF88] |= (ht&64)<<1;		// ...by bit 6 of bye counter (>=64)
/* check hardware interrupt counter */
		if (it >= 6144)		// 250 Hz interrupt @ 1.536 MHz
		{
			it -= 6144;		// restore for next
/* get keypresses from SDL here, as this get executed every 4 ms */
			vdu_read_keyboard();	// ***is it possible to read keys without initing graphics?
/* may check for emulated keystrokes here */
			if (typing) {
				if (--type_delay == 0) {
					stroke = fgetc(typed);
					if (stroke == EOF) {
						typing = 0;
						printf(" OK!\n");
						fclose(typed);
						mem[0xDF9A] = 0;
					} else {
						type_delay = 25;		// just in case it scrolls
						if (stroke == 10) {
							stroke = 13;		// standard minimOS NEWLINE
							type_delay = 50;	// extra safe value for parsers
						}
						mem[0xDF9A] = stroke;
					}
				} else mem[0xDF9A] = 0;			// simulate PASK key up
			}
/* generate periodic interrupt */ 
			if (mem[0xDFA0] & 1) {
				irq();							// if hardware interrupts are enabled, send signal to CPU
			}
			fflush(stdout);						// update terminal screen
		}
/* generate asynchronous interrupts */ 
		if (irq_flag) {		// 'spurious' cartridge interrupt emulation!
			irq_flag = 0;
 			irq();
 		}
		if (nmi_flag) {
			nmi_flag = 0;
			nmi();			// NMI gets executed always
		}
/* check pause and step execution */
		if (run == 2)	run = 1;		// back to PAUSE after single-step execution
		if (run == 1) {
			if (graf && scr_dirty)		vdu_draw_full();	// get latest screen contents
			stat();						// display status at every pause
			while (run == 1) {			// wait until resume or step...
				usleep(20000);
				vdu_read_keyboard();	// ...but keep checking those keys for changes in 'run'
			}
		}
	}

	if (graf)	vdu_draw_full();		// last screen update
	printf(" *** CPU halted after %ld clock cycles ***\n", cont);
	stat();								// display final status

/* performance statistics */
	if (!frames)	frames = 1;			// whatever
	printf("\nSkipped frames: %ld (%f%%)\n", skip, skip*100.0/frames);
	printf("Average CPU time use: %f%%\n", 100-(ticks/200.0/frames));
	printf("Peak CPU time use: %f%%\n", 100-(min_sleep/200.0));
	printf("Average Rendering time: %ld µs (%f%%)\n", us_render/frames, us_render/frames/200.0);
	printf("Peak Rendering time: %ld µs (%f%%)\n", max_render, max_render/200.0);
	if(keep_open) {
		printf("\nPress ENTER key to exit\n");
		getchar();
	}
    
    if(dump_on_exit) {
        full_dump();
    }

	if (graf)	close_vdu();
}

/* execute a single opcode, returning cycle count */
int exec(void) {
	int per = 2;			// base cycle count
	int page = 0;			// page boundary flag, for speed penalties
	byte opcode, temp;
	word adr;

	opcode = peek(pc++);	// get opcode and point to next one (or operand)

	switch(opcode) {
/* *** ADC: Add Memory to Accumulator with Carry *** */
		case 0x69:
			adc(peek(pc++));
			if (ver > 3) printf("[ADC#]");
			per += dec;
			break;
		case 0x6D:
			adc(peek(am_a()));
			if (ver > 3) printf("[ADCa]");
			per = 4 + dec;
			break;
		case 0x65:
			adc(peek(peek(pc++)));
			if (ver > 3) printf("[ADCz]");
			per = 3 + dec;
			break;
		case 0x61:
			adc(peek(am_ix()));
			if (ver > 3) printf("[ADC(x)]");
			per = 6 + dec;
			break;
		case 0x71:
			adc(peek(am_iy(&page)));
			if (ver > 3) printf("[ADC(y)]");
			per = 5 + dec + page;
			break;
		case 0x75:
			adc(peek(am_zx()));
			if (ver > 3) printf("[ADCzx]");
			per = 4 + dec;
			break;
		case 0x7D:
			adc(peek(am_ax(&page)));
			if (ver > 3) printf("[ADCx]");
			per = 4 + dec + page;
			break;
		case 0x79:
			adc(peek(am_ay(&page)));
			if (ver > 3) printf("[ADCy]");
			per = 4 + dec + page;
			break;
		case 0x72:			// CMOS only
			adc(peek(am_iz()));
			if (ver > 3) printf("[ADC(z)]");
			per = 5 + dec;
			break;
/* *** AND: "And" Memory with Accumulator *** */
		case 0x29:
			a &= peek(pc++);
			bits_nz(a);
			if (ver > 3) printf("[AND#]");
			break;
		case 0x2D:
			a &= peek(am_a());
			bits_nz(a);
			if (ver > 3) printf("[ANDa]");
			per = 4;
			break;
		case 0x25:
			a &= peek(peek(pc++));
			bits_nz(a);
			if (ver > 3) printf("[ANDz]");
			per = 3;
			break;
		case 0x21:
			a &= peek(am_ix());
			bits_nz(a);
			if (ver > 3) printf("[AND(x)]");
			per = 6;
			break;
		case 0x31:
			a &= peek(am_iy(&page));
			bits_nz(a);
			if (ver > 3) printf("[AND(y)]");
			per = 5 + page;
			break;
		case 0x35:
			a &= peek(am_zx());
			bits_nz(a);
			if (ver > 3) printf("[ANDzx]");
			per = 4;
			break;
		case 0x3D:
			a &= peek(am_ax(&page));
			bits_nz(a);
			if (ver > 3) printf("[ANDx]");
			per = 4 + page;
			break;
		case 0x39:
			a &= peek(am_ay(&page));
			bits_nz(a);
			if (ver > 3) printf("[ANDy]");
			per = 4 + page;
			break;
		case 0x32:			// CMOS only
			a &= peek(am_iz());
			bits_nz(a);
			if (ver > 3) printf("[AND(z)]");
			per = 5;
			break;
/* *** ASL: Shift Left one Bit (Memory or Accumulator) *** */
		case 0x0E:
			adr = am_a();
			temp = peek(adr);
			asl(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[ASLa]");
			per = 6;
			break;
		case 0x06:
			temp = peek(peek(pc));
			asl(&temp);
			poke(peek(pc++), temp);
			if (ver > 3) printf("[ASLz]");
			per = 5;
			break;
		case 0x0A:
			asl(&a);
			if (ver > 3) printf("[ASL]");
			break;
		case 0x16:
			adr = am_zx();
			temp = peek(adr);
			asl(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[ASLzx]");
			per = 6;
			break;
		case 0x1E:
			adr = am_ax(&page);
			temp = peek(adr);
			asl(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[ASLx]");
			per = 6 + page;	// 7 on NMOS
			break;
/* *** Bxx: Branch on flag condition *** */
		case 0x90:
			if(!(p & 0b00000001)) {
				rel(&page);
				per = 3 + page;
				if (ver > 2) printf("[BCC]");
			} else pc++;	// must skip offset if not done EEEEEK
			break;
		case 0xB0:
			if(p & 0b00000001) {
				rel(&page);
				per = 3 + page;
				if (ver > 2) printf("[BCS]");
			} else pc++;	// must skip offset if not done EEEEEK
			break;
		case 0xF0:
			if(p & 0b00000010) {
				rel(&page);
				per = 3 + page;
				if (ver > 2) printf("[BEQ]");
			} else pc++;	// must skip offset if not done EEEEEK
			break;
/* *** BIT: Test Bits in Memory with Accumulator *** */
		case 0x2C:
			temp = peek(am_a());
			p &= 0b00111101;			// pre-clear N, V & Z
			p |= (temp & 0b11000000);	// copy bits 7 & 6 as N & Z
			p |= (a & temp)?0:2;		// set Z accordingly
			if (ver > 3) printf("[BITa]");
			per = 4;
			break;
		case 0x24:
			temp = peek(peek(pc++));
			p &= 0b00111101;			// pre-clear N, V & Z
			p |= (temp & 0b11000000);	// copy bits 7 & 6 as N & Z
			p |= (a & temp)?0:2;		// set Z accordingly
			if (ver > 3) printf("[BITz]");
			per = 3;
			break;
		case 0x89:			// CMOS only
			temp = peek(pc++);
			p &= 0b11111101;			// pre-clear Z only, is this OK?
			p |= (a & temp)?0:2;		// set Z accordingly
			if (ver > 3) printf("[BIT#]");
			break;
		case 0x3C:			// CMOS only
			temp = peek(am_ax(&page));
			p &= 0b00111101;			// pre-clear N, V & Z
			p |= (temp & 0b11000000);	// copy bits 7 & 6 as N & Z
			p |= (a & temp)?0:2;		// set Z accordingly
			if (ver > 3) printf("[BITx]");
			per = 4 + page;
			break;
		case 0x34:			// CMOS only
			temp = peek(am_zx());
			p &= 0b00111101;			// pre-clear N, V & Z
			p |= (temp & 0b11000000);	// copy bits 7 & 6 as N & Z
			p |= (a & temp)?0:2;		// set Z accordingly
			if (ver > 3) printf("[BITzx]");
			per = 4;
			break;
/* *** Bxx: Branch on flag condition *** */
		case 0x30:
			if(p & 0b10000000) {
				rel(&page);
				per = 3 + page;
			} else pc++;	// must skip offset if not done EEEEEK
			if (ver > 2) printf("[BMI]");
			break;
		case 0xD0:
			if(!(p & 0b00000010)) {
				rel(&page);
				per = 3 + page;
			} else pc++;	// must skip offset if not done EEEEEK
			if (ver > 2) printf("[BNE]");
			break;
		case 0x10:
			if(!(p & 0b10000000)) {
				rel(&page);
				per = 3 + page;
			} else pc++;	// must skip offset if not done EEEEEK
			if (ver > 2) printf("[BPL]");
			break;
		case 0x80:			// CMOS only
			rel(&page);
			per = 3 + page;
			if (ver > 2) printf("[BRA]");
			break;
/* *** BRK: force break *** */
		case 0x00:
			pc++;
			if (ver > 1) printf("[BRK]");
			if (safe)	run = 0;
			else {
				p |= 0b00010000;		// set B, just in case
				intack();
				p &= 0b11101111;		// clear B, just in case
				pc = peek(0xFFFE) | peek(0xFFFF)<<8;	// IRQ/BRK vector
				if (ver > 1) printf("\b PC=>%04X]", pc);
			}
			break;
/* *** Bxx: Branch on flag condition *** */
		case 0x50:
			if(!(p & 0b01000000)) {
				rel(&page);
				per = 3 + page;
			} else pc++;	// must skip offset if not done EEEEEK
			if (ver > 2) printf("[BVC]");
			break;
		case 0x70:
			if(p & 0b01000000) {
				rel(&page);
				per = 3 + page;
			} else pc++;	// must skip offset if not done EEEEEK
			if (ver > 2) printf("[BVS]");
			break;
/* *** CLx: Clear flags *** */
		case 0x18:
			p &= 0b11111110;
			if (ver > 3) printf("[CLC]");
			break;
		case 0xD8:
			p &= 0b11110111;
			dec = 0;
			if (ver > 3) printf("[CLD]");
			break;
		case 0x58:
			p &= 0b11111011;
			if (ver > 3) printf("[CLI]");
			break;
		case 0xB8:
			p &= 0b10111111;
			if (ver > 3) printf("[CLV]");
			break;
/* *** CMP: Compare Memory And Accumulator *** */
		case 0xC9:
			cmp(a, peek(pc++));
			if (ver > 3) printf("[CMP#]");
			break;
		case 0xCD:
			cmp(a, peek(am_a()));
			if (ver > 3) printf("[CMPa]");
			per = 4;
			break;
		case 0xC5:
			cmp(a, peek(peek(pc++)));
			if (ver > 3) printf("[CMPz]");
			per = 3;
			break;
		case 0xC1:
			cmp(a, peek(am_ix()));
			if (ver > 3) printf("[CMP(x)]");
			per = 6;
			break;
		case 0xD1:
			cmp(a, peek(am_iy(&page)));
			if (ver > 3) printf("[CMP(y)]");
			per = 5 + page;
			break;
		case 0xD5:
			cmp(a, peek(am_zx()));
			if (ver > 3) printf("[CMPzx]");
			per = 4;
			break;
		case 0xDD:
			cmp(a, peek(am_ax(&page)));
			if (ver > 3) printf("[CMPx]");
			per = 4 + page;
			break;
		case 0xD9:
			cmp(a, peek(am_ay(&page)));
			if (ver > 3) printf("[CMPy]");
			per = 4 + page;
			break;
		case 0xD2:			// CMOS only
			cmp(a, peek(am_iz()));
			if (ver > 3) printf("[CMP(z)]");
			per = 5;
			break;
/* *** CPX: Compare Memory And Index X *** */
		case 0xE0:
			cmp(x, peek(pc++));
			if (ver > 3) printf("[CPX#]");
			break;
		case 0xEC:
			cmp(x, peek(am_a()));
			if (ver > 3) printf("[CPXa]");
			per = 4;
			break;
		case 0xE4:
			cmp(x, peek(peek(pc++)));
			if (ver > 3) printf("[CPXz]");
			per = 3;
			break;
/* *** CPY: Compare Memory And Index Y *** */
		case 0xC0:
			cmp(y, peek(pc++));
			if (ver > 3) printf("[CPY#]");
			break;
		case 0xCC:
			cmp(y, peek(am_a()));
			if (ver > 3) printf("[CPYa]");
			per = 4;
			break;
		case 0xC4:
			cmp(y, peek(peek(pc++)));
			if (ver > 3) printf("[CPYz]");
			per = 3;
			break;
/* *** DEC: Decrement Memory (or Accumulator) by One *** */
		case 0xCE:
			adr = am_a();	// EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEK
			temp = peek(adr);
			temp--;
			poke(adr, temp);
			bits_nz(temp);
			if (ver > 3) printf("[DECa]");
			per = 6;
			break;
		case 0xC6:
			temp = peek(peek(pc));
			temp--;
			poke(peek(pc++), temp);
			bits_nz(temp);
			if (ver > 3) printf("[DECz]");
			per = 5;
			break;
		case 0xD6:
			adr = am_zx();	// EEEEEEEEEEK
			temp = peek(adr);
			temp--;
			poke(adr, temp);
			bits_nz(temp);
			if (ver > 3) printf("[DECzx]");
			per = 6;
			break;
		case 0xDE:
			adr = am_ax(&page);	// EEEEEEEEK
			temp = peek(adr);
			temp--;
			poke(adr, temp);
			bits_nz(temp);
			if (ver > 3) printf("[DECx]");
			per = 7;		// 6+page for WDC?
			break;
		case 0x3A:			// CMOS only (OK)
			a--;
			bits_nz(a);
			if (ver > 3) printf("[DEC]");
			break;
/* *** DEX: Decrement Index X by One *** */
		case 0xCA:
			x--;
			bits_nz(x);
			if (ver > 3) printf("[DEX]");
			break;
/* *** DEY: Decrement Index Y by One *** */
		case 0x88:
			y--;
			bits_nz(y);
			if (ver > 3) printf("[DEY]");
			break;
/* *** EOR: "Exclusive Or" Memory with Accumulator *** */
		case 0x49:
			a ^= peek(pc++);
			bits_nz(a);
			if (ver > 3) printf("[EOR#]");
			break;
		case 0x4D:
			a ^= peek(am_a());
			bits_nz(a);
			if (ver > 3) printf("[EORa]");
			per = 4;
			break;
		case 0x45:
			a ^= peek(peek(pc++));
			bits_nz(a);
			if (ver > 3) printf("[EORz]");
			per = 3;
			break;
		case 0x41:
			a ^= peek(am_ix());
			bits_nz(a);
			if (ver > 3) printf("[EOR(x)]");
			per = 6;
			break;
		case 0x51:
			a ^= peek(am_iy(&page));
			bits_nz(a);
			if (ver > 3) printf("[EOR(y)]");
			per = 5 + page;
			break;
		case 0x55:
			a ^= peek(am_zx());
			bits_nz(a);
			if (ver > 3) printf("[EORzx]");
			per = 4;
			break;
		case 0x5D:
			a ^= peek(am_ax(&page));
			bits_nz(a);
			if (ver > 3) printf("[EORx]");
			per = 4 + page;
			break;
		case 0x59:
			a ^= peek(am_ay(&page));
			bits_nz(a);
			if (ver > 3) printf("[EORy]");
			per = 4 + page;
			break;
		case 0x52:			// CMOS only
			a ^= peek(am_iz());
			bits_nz(a);
			if (ver > 3) printf("[EOR(z)]");
			per = 5;
			break;
/* *** INC: Increment Memory (or Accumulator) by One *** */
		case 0xEE:
			adr = am_a();	// EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEK
			temp = peek(adr);
			temp++;
			poke(adr, temp);
			bits_nz(temp);
			if (ver > 3) printf("[INCa]");
			per = 6;
			break;
		case 0xE6:
			temp = peek(peek(pc));
			temp++;
			poke(peek(pc++), temp);
			bits_nz(temp);
			if (ver > 3) printf("[INCz]");
			per = 5;
			break;
		case 0xF6:
			adr = am_zx();	// EEEEEEEEEEK
			temp = peek(adr);
			temp++;
			poke(adr, temp);
			bits_nz(temp);
			if (ver > 3) printf("[INCzx]");
			per = 6;
			break;
		case 0xFE:
			adr = am_ax(&page);	// EEEEEEEEEEK
			temp = peek(adr);
			temp++;
			poke(adr, temp);
			bits_nz(temp);
			if (ver > 3) printf("[INCx]");
			per = 7;		// 6+page for WDC?
			break;
		case 0x1A:			// CMOS only
			a++;
			bits_nz(a);
			if (ver > 3) printf("[INC]");
			break;
/* *** INX: Increment Index X by One *** */
		case 0xE8:
			x++;
			bits_nz(x);
			if (ver > 3) printf("[INX]");
			break;
/* *** INY: Increment Index Y by One *** */
		case 0xC8:
			y++;
			bits_nz(y);
			if (ver > 3) printf("[INY]");
			break;
/* *** JMP: Jump to New Location *** */
		case 0x4C:
			pc = am_a();
			if (ver > 2)	printf("[JMP]");
			per = 3;
			break;
		case 0x6C:
			pc = am_ai();
			if (ver > 2)	printf("[JMP()]");
			per = 6;		// 5 for NMOS!
			break;
		case 0x7C:			// CMOS only
			pc = am_aix();
			if (ver > 2)	printf("[JMP(x)]");
			per = 6;
			break;
/* *** JSR: Jump to New Location Saving Return Address *** */
		case 0x20:
			push((pc+1)>>8);		// stack one byte before return address, right at MSB
			push((pc+1)&255);
			pc = am_a();			// get operand
			if (ver > 2)	printf("[JSR]");
			per = 6;
			break;
/* *** LDA: Load Accumulator with Memory *** */
		case 0xA9:
			a = peek(pc++);
			bits_nz(a);
			if (ver > 3) printf("[LDA#]");
			break;
		case 0xAD:
			a = peek(am_a());
			bits_nz(a);
			if (ver > 3) printf("[LDAa]");
			per = 4;
			break;
		case 0xA5:
			a = peek(peek(pc++));
			bits_nz(a);
			if (ver > 3) printf("[LDAz]");
			per = 3;
			break;
		case 0xA1:
			a = peek(am_ix());
			bits_nz(a);
			if (ver > 3) printf("[LDA(x)]");
			per = 6;
			break;
		case 0xB1:
			a = peek(am_iy(&page));
			bits_nz(a);
			if (ver > 3) printf("[LDA(y)]");
			per = 5 + page;
			break;
		case 0xB5:
			a = peek(am_zx());
			bits_nz(a);
			if (ver > 3) printf("[LDAzx]");
			per = 4;
			break;
		case 0xBD:
			a = peek(am_ax(&page));
			bits_nz(a);
			if (ver > 3) printf("[LDAx]");
			per = 4 + page;
			break;
		case 0xB9:
			a = peek(am_ay(&page));
			bits_nz(a);
			if (ver > 3) printf("[LDAy]");
			per = 4 + page;
			break;
		case 0xB2:			// CMOS only
			a = peek(am_iz());
			bits_nz(a);
			if (ver > 3) printf("[LDA(z)]");
			per = 5;
			break;
/* *** LDX: Load Index X with Memory *** */
		case 0xA2:
			x = peek(pc++);
			bits_nz(x);
			if (ver > 3) printf("[LDX#]");
			break;
		case 0xAE:
			x = peek(am_a());
			bits_nz(x);
			if (ver > 3) printf("[LDXa]");
			per = 4;
			break;
		case 0xA6:
			x = peek(peek(pc++));
			bits_nz(x);
			if (ver > 3) printf("[LDXz]");
			per = 3;
			break;
		case 0xB6:
			x = peek(am_zy());
			bits_nz(x);
			if (ver > 3) printf("[LDXzy]");
			per = 4;
			break;
		case 0xBE:
			x = peek(am_ay(&page));
			bits_nz(x);
			if (ver > 3) printf("[LDXy]");
			per = 4 + page;
			break;
/* *** LDY: Load Index Y with Memory *** */
		case 0xA0:
			y = peek(pc++);
			bits_nz(y);
			if (ver > 3) printf("[LDY#]");
			break;
		case 0xAC:
			y = peek(am_a());
			bits_nz(y);
			if (ver > 3) printf("[LDYa]");
			per = 4;
			break;
		case 0xA4:
			y = peek(peek(pc++));
			bits_nz(y);
			if (ver > 3) printf("[LDYz]");
			per = 3;
			break;
		case 0xB4:
			y = peek(am_zx());
			bits_nz(y);
			if (ver > 3) printf("[LDYzx]");
			per = 4;
			break;
		case 0xBC:
			y = peek(am_ax(&page));
			bits_nz(y);
			if (ver > 3) printf("[LDYx]");
			per = 4 + page;
			break;
/* *** LSR: Shift One Bit Right (Memory or Accumulator) *** */
		case 0x4E:
			adr=am_a();
			temp = peek(adr);
			lsr(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[LSRa]");
			per = 6;
			break;
		case 0x46:
			temp = peek(peek(pc));
			lsr(&temp);
			poke(peek(pc++), temp);
			if (ver > 3) printf("[LSRz]");
			per = 5;
			break;
		case 0x4A:
			lsr(&a);
			if (ver > 3) printf("[LSR]");
			break;
		case 0x56:
			adr = am_zx();
			temp = peek(adr);
			lsr(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[LSRzx]");
			per = 6;
			break;
		case 0x5E:
			adr = am_ax(&page);
			temp = peek(adr);
			lsr(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[LSRx]");
			per = 6 + page;	// 7 for NMOS
			break;
/* *** NOP: No Operation *** */
		case 0xEA:
			if (ver > 3) printf("[NOP]");
			break;
/* *** ORA: "Or" Memory with Accumulator *** */
		case 0x09:
			a |= peek(pc++);
			bits_nz(a);
			if (ver > 3) printf("[ORA#]");
			break;
		case 0x0D:
			a |= peek(am_a());
			bits_nz(a);
			if (ver > 3) printf("[ORAa]");
			per = 4;
			break;
		case 0x05:
			a |= peek(peek(pc++));
			bits_nz(a);
			if (ver > 3) printf("[ORAz]");
			per = 3;
			break;
		case 0x01:
			a |= peek(am_ix());
			bits_nz(a);
			if (ver > 3) printf("[ORA(x)]");
			per = 6;
			break;
		case 0x11:
			a |= peek(am_iy(&page));
			bits_nz(a);
			if (ver > 3) printf("[ORA(y)]");
			per = 5 + page;
			break;
		case 0x15:
			a |= peek(am_zx());
			bits_nz(a);
			if (ver > 3) printf("[ORAzx]");
			per = 4;
			break;
		case 0x1D:
			a |= peek(am_ax(&page));
			bits_nz(a);
			if (ver > 3) printf("[ORAx]");
			per = 4 + page;
			break;
		case 0x19:
			a |= peek(am_ay(&page));
			bits_nz(a);
			if (ver > 3) printf("[ORAy]");
			per = 4 + page;
			break;
		case 0x12:			// CMOS only
			a |= peek(am_iz());
			bits_nz(a);
			if (ver > 3) printf("[ORA(z)]");
			per = 5;
			break;
/* *** PHA: Push Accumulator on Stack *** */
		case 0x48:
			push(a);
			if (ver > 3) printf("[PHA]");
			per = 3;
			break;
/* *** PHP: Push Processor Status on Stack *** */
		case 0x08:
			push(p);
			if (ver > 3) printf("[PHP]");
			per = 3;
			break;
/* *** PHX: Push Index X on Stack *** */
		case 0xDA:			// CMOS only
			push(x);
			if (ver > 3) printf("[PHX]");
			per = 3;
			break;
/* *** PHY: Push Index Y on Stack *** */
		case 0x5A:			// CMOS only
			push(y);
			if (ver > 3) printf("[PHY]");
			per = 3;
			break;
/* *** PLA: Pull Accumulator from Stack *** */
		case 0x68:
			a = pop();
			if (ver > 3) printf("[PLA]");
			bits_nz(a);
			per = 4;
			break;
/* *** PLP: Pull Processor Status from Stack *** */
		case 0x28:
			p = pop();
			if (p & 0b00001000)	dec = 1;	// check for decimal flag
			else				dec = 0;
			if (ver > 3) printf("[PLP]");
			per = 4;
			break;
/* *** PLX: Pull Index X from Stack *** */
		case 0xFA:			// CMOS only
			x = pop();
			if (ver > 3) printf("[PLX]");
			bits_nz(x);		// EEEEEEEEEEEEEEEEEEEEK
			per = 4;
			break;
/* *** PLX: Pull Index X from Stack *** */
		case 0x7A:			// CMOS only
			y = pop();
			if (ver > 3) printf("[PLY]");
			bits_nz(y);		// EEEEEEEEEEEEEEEEEEEEK
			per = 4;
			break;
/* *** ROL: Rotate One Bit Left (Memory or Accumulator) *** */
		case 0x2E:
			adr = am_a();
			temp = peek(adr);
			rol(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[ROLa]");
			per = 6;
			break;
		case 0x26:
			temp = peek(peek(pc));
			rol(&temp);
			poke(peek(pc++), temp);
			if (ver > 3) printf("[ROLz]");
			per = 5;
			break;
		case 0x36:
			adr = am_zx();
			temp = peek(adr);
			rol(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[ROLzx]");
			per = 6;
			break;
		case 0x3E:
			adr = am_ax(&page);
			temp = peek(adr);
			rol(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[ROLx]");
			per = 6 + page;	// 7 for NMOS
			break;
		case 0x2A:
			rol(&a);
			if (ver > 3) printf("[ROL]");
			break;
/* *** ROR: Rotate One Bit Right (Memory or Accumulator) *** */
		case 0x6E:
			adr = am_a();
			temp = peek(adr);
			ror(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[RORa]");
			per = 6;
			break;
		case 0x66:
			temp = peek(peek(pc));
			ror(&temp);
			poke(peek(pc++), temp);
			if (ver > 3) printf("[RORz]");
			per = 5;
			break;
		case 0x6A:
			ror(&a);
			if (ver > 3) printf("[ROR]");
			break;
		case 0x76:
			adr = am_zx();
			temp = peek(adr);
			ror(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[RORzx]");
			per = 6;
			break;
		case 0x7E:
			adr = am_ax(&page);
			temp = peek(adr);
			ror(&temp);
			poke(adr, temp);
			if (ver > 3) printf("[RORx]");
			per = 6 + page;	// 7 for NMOS
			break;
/* *** RTI: Return from Interrupt *** */
		case 0x40:
			p = pop();					// retrieve status
			p |= 0b00010000;			// forget possible B flag
			pc = pop();					// extract LSB...
			pc |= (pop() << 8);			// ...and MSB, address is correct
			if (ver > 2)	printf("[RTI]");
			per = 6;
			break;
/* *** RTS: Return from Subroutine *** */
		case 0x60:
			pc = pop();					// extract LSB...
			pc |= (pop() << 8);			// ...and MSB, but is one byte off
			pc++;						// return instruction address
			if (ver > 2)	printf("[RTS]");
			per = 6;
			break;
/* *** SBC: Subtract Memory from Accumulator with Borrow *** */
		case 0xE9:
			sbc(peek(pc++));
			if (ver > 3) printf("[SBC#]");
			per += dec;
			break;
		case 0xED:
			sbc(peek(am_a()));
			if (ver > 3) printf("[SBCa]");
			per = 4 + dec;
			break;
		case 0xE5:
			sbc(peek(peek(pc++)));
			if (ver > 3) printf("[SBCz]");
			per = 3 + dec;
			break;
		case 0xE1:
			sbc(peek(am_ix()));
			if (ver > 3) printf("[SBC(x)]");
			per = 6 + dec;
			break;
		case 0xF1:
			sbc(peek(am_iy(&page)));
			if (ver > 3) printf("[SBC(y)]");
			per = 5 + dec + page;
			break;
		case 0xF5:
			sbc(peek(am_zx()));
			if (ver > 3) printf("[SBCzx]");
			per = 4 + dec;
			break;
		case 0xFD:
			sbc(peek(am_ax(&page)));
			if (ver > 3) printf("[SBCx]");
			per = 4 + dec + page;
			break;
		case 0xF9:
			sbc(peek(am_ay(&page)));
			if (ver > 3) printf("[SBCy]");
			per = 4 + dec + page;
			break;
		case 0xF2:			// CMOS only
			sbc(peek(am_iz()));
			if (ver > 3) printf("[SBC(z)]");
			per = 5 + dec;
			break;
// *** SEx: Set Flags *** */
		case 0x38:
			p |= 0b00000001;
			if (ver > 3) printf("[SEC]");
			break;
		case 0xF8:
			p |= 0b00001000;
			dec = 1;
			if (ver > 3) printf("[SED]");
			break;
		case 0x78:
			p |= 0b00000100;
			if (ver > 3) printf("[SEI]");
			break;
/* *** STA: Store Accumulator in Memory *** */
		case 0x8D:
			poke(am_a(), a);
			if (ver > 3) printf("[STAa]");
			per = 4;
			break;
		case 0x85:
			poke(peek(pc++), a);
			if (ver > 3) printf("[STAz]");
			per = 3;
			break;
		case 0x81:
			poke(am_ix(), a);
			if (ver > 3) printf("[STA(x)]");
			per = 6;
			break;
		case 0x91:
			poke(am_iy(&page), a);
			if (ver > 3) printf("[STA(y)]");
			per = 6;		// ...and not 5, as expected
			break;
		case 0x95:
			poke(am_zx(), a);
			if (ver > 3) printf("[STAzx]");
			per = 4;
			break;
		case 0x9D:
			poke(am_ax(&page), a);
			if (ver > 3) printf("[STAx]");
			per = 5;		// ...and not 4, as expected
			break;
		case 0x99:
			poke(am_ay(&page), a);
			if (ver > 3) printf("[STAy]");
			per = 5;		// ...and not 4, as expected
			break;
		case 0x92:			// CMOS only
			poke(am_iz(), a);
			if (ver > 3) printf("[STA(z)]");
			per = 5;
			break;
/* *** STX: Store Index X in Memory *** */
		case 0x8E:
			poke(am_a(), x);
			if (ver > 3) printf("[STXa]");
			per = 4;
			break;
		case 0x86:
			poke(peek(pc++), x);
			if (ver > 3) printf("[STXz]");
			per = 3;
			break;
		case 0x96:
			poke(am_zy(), x);
			if (ver > 3) printf("[STXzy]");
			per = 4;
			break;
/* *** STY: Store Index Y in Memory *** */
		case 0x8C:
			poke(am_a(), y);
			if (ver > 3) printf("[STYa]");
			per = 4;
			break;
		case 0x84:
			poke(peek(pc++), y);
			if (ver > 3) printf("[STYz]");
			per = 3;
			break;
		case 0x94:
			poke(am_zx(), y);
			if (ver > 3) printf("[STYzx]");
			per = 4;
			break;
// *** STZ: Store Zero in Memory, CMOS only ***
		case 0x9C:
			poke(am_a(), 0);
			if (ver > 3) printf("[STZa]");
			per = 4;
			break;
		case 0x64:
			poke(peek(pc++), 0);
			if (ver > 3) printf("[STZz]");
			per = 3;
			break;
		case 0x74:
			poke(am_zx(), 0);
			if (ver > 3) printf("[STZzx]");
			per = 4;
			break;
		case 0x9E:
			poke(am_ax(&page), 0);
			if (ver > 3) printf("[STZx]");
			per = 5;		// ...and not 4, as expected
			break;
/* *** TAX: Transfer Accumulator to Index X *** */
		case 0xAA:
			x = a;
			bits_nz(x);
			if (ver > 3) printf("[TAX]");
			break;
/* *** TAY: Transfer Accumulator to Index Y *** */
		case 0xA8:
			y = a;
			bits_nz(y);
			if (ver > 3) printf("[TAY]");
			break;
/* *** TRB: Test and Reset Bits, CMOS only *** */
		case 0x1C:
			adr = am_a();
			temp = peek(adr);
			if (temp & a)		p &= 0b11111101;	// set Z accordingly
			else 				p |= 0b00000010;
			poke(adr, temp & ~a);
			if (ver > 3) printf("[TRBa]");
			per = 6;
			break;
		case 0x14:
			adr = peek(pc++);
			temp = peek(adr);
			if (temp & a)		p &= 0b11111101;	// set Z accordingly
			else 				p |= 0b00000010;
			poke(adr, temp & ~a);
			if (ver > 3) printf("[TRBz]");
			per = 5;
			break;
/* *** TSB: Test and Set Bits, CMOS only *** */
		case 0x0C:
			adr = am_a();
			temp = peek(adr);
			if (temp & a)		p &= 0b11111101;	// set Z accordingly
			else 				p |= 0b00000010;
			poke(adr, temp | a);
			if (ver > 3) printf("[TSBa]");
			per = 6;
			break;
		case 0x04:
			adr = peek(pc++);
			temp = peek(adr);
			if (temp & a)		p &= 0b11111101;	// set Z accordingly
			else 				p |= 0b00000010;
			poke(adr, temp | a);
			if (ver > 3) printf("[TSBz]");
			per = 5;
			break;
/* *** TSX: Transfer Stack Pointer to Index X *** */
		case 0xBA:
			x = s;
			bits_nz(x);
			if (ver > 3) printf("[TSX]");
			break;
/* *** TXA: Transfer Index X to Accumulator *** */
		case 0x8A:
			a = x;
			bits_nz(a);
			if (ver > 3) printf("[TXA]");
			break;
/* *** TXS: Transfer Index X to Stack Pointer *** */
		case 0x9A:
			s = x;
			bits_nz(s);
			if (ver > 3) printf("[TXS]");
			break;
/* *** TYA: Transfer Index Y to Accumulator *** */
		case 0x98:
			a = y;
			bits_nz(a);
			if (ver > 3) printf("[TYA]");
			break;
/* *** *** special control 'opcodes' *** *** */
/* *** Emulator Breakpoint  (WAI on WDC) *** */
		case 0xCB:
//			if (ver)	printf(" Status @ $%x04:", pc-1);	// must allow warnings to display status request
//			stat();
			run = 1;		// pause execution
			break;
/* *** Graceful Halt (STP on WDC) *** */
		case 0xDB:
			printf(" ...HALT!");
			run = per = 0;	// definitively stop execution
			break;
/* *** *** unused (illegal?) opcodes *** *** */
/* *** remaining opcodes (illegal on NMOS) executed as pseudoNOPs, according to 65C02 byte and cycle usage *** */
		case 0x03:
		case 0x13:
		case 0x23:
		case 0x33:
		case 0x43:
		case 0x53:
		case 0x63:
		case 0x73:
		case 0x83:
		case 0x93:
		case 0xA3:
		case 0xB3:
		case 0xC3:
		case 0xD3:
		case 0xE3:
		case 0xF3:
		case 0x0B:
		case 0x1B:
		case 0x2B:
		case 0x3B:
		case 0x4B:
		case 0x5B:
		case 0x6B:
		case 0x7B:
		case 0x8B:
		case 0x9B:
		case 0xAB:
		case 0xBB:
		case 0xEB:
		case 0xFB:	// minus WDC opcodes, used for emulator control
		case 0x07:
		case 0x17:
		case 0x27:
		case 0x37:
		case 0x47:
		case 0x57:
		case 0x67:
		case 0x77:
		case 0x87:
		case 0x97:
		case 0xA7:
		case 0xB7:
		case 0xC7:
		case 0xD7:
		case 0xE7:
		case 0xF7:	// Rockwell RMB/SMB opcodes
		case 0x0F:
		case 0x1F:
		case 0x2F:
		case 0x3F:
		case 0x4F:
		case 0x5F:
		case 0x6F:
		case 0x7F:
		case 0x8F:
		case 0x9F:
		case 0xAF:
		case 0xBF:
		case 0xCF:
		case 0xDF:
		case 0xEF:
		case 0xFF:	// Rockwell BBR/BBS opcodes
			per = 1;		// ultra-fast 1 byte NOPs!
			if (ver)	printf("[NOP!]");
			if (safe)	illegal(1, opcode);
			break;
		case 0x02:
		case 0x22:
		case 0x42:
		case 0x62:
		case 0x82:
		case 0xC2:
		case 0xE2:
			pc++;			// 2-byte, 2-cycle NOPs
			if (ver)	printf("[NOP#]");
			if (safe)	illegal(2, opcode);
			break;
		case 0x44:
			pc++;
			per++;			// only case of 2-byte, 3-cycle NOP
			if (ver)	printf("[NOPz]");
			if (safe)	illegal(2, opcode);
			break;
		case 0x54:
		case 0xD4:
		case 0xF4:
			pc++;
			per = 4;		// only cases of 2-byte, 4-cycle NOP
			if (ver)	printf("[NOPzx]");
			if (safe)	illegal(2, opcode);
			break;
		case 0xDC:
		case 0xFC:
			pc += 2;
			per = 4;		// only cases of 3-byte, 4-cycle NOP
			if (ver)	printf("[NOPa]");
			if (safe)	illegal(3, opcode);
			break;
		case 0x5C:
			pc += 2;
			per = 8;		// extremely slow 8-cycle NOP
			if (ver)	printf("[NOP?]");
			if (safe)	illegal(3, opcode);
			break;			// not needed as it's the last one, but just in case
	}

	return per;
}

/* *** *** *** halt CPU on illegal opcodes *** *** *** */
void illegal(byte s, byte op) {
	printf("\n*** ($%04X) Illegal opcode $%02X ***\n", pc-s, op);
	run = 0;
}



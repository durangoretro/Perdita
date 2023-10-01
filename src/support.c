#include "support.h"

/* **************************** */
/* support functions definition */
/* **************************** */

/* displays usage */
void usage(char name[]) {
	printf("usage: %s [-a rom_address] [-v] rom_file\n", name);	// in case user renames the executable
	printf("-a: load ROM at supplied address, example 0x8000\n");
	printf("-f fast mode\n");
	printf("-s safe mode (will stop on warnings and BRK)\n");
	printf("-p start in STEP mode\n");
	printf("-l enable error LED(s)\n");
	printf("-k keep GUI open after program end\n");
	printf("-h headless -- no graphics!\n");
	printf("-v verbose (warnings/interrupts/jumps/events/all)\n");
	printf("-r do NOT randomize memory at startup\n");
	printf("-g emulate controllers\n");
	printf("-m do NOT emulate Minstrel-type keyboard\n");
    printf("-d Generate dump after emulation\n");
}

/* display CPU status */
void stat(void)	{ 
	int i;
	byte psr = p;			// local copy of status
	const char flag[8]="NV.bDIZC";	// flag names

	printf("<PC=$%04X, A=$%02X, X=$%02X, Y=$%02X, S=$%02X, CLK=%ld>\n<PSR: ", pc-1, a, x, y, s, cont);
	for (i=0; i<8; i++) {
		if (psr&128)	printf("%c", flag[i]);
		else			printf("·");
		psr<<=1;			// next flag
	}
	printf("> [%04X]=%02X\n",pc-1, mem[pc-1]);
}

/* Display STACK status */
void stack_stat(void) {
	// Copy stack pointer
	byte i=s;

	// If stack is not empty
	if(i!=0xff) {
		// Iterate stack
		do {
			i++;
			printf("|%02X| \n", mem[0x0100+i]);
		} while (i<0xff);
	}
	printf("|==|\n\n");
}

/* display 16 bytes of memory */
void dump(word dir) {
	int i;

	printf("$%04X: ", dir);
	for (i=0; i<16; i++)		printf("%02X ", mem[dir+i]);
	printf ("[");
	for (i=0; i<16; i++)
		if ((mem[dir+i]>31)&&(mem[dir+i]<127))	printf("%c", mem[dir+i]);
		else 									printf("·");
	printf ("]\n");
}

void full_dump() {
	FILE *f;

	f = fopen("dump.bin", "wb");
	if (f != NULL) {
		// Write memory
		fwrite(mem, sizeof(byte), 65536, f); 
		// Write registers
		fputc(a, f);
		fputc(x, f);
		fputc(y, f);
		fputc(s, f);
		fputc(p, f);
		// Write PC
		fwrite(&pc, sizeof(word), 1, f);
		// Close file
		fclose(f);
		printf("dump.bin generated\n");
	}
	else {
		printf("*** Could not write dump ***\n");
		run = 0;
	}
}

void load_dump(const char name[]) {
	FILE *f;

	f = fopen(name, "rb");
	if (f != NULL) {
		// Read memory
		fread(mem, sizeof(byte), 65536, f); 
		// Read registers
		a = fgetc(f);
		x = fgetc(f);
		y = fgetc(f);
		s = fgetc(f);
		p = fgetc(f);
		// Read PC
		fread(&pc, sizeof(word), 1, f);
		// Close file
		fclose(f);
		printf("%s loaded\n",name);
	}
	else {
		printf("*** No available dump ***\n");
		run = 0;
	}
}

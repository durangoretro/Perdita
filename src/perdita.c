/*
 * Copyright 2007-2023 Carlos J. Santisteban, Emilio López Berenguer
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * Perdita 65C02 Durango-X emulator!
 * last modified 20230607-2032
 * */

// Compile using MakeFile

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
// SDL Install: apt-get install libsdl2-dev. Build with -lSDL2 flag
#include <SDL2/SDL.h>
// arguments parser
#include <unistd.h>

#include "version.h"
#include "globals.h"

/* ******************* */
/* function prototypes */
/* ******************* */

#include "vdu.h"

/* emulator control */
#include "emulator.h"

/* ROM management */
#include "roms.h"

/* support functions */
#include "support.h"

/* VPS config and emulation */
#include "vps.h"

/* PSV file support */
#include "psv.h"

/* memory management */
#include "memory_management.h"

/* interrupt support */
#include "interrupt.h"

/* opcode support */
#include "opcodes.h"

/* addressing modes */
#include "address_modes.h"

/* Process keyboard / mouse events */
#include "keyboard_mouse.h"

/* ************************************************* */
/* ******************* main loop ******************* */
/* ************************************************* */
int main(int argc, char *argv[])
{
	int index;
	int arg_index;
	int c, do_rand=1;
	char *filename;
	char *rom_addr=NULL;
	char *exe_addr=NULL;
	int rom_addr_int;
	int exe_addr_int;

	redefine();		// finish keyboard layout definitions
	//Show Title And Version
	printf("Perdita Emulator Ver: %s\n",perdita_version);
	if(argc==1) {
		usage(argv[0]);
		return 1;
	}

	opterr = 0;

	while ((c = getopt (argc, argv, "a:fvlksphrgmdx")) != -1)
	switch (c) {
		case 'a':
			rom_addr = optarg;
			break;
		case 'f':
			fast = 1;
			break;
		case 'k':
			keep_open = 1;
			break;
		case 'v':
			ver++;			// not that I like this, but...
			break;
		case 'l':
			err_led = 1;
			break;
		case 's':
			safe = 1;
			break;
		case 'p':
			run = 2;
			break;
		case 'h':
			graf = 0;
			break;
		case 'r':
			do_rand = 0;
			break;
		case 'g':
			emulate_gamepads = 1;
			break;
		case 'm':
			emulate_minstrel = 0;
			break;
        case 'd':
            dump_on_exit = 1;
            break;
		case 'x':
			exe_addr = optarg;
			break;
		case '?':
			fprintf (stderr, "Unknown option\n");
			usage(argv[0]);
			return 1;
		default:
			abort ();
	}

	for (arg_index = 0, index = optind; index < argc; index++, arg_index++) {
		switch(arg_index) {
			case 0: filename = argv[index]; break;
		}
	}

	if(arg_index == 0) {
		printf("Filename is mandatory\n");
		return 1;
	}
	
	if(rom_addr != NULL && (strlen(rom_addr) != 6 || rom_addr[0]!='0' || rom_addr[1]!='x')) {
		printf("ROM address format: 0x0000\n");
		return 1;
	}

	if(exe_addr != NULL && (strlen(exe_addr) != 6 || exe_addr[0]!='0' || exe_addr[1]!='x')) {
		printf("Execution address format: 0x0000\n");
		return 1;
	}

	if (do_rand)		randomize();		// randomize memory contents

	if(rom_addr == NULL) {
		run_emulation(ROMload(filename)==1);
	}
	else {
		rom_addr_int = (int)strtol(rom_addr, NULL, 0);
		if (exe_addr == NULL) {
			exe_addr_int = rom_addr_int;
		} else {
			rom_addr_int = (int)strtol(exe_addr, NULL, 0);
		}
		load(filename, rom_addr_int);
/* set some standard vectors and base ROM contents */
/* TO DO: add BIOS/BDOS support for Pocket executables */
		mem[0xFFF4] = 0x6C;					// JMP ($0200) as recommended for IRQ
		mem[0xFFF5] = 0x00;
		mem[0xFFF6] = 0x02;
		mem[0xFFF7] = 0x6C;					// JMP ($0202) as recommended for NMI
		mem[0xFFF8] = 0x02;
		mem[0xFFF9] = 0x02;
		mem[0xFFFA] = 0xF7;					// standard NMI vector points to recommended indirect jump
		mem[0xFFFB] = 0xFF;
		mem[0xFFFC] = exe_addr_int & 0xFF;	// set RESET vector pointing to loaded code (or specified execution address)
	 	mem[0xFFFD] = exe_addr_int >> 8;
		mem[0xFFFE] = 0xF4;					// standard IRQ vector points to recommended indirect jump
		mem[0xFFFF] = 0xFF;
                run_emulation(0);
	}

	return 0;
}


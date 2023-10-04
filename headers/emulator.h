#include <time.h>
#include <unistd.h>

#include "globals.h"
#include "vdu.h"
#include "interrupt.h"
#include "opcodes.h"
#include "support.h"
#include "memory_management.h"
#include "address_modes.h"

#ifndef EMULATOR_H
#define EMULATOR_H

/* emulator control */
    void run_emulation(int ready);	// Run emulator
    int  exec(void);		// execute one opcode, returning number of cycles
    void illegal(byte s, byte op);				// if in safe mode, abort on illegal opcodes

#endif

#include <stdio.h>

#include "globals.h"
#include "support.h"

#ifndef ROMS_H
#define ROMS_H

	void load(const char name[], word adr);		// load firmware
	int ROMload(const char name[]);			// load ROM at the end, calling load()
    void displayInfoRom(const char name[]); // Display ROM header information

#endif

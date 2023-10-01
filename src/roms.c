#include "roms.h"

/* load firmware, arbitrary position */
void load(const char name[], word adr) {
	FILE *f;
	int c, b = 0;

	f = fopen(name, "rb");
	if (f != NULL) {
		do {
			c = fgetc(f);
			mem[adr+(b++)] = c;	// load one byte
		} while( c != EOF);

		fclose(f);
		printf("%s: %d bytes loaded at $%04X\n", name, --b, adr);
	}
	else {
		printf("*** Could not load image ***\n");
		run = 0;
	}
}

/* load ROM at the end of memory map */
int ROMload(const char name[]) {
	FILE *f;
	word pos = 0;
	long siz;

	f = fopen(name, "rb");
	if (f != NULL) {
		fseek(f, 0, SEEK_END);	// go to end of file
		siz = ftell(f);			// get size
		fclose(f);				// done for now, load() will reopen
		// If dump file
		if(siz == 65543) {
			printf("Loading memory dump file\n");
			load_dump(name);	// Load dump
			return 1;
		}
		// If rom bigger than 32K
		else if (siz > 32768) {
			printf("*** ROM too large! ***\n");
			run = 0;
			return -1;
		} else {
			pos -= siz;
            displayInfoRom(name);
			printf("Loading %s... (%ld K ROM image)\n", name, siz>>10);
			load(name, pos);	// get actual ROM image
			return 0;
		}
	}
	else {
		printf("*** Could not load ROM ***\n");
		run = 0;
		return -1;
	}
}

/* Read ROM header and display information */
void displayInfoRom(const char name[]) {
	FILE *f;
    byte header[256];
	int c, b = 0;
    char *title;
    char * description;

	f = fopen(name, "rb");
	if (f != NULL) {
		do {
			c = fgetc(f);
			header[b++] = c;	// load one byte
		} while( b <= 255);

		fclose(f);
        
        if(header[0]==0x0 && header[1]==0x64 && header[2]==0x58) {
            printf("DURANGO STANDARD ROM\n");
        
            title = (char*) header+0x0008;
            strcpy(romTitle, title);
            description = (char*) title+strlen(title)+1;
        
            printf("Title: %s\n", title);
            printf("Description: %s\n", description);
        }
        else {
            strcpy(romTitle, "");
        }
	}
	else {
		printf("*** Error reading image ***\n");
		run = 0;
	}
}


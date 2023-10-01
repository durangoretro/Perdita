#include "globals.h"
#include "vps.h"

void vps_config(word dir, byte v) {
    // If stat print mode
    if(v==PSV_STAT) {
        // Print stat
        stat();
    }
    // If stack print mode
    else if(v==PSV_STACK) {
        stack_stat();
    }
    // If memory dump mode
    else if(v==PSV_DUMP) {
        full_dump();
    }
    // If stop stopwatch
    else if(v==PSV_STOPWATCH_STOP) {
        printf("t=%ld cycles\n", cont-stopwatch);
    }
    // If start stopwatch
    else if(v==PSV_STOPWATCH_START) {
        stopwatch = cont;
    }
    // Cache value
    else {
        mem[dir]=v;
    }
    // PSV file open
    if(v==PSV_FOPEN) {
        psv_index = 0;
        if (psv_file != NULL) {
            fclose(psv_file);	// there was something open
            psv_file = NULL;
            printf("WARNING: there was another open file\n");
        }
    }
    // PSV file write
    if(v==PSV_FWRITE) {
        psv_filename[psv_index] = '\0';
        // actual file opening
        if(psv_file == NULL) {
            psv_file =fopen(psv_filename,"wb");
            if (psv_file == NULL) {	// we want a brand new file
                printf("[%d] ERROR: can't write to file %s\n", psv_index, psv_filename);
                mem[0xDF94] = 0;								// disable VSP
            } else {
                printf("Opening file %s for writing...\n", psv_filename);
            }
        } else {
            printf("ERROR: file already open\n");
            mem[0xDF94] = 0;									// disable VSP
        }
    }
    // PSV file read
    if(v==PSV_FREAD) {
        psv_filename[psv_index] = '\0';		// I believe this is needed
        if(psv_file == NULL) {
            psv_file =fopen(psv_filename,"rb");
            if (psv_file == NULL) {	// we want a brand new file
                printf("[%d] ERROR: can't open file %s\n", psv_index, psv_filename);
                mem[0xDF94] = 0;			// disable VSP
            } else {
                printf("Opening file %s for reading...\n", psv_filename);
            }
        } else {
            printf("ERROR: file already open\n");
            mem[0xDF94] = 0;									// disable VSP
        }
//				mem[0xDF93]=fgetc(psv_file);		// not done at config time, wait for actual read!
    }
    // PSV file close
    if(v==PSV_FCLOSE && psv_file!=NULL) {
        // close file
        if(fclose(psv_file)!=0) {
            printf("WARNING: Error closing file %s\n", psv_filename);
        } else {
            printf(" Done with file!\n");
        }
        psv_file = NULL;
    }
    // PSV raw file init
    if(v==PSV_RAW_INIT) {
        printf("PSV RAW INIT\n");
        open_psv_raw_file();
    }
    // PSV raw seek
    if(v==PSV_RAW_SEEK) {
        psv_index=0;
    }    
    // PSV raw file write
    if(v==PSV_RAW_WRITE) {
        open_psv_raw_file();
        fseek(psv_file, psv_raw_block*512, SEEK_SET);
        fwrite(&(mem[psv_raw_buffer]) , sizeof(char), 512, psv_file);
        printf("PSV RAW WRITE\n");
        close_psv_raw_file();
    }
    // If PSV raw file read
    if(v==PSV_RAW_READ) {
        open_psv_raw_file();
        fseek(psv_file, psv_raw_block*512, SEEK_SET);
        fread(&(mem[psv_raw_buffer]) , sizeof(char), 512, psv_file);
        printf("PSV RAW READ\n");
        close_psv_raw_file();
    }
    // PSV raw file init
    if(v==PSV_RAW_CLOSE) {
        printf("PSV RAW CLOSE\n");
        close_psv_raw_file();
    }
    
    // flush stdout
    fflush(stdout);
}

void vps_run(word dir, byte v) {
    word psv_int;
    int psv_value;
	uint32_t psv_value32;
    
    // Cache value
    mem[dir]=v;
    // If hex mode enabled
    if(mem[0xDF94]==PSV_HEX) {
        // Print hex value
        printf("[%02X]", mem[dir]);	
    }
    // If ascii mode enabled
    else if(mem[0xDF94]==PSV_ASCII) {
        // Print ascii
        printf("%c", mem[dir]);
    }
    // If binary mode enabled
    else if(mem[0xDF94]==PSV_BINARY) {
        // Print binary
        printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(mem[dir]));
    }
    // If decimal mode enabled
    else if(mem[0xDF94]==PSV_DECIMAL) {
        // Print decimal
        printf("[%u]", mem[dir]);
    }
    // If int8 mode enabled
    else if(mem[0xDF94]==PSV_INT8) {
        psv_int=mem[dir];
        if(psv_int<=0x7F) {
            psv_value=psv_int;
        }
        else {
            psv_value=psv_int-256;
        }
        printf("[%d]", psv_value);	
        psv_index=0;
    }
    // If int16 mode enabled
    else if(mem[0xDF94]==PSV_INT16) {
        // Save value
        psv_filename[psv_index++] = mem[dir];
        // Display value
        if(psv_index==2) {
            // Print decimal
            psv_int=psv_filename[0] | psv_filename[1]<<8;
            if(psv_int<=0x7FFF) {
                psv_value=psv_int;
            }
            else {
                psv_value=psv_int-65536;
            }
            printf("[%d]", psv_value);	
            psv_index=0;
        }
    }
	// If int32 mode enabled
    else if(mem[0xDF94]==PSV_INT32) {
        // Save value
        psv_filename[psv_index++] = mem[dir];
        // Display value
        if(psv_index==4) {
            // Print decimal
            psv_value32=psv_filename[0] + ((psv_filename[1]&0xFF)<<8) + ((psv_filename[2]&0xFF)<<16) + ((psv_filename[3]&0xFF)<<24);
			printf("[%d]", psv_value32);	
            psv_index=0;
        }
    }
    // If int mode enabled
    else if(mem[0xDF94]==PSV_HEX16) {
        // Save value
        psv_filename[psv_index++] = mem[dir];
        // Display value
        if(psv_index==2) {
            // Print hex
            printf("[%02X%02X]", psv_filename[0], psv_filename[1]);	
            psv_index=0;
        }
    }
    // If file open mode enabled
    else if(mem[0xDF94]==PSV_FOPEN) {
        // Filter filename
        if(mem[dir] >= ' ') {
        // Save filename
            psv_filename[psv_index++] = mem[dir];
        } else {
            psv_filename[psv_index++] = '_';
        }
    }
    // If file write mode enabled
    else if(mem[0xDF94]==PSV_FWRITE) {
        // write to file
        fputc(mem[dir], psv_file);
    }
    // If PSV raw file write mode enabled
    else if(mem[0xDF94]==PSV_RAW_SEEK) {
        // Save value
        psv_filename[psv_index++] = mem[dir];
        // Display value
        if(psv_index==6) {
            psv_raw_buffer=psv_filename[0] | psv_filename[1]<<8;
            psv_raw_block=psv_filename[5] | psv_filename[4]<<8 | psv_filename[3]<<16 | psv_filename[2]<<24;
            printf("PSV raw file seek. Buffer addr: [%02X%02X] (%d)\n", psv_filename[0], psv_filename[1], psv_raw_buffer);	
            printf("PSV raw file seek. File addr: [%02X%02X%02X%02X] (%ld)\n", psv_filename[2], psv_filename[3], psv_filename[4], psv_filename[5], psv_raw_block);	
            psv_index=0;
        }
    }
    
    // flush stdout
    fflush(stdout);
}

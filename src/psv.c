#include "psv.h"

void open_psv_raw_file() {
    // file opening
    if(psv_file == NULL) {
        psv_file =fopen("durango.av","rb+");
        if (psv_file == NULL) {
            psv_file =fopen("durango.av","wb+");
            if (psv_file == NULL) {
                printf("[%d] ERROR: can't write to file %s\n", psv_index, psv_filename);
                mem[0xDF94] = 0;
            }
        }
    }
}
void close_psv_raw_file() {
    if(psv_file!=NULL) {
        // close file
        if(fclose(psv_file)!=0) {
            printf("WARNING: Error closing file durango.av\n");
        }
        psv_file = NULL;
    }
}


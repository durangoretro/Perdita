#include <stdint.h>

#include "globals.h"
#include "support.h"
#include "keyboard_mouse.h"
#include "interrupt.h"

#ifndef VDU_H
#define VDU_H

/* *********************** */
/* vdu function prototypes */
/* *********************** */

	int  init_vdu();		// Initialize vdu display window
	void close_vdu();		// Close vdu display window
	void vdu_draw_full();	// Draw full screen
	void vdu_read_keyboard();	// Read keyboard
/* vdu internal functions */
	void vdu_set_color_pixel(byte);
	void vdu_set_hires_pixel(byte);
	void vdu_draw_color_pixel(word);
	void vdu_draw_hires_pixel(word);
	void draw_circle(SDL_Renderer*, int32_t, int32_t, int32_t);
/* audio functions */
	void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes);
	void sample_audio(int time, int value);

#endif

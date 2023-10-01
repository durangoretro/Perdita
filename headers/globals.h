#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <bits/types/FILE.h>
#include <stdint.h>

#ifndef GLOBALS_H
#define GLOBALS_H

/* Gamepad buttons constants */
#define BUTTON_A			0x80
#define BUTTON_START		0x40
#define BUTTON_B			0x20
#define BUTTON_SELECT		0x10
#define BUTTON_UP			0x08
#define BUTTON_LEFT			0x04
#define BUTTON_DOWN			0x02
#define BUTTON_RIGHT		0x01
/* PSV Constants */
#define	PSV_DISABLE			0
#define PSV_FOPEN			0x11
#define PSV_FREAD			0x12
#define PSV_FWRITE			0x13
#define PSV_FCLOSE			0x1F
#define PSV_RAW_INIT        0x20
#define PSV_RAW_SEEK        0x21
#define PSV_RAW_READ        0x22
#define PSV_RAW_WRITE       0x23
#define PSV_RAW_CLOSE       0x24
#define PSV_HEX				0xF0
#define PSV_ASCII			0xF1
#define PSV_BINARY			0xF2
#define PSV_DECIMAL			0xF3
#define PSV_INT16 			0xF4
#define PSV_HEX16			0xF5
#define PSV_INT8 			0xF6
#define PSV_INT32 			0xF7
#define PSV_STOPWATCH_START	0xFB
#define PSV_STOPWATCH_STOP	0xFC
#define PSV_DUMP			0xFD
#define PSV_STACK			0xFE
#define PSV_STAT			0xFF


/* Binary conversion */
#define BYTE_TO_BINARY_PATTERN "[%c%c%c%c%c%c%c%c]"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 


/* type definitions */
	typedef uint8_t byte;
	typedef uint16_t word;
/* global variables */
	extern byte mem[65536];			// unified memory map
	extern byte gamepads[2];		 	// 2 gamepad hardware status
	extern byte gamepads_latch[2];	 	// 2 gamepad register latch
	extern int emulate_gamepads;	// Allow gamepad emulation
	extern int emulate_minstrel;	// Emulate Minstrel keyboard
	extern int gp1_emulated; 		// Use keyboard as gamepad 1 (new layout WASD, Shift=start, Z=select, X='B', C=fire)
	extern int gp2_emulated; 		// Use keyboard as gamepad 2 (new layout IJKL, N=start, M=select, Alt='B', Space=fire)
	extern int gp_shift_counter;	// gamepad shift counter

	extern byte a, x, y, s, p;			// 8-bit registers
	extern word pc;					// program counter

	extern word screen;			// Durango screen switcher, xSSxxxxx xxxxxxxx
	extern int scr_dirty;			// screen update flag
	extern int	err_led;
	extern int dec;					// decimal flag for speed penalties (CMOS only)
	extern int run;				// allow execution, 0 = stop, 1 = pause, 2 = single step, 3 = run
	extern int ver;				// verbosity mode, 0 = none, 1 = warnings, 2 = interrupts, 3 = jumps, 4 = events, 5 = all
	extern int fast;				// speed flag
	extern int graf;				// enable SDL2 graphic display
	extern int safe;				// enable safe mode (stops on warnings and BRK)
	extern int nmi_flag;			// interrupt control
	extern int irq_flag;
	extern int typing;				// auto-typing flag
	extern int type_delay;				// received keystroke timing
	extern FILE *typed;				// keystroke file
	extern long cont;				// total elapsed cycles
	extern long stopwatch;			// cycles stopwatch
    extern int dump_on_exit;       // Generate dump after emulation

/* global vdu variables */
	// Screen width in pixels
	extern int VDU_SCREEN_WIDTH;
	// Screen height in pixels
	extern int VDU_SCREEN_HEIGHT;
	// Pixel size, both colout and HIRES modes
	extern int pixel_size, hpixel_size;
	//The window we'll be rendering to
	extern SDL_Window *sdl_window;
	//The window renderer
	extern SDL_Renderer* sdl_renderer;
	// Display mode
	extern SDL_DisplayMode sdl_display_mode;
	// Game gamepads
	extern SDL_Joystick *sdl_gamepads[2];
    // Rom title from header
    extern char romTitle[223];
	// Minstrel keyboard
	extern byte minstrel_keyboard[5];
	// Do not close GUI after program end
	extern int keep_open;
/* global sound variables */
	extern int sample_nr;
	extern SDL_AudioSpec want;

//	Uint8	aud_buff[192];	// room for 4 ms of 48 kHz 8-bit mono audio
	extern Uint8	aud_buff[30576];// room for ~20 ms (one field) of 8-bit mono audio at CPU rate!
	extern int		old_t;		// time of last sample creation
	extern int		old_v;		// last sample value

	extern byte keys[8][256];		// keyboard map

/* Global PSV Variables */
	// PSV filename
	extern char psv_filename[100];
	extern int psv_index;
	extern FILE* psv_file;
    extern long psv_raw_block;
    extern int psv_raw_buffer;

#endif

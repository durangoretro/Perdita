#include "globals.h"

/* global variables */
	byte mem[65536];			// unified memory map
	byte gamepads[2];		 	// 2 gamepad hardware status
	byte gamepads_latch[2];	 	// 2 gamepad register latch
	int emulate_gamepads = 0;	// Allow gamepad emulation
	int emulate_minstrel = 1;	// Emulate Minstrel keyboard
	int gp1_emulated = 0; 		// Use keyboard as gamepad 1 (new layout WASD, Shift=start, Z=select, X='B', C=fire)
	int gp2_emulated = 0; 		// Use keyboard as gamepad 2 (new layout IJKL, N=start, M=select, Alt='B', Space=fire)
	int gp_shift_counter = 0;	// gamepad shift counter

	byte a, x, y, s, p;			// 8-bit registers
	word pc;					// program counter

	word screen = 0;			// Durango screen switcher, xSSxxxxx xxxxxxxx
	int scr_dirty = 0;			// screen update flag
	int	err_led = 0;
	int dec;					// decimal flag for speed penalties (CMOS only)
	int run = 3;				// allow execution, 0 = stop, 1 = pause, 2 = single step, 3 = run
	int ver = 0;				// verbosity mode, 0 = none, 1 = warnings, 2 = interrupts, 3 = jumps, 4 = events, 5 = all
	int fast = 0;				// speed flag
	int graf = 1;				// enable SDL2 graphic display
	int safe = 0;				// enable safe mode (stops on warnings and BRK)
	int nmi_flag = 0;			// interrupt control
	int irq_flag = 0;
	int typing = 0;				// auto-typing flag
	int type_delay;				// received keystroke timing
	FILE *typed;				// keystroke file
	long cont = 0;				// total elapsed cycles
	long stopwatch = 0;			// cycles stopwatch
    int dump_on_exit = 0;       // Generate dump after emulation

/* global vdu variables */
	// Screen width in pixels
	int VDU_SCREEN_WIDTH;
	// Screen height in pixels
	int VDU_SCREEN_HEIGHT;
	// Pixel size, both colout and HIRES modes
	int pixel_size, hpixel_size;
	//The window we'll be rendering to
	SDL_Window *sdl_window;
	//The window renderer
	SDL_Renderer* sdl_renderer;
	// Display mode
	SDL_DisplayMode sdl_display_mode;
	// Game gamepads
	SDL_Joystick *sdl_gamepads[2];
    // Rom title from header
    char romTitle[223];
	// Minstrel keyboard
	byte minstrel_keyboard[5];
	// Do not close GUI after program end
	int keep_open = 0;
/* global sound variables */
	int sample_nr = 0;
	SDL_AudioSpec want;

//	Uint8	aud_buff[192];	// room for 4 ms of 48 kHz 8-bit mono audio
	Uint8	aud_buff[30576];// room for ~20 ms (one field) of 8-bit mono audio at CPU rate!
	int		old_t = 0;		// time of last sample creation
	int		old_v = 0;		// last sample value

	byte keys[8][256];		// keyboard map

/* Global PSV Variables */
	// PSV filename
	char psv_filename[100];
	int psv_index = 0;
	FILE* psv_file;
    long psv_raw_block;
    int psv_raw_buffer;

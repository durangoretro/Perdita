#include <SDL2/SDL.h>
#include <string.h>
#include <math.h>

#include "vdu.h"
#include "globals.h"

/* *** *** VDU SECTION *** *** */

/* Initialize vdu display window */
int init_vdu() {
	char windowTitle[300];
    strcpy(windowTitle, "Durango-X ");
    if(strlen(romTitle)>0) {
        strcat(windowTitle, " - ");
        strcat(windowTitle, romTitle);        
    }
    
    //Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO ) < 0 )
	{
		printf("SDL could not be initialized! SDL Error: %s\n", SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	//Check for joysticks
	for(int i=0; i<2 && i<SDL_NumJoysticks(); i++)
	{
		//Load joystick
		sdl_gamepads[i] = SDL_JoystickOpen(i);
		if(sdl_gamepads[i] == NULL)
		{
		 printf("Unable to open game gamepad #%d! SDL Error: %s\n", i, SDL_GetError());
		 return -2;
		}
	}
	if(emulate_gamepads && SDL_NumJoysticks()==0) {
		gp1_emulated = 1;
		gp2_emulated = 1;
	}
	else if(emulate_gamepads && SDL_NumJoysticks()==1) {
		gp1_emulated = 0;
		gp2_emulated = 1;
	}

	// Get display mode
	if (SDL_GetDesktopDisplayMode(0, &sdl_display_mode) != 0) {
		printf("SDL_GetDesktopDisplayMode faile! SDL Error: %s\n", SDL_GetError());
		return -3;
	}

	pixel_size=4;
	hpixel_size=2;
	VDU_SCREEN_WIDTH=128*pixel_size;
	VDU_SCREEN_HEIGHT=VDU_SCREEN_WIDTH;

	//Create window
	sdl_window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, VDU_SCREEN_WIDTH, VDU_SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
	if( sdl_window == NULL )
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return -4;
	}

	//Create renderer for window
	sdl_renderer = SDL_CreateRenderer( sdl_window, -1, SDL_RENDERER_ACCELERATED );
	if(sdl_renderer == NULL)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return -5;
	}

    //Clear screen
	SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(sdl_renderer);
	SDL_RenderPresent(sdl_renderer);

	// Initialize sound
	want.freq = 48000; // number of samples per second
	want.format = AUDIO_U8; // sample type (trying UNsigned 8 bit)
	want.channels = 1; // only one channel
	want.samples = 956; // buffer-size, does it need to be power of two? now every frame
	want.callback = audio_callback; // function SDL calls periodically to refill the buffer
	want.userdata = &(want.silence);

    SDL_AudioSpec have;
	if(SDL_OpenAudio(&want, &have) != 0)
	{
		printf("Failed to open SDL audio! SDL Error: %s\n", SDL_GetError());
		return -6;
	}
	if(want.format != have.format)
	{
		printf("Failed to setup SDL audio! SDL Error: %s\n", SDL_GetError());
if (have.format == AUDIO_U8) printf("**** era U8 *****");
		return -7;
	}

	// Start audio playback
	SDL_PauseAudio(0);
    
    return 0;
}

/* Close vdu display window */
void close_vdu() {
	// Stop audio
	SDL_PauseAudio(1);

	// Close audio module
	SDL_CloseAudio();

	// Close gamepads
	for(int i=0; i<2 && i<SDL_NumJoysticks(); i++)
	{
		SDL_JoystickClose(sdl_gamepads[i]);
		sdl_gamepads[i]=NULL;
	}

	//Destroy renderer
	if(sdl_renderer!=NULL)
	{
		SDL_DestroyRenderer(sdl_renderer);
		sdl_renderer=NULL;
	}

	if(sdl_window != NULL)
	{
		// Destroy window
		SDL_DestroyWindow(sdl_window);
		sdl_window=NULL;
	}

	
	// Close SDL
	SDL_Quit();
}

/* Set current color in SDL from palette */
void vdu_set_color_pixel(byte c) {
	// Color components
	byte red=0, green=0, blue=0;

	// Process invert flag
	if(mem[0xdf80] & 0x40) {
		c ^= 0x0F;		// just invert the index
	}

	// Durango palette
	switch(c) {
		case 0x00: red = 0x00; green = 0x00; blue = 0x00; break; // 0
		case 0x01: red = 0x00; green = 0xaa; blue = 0x00; break; // 1
		case 0x02: red = 0xff; green = 0x00; blue = 0x00; break; // 2
		case 0x03: red = 0xff; green = 0xaa; blue = 0x00; break; // 3
		case 0x04: red = 0x00; green = 0x55; blue = 0x00; break; // 4
		case 0x05: red = 0x00; green = 0xff; blue = 0x00; break; // 5
		case 0x06: red = 0xff; green = 0x55; blue = 0x00; break; // 6
		case 0x07: red = 0xff; green = 0xff; blue = 0x00; break; // 7
		case 0x08: red = 0x00; green = 0x00; blue = 0xff; break; // 8
		case 0x09: red = 0x00; green = 0xaa; blue = 0xff; break; // 9
		case 0x0a: red = 0xff; green = 0x00; blue = 0xff; break; // 10
		case 0x0b: red = 0xff; green = 0xaa; blue = 0xff; break; // 11
		case 0x0c: red = 0x00; green = 0x55; blue = 0xff; break; // 12
		case 0x0d: red = 0x00; green = 0xff; blue = 0xff; break; // 13
		case 0x0e: red = 0xff; green = 0x55; blue = 0xff; break; // 14
		case 0x0f: red = 0xff; green = 0xff; blue = 0xff; break; // 15
	}

	// Process RGB flag
	if(!(mem[0xdf80] & 0x08)) {
		red   = ((c&1)?0x88:0) | ((c&2)?0x44:0) | ((c&4)?0x22:0) | ((c&8)?0x11:0);
		green = red;
		blue  = green;	// that, or a switch like above for some sort of gamma correction, note bits are in reverse order!
	}

	SDL_SetRenderDrawColor(sdl_renderer, red, green, blue, 0xff);
}

/* Set current color in SDL HiRes mode */
void vdu_set_hires_pixel(byte color_index) {
	byte color = color_index ? 0xFF : 0x00;

	// Process invert flag
	if(mem[0xdf80] & 0x40) {
		color = ~color;
	}

	SDL_SetRenderDrawColor(sdl_renderer, color, color, color, 0xff);
}

/* Draw color pixel in supplied address */
void vdu_draw_color_pixel(word addr) {
	SDL_Rect fill_rect;
	// Calculate screen address
	word screen_address = (mem[0xdf80] & 0x30) << 9;

	// Calculate screen y coord
	int y = floor((addr - screen_address) >> 6);
	// Calculate screen x coord
	int x = ((addr - screen_address) << 1) & 127;

	// Draw Left Pixel
	vdu_set_color_pixel((mem[addr] & 0xf0) >> 4);
	fill_rect.x = x << 2;				// * pixel_size;
	fill_rect.y = y << 2;				// * pixel_size;
	fill_rect.w = pixel_size;
	fill_rect.h = pixel_size;
	SDL_RenderFillRect(sdl_renderer, &fill_rect);
	// Draw Right Pixel
	vdu_set_color_pixel(mem[addr] & 0x0f);
	fill_rect.x += pixel_size;
	SDL_RenderFillRect(sdl_renderer, &fill_rect);
}

void vdu_draw_hires_pixel(word addr) {
	SDL_Rect fill_rect;
	int i;
	// Calculate screen address
	word screen_address = (mem[0xdf80] & 0x30) << 9;
	// Calculate screen y coord
	int y = floor((addr - screen_address) >> 5);
	// Calculate screen x coord
	int x = ((addr - screen_address) << 3) & 255;
	byte b = mem[addr];

	fill_rect.x = (x << 1) -2;			// * hpixel_size;
	fill_rect.y = y << 1;				// * hpixel_size;
	fill_rect.w = hpixel_size;
	fill_rect.h = hpixel_size;
	for(i=0; i<8; i++) {
		vdu_set_hires_pixel(b & 0x80);		// set function doesn't tell any non-zero value
		b <<= 1;
		fill_rect.x += hpixel_size;
		SDL_RenderFillRect(sdl_renderer, &fill_rect);
	}
}

/* Render Durango screen. */
void vdu_draw_full() {
	word i;
	byte hires_flag = mem[0xdf80] & 0x80;
	word screen_address = (mem[0xdf80] & 0x30) << 9;
	word screen_address_end = screen_address + 0x2000;

	//Clear screen
    SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(sdl_renderer);

	// Color
	if(!hires_flag) {
		for(i=screen_address; i<screen_address_end; i++) {
			vdu_draw_color_pixel(i);
		}
	}
	// HiRes
	else {
		for(i=screen_address; i<screen_address_end; i++) {
			vdu_draw_hires_pixel(i);
		}
	}

	// Display something resembling the error LED at upper right corner, if lit
	if (err_led && !(mem[0xdfa0] & 1)) {	// check interrupt status
		// black surrounding
		SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 0xff);
		draw_circle(sdl_renderer, 490, 490, 10);
		// Set color to red
		SDL_SetRenderDrawColor(sdl_renderer, 0xff, 0x00, 0x00, 0xff);
		// Draw red led
		draw_circle(sdl_renderer, 490, 490, 8);
	}

	// A similar code may be used for other LEDs
	if (err_led && (mem[0xdf80] & 4)) {		// check free bit from '174
		// black surrounding
		SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 0xff);
		draw_circle(sdl_renderer, 460, 490, 10);
		// Set color to white
		SDL_SetRenderDrawColor(sdl_renderer, 0xff, 0xff, 0xff, 0xff);
		// Draw white led
		draw_circle(sdl_renderer, 460, 490, 8);
	}

	//Update screen
	SDL_RenderPresent(sdl_renderer);
	
	scr_dirty = 0;			// window has been updated
}

/* Process GUI events in VDU window */
void vdu_read_keyboard() {
	//Event handler
    SDL_Event e;
    
	//Handle events on queue
	while( SDL_PollEvent( &e ) != 0 )
	{
		// Vdu window is closed
		if(e.type == SDL_QUIT)
		{
			run = 0;
		}
		// Press F1 = STOP
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F1) {
			run = 0;		// definitively stop execution
		}
		// Press F2 = NMI
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F2) {
			nmi_flag = 1;	// simulate NMI signal
		}
		// Press F3 = IRQ?
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F3) {
			irq_flag = 1;	// simulate (spurious) IRQ
		}
		// Press F4 = RESET
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F4) {
			reset();
		}
		// Press F5 = PAUSE
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F5) {
			run = 1;		// pause execution and display status
		}
		// Press F6 = DUMP memory to file
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F6) {
			full_dump();
		}
		// Press F7 = STEP
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F7) {
			run = 2;		// will execute a single opcode, then back to PAUSE mode
		}
		// Press F8 = RESUME
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F8) {
			run = 3;		// resume normal execution
		}
		// Press F9 = LOAD DUMP
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F9) {
			load_dump("dump.bin");	// load saved status...
			run = 3;		// ...and resume execution
			scr_dirty = 1;	// but update screen! EEEEEK
		}
		// Press F10 = KEYSTROKES
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F10) {
			typed=fopen("keystrokes.txt","r");
			if (typed==NULL) {
				printf("\n*** No keystrokes file! ***\n");
			} else {
				printf("Sending keystrokes...");
				type_delay = 1;
				typing = 1;	// start typing from file
			}
		}
		// Press F11
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F11) {
		}
		// Press F12
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_F12) {
			run = 0;
		}
		// Event forwarded to Durango
		else {
			// Emulate gamepads
			if(emulate_gamepads) {
				if(gp1_emulated) {
					emulate_gamepad1(&e);
				}
				if(gp2_emulated) {
					emulate_gamepad2(&e);
				}
			}
			// Emulate minstrel keyboard
			if(emulate_minstrel) {
				emulation_minstrel(&e);
			}
			// Full PASK keyboard is always emulated!
			process_keyboard(&e);
		}
	}
}

/* Aux procedure to draw circles using SDL */
void draw_circle(SDL_Renderer * renderer, int32_t x, int32_t y, int32_t radius) {
   for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius))
            {
                SDL_RenderDrawPoint(sdl_renderer, x + dx, y + dy);
            }
        }
    }
}

/* SDL audio call back function */
void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes) {
	// Fill buffer with new audio to play
	for(int i=0; i<bytes; i++) {
		raw_buffer[i] = aud_buff[i<<5];
	}
}

/* custom audio function */
void sample_audio(int time, int value) {
	if (time <= old_t) {			// EEEEEEEEEEK, not '<'
		time = 30576;
	}
	while (old_t != time) {
		aud_buff[old_t++] = old_v;
	}
	old_v = value;
	old_t %= 30576;
}

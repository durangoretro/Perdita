#include "globals.h"
#include <SDL2/SDL_events.h>

#ifndef KEYBOARD_MOUSE_H
#define KEYBOARD_MOUSE_H

	void redefine(void);
    void process_keyboard(SDL_Event*);
    void emulate_gamepad1(SDL_Event *e);
    void emulate_gamepad2(SDL_Event *e);
    void emulation_minstrel(SDL_Event *e);

#endif

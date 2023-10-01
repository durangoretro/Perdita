#include "keyboard_mouse.h"

/* *** *** Process keyboard / mouse events *** *** */
/* *** redefine keymap *** */
void redefine(void) {
	int i, j;
/* some awkward sequences */
	byte n_a[10]	= {0x7e, 0x7c, 0x40, 0x23, 0xa4, 0xba, 0xac, 0xa6, 0x7b, 0x7d};	// number keys with ALT
	byte n_as[10]	= {0x9d, 0xa1,    0, 0xbc, 0xa3, 0xaa, 0xb4, 0x5c, 0xab, 0xbb}; // number keys with SHIFT+ALT
	byte n_ac[10]	= {0xad, 0x2a, 0xa2, 0xb1, 0xa5, 0xf7, 0x60, 0xbf, 0x96, 0x98}; // number keys with CTRL+ALT
	byte nl_c[5]	= {0x5f, 0x2b, 0x27, 0x2e, 0x2c};	// lower half number keys with CTRL
	byte n_cs[10]	= {0x7f, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x5e, 0x3f, 0x5b, 0x5d};	// number keys with CTRL+SHIFT
	byte curs[8]	= {0x02, 0x0b, 0x0a, 0x06, 0x19, 0x16, 0x01, 0x05};				// ASCII for left, up, down, right, pgup, pgnd, home, end
/* diacritics and indices */
	byte acute[5]	= {0xe1, 0xe9, 0xed, 0xf3, 0xfa};	// lowercase acute accents (sub $20 for uppercase, 1 for grave; add 1 for circumflex, 2 for umlaut*) except ä ($e4) and ö ($f6, both acute+3)
	byte tilde[5]	= {0xe3, 0xe6,    0, 0xf5, 0xe5};	// lowercase ã, ae, -, õ, å (sub $20 for uppercase)
	byte vowel[5]	= {'a', 'e', 'i', 'o', 'u'};		// position of lowercase circumflex, acute and umlaut vowels
	byte middle[5]	= {'g', 'h', 'j', 'k', 'l'};		// position of uppercase circumflex, grave and some with tilde

	for (i=0; i<8; i++)
		for (j=0;j<256;j++)
			keys[i][j]=0;		// clear entries by default

/* * some dedicated keys * */
	for (i=0; i<8; i++) {		// set some common keys to (almost) all shifted combos
		keys[i][0x08]	=0x08;	// backspace
		if (i<4)
			keys[i][0x09]=0x09;	// tab, except all ALTs (SWTC, 0 or $1A?)
		else
			keys[i][0x09]=0x00;	// tab, except all ALTs (SWTC, 0 or $1A?)***TBD
		keys[i][0x0d]	=0x0d;	// newline
		keys[i][0x1b]	=0x1b;	// escape
		switch(i) {
			case 2:
				keys[2][0x20]	=0x80;	// CTRL-SPACE
				break;
			case 4:
				keys[4][0x20]	=0xA0;	// ALT-SPACE
				break;
			case 6:
				keys[6][0x20]	=0;		// ALT+CTRL-SPACE
				break;
			default:
				keys[i][0x20]	=0x20;	// space bar, except CTRL ($80), ALT ($A0) and CTRL+ALT (0)
		}
		keys[i][0x7f]	=0x7f;	// delete
		if (i&1) {				// some SHIFTed combos, not affected by ALT/CTRL
			keys[i][',']=';';
			keys[i]['.']=':';
			keys[i]['-']='_';	// no dashes here, just hyphen/underscore
			keys[i][0x27]='?';	// apostrophe key
			keys[i][0xa1]=0xbf;	// reverse question
		} else {
			keys[i][',']=',';
			keys[i]['.']='.';
			keys[i]['-']='-';	// no dashes here, just hyphen/underscore
			keys[i][0x27]=0x27;	// apostrophe
			keys[i][0xa1]=0xa1;	// reverse exclamation
		}
		for (j=0; j<8; j++) {
			keys[i][curs[j]] = curs[j];			// set cursor keys for every modifier
		}
		if (i&2) {								// if CTRL is pressed...
			keys[i][curs[6]]	=0x15;			// ...both home and end...
			keys[i][curs[7]]	=0x04;			// ...refer to whole document
		}
	}
/* set top left key manually */
	keys[0][0xba]	=0xba;		// ord m.
	keys[1][0xba]	=0xaa;		// ord f.
	keys[4][0xba]	=0x5c;		// backslash
	keys[5][0xba]	=0xb0;		// degree (Mac)

/* * number keys * */
	for (i='0'; i<='9'; i++) {
		keys[0][i]		=i;				// unshifted numbers
		keys[1][i]		=i-0x10;		// shift all except 3 ($B7), 7 ($2F) and 0 ($3D)
		if (i>'4')
			keys[2][i]	=i+5;			// ctrl over 5 OK except 7 ($2D) and 8 ($3C)
		else
			keys[2][i]	=nl_c[i-'0'];	// first half of numbers + CTRL
		keys[3][i]	=n_cs[i-'0'];		// numbers with CTRL-SHIFT
		keys[4][i]	=n_a[i-'0'];		// alt
		keys[5][i]	=n_as[i-'0'];		// alt+shift
		keys[6][i]	=n_ac[i-'0'];		// alt+ctrl
		if ((i>'1')&&(i<'4'))
			keys[7][i]	=i+0x80;		//only these two ctrl-alt-shift number keys make sense
	}
/* manually assigned number keys */
	keys[1]['0']		=0x3d;	// equals
	keys[1]['3']		=0xb7;	// interpunct (shift-3)
	keys[1]['7']		=0x2f;	// slash
	keys[2]['7']		=0x2d;	// minus (ctrl-7)
	keys[2]['8']		=0x3c;	// less than
	keys[7]['0']		=0xaf;	// macron
	keys[7]['8']		=0x9c;	// infinity

/* * letter keys * */
	for (i='a'; i<='z'; i++) {
		keys[0][i]	=i;			// standard unshifted letter
		keys[1][i]	=i-0x20;	// uppercase letters
		keys[2][i]	=i-0x60;	// control codes
	}							// other combos must be computed differently
/* diacritics */
	for (i=0; i<5; i++) {		// check vowels
		keys[4][vowel[i]]	=acute[i];		// ALT vowel = acute
		keys[5][vowel[i]]	=acute[i]-0x20;	// ALT+SHIFT vowel = upper acute
		keys[4][middle[i]]	=tilde[i];		// ALT middle = tildes etc
		if (i!=2)
			keys[3][middle[i]]	=tilde[i]-0x20;	// CTRL+SHIFT middle = upper tildes etc
		else
			keys[3][middle[i]]	=0x8b;		// semigraphic alternative
		keys[6][middle[i]]	=acute[i]-1;	// ALT+CTRL middle = grave
		keys[7][middle[i]]	=acute[i]-0x21;	// ALT+CTRL+SHIFT middle = upper grave
		keys[3][vowel[i]]	=acute[i]+1;	// CTRL+SHIFT vowel = circum
		keys[5][middle[i]]	=acute[i]-0x1f;	// ALT+SHIFT middle = upper circum
		if ((i!=0) && (i!=3)) {
			keys[6][vowel[i]]	=acute[i]+2;	// ALT+CTRL vowel = umlaut
			keys[7][vowel[i]]	=acute[i]-0x1e;	// ALT+CTRL+SHIFT vowel = upper umlaut
		} else {
			keys[6][vowel[i]]	=acute[i]+3;	// special cases for ä and ö, b/c tilde
			keys[7][vowel[i]]	=acute[i]-0x1d;
		}
	}
// ÿ and some others TBD...

/* misc Spanish ISO layout characters, usually disabled with CTRL */
	keys[0]['+']	=0x2b;	// plus
	keys[1]['+']	=0x2a;	// star
	keys[4]['+']	=0x5d;	// close bracket
	keys[5]['+']	=0xb1;	// plus-minus
	keys[0]['`']	=0x60;	// grave accent
	keys[1]['`']	=0x5e;	// caret
	keys[4]['`']	=0x5b;	// open bracket

	keys[0][0xb4]	=0xb4;	// acute accent
	keys[1][0xb4]	=0xa8;	// diaeresis 
	keys[4][0xb4]	=0x7b;	// open braces 
	keys[5][0xb4]	=0xab;	// open guillemet
	keys[0][0xf1]	=0xf1;	// ñ
	keys[1][0xf1]	=0xd1;	// Ñ 
	keys[4][0xf1]	=0x7e;	// tilde

	keys[0][0xe7]	=0xe7;	// ç
	keys[1][0xe7]	=0xc7;	// Ç
	keys[4][0xe7]	=0x7d;	// close braces
	keys[5][0xe7]	=0xbb;	// close guillemet
	keys[0][0x3c]	=0x3c;	// <
	keys[1][0x3c]	=0x3e;	// >
	keys[4][0x3c]	=0x96;	// less or equal
	keys[5][0x3c]	=0x98;	// more or equal
}

void process_keyboard(SDL_Event *e) {
	int asc, shift;
	/*
	 * Type:
	 * SDL_KEYDOWN
	 * SDL_KEYUP
	 * SDL_JOYAXISMOTION
	 * SDL_JOYBUTTONDOWN
	 * SDL_JOYBUTTONUP
	 * SDL_MOUSEMOTION
	 * SDL_MOUSEBUTTONDOWN
	 * SDL_MOUSEBUTTONUP
	 * SDL_MOUSEWHEEL
	 * 
	 * Code:
	 * https://wiki.libsdl.org/SDL_Keycode
	 * 
	 * Modifiers:
	 * KMOD_NONE -> no modifier is applicable
	 * KMOD_LSHIFT -> the left Shift key is down
	 * KMOD_RSHIFT -> the right Shift key is down
	 * KMOD_LCTRL -> the left Ctrl (Control) key is down
	 * KMOD_RCTRL -> the right Ctrl (Control) key is down
	 * KMOD_LALT -> the left Alt key is down
	 * KMOD_RALT -> the right Alt key is down
	 * KMOD_CTRL -> any control key is down
	 * KMOD_SHIFT-> any shift key is down
	 * KMOD_ALT -> any alt key is down
	 * KMOD_CAPS -> caps key is down
	 */
	if(e->type == SDL_KEYDOWN) {
		if (ver)		printf("key: %c ($%x)\n", e->key.keysym.sym, e->key.keysym.scancode);
		
		shift = 0;			// default unshifted state
		if(SDL_GetModState() & KMOD_SHIFT) {			// SHIFT
			shift |= 1;		// d0 = SHIFT
		}
		if(SDL_GetModState() & KMOD_CTRL) {				// CONTROL
			shift |= 2;		// d1 = CONTROL
		}
		if(SDL_GetModState() & KMOD_ALT) {				// ALT
			shift |= 4;		// d2 = ALTERNATE
		}
		if (SDL_GetModState() & KMOD_CAPS)
		{
			printf("KMOD_CAPS is pressed\n");	// no CAPS LOCK support yet!
		}
		switch(e->key.keysym.scancode) {
			case 0x2f:				// grave accent
				asc = 0x60;
				break;
			case 0x34:				// acute accent
				asc = 0xb4;
				break;
			case 0x50:				// cursor left
				asc = 0x02;
				break;
			case 0x52:				// cursor up
				asc = 0x0b;
				break;
			case 0x51:				// cursor down
				asc = 0x0a;
				break;
			case 0x4f:				// cursor right
				asc = 0x06;
				break;
			case 0x4b:				// page up
				asc = 0x19;
				break;
			case 0x4e:				// page down
				asc = 0x16;
				break;
			case 0x4a:				// home
				asc = 0x01;
				break;
			case 0x4d:				// end
				asc = 0x05;
				break;
			case 0x62:				// numpad 0
				shift = 0;			// any numpad key will disable all modificators
				asc = '0';
				break;
			case 0x59:				// numpad 1-9
			case 0x5a:
			case 0x5b:
			case 0x5c:
			case 0x5d:
			case 0x5e:
			case 0x5f:
			case 0x60:
			case 0x61:
				shift = 0;
				asc = e->key.keysym.scancode - 0x28;
				break;
			case 0x58:				// numpad ENTER
				shift = 0;
				asc = 0x0d;
				break;
			case 0x63:				// numpad DECIMAL POINT
				shift = 0;
				asc = '.';			// desired value for EhBASIC
				break;
			case 0x57:				// numpad +
				shift = 0;
				asc = '+';
				break;
			case 0x56:				// numpad -
				shift = 0;
				asc = '-';
				break;
			case 0x55:				// numpad *
				shift = 1;			// actually SHIFT and '+'
				asc = '+';
				break;
			case 0x54:				// numpad /
				shift = 1;			// actually SHIFT and 7
				asc = '7';
				break;
			default:
				asc = e->key.keysym.sym;
		}
		if (asc<256) {
			asc = keys[shift][asc];	// read from keyboard table
			mem[0xDF9A] = asc;		// will temporarily store ASCII at 0xDF9A, as per PASK standard :-)
		}
	}
	// detect key release for PASK compatibility
	else if(e->type == SDL_KEYUP) {
		if (ver)	printf("·");
		mem[0xDF9A] = 0;
	}
	// gamepad button down
	else if(e->type == SDL_JOYBUTTONDOWN) {
		if (ver) printf("gamepad: %d button: %d\n",e->jbutton.which, e->jbutton.button);
		switch( e->jbutton.button) {
        	case 0: gamepads[e->jbutton.which] |= BUTTON_A; break;
        	case 1: gamepads[e->jbutton.which] |= BUTTON_B; break;
        	case 2: gamepads[e->jbutton.which] |= BUTTON_A; break;
        	case 3: gamepads[e->jbutton.which] |= BUTTON_B; break;
			case 8: gamepads[e->jbutton.which] |= BUTTON_SELECT; break;
        	case 9: gamepads[e->jbutton.which] |= BUTTON_START; break;
        }
		if (ver > 2) printf("gamepads[0] = $%x\n", gamepads[0]);
		if (ver > 2) printf("gamepads[0] = $%x\n", gamepads[1]);
	}
	// gamepad button up
	else if(e->type == SDL_JOYBUTTONUP) {
		switch( e->jbutton.button) {
        	case 0: gamepads[e->jbutton.which] &= ~BUTTON_A; break;
        	case 1: gamepads[e->jbutton.which] &= ~BUTTON_B; break;
			case 2: gamepads[e->jbutton.which] &= ~BUTTON_A; break;
        	case 3: gamepads[e->jbutton.which] &= ~BUTTON_B; break;
        	case 8: gamepads[e->jbutton.which] &= ~BUTTON_SELECT; break;
        	case 9: gamepads[e->jbutton.which] &= ~BUTTON_START; break;
        }
		if (ver > 2) printf("gamepads[0] = $%x\n", gamepads[0]);
		if (ver > 2) printf("gamepads[0] = $%x\n", gamepads[1]);
	}
	else if( e->type == SDL_JOYAXISMOTION) {
		if (ver) printf("gamepad: %d, axis: %d, value: %d\n", e->jaxis.which, e->jaxis.axis, e->jaxis.value);
		// Left
		if(e->jaxis.axis==0 && e->jaxis.value<0) {
			gamepads[e->jaxis.which] |= BUTTON_LEFT;
			gamepads[e->jaxis.which] &= ~BUTTON_RIGHT;
		}
		// Right
		else if(e->jaxis.axis==0 && e->jaxis.value>0) {
			gamepads[e->jaxis.which] &= ~BUTTON_LEFT;
			gamepads[e->jaxis.which] |= BUTTON_RIGHT;
		}
		// None
		else if(e->jaxis.axis==0 && e->jaxis.value==0) {
			gamepads[e->jaxis.which] &= ~BUTTON_LEFT;
			gamepads[e->jaxis.which] &= ~BUTTON_RIGHT;
		}
		// Up
		if(e->jaxis.axis==1 && e->jaxis.value<0) {
			gamepads[e->jaxis.which] |= BUTTON_UP;
			gamepads[e->jaxis.which] &= ~BUTTON_DOWN;
		}
		// Down
		else if(e->jaxis.axis==1 && e->jaxis.value>0) {
			gamepads[e->jaxis.which] &= ~BUTTON_UP;
			gamepads[e->jaxis.which] |= BUTTON_DOWN;
		}
		// None
		else if(e->jaxis.axis==1 && e->jaxis.value==0) {
			gamepads[e->jaxis.which] &= ~BUTTON_UP;
			gamepads[e->jaxis.which] &= ~BUTTON_DOWN;
		}
		if (ver > 2) printf("gamepads[0] = $%x\n", gamepads[0]);
		if (ver > 2) printf("gamepads[1] = $%x\n", gamepads[1]);
	}
}

/* Emulate first gamepad. */
void emulate_gamepad1(SDL_Event *e) {
	// Left key down p1 at o, now A
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'a') {
		// Left down
		gamepads[0] |= BUTTON_LEFT;		
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'a') {
		// Left up
		gamepads[0] &= ~BUTTON_LEFT;		
	}
	// Right key down p1 at p, now D
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'd') {
		// Right down
		gamepads[0] |= BUTTON_RIGHT;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'd') {
		// Right up
		gamepads[0] &= ~BUTTON_RIGHT;
	}
	// Up key p1 at q, now W
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'w') {
		// Up down
		gamepads[0] |= BUTTON_UP;		
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'w') {
		// Up up
		gamepads[0] &= ~BUTTON_UP;
	}
	// Down key p1 at a, now S
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 's') {
		// Down down
		gamepads[0] |= BUTTON_DOWN;		
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 's') {
		// Down up
		gamepads[0] &= ~BUTTON_DOWN;
	}
	// A key down p1 at space, now C
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'c') {
		// A down
		gamepads[0] |= BUTTON_A;		
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'c') {
		// A up
		gamepads[0] &= ~BUTTON_A;
	}
	// B key p1 at c, now X
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'x') {
		// B down
		gamepads[0] |= BUTTON_B;		
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'x') {
		// B up
		gamepads[0] &= ~BUTTON_B;
	}
	// START key p1 at space, now left shift
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_LSHIFT) {
		// START down
		gamepads[0] |= BUTTON_START;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == SDLK_LSHIFT) {
		// START up
		gamepads[0] &= ~BUTTON_START;
	}
	// SELECT key p1 at x, now Z
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'z') {
		// SELECT down
		gamepads[0] |= BUTTON_SELECT;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'z') {
		// SELECT up
		gamepads[0] &= ~BUTTON_SELECT;
	}
}

/* Emulate second gamepad. */
void emulate_gamepad2(SDL_Event *e) {
	// Left key down p2 at u, now J
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'j') {
		// Left down
		gamepads[1] |= BUTTON_LEFT;		
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'j') {
		// Left up
		gamepads[1] &= ~BUTTON_LEFT;		
	}
	// Right key down p2 at i, now L
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'l') {
		// Right down
		gamepads[1] |= BUTTON_RIGHT;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'l') {
		// Right up
		gamepads[1] &= ~BUTTON_RIGHT;
	}
	// Up key p2 at w, now I
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'i') {
		// Up down
		gamepads[1] |= BUTTON_UP;		
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'i') {
		// Up up
		gamepads[1] &= ~BUTTON_UP;
	}
	// Down key p2 at s, now K
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'k') {
		// Down down
		gamepads[1] |= BUTTON_DOWN;		
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'k') {
		// Down up
		gamepads[1] &= ~BUTTON_DOWN;
	}
	// A key down p2 at e, now SPACE
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == ' ') {
		// A down
		gamepads[1] |= BUTTON_A;		
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == ' ') {
		// A up
		gamepads[1] &= ~BUTTON_A;
	}
	// B key p2 at d, now ALT-GR
	if(e->type == SDL_KEYDOWN && e->key.keysym.scancode == SDL_SCANCODE_RALT) {
		// B down
		gamepads[1] |= BUTTON_B;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.scancode == SDL_SCANCODE_RALT) {
		// B up
		gamepads[1] &= ~BUTTON_B;
	}
	// START key p2 at r, now N
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'n') {
		// START down
		gamepads[1] |= BUTTON_START;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'n') {
		// START up
		gamepads[1] &= ~BUTTON_START;
	}
	// SELECT key p2 at f, now M
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'm') {
		// SELECT down
		gamepads[1] |= BUTTON_SELECT;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'm') {
		// SELECT up
		gamepads[1] &= ~BUTTON_SELECT;
	}
}

void emulation_minstrel(SDL_Event *e) {
	// COL 1 DOWN
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == ' ') {
		minstrel_keyboard[0] |= 128;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 13) {
		minstrel_keyboard[0] |= 64;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_LSHIFT) {	// LEFT SHIFT
		minstrel_keyboard[0] |= 32;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'p') {
		minstrel_keyboard[0] |= 16;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == '0') {
		minstrel_keyboard[0] |= 8;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'a') {
		minstrel_keyboard[0] |= 4;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'q') {
		minstrel_keyboard[0] |= 2;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == '1') {
		minstrel_keyboard[0] |= 1;
	}
	// COL 1 UP
	if(e->type == SDL_KEYUP && e->key.keysym.sym == ' ') {
		minstrel_keyboard[0] &= ~128;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 13) {
		minstrel_keyboard[0] &= ~64;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == SDLK_LSHIFT) {	// LEFT SHIFT
		minstrel_keyboard[0] &= ~32;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'p') {
		minstrel_keyboard[0] &= ~16;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == '0') {
		minstrel_keyboard[0] &= ~8;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'a') {
		minstrel_keyboard[0] &= ~4;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'q') {
		minstrel_keyboard[0] &= ~2;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == '1') {
		minstrel_keyboard[0] &= ~1;
	}
	// COL 2 DOWN
	if(e->type == SDL_KEYDOWN && e->key.keysym.scancode == SDL_SCANCODE_RALT) {	// ALT (GR) SDLK_RALT
		minstrel_keyboard[1] |= 128;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'l') {
		minstrel_keyboard[1] |= 64;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'z') {
		minstrel_keyboard[1] |= 32;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'o') {
		minstrel_keyboard[1] |= 16;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == '9') {
		minstrel_keyboard[1] |= 8;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 's') {
		minstrel_keyboard[1] |= 4;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'w') {
		minstrel_keyboard[1] |= 2;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == '2') {
		minstrel_keyboard[1] |= 1;
	}
	// COL 2 UP
	if(e->type == SDL_KEYUP && e->key.keysym.scancode == SDL_SCANCODE_RALT) {	// ALT (GR)
		minstrel_keyboard[1] &= ~128;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'l') {
		minstrel_keyboard[1] &= ~64;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'z') {
		minstrel_keyboard[1] &= ~32;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'o') {
		minstrel_keyboard[1] &= ~16;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == '9') {
		minstrel_keyboard[1] &= ~8;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 's') {
		minstrel_keyboard[1] &= ~4;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'w') {
		minstrel_keyboard[1] &= 24;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == '2') {
		minstrel_keyboard[1] &= ~1;
	}
	// COL 3 DOWN
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'm') {
		minstrel_keyboard[2] |= 128;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'k') {
		minstrel_keyboard[2] |= 64;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'x') {
		minstrel_keyboard[2] |= 32;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'i') {
		minstrel_keyboard[2] |= 16;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == '8') {
		minstrel_keyboard[2] |= 8;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'd') {
		minstrel_keyboard[2] |= 4;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'e') {
		minstrel_keyboard[2] |= 2;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == '3') {
		minstrel_keyboard[2] |= 1;
	}
	// COL 3 UP
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'm') {
		minstrel_keyboard[2] &= ~128;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'k') {
		minstrel_keyboard[2] &= ~64;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'x') {
		minstrel_keyboard[2] &= ~32;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'i') {
		minstrel_keyboard[2] &= ~16;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == '8') {
		minstrel_keyboard[2] &= ~8;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'd') {
		minstrel_keyboard[2] &= ~4;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'e') {
		minstrel_keyboard[2] &= ~2;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == '3') {
		minstrel_keyboard[2] &= ~1;
	}
	// COL 4 DOWN
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'n') {
		minstrel_keyboard[3] |= 128;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'j') {
		minstrel_keyboard[3] |= 64;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'c') {
		minstrel_keyboard[3] |= 32;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'u') {
		minstrel_keyboard[3] |= 16;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == '7') {
		minstrel_keyboard[3] |= 8;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'f') {
		minstrel_keyboard[3] |= 4;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'r') {
		minstrel_keyboard[3] |= 2;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == '4') {
		minstrel_keyboard[3] |= 1;
	}
	// COL 4 UP
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'n') {
		minstrel_keyboard[3] &= ~128;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'j') {
		minstrel_keyboard[3] &= ~64;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'c') {
		minstrel_keyboard[3] &= ~32;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'u') {
		minstrel_keyboard[3] &= ~16;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == '7') {
		minstrel_keyboard[3] &= ~8;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'f') {
		minstrel_keyboard[3] &= ~4;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'r') {
		minstrel_keyboard[3] &= ~2;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == '4') {
		minstrel_keyboard[3] &= ~1;
	}
	// COL 5 DOWN
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'b') {
		minstrel_keyboard[4] |= 128;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'h') {
		minstrel_keyboard[4] |= 64;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'v') {
		minstrel_keyboard[4] |= 32;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'y') {
		minstrel_keyboard[4] |= 16;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == '6') {
		minstrel_keyboard[4] |= 8;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 'g') {
		minstrel_keyboard[4] |= 4;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == 't') {
		minstrel_keyboard[4] |= 2;
	}
	if(e->type == SDL_KEYDOWN && e->key.keysym.sym == '5') {
		minstrel_keyboard[4] |= 1;
	}
	// COL 5 UP
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'b') {
		minstrel_keyboard[4] &= ~128;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'h') {
		minstrel_keyboard[4] &= ~64;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'v') {
		minstrel_keyboard[4] &= ~32;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'y') {
		minstrel_keyboard[4] &= ~16;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == '6') {
		minstrel_keyboard[4] &= ~8;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 'g') {
		minstrel_keyboard[4] &= ~4;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == 't') {
		minstrel_keyboard[4] &= ~2;
	}
	if(e->type == SDL_KEYUP && e->key.keysym.sym == '5') {
		minstrel_keyboard[4] &= ~1;
	}
}

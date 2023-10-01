COMPILER_FLAGS = -Wall -O2
COMPILER_FLAGS_WIN = $(COMPILER_FLAGS) -c
LINKER_FLAGS = -lstdc++ -lSDL2 -lm
VERSION?=1.0.2

SDL_WIN = ${HOME}/sdl_win/SDL2-2.0.18/x86_64-w64-mingw32
LIB_WIN = -L$(SDL_WIN)/lib
INCL_WIN = -I$(SDL_WIN)/include -I$(SDL_WIN)/include/SDL2

SRCDIR = src

INCDIR = headers/
SRCFILES = src/perdita.c

all:perdita

perdita: version.h $(SRCFILES)
	gcc $(SRCFILES) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o perdita -I$(INCDIR)
	
perdita_debug: $(SRCFILES)
	gcc -g $(SRCFILES) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o perdita_debug -I$(INCDIR)
version.h: $(SRCFILES)
	
	echo "char * perdita_version=\"$(VERSION)-$$(git rev-parse --short HEAD)\";"> version.h
# Generate Windows Version. You can use the Docker file in https://github.com/zerasul/dockerretro/blob/master/sdl/Dockerfile
# To generate this file.
perdita.zip: perdita.exe
	zip -j perdita.zip perdita.exe $(SDL_WIN)/bin/SDL2.dll
perdita.exe: perdita.win.o
	x86_64-w64-mingw32-g++ $(LIB_WIN) -static perdita.win.o  `$(SDL_WIN)/bin/sdl2-config --static-libs` -o perdita.exe

perdita.win.o: version.h $(SRCFILES)
	x86_64-w64-mingw32-gcc $(INCL_WIN) $(LIB_WIN) $(COMPILER_FLAGS_WIN) $(SRCFILES) -I$(INCDIR) -o perdita.win.o

clean:
	rm -f perdita; rm -f perdita_debug; rm -f perdita.exe; rm -f perdita.zip; rm -f perdita.win.o version.h


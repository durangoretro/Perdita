COMPILER_FLAGS = -Wall -O2
COMPILER_FLAGS_WIN = $(COMPILER_FLAGS) -c
LINKER_FLAGS = -lstdc++ -lSDL2 -lm
VERSION ?= 1.0.2
ARCH ?= amd64

TEMP_DIR ?= perdita_$(VERSION)_$(ARCH)
CONTROL ?= $(TEMP_DIR)/DEBIAN/control

SDL_WIN = ${HOME}/sdl_win/SDL2-2.0.18/x86_64-w64-mingw32
LIB_WIN = -L$(SDL_WIN)/lib
INCL_WIN = -I$(SDL_WIN)/include -I$(SDL_WIN)/include/SDL2

SRCDIR = src
INCDIR = headers/
OBJDIR = obj
OBJDIR_WIN = obj_win

# List all source files in the src/ directory
SRCFILES := $(wildcard $(SRCDIR)/*.c)

# Generate object file names from source files
OBJFILES := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCFILES))

# Generate object file names from source files for windows
OBJFILES_WIN := $(patsubst $(SRCDIR)/%.c, $(OBJDIR_WIN)/%.win.o, $(SRCFILES))

all: perdita

###############
# Linux BUILD #
###############

# Rule to generate object files from source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	gcc $(COMPILER_FLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

perdita: version.h $(OBJFILES)
	gcc $(OBJFILES) $(LINKER_FLAGS) -o perdita

perdita_debug: $(OBJFILES)
	gcc -g $(OBJFILES) $(LINKER_FLAGS) -o perdita_debug

version.h: $(SRCFILES)
	echo "char * perdita_version=\"$(VERSION)-$$(git rev-parse --short HEAD)\";"> version.h

#################
# WINDOWS BUILD #
#################W

# Generate Windows Version. You can use the Docker file in https://github.com/zerasul/dockerretro/blob/master/sdl/Dockerfile
# To generate this file.
$(OBJDIR_WIN)/%.win.o: $(SRCDIR)/%.c | $(OBJDIR_WIN)
	x86_64-w64-mingw32-gcc $(INCL_WIN) $(LIB_WIN) $(COMPILER_FLAGS_WIN) $< -o $@ -I$(INCDIR)

$(OBJDIR_WIN):
	mkdir -p $(OBJDIR_WIN)

perdita.zip: perdita.exe
	zip -j perdita.zip perdita.exe $(SDL_WIN)/bin/SDL2.dll

perdita.exe: version.h $(OBJFILES_WIN)
	x86_64-w64-mingw32-g++ $(LIB_WIN) -static $(OBJFILES_WIN) `$(SDL_WIN)/bin/sdl2-config --static-libs` -o perdita.exe

# Clean for linux
clean:
	clean:
	rm -f perdita perdita_debug perdita.exe perdita.zip version.h
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.win.o
	rm -rf $(TEMP_DIR)

# Clean for windows
clean-win:
	- del /q perdita perdita_debug perdita.exe perdita.zip version.h 2>nul
	- del /q $(OBJDIR_WIN)\*.o $(OBJDIR_WIN)\*.win.o 2>nul
	- rmdir /s /q $(TEMP_DIR) 2>nul

make-deb: perdita
	mkdir -p $(TEMP_DIR)/usr/local/bin
	mkdir -p $(TEMP_DIR)/DEBIAN

	cp perdita $(TEMP_DIR)/usr/local/bin

	echo "Package: perdita" > $(CONTROL)
	echo "Version: $(VERSION)" >> $(CONTROL)
	echo "Architecture: $(ARCH)" >> $(CONTROL)
	echo "Maintainer: Durango Computer Team" >> $(CONTROL)
	echo "Description: Perdita: Durango computer emulator." >> $(CONTROL)
	echo "Depends: libsdl2-dev (>=2.0.0)" >> $(CONTROL)
	dpkg-deb --build --root-owner-group $(TEMP_DIR)
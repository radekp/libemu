# Settings via env variables
#  CC          - Your C compiler
#  AR          - Your AR archiver
#  CFLAGS      - Your CFLAGS (we add a few we find essential)
#  LDFLAGS     - Your LDFLAGS (we add a few we find essential)
#  SDL_INCLUDE - Where to find SDL includes
#  LIBS        - Which libs to include (SDL.dll for windows)
#
# Settings via parameters
#  OSX:=1      - To compile on/for OSX
#  WIN32:=1    - To compile on/for Windows

ifdef OSX
LDFLAGS := $(LDFLAGS) -dynamiclib -Wl,-single_module
else
LDFLAGS := $(LDFLAGS) -shared
endif
ifdef WIN32
LIB_EXTENSION := .dll
CFLAGS := $(CFLAGS) -DWIN32
else
ifdef OSX
LIB_EXTENSION := .dylib
else
LIB_EXTENSION := .so
endif
CFLAGS := $(CFLAGS) -fPIC
endif
ifndef SDL_INCLUDE
ifdef OSX
SDL_INCLUDE := /opt/local/include/SDL
else
SDL_INCLUDE := /usr/include/SDL
endif
endif
ifndef LIBS
LIBS :=
endif

ifdef WIN32
LIBS := $(LIBS) -lSDL -lwinmm
endif

ifdef OSX
LIBS := $(LIBS) -lSDL -L/opt/local/lib
endif

ifdef ALSA
LIBS := $(LIBS) -lasound
CFLAGS := $(CFLAGS) -DMPU_ALSA
endif

# OpenDUNE flag: we don't need ncurses
CFLAGS := $(CFLAGS) -DWITHOUT_NCURSES

CFLAGS := $(CFLAGS) -g -Wall -Wextra -Wno-unused-parameter -Werror
CFLAGS := $(CFLAGS) -ansi -pedantic
LDFLAGS := $(LDFLAGS) -g

SOURCE := $(shell ls src/*.c src/*/*.c 2>/dev/null)
SOURCE := $(SOURCE:%.c=objs/%.o)
RES := $(shell mkdir -p objs/src)

ifdef VERBOSE
	Q =
else
	Q = @
endif

all: libemu$(LIB_EXTENSION) libemu.a

objs/%.o: %.c
	$(shell mkdir -p `dirname $@`)
	@echo "[Compiling] $^"
	$(Q)$(CC) $(CFLAGS) -c $^ -o $@ -I include/ -I $(SDL_INCLUDE)

libemu$(LIB_EXTENSION): $(SOURCE)
	@echo "[Dynamic] $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

libemu.a: $(SOURCE)
	@echo "[Static] $@"
	$(Q)$(AR) rcs $@ $(SOURCE)

clean:
	@echo "[Cleaning] libemu"
	$(Q)rm -f libemu$(LIB_EXTENSION) libemu.a $(SOURCE)


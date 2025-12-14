# tools
CC = cc

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# libs
# remove the Xinerama parts if you don't want Xinerama support
LIBS = -lX11 -lXINERAMA
CPPFLAGS = -DXINERAMA

# flags
# CFLAGS = -std=c99 -Wall -Wextra -O0 -g ${CPPFLAGS} -fdiagnostics-color=always # debug
CFLAGS = -std=c99 -Wall -Wextra -Os ${CPPFLAGS} -fdiagnostics-color=always -I/usr/X11R6/include
LDFLAGS = ${LIBS} -L/usr/X11R6/lib

# files
SRC = xnap.c

all: xnap

# rules
sxwm: ${OBJ}
	${CC} -o xnap ${LDFLAGS}

clean:
	rm -rf xnap

install: all
	# TODO

uninstall:
	# TODO

clangd:
	rm -f compile_flags.txt
	for f in ${CFLAGS}; do echo $$f >> compile_flags.txt; done

.PHONY: all clean install uninstall clangd

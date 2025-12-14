# tools
CC = cc

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# libs
LIBS = -lX11

# flags
# CPPFLAGS =
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Os ${CPPFLAGS} -fdiagnostics-color=always
LDFLAGS = ${LIBS}

# uncomment this block for OpenBSD
# CFLAGS += -I/usr/X11R6/include
# LDFLAGS += -L/usr/X11R6/lib

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

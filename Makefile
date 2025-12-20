# tools
CC = cc

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# libs
# remove the Xinerama parts if you don't want Xinerama support
LIBS = -lX11 -lXinerama

# flags
CPPFLAGS = -DXINERAMA
CFLAGS = -std=c99 -Wall -Wextra -O0 -g -fdiagnostics-color=always # debug
# CFLAGS = -std=c99 -Wall -Wextra -Os -fdiagnostics-color=always
LDFLAGS = ${LIBS}

# files
SRC = xnap.c

all: xnap

# rules
xnap:
	${CC} ${CPPFLAGS} ${CFLAGS} ${SRC} -o xnap ${LDFLAGS}

clean:
	rm -rf xnap

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f xnap ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/xnap
	cp -f xnap.1 ${DESTDIR}${MANPREFIX}/man1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/xnap.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/xnap
	rm -f ${DESTDIR}${MANPREFIX}/man1/xnap.1

clangd:
	rm -f compile_flags.txt
	for f in ${CPPFLAGS} ${CFLAGS}; do echo $$f >> compile_flags.txt; done

.PHONY: all clean install uninstall clangd

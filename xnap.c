#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>

void die(const char* s);
void quit(void);
void xsetup(void);

Display* dpy = NULL;
Window root = None;
int scr = -1;

void die(const char* s)
{
	fprintf(stderr, "xnap: %s", s);
	exit(1);
}

void quit(void)
{
	XCloseDisplay(dpy);
}

void xsetup(void)
{
	if (!(dpy = XOpenDisplay(NULL)))
		die ("failed to open display");

	int scr = DefaultScreen(dpy);
	root = RootWindow(dpy, scr);
}

int main(void)
{
	return EXIT_SUCCESS;
}

#include <stdlib.h>
#include <stdio.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

struct pointer_t {
	int    ret;
	Bool   sel;
	int    x0;
	int    y0;
	int    x1;
	int    y1;
};

void compimg(void);
void die(const char* s);
void quit(Bool ex);
void run(void);
void setup(void);

Display* dpy = NULL;
Window root = None;
int scr = -1;
struct pointer_t p = {0};
XImage* img = NULL;

void die(const char* s)
{
	fprintf(stderr, "xnap: %s", s);
	exit(EXIT_FAILURE);
}

void quit(Bool ex)
{
	XUngrabPointer(dpy, CurrentTime);
	XCloseDisplay(dpy);
	if (ex)
		exit(EXIT_SUCCESS);
}

void run(void)
{
	XEvent ev;
	for (;;) {
		XNextEvent(dpy, &ev);

		if (ev.type == KeyPress) {
			KeySym ks = XLookupKeysym(&ev.xkey, 0);
			if (ks == XK_Escape || ks == XK_q)
				quit(True);
		}

		unsigned int b = ev.xbutton.button;
		if (ev.type == ButtonPress && b == Button1) { /* start selecting */
			p.sel = True;
			p.x0 = p.x1 = ev.xbutton.x_root;
			p.y0 = p.y1 = ev.xbutton.y_root;
		}
		else if (ev.type == MotionNotify && p.sel) { /* dragging selection */
			p.x1 = ev.xmotion.x_root;
			p.y1 = ev.xmotion.y_root;
		}
		else if (ev.type == ButtonRelease && p.sel && b == Button1) { /* release selection */
			p.sel = False;

			/* creating selection rectangle */
			int recx = MIN(p.x0, p.x1);
			int recy = MIN(p.y0, p.y1);
			int recw = MAX(p.x0, p.x1) - recx;
			int rech = MAX(p.y0, p.y1) - recy;

			quit(False);
			compimg()
		}
	}
}

void setup(void)
{
	/* X surface */
	if (!(dpy = XOpenDisplay(NULL)))
		die ("failed to open display");

	int scr = DefaultScreen(dpy);
	root = RootWindow(dpy, scr);

	/* pointer */
	p.ret = XGrabPointer(
		dpy, root, False,
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask,
		GrabModeAsync, GrabModeAsync, None, None, CurrentTime
	);
	if (p.ret != GrabSuccess)
		die("XGrabPointer failed");
}

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;

	setup();
	quit(True); /* unreachable */

	return EXIT_SUCCESS;
}
